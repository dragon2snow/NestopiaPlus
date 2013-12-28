////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2006 Martin Freij
//
// This file is part of Nestopia.
// 
// Nestopia is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// Nestopia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Nestopia; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include "../NstMapper.hpp"
#include "../NstClock.hpp"
#include "../NstBarcodeReader.hpp"
#include "NstBrdFme7.hpp"
		   
namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif
		
			class Fme07::BarcodeWorld : public BarcodeReader
			{
				bool SubTransfer(cstring,uint,u8*);

				NES_DECL_PEEK( 4017 )
				NES_DECL_POKE( 4017 )

				Io::Port p4017;

				enum
				{
					NUM_DIGITS = 13
				};

				bool IsDigitsSupported(uint count) const
				{
					return count == NUM_DIGITS;
				}

			public:

				void Reset(Cpu& cpu)
				{
					BarcodeReader::Reset();

					p4017 = cpu.Map( 0x4017 );
					cpu.Map( 0x4017 ).Set( this, &BarcodeWorld::Peek_4017, &BarcodeWorld::Poke_4017 );
				}

				void SaveState(State::Saver& state) const
				{
					BarcodeReader::SaveState( state );
				}

				void LoadState(State::Loader& state)
				{
					BarcodeReader::Reset();

					while (const dword chunk = state.Begin())
					{
						BarcodeReader::LoadState( state, chunk );
						state.End();
					}
				}
			};

			const u16 Fme07::Sound::Square::voltages[16] = 
			{
				// 16 levels, 3dB per step
				46,65,92,130,183,259,366,517,730,1031,1457,2057,2906,4105,5799,8191
			};

			Fme07::Sound::Sound(Cpu& cpu,bool hook)
			: apu(cpu.GetApu()), hooked(hook)
			{
				if (hook)
					apu.HookChannel( this );
			}
		
			Fme07::Sound::~Sound()
			{
				if (hooked)
					apu.ReleaseChannel();
			}
	
			Fme07::Fme07(Context& c)
			: 
			Mapper       (c,WRAM_8K),
			irq          (c.cpu),
			sound        (c.cpu),
			barcodeWorld (c.pRomCrc == 0x67898319UL ? new BarcodeWorld : NULL)
			{}
		
			Fme07::~Fme07()
			{
				delete barcodeWorld;
			}

			Fme07::Device Fme07::QueryDevice(DeviceType type)
			{
				if (type == DEVICE_BARCODE_READER && barcodeWorld)
					return barcodeWorld;
				else
					return Mapper::QueryDevice( type );
			}

			void Fme07::Irq::Reset(const bool hard)
			{
				if (hard)
				{
					enabled = false;
					count = 0;
				}
			}

			void Fme07::Sound::Square::Reset(const uint fixed)
			{
				enabled = false;
				active = false;
				waveLength = 0;
				frequency = (1UL << FRQ_SHIFT) * fixed;
				timer = 0;
				voltage = 46;
				dc = 0;
			}
	
			void Fme07::Sound::Reset()
			{
				regSelect = 0x0;
	
				for (uint i=0; i < NUM_SQUARES; ++i)
					squares[i].Reset( fixed );
			}
	
			void Fme07::SubReset(const bool hard)
			{
				if (hard)
					command = 0x0;

				irq.Reset( hard, hard ? false : irq.IsLineEnabled() );

				if (barcodeWorld)
					barcodeWorld->Reset( cpu );

				Map( WRK_PEEK );		
				Map( WRK_POKE_BUS );		
				Map( 0x8000U, 0x9FFFU, &Fme07::Poke_8000 );
				Map( 0xA000U, 0xBFFFU, &Fme07::Poke_A000 );
				Map( 0xC000U, 0xDFFFU, &Fme07::Poke_C000 );
				Map( 0xE000U, 0xFFFFU, &Fme07::Poke_E000 );
			}
		
			void Fme07::Sound::Square::UpdateContext(const uint fixed)
			{
				UpdateFrequency( fixed );
				timer = 0;
				dc = 0;
			}

			void Fme07::Sound::UpdateContext(uint)
			{
				for (uint i=0; i < NUM_SQUARES; ++i)
					squares[i].UpdateContext( fixed );
			}

			inline bool Fme07::Sound::Square::CanOutput() const
			{
				return enabled && waveLength;
			}

			void Fme07::BaseLoad(State::Loader& state,const dword id)
			{
				NST_ASSERT( id == NES_STATE_CHUNK_ID('F','M','7','\0') );

				if (id == NES_STATE_CHUNK_ID('F','M','7','\0'))
				{
					while (const dword chunk = state.Begin())
					{
						switch (chunk)
						{
							case NES_STATE_CHUNK_ID('R','E','G','\0'):
						
								command = state.Read8();
								break;
						
							case NES_STATE_CHUNK_ID('I','R','Q','\0'):
							{
								const State::Loader::Data<3> data( state );
						
								irq.EnableLine( data[0] & 0x80 );
								irq.unit.enabled = data[0] & 0x01;
								irq.unit.count = data[1] | (data[2] << 8);
						
								break;
							}
						
							case NES_STATE_CHUNK_ID('S','N','D','\0'):
						
								sound.LoadState( State::Loader::Subset(state).Ref() );
								break;
						
							case NES_STATE_CHUNK_ID('B','R','C','\0'):
						
								if (barcodeWorld)
									barcodeWorld->LoadState( State::Loader::Subset(state).Ref() );
						
								break;
						}

						state.End();
					}
				}
			}
		
			void Fme07::BaseSave(State::Saver& state) const
			{
				state.Begin('F','M','7','\0');
				state.Begin('R','E','G','\0').Write8( command ).End();

				{
					const u8 data[3] =
					{
						(irq.IsLineEnabled() ? 0x80 : 0x00) | (irq.unit.enabled ? 0x1 : 0x0),
						irq.unit.count & 0xFF,
						irq.unit.count >> 8
					};

					state.Begin('I','R','Q','\0').Write( data ).End();
				}

				sound.SaveState( State::Saver::Subset(state,'S','N','D','\0').Ref() );

				if (barcodeWorld && barcodeWorld->IsTransferring())
					barcodeWorld->SaveState( State::Saver::Subset(state,'B','R','C','\0').Ref() );

				state.End();
			}
				
			void Fme07::Sound::SaveState(State::Saver& state) const
			{
				state.Begin('R','E','G','\0').Write8( regSelect ).End();

				squares[0].SaveState( State::Saver::Subset(state,'S','Q','0','\0').Ref() );
				squares[1].SaveState( State::Saver::Subset(state,'S','Q','1','\0').Ref() );
				squares[2].SaveState( State::Saver::Subset(state,'S','Q','2','\0').Ref() );
			}
				
			void Fme07::Sound::LoadState(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					switch (chunk)
					{
     					case NES_STATE_CHUNK_ID('R','E','G','\0'):

							regSelect = state.Read8();
							break;

						case NES_STATE_CHUNK_ID('S','Q','0','\0'):

							squares[0].LoadState( State::Loader::Subset(state).Ref(), fixed );
							break;

						case NES_STATE_CHUNK_ID('S','Q','1','\0'):

							squares[1].LoadState( State::Loader::Subset(state).Ref(), fixed );
							break;

						case NES_STATE_CHUNK_ID('S','Q','2','\0'):

							squares[2].LoadState( State::Loader::Subset(state).Ref(), fixed );
							break;
					}

					state.End();
				}

				active = CanOutput();
			}

			void Fme07::Sound::Square::SaveState(State::Saver& state) const
			{
				uint volume = 0;

				for (uint i=0; i < NST_COUNT(voltages); ++i)
				{
					if (voltage == voltages[i])
					{
						volume = i;
						break;
					}
				}

				const u8 data[3] =
				{
					(enabled != 0) | (volume << 1),
					waveLength & 0xFF,
					waveLength >> 8,
				};

				state.Begin('R','E','G','\0').Write( data ).End();
			}

			void Fme07::Sound::Square::LoadState(State::Loader& state,const uint fixed)
			{
				while (const dword chunk = state.Begin())
				{
					if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					{
						const State::Loader::Data<3> data( state );

						enabled = data[0] & 0x1;
						voltage = voltages[(data[0] >> 1) & 0xF];
						waveLength = data[1] | ((data[2] & 0xF) << 8);

						UpdateContext( fixed );
						active = CanOutput();
					}

					state.End();
				}
			}

			bool Fme07::BarcodeWorld::SubTransfer(cstring const string,const uint length,u8* NST_RESTRICT stream)
			{
				NST_COMPILE_ASSERT( MAX_DATA_LENGTH >= 191 );

				if (length != NUM_DIGITS)
					return false;

				const u8 code[20] =
				{
					string[0],
					string[1],
					string[2],
					string[3],
					string[4],
					string[5],
					string[6],
					string[7],
					string[8],
					string[9],
					string[10],
					string[11],
					string[12],
					'S',
					'U',
					'N',
					'S',
					'O',
					'F',
					'T'				
				};

				*stream++ = 0x04;

				for (uint i=0; i < 20; ++i)
				{
					*stream++ = 0x04;

					for (uint j=0x01, c=code[i]; j != 0x100; j <<= 1)
						*stream++ = (c & j) ? 0x00 : 0x04;

					*stream++ = 0x00;
				}

				return true;
			}

            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif

			NES_POKE(Fme07::BarcodeWorld,4017)
			{
				p4017.Poke( address, data );
			}

			NES_PEEK(Fme07::BarcodeWorld,4017)
			{
				return (IsTransferring() ? Fetch() : 0x00) | p4017.Peek( address );
			}

			NES_POKE(Fme07,8000) 
			{ 
				command = data;
			}
		
			NES_POKE(Fme07,A000) 
			{ 
				switch (const uint bank = (command & 0xF))
				{
					case 0x0:
					case 0x1:
					case 0x2:
					case 0x3:
					case 0x4:
					case 0x5:
					case 0x6:
					case 0x7:
			
						ppu.Update();
						chr.SwapBank<SIZE_1K>( bank << 10, data );
						break;
		
					case 0x8:

						if (!(data & 0x40) || (data & 0x80))
							wrk.Source( !(data & 0x40) ).SwapBank<SIZE_8K,0x0000U>( data );

						break;
			
					case 0x9:
					case 0xA:
					case 0xB:
		
						prg.SwapBank<SIZE_8K>( (command - 0x9) << 13, data );
						break;
			
					case 0xC:
					{
						static const uchar lut[4] =
						{
							Ppu::NMT_VERTICAL,
							Ppu::NMT_HORIZONTAL,
							Ppu::NMT_ZERO,
							Ppu::NMT_ONE
						};
		
						ppu.SetMirroring( lut[data & 0x3] );
						break;
					}
	
					case 0xD: 
						
						irq.Update();
						irq.unit.enabled = data & 0x01;

						if (!irq.EnableLine( data & 0x80 ))
							irq.ClearIRQ();

						break;
		
					case 0xE: 

						irq.Update();
						irq.unit.count = (irq.unit.count & 0xFF00U) | data;
						break;
		
					case 0xF: 

						irq.Update();
						irq.unit.count = (irq.unit.count & 0x00FFU) | (data << 8);
						break;
				}
			}
		
			NES_POKE(Fme07,C000) 
			{ 
				sound.Poke_C000( data ); 
			}

			void Fme07::Sound::Square::UpdateFrequency(const uint fixed)
			{
				frequency = ((dword(waveLength) + 1) << FRQ_SHIFT) * fixed;
			}
						
			void Fme07::Sound::Square::WriteReg0(const uint data,const uint fixed)
			{
				waveLength &= uint(REG1_WAVELENGTH_HIGH) << 8;
				waveLength |= data;
				UpdateFrequency( fixed );	
				active = CanOutput();
			}
		
			void Fme07::Sound::Square::WriteReg1(const uint data,const uint fixed)
			{
				waveLength &= REG0_WAVELENGTH_LOW;
				waveLength |= (data & REG1_WAVELENGTH_HIGH) << 8;
				UpdateFrequency( fixed );	
				active = CanOutput();
			}
		
			void Fme07::Sound::Square::WriteReg2(const uint data)
			{
				enabled = (data & REG2_DISABLE) ^ REG2_DISABLE;
				active = CanOutput();
			}
		
			inline void Fme07::Sound::Square::WriteReg3(const uint data)
			{
				NST_COMPILE_ASSERT( Apu::OUTPUT_MUL == 256 );
				voltage = voltages[data & REG3_VOLTAGE];
			}
		
			bool Fme07::Sound::CanOutput() const
			{
				return emulate && (squares[0].IsActive() | squares[1].IsActive() | squares[2].IsActive());
			}

			void Fme07::Sound::Poke_E000(const uint data)
			{ 
				apu.Update(); 
		
				switch (regSelect & 0xF)
				{
					case 0x0:
					case 0x2:
					case 0x4:
				
						squares[regSelect >> 1].WriteReg0( data, fixed );
						break;
				
					case 0x1:
					case 0x3:
					case 0x5:
				
						squares[regSelect >> 1].WriteReg1( data, fixed );
						break;
				
					case 0x7: 
				
						for (uint i=0; i < NUM_SQUARES; ++i)
							squares[i].WriteReg2( data >> i );
				
						break;
				
					case 0x8:
					case 0x9:
					case 0xA:
				
						squares[regSelect - 0x8].WriteReg3( data );
						break;
				}
		
				active = CanOutput();
			}
		
			NES_POKE(Fme07,E000) 
			{
				sound.Poke_E000( data ); 
			}

			ibool Fme07::Irq::Signal()
			{
				count = (count - 1) & 0xFFFFU;
				return count < enabled;
			}
		
			NST_FORCE_INLINE Fme07::Sound::Sample Fme07::Sound::Square::GetSample(const Cycle rate)
			{
				if (active)
				{
					Sample sum;
					Sample amp;

					if (Cycle(timer) >= rate)
					{
						timer -= idword(rate);
						amp = sum = voltage;

						if (!dc)
							sum = -sum;
					}
					else
					{
						sum = timer;
						timer -= idword(rate);

						if (!dc)
							sum = -sum;

						do 
						{		
							idword weight = frequency;
							timer += weight;

							if (timer > 0)
								weight -= timer;

							dc ^= 0x1;

							if (!dc)
								weight = -weight;

							sum += weight;
						} 
						while (timer < 0);

						amp = ((voltage * ulong(std::labs(sum))) + (rate / 2)) / rate; 
					}

					return (sum < 0 ? -amp : amp);
				}

				return 0;
			}

			Fme07::Sound::Sample Fme07::Sound::GetSample()
			{
				Sample sample = 0;

				if (active)
				{
					for (uint i=0; i < NUM_SQUARES; ++i)
						sample += squares[i].GetSample( rate );
				}

				return sample;
			}

			void Fme07::VSync()
			{
				irq.VSync();
			}
		}
	}
}
