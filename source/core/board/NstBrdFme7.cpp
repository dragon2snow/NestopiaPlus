////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
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
#include "../NstPrpBarcodeReader.hpp"
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

			class Fme7::BarcodeWorld : public Peripherals::BarcodeReader
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

			const u16 Fme7::Sound::levels[32] =
			{
				// 32 levels, 1.5dB per step
				0,89,106,127,152,181,216,257,306,364,433,515,613,729,867,1031,1226,1458,
				1733,2060,2449,2911,3460,4113,4889,5811,6907,8209,9757,11597,13784,16383
			};

			Fme7::Sound::Sound(Cpu& cpu,bool hook)
			: apu(cpu.GetApu()), active(false), hooked(hook)
			{
				if (hook)
					apu.HookChannel( this );
			}

			Fme7::Sound::Envelope::Envelope()
			{
				Reset(1);
			}

			Fme7::Sound::Noise::Noise()
			{
				Reset(1);
			}

			Fme7::Sound::Square::Square()
			{
				Reset(1);
			}

			Fme7::Sound::~Sound()
			{
				if (hooked)
					apu.ReleaseChannel();
			}

			Fme7::Fme7(Context& c)
			:
			Mapper       (c,WRAM_8K),
			irq          (c.cpu),
			sound        (c.cpu),
			barcodeWorld (c.pRomCrc == 0x67898319UL ? new BarcodeWorld : NULL)
			{}

			Fme7::~Fme7()
			{
				delete barcodeWorld;
			}

			Fme7::Device Fme7::QueryDevice(DeviceType type)
			{
				if (type == DEVICE_BARCODE_READER && barcodeWorld)
					return barcodeWorld;
				else
					return Mapper::QueryDevice( type );
			}

			void Fme7::Irq::Reset(const bool hard)
			{
				if (hard)
				{
					enabled = false;
					count = 0;
				}
			}

			void Fme7::Sound::Envelope::Reset(const uint fixed)
			{
				holding = false;
				hold = 0;
				alternate = 0;
				attack = 0;
				timer = 0;
				length = 0;
				count = 0;
				volume = 0;
				UpdateFrequency( fixed );
			}

			void Fme7::Sound::Noise::Reset(const uint fixed)
			{
				timer = 0;
				length = 0;
				rng = 1;
				dc = 0;
				UpdateFrequency( fixed );
			}

			void Fme7::Sound::Square::Reset(const uint fixed)
			{
				timer = 0;
				status = 0;
				ctrl = 0;
				volume = 0;
				dc = 0;
				length = 0;
				UpdateFrequency( fixed );
			}

			void Fme7::Sound::Reset()
			{
				regSelect = 0x0;
				envelope.Reset( fixed );

				for (uint i=0; i < NUM_SQUARES; ++i)
					squares[i].Reset( fixed );

				noise.Reset( fixed );
				dcBlocker.Reset();
			}

			void Fme7::SubReset(const bool hard)
			{
				if (hard)
					command = 0x0;

				irq.Reset( hard, hard ? false : irq.IsLineEnabled() );

				if (barcodeWorld)
					barcodeWorld->Reset( cpu );

				Map( WRK_PEEK );
				Map( WRK_POKE_BUS );
				Map( 0x8000U, 0x9FFFU, &Fme7::Poke_8000 );
				Map( 0xA000U, 0xBFFFU, &Fme7::Poke_A000 );
				Map( 0xC000U, 0xDFFFU, &Fme7::Poke_C000 );
				Map( 0xE000U, 0xFFFFU, &Fme7::Poke_E000 );
			}

			void Fme7::Sound::Envelope::UpdateContext(const uint fixed)
			{
				timer = 0;
				UpdateFrequency( fixed );
			}

			void Fme7::Sound::Noise::UpdateContext(const uint fixed)
			{
				timer = 0;
				UpdateFrequency( fixed );
			}

			void Fme7::Sound::Square::UpdateContext(const uint fixed)
			{
				timer = 0;
				UpdateFrequency( fixed );
			}

			void Fme7::Sound::UpdateContext(uint,const u8 (&volumes)[MAX_CHANNELS])
			{
				outputVolume = volumes[Apu::CHANNEL_S5B] * 94 / DEFAULT_VOLUME;
				envelope.UpdateContext( fixed );

				for (uint i=0; i < NUM_SQUARES; ++i)
					squares[i].UpdateContext( fixed );

				noise.UpdateContext( fixed );
				dcBlocker.Reset();
			}

			void Fme7::BaseLoad(State::Loader& state,const dword id)
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

			void Fme7::BaseSave(State::Saver& state) const
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

			void Fme7::Sound::SaveState(State::Saver& state) const
			{
				state.Begin('R','E','G','\0').Write8( regSelect ).End();

				envelope.SaveState( State::Saver::Subset(state,'E','N','V','\0').Ref() );
				noise.SaveState( State::Saver::Subset(state,'N','O','I','\0').Ref() );
				squares[0].SaveState( State::Saver::Subset(state,'S','Q','0','\0').Ref() );
				squares[1].SaveState( State::Saver::Subset(state,'S','Q','1','\0').Ref() );
				squares[2].SaveState( State::Saver::Subset(state,'S','Q','2','\0').Ref() );
			}

			void Fme7::Sound::LoadState(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					switch (chunk)
					{
						case NES_STATE_CHUNK_ID('R','E','G','\0'):

							regSelect = state.Read8();
							break;

						case NES_STATE_CHUNK_ID('E','N','V','\0'):

							envelope.LoadState( State::Loader::Subset(state).Ref(), fixed );
							break;

						case NES_STATE_CHUNK_ID('N','O','I','\0'):

							noise.LoadState( State::Loader::Subset(state).Ref(), fixed );
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
			}

			void Fme7::Sound::Envelope::SaveState(State::Saver& state) const
			{
				const u8 data[4] =
				{
					(holding   ? 0x1 : 0x0) |
					(hold      ? 0x2 : 0x1) |
					(alternate ? 0x4 : 0x0) |
					(attack    ? 0x8 : 0x0),
					count,
					length & 0xFF,
					length >> 8
				};

				state.Begin('R','E','G','\0').Write( data ).End();
			}

			void Fme7::Sound::Noise::SaveState(State::Saver& state) const
			{
				state.Begin('R','E','G','\0').Write8( length ).End();
			}

			void Fme7::Sound::Square::SaveState(State::Saver& state) const
			{
				const u8 data[3] =
				{
					((status & 0x1) ^ 0x1) | (ctrl << 1),
					length & 0xFF,
					(length >> 8) | ((status & 0x8) << 1),
				};

				state.Begin('R','E','G','\0').Write( data ).End();
			}

			void Fme7::Sound::Envelope::LoadState(State::Loader& state,const uint fixed)
			{
				while (const dword chunk = state.Begin())
				{
					if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					{
						const State::Loader::Data<4> data( state );

						holding = data[0] & 0x1;
						hold = data[0] & 0x2;
						alternate = data[0] & 0x4;
						attack = (data[0] & 0x8) ? 0x1F : 0x00;
						count = data[1] & 0x1F;
						length = data[2] | ((data[3] & 0xF) << 8);
						volume = levels[count ^ attack];

						UpdateContext( fixed );
					}

					state.End();
				}
			}

			void Fme7::Sound::Noise::LoadState(State::Loader& state,const uint fixed)
			{
				while (const dword chunk = state.Begin())
				{
					if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					{
						length = state.Read8() & 0x1F;
						dc = 0;
						rng = 1;

						UpdateContext( fixed );
					}

					state.End();
				}
			}

			void Fme7::Sound::Square::LoadState(State::Loader& state,const uint fixed)
			{
				while (const dword chunk = state.Begin())
				{
					if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					{
						const State::Loader::Data<3> data( state );

						status = ((data[0] & 0x1) ^ 0x1) | ((data[2] >> 1) & 0x8);
						ctrl = (data[0] >> 1) & 0x1F;
						length = data[1] | ((data[2] & 0xF) << 8);
						volume = levels[(ctrl & 0xF) ? (ctrl & 0xF) * 2 + 1 : 0];
						dc = (status & 0x1) ? ~0UL : 0UL;

						UpdateContext( fixed );
					}

					state.End();
				}
			}

			bool Fme7::BarcodeWorld::SubTransfer(cstring const string,const uint length,u8* NST_RESTRICT stream)
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

			NES_POKE(Fme7::BarcodeWorld,4017)
			{
				p4017.Poke( address, data );
			}

			NES_PEEK(Fme7::BarcodeWorld,4017)
			{
				return (IsTransferring() ? Fetch() : 0x00) | p4017.Peek( address );
			}

			NES_POKE(Fme7,8000)
			{
				command = data;
			}

			NES_POKE(Fme7,A000)
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

			NES_POKE(Fme7,C000)
			{
				sound.Poke_C000( data );
			}

			void Fme7::Sound::Square::UpdateFrequency(const uint fixed)
			{
				const iword prev = frequency;
				frequency = NST_MAX(length,1) * 16 * fixed;
				timer = NST_MAX(timer + iword(frequency) - prev,0);
			}

			void Fme7::Sound::Square::WriteReg0(const uint data,const uint fixed)
			{
				length = (length & 0x0F00) | data;
				UpdateFrequency( fixed );
			}

			void Fme7::Sound::Square::WriteReg1(const uint data,const uint fixed)
			{
				length = (length & 0x00FF) | ((data & 0xF) << 8);
				UpdateFrequency( fixed );
			}

			void Fme7::Sound::Square::WriteReg2(const uint data)
			{
				status = data & (0x1|0x8);

				if (status & 0x1)
					dc = ~0UL;
			}

			void Fme7::Sound::Square::WriteReg3(const uint data)
			{
				ctrl = data & 0x1F;
				volume = levels[(ctrl & 0xF) ? (ctrl & 0xF) * 2 + 1 : 0];
			}

			void Fme7::Sound::Envelope::UpdateFrequency(const uint fixed)
			{
				const iword prev = frequency;
				frequency = NST_MAX(length*16,1*8) * fixed;
				timer = NST_MAX(timer + iword(frequency) - prev,0);
			}

			void Fme7::Sound::Envelope::WriteReg0(const uint data,const uint fixed)
			{
				length = (length & 0xFF00) | data;
				UpdateFrequency( fixed );
			}

			void Fme7::Sound::Envelope::WriteReg1(const uint data,const uint fixed)
			{
				length = (length & 0x00FF) | (data << 8);
				UpdateFrequency( fixed );
			}

			void Fme7::Sound::Envelope::WriteReg2(const uint data)
			{
				holding = false;
				attack = (data & 0x04) ? 0x1F : 0x00;

				if (data & 0x8)
				{
					hold = data & 0x1;
					alternate = data & 0x2;
				}
				else
				{
					hold = 1;
					alternate = attack;
				}

				timer = frequency;
				count = 0x1F;
				volume = levels[count ^ attack];
			}

			void Fme7::Sound::Noise::UpdateFrequency(const uint fixed)
			{
				const iword prev = frequency;
				frequency = NST_MAX(length,1) * 16 * fixed;
				timer = NST_MAX(timer + iword(frequency) - prev,0);
			}

			void Fme7::Sound::Noise::WriteReg(const uint data,const uint fixed)
			{
				length = data & 0x1F;
				UpdateFrequency( fixed );
			}

			void Fme7::Sound::Poke_E000(const uint data)
			{
				apu.Update();
				active = true;

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

					case 0x6:

						noise.WriteReg( data, fixed );
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

					case 0xB:

						envelope.WriteReg0( data, fixed );
						break;

					case 0xC:

						envelope.WriteReg1( data, fixed );
						break;

					case 0xD:

						envelope.WriteReg2( data );
						break;
				}
			}

			NES_POKE(Fme7,E000)
			{
				sound.Poke_E000( data );
			}

			ibool Fme7::Irq::Signal()
			{
				count = (count - 1) & 0xFFFFU;
				return count < enabled;
			}

			NST_FORCE_INLINE dword Fme7::Sound::Envelope::Clock(const Cycle rate)
			{
				if (!holding)
				{
					timer -= iword(rate);

					if (timer < 0)
					{
						do
						{
							--count;
							timer += iword(frequency);
						}
						while (timer < 0);

						if (count > 0x1F)
						{
							if (hold)
							{
								if (alternate)
									attack ^= 0x1F;

								holding = true;
								count = 0x00;
							}
							else
							{
								if (alternate && (count & 0x20))
									attack ^= 0x1F;

								count = 0x1F;
							}
						}

						volume = levels[count ^ attack];
					}
				}

				return volume;
			}

			NST_FORCE_INLINE dword Fme7::Sound::Noise::Clock(const Cycle rate)
			{
				for (timer -= iword(rate); timer < 0; timer += iword(frequency))
				{
					if ((rng + 1) & 0x2) dc = ~dc;
					if ((rng + 0) & 0x1) rng ^= 0x24000UL;

					rng >>= 1;
				}

				return dc;
			}

			NST_FORCE_INLINE dword Fme7::Sound::Square::GetSample(const Cycle rate,const uint envelope,const uint noise)
			{
				dword sum = timer;
				timer -= iword(rate);

				const uint out = (ctrl & 0x10) ? envelope : volume;

				if (((noise|status) & 0x8) && out)
				{
					if (timer >= 0)
					{
						return out & dc;
					}
					else
					{
						sum &= dc;

						do
						{
							dc ^= (status & 0x1) - 1UL;
							sum += NST_MIN(dword(-timer),frequency) & dc;
							timer += iword(frequency);
						}
						while (timer < 0);

						NST_VERIFY( sum <= ULONG_MAX / out + rate/2 );
						return (sum * out + rate/2) / rate;
					}
				}
				else
				{
					while (timer < 0)
					{
						dc ^= (status & 0x1) - 1UL;
						timer += iword(frequency);
					}

					return 0;
				}
			}

			Fme7::Sound::Sample Fme7::Sound::GetSample()
			{
				if (active && outputVolume)
				{
					dword sample = 0;

					for (dword i=0, e=envelope.Clock( rate ), n=noise.Clock( rate ); i < NUM_SQUARES; ++i)
						sample += squares[i].GetSample( rate, e, n );

					return dcBlocker.Apply( sample * outputVolume / DEFAULT_VOLUME );
				}
				else
				{
					return 0;
				}
			}

			void Fme7::VSync()
			{
				irq.VSync();
			}
		}
	}
}
