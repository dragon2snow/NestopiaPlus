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
#include "NstBrdVrc6.hpp"
			 
namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif
	
			Vrc6::Sound::Square::Square()
			{ 
				Reset(); 
			}
	
			Vrc6::Sound::Saw::Saw()
			{ 
				Reset(); 
			}
	
			Vrc6::Sound::Sound(Cpu& cpu,bool hook)
			: apu(cpu.GetApu()), hooked(hook)
			{
				if (hook)
					apu.HookChannel( this );
			}
	
			Vrc6::Sound::~Sound()
			{
				if (hooked)
					apu.ReleaseChannel();
			}
	
			Vrc6::Vrc6(Context& c,const Type t)
			: Mapper(c,WRAM_8K), irq(c.cpu), sound(c.cpu), type(t)
			{
			}

			void Vrc6::Sound::BaseChannel::Reset()
			{
				enabled = false;
				waveLength = 1;
				active = false;
				timer = 0;
				frequency = 0;
				step = 0;
			}
	
			void Vrc6::Sound::Square::Reset()
			{
				BaseChannel::Reset();
	
				duty = 1;
				volume = 0;
				digitized = false;
			}
	
			void Vrc6::Sound::Saw::Reset()
			{
				BaseChannel::Reset();
	
				phase = 0;
				amp = 0;
				frequency = 0;
			}
	
			void Vrc6::Sound::Reset()
			{
				Apu::Channel::Reset();
	
				square[0].Reset();
				square[1].Reset();
				saw.Reset();
				dcBlocker.Reset();
			}
	
			void Vrc6::SubReset(const bool hard)
			{
				irq.Reset( hard, hard ? false : irq.IsLineEnabled() );
	
				for (dword i=0x8000U; i <= 0xFFFFUL; ++i)
				{
					switch ((type == TYPE_NORMAL ? i : ((i & 0xFFFCU) | ((i >> 1) & 0x1) | ((i << 1) & 0x2))) & 0xF003U)
					{
						case 0x8000U: Map( i, PRG_SWAP_16K     ); break;			
						case 0x9000U: Map( i, &Vrc6::Poke_9000 ); break;
						case 0x9001U: Map( i, &Vrc6::Poke_9001 ); break;
						case 0x9002U: Map( i, &Vrc6::Poke_9002 ); break;
						case 0xA000U: Map( i, &Vrc6::Poke_A000 ); break;
						case 0xA001U: Map( i, &Vrc6::Poke_A001 ); break;
						case 0xA002U: Map( i, &Vrc6::Poke_A002 ); break;
						case 0xB000U: Map( i, &Vrc6::Poke_B000 ); break;
						case 0xB001U: Map( i, &Vrc6::Poke_B001 ); break;
						case 0xB002U: Map( i, &Vrc6::Poke_B002 ); break;
						case 0xB003U: Map( i, &Vrc6::Poke_B003 ); break;
						case 0xC000U: Map( i, PRG_SWAP_8K_2    ); break;
						case 0xD000U: Map( i, CHR_SWAP_1K_0    ); break;
						case 0xD001U: Map( i, CHR_SWAP_1K_1    ); break;
						case 0xD002U: Map( i, CHR_SWAP_1K_2    ); break;
						case 0xD003U: Map( i, CHR_SWAP_1K_3    ); break;
						case 0xE000U: Map( i, CHR_SWAP_1K_4    ); break;
						case 0xE001U: Map( i, CHR_SWAP_1K_5    ); break;
						case 0xE002U: Map( i, CHR_SWAP_1K_6    ); break;
						case 0xE003U: Map( i, CHR_SWAP_1K_7    ); break;
						case 0xF000U: Map( i, &Vrc6::Poke_F000 ); break;
						case 0xF001U: Map( i, &Vrc6::Poke_F001 ); break;
						case 0xF002U: Map( i, &Vrc6::Poke_F002 ); break;
					}																		
				}
			}
	
			bool Vrc6::Sound::Square::CanOutput() const
			{
				return volume && enabled && !digitized && waveLength >= MIN_FRQ;
			}
	
			bool Vrc6::Sound::Saw::CanOutput() const
			{
				return enabled && phase && waveLength >= MIN_FRQ;
			}
	
			void Vrc6::Sound::Square::UpdateContext(const uint fixed)
			{
				active = CanOutput();
				frequency = (waveLength + 1U) * fixed;
			}
	
			void Vrc6::Sound::Saw::UpdateContext(const uint fixed)
			{
				active = CanOutput();
				frequency = ((waveLength + 1UL) << FRQ_SHIFT) * fixed;
			}
	
			void Vrc6::Sound::UpdateContext(uint,const u8 (&volumes)[MAX_CHANNELS])
			{
				outputVolume = volumes[Apu::CHANNEL_VRC6];

				square[0].UpdateContext( fixed );
				square[1].UpdateContext( fixed );
				saw.UpdateContext( fixed );
				dcBlocker.Reset();
			}
	
			void Vrc6::BaseLoad(State::Loader& state,const dword id)
			{
				NST_VERIFY( id == NES_STATE_CHUNK_ID('V','R','6','\0') );

				if (id == NES_STATE_CHUNK_ID('V','R','6','\0'))
				{
					while (const dword chunk = state.Begin())
					{
						switch (chunk)
						{
							case NES_STATE_CHUNK_ID('I','R','Q','\0'):
						
								irq.LoadState( State::Loader::Subset(state).Ref() );
								break;
						
							case NES_STATE_CHUNK_ID('S','N','D','\0'):
						
								sound.LoadState( State::Loader::Subset(state).Ref() );
								break;
						}

						state.End();
					}
				}
			}
	
			void Vrc6::BaseSave(State::Saver& state) const
			{
				state.Begin('V','R','6','\0');
				irq.SaveState(   State::Saver::Subset(state,'I','R','Q','\0').Ref() );
				sound.SaveState( State::Saver::Subset(state,'S','N','D','\0').Ref() );
				state.End();
			}
	
			void Vrc6::Sound::SaveState(State::Saver& state) const
			{
				square[0].SaveState( State::Saver::Subset(state,'S','Q','0','\0').Ref() );
				square[1].SaveState( State::Saver::Subset(state,'S','Q','1','\0').Ref() );
				saw.SaveState(       State::Saver::Subset(state,'S','A','W','\0').Ref() );
			}
	
			void Vrc6::Sound::LoadState(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					switch (chunk)
					{
			       		case NES_STATE_CHUNK_ID('S','Q','0','\0'): 
							
							square[0].LoadState( State::Loader::Subset(state).Ref(), fixed );
							break;
	
						case NES_STATE_CHUNK_ID('S','Q','1','\0'): 
							
							square[1].LoadState( State::Loader::Subset(state).Ref(), fixed );
							break;
	
						case NES_STATE_CHUNK_ID('S','A','W','\0'): 
							
							saw.LoadState( State::Loader::Subset(state).Ref(), fixed );
							break;
					}
	
					state.End();
				}
			}
	
			void Vrc6::Sound::Square::SaveState(State::Saver& state) const
			{
				const u8 data[4] =
				{
					(enabled ? 0x1 : 0x0) | (digitized ? 0x2 : 0x0),
					waveLength & 0xFF,
					waveLength >> 8,
					(duty - 1) | ((volume / VOLUME) << 3)
				};

				state.Begin('R','E','G','\0').Write( data ).End();
			}
	
			void Vrc6::Sound::Square::LoadState(State::Loader& state,const uint fixed)
			{
				while (const dword chunk = state.Begin())
				{
					if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					{
						const State::Loader::Data<4> data( state );
	
						enabled = data[0] & b01;
						digitized = data[0] & b10;
						waveLength = data[1] | ((data[2] & b1111) << 8);
						duty = (data[3] & b111) + 1;
						volume = ((data[3] >> 3) & b1111) * VOLUME;
	
						timer = 0;
						step = 0;
	
						UpdateContext( fixed );
					}
	
					state.End();
				}
			}
	
			void Vrc6::Sound::Saw::SaveState(State::Saver& state) const
			{
				const u8 data[3] =
				{
					(enabled != 0) | (phase << 1),
					waveLength & 0xFF,
					waveLength >> 8
				};

				state.Begin('R','E','G','\0').Write( data ).End();
			}
	
			void Vrc6::Sound::Saw::LoadState(State::Loader& state,const uint fixed)
			{
				while (const dword chunk = state.Begin())
				{
					if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					{
						const State::Loader::Data<3> data( state );
	
						enabled = data[0] & b1;
						phase = (data[0] >> 1) & b111111;
						waveLength = data[1] | ((data[2] & b1111) << 8);
	
						timer = 0;
						step = 0;
						amp = 0;
	
						UpdateContext( fixed );
					}
	
					state.End();
				}
			}
	
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif
			
			NST_FORCE_INLINE void Vrc6::Sound::Square::WriteReg0(const uint data)
			{
				volume = (data & REG0_VOLUME) * VOLUME;
				duty = ((data & REG0_DUTY) >> REG0_DUTY_SHIFT) + 1;
				digitized = data & REG0_DIGITIZED;	
				NST_VERIFY( !digitized );	
				active = CanOutput();
			}
	
			NST_FORCE_INLINE void Vrc6::Sound::Square::WriteReg1(const uint data,const dword fixed)
			{
				waveLength &= REG2_WAVELENGTH_HIGH << 8;
				waveLength |= data;
				frequency = (waveLength + 1U) * fixed;	
				active = CanOutput();
			}
	
			NST_FORCE_INLINE void Vrc6::Sound::Square::WriteReg2(const uint data,const dword fixed)
			{
				waveLength &= REG1_WAVELENGTH_LOW;
				waveLength |= (data & REG2_WAVELENGTH_HIGH) << 8;
				frequency = (waveLength + 1U) * fixed;
				enabled = data & REG2_ENABLE;	
				active = CanOutput();
			}
	
			NST_FORCE_INLINE void Vrc6::Sound::Saw::WriteReg0(const uint data)
			{
				phase = data & REG0_PHASE;
				active = CanOutput();
			}
	
			NST_FORCE_INLINE void Vrc6::Sound::Saw::WriteReg1(const uint data,const dword fixed)
			{
				waveLength &= REG2_WAVELENGTH_HIGH << 8;
				waveLength |= data;
				frequency = ((waveLength + 1UL) << FRQ_SHIFT) * fixed;
				active = CanOutput();
			}
	
			NST_FORCE_INLINE void Vrc6::Sound::Saw::WriteReg2(const uint data,const dword fixed)
			{
				waveLength &= REG1_WAVELENGTH_LOW;
				waveLength |= (data & REG2_WAVELENGTH_HIGH) << 8;
				frequency = ((waveLength + 1UL) << FRQ_SHIFT) * fixed;
				enabled = data & REG2_ENABLE;
				active = CanOutput();
			}
	
			void Vrc6::Sound::WriteSquareReg0(uint i,uint data) 
			{ 
				apu.Update(); 
				square[i].WriteReg0( data ); 
			}
	
			void Vrc6::Sound::WriteSquareReg1(uint i,uint data) 
			{
				apu.Update(); 
				square[i].WriteReg1( data, fixed ); 
			}
	
			void Vrc6::Sound::WriteSquareReg2(uint i,uint data) 
			{ 
				apu.Update(); 
				square[i].WriteReg2( data, fixed ); 
			}
	
			void Vrc6::Sound::WriteSawReg0(uint data) 
			{ 
				apu.Update(); 
				saw.WriteReg0( data ); 
			}
	
			void Vrc6::Sound::WriteSawReg1(uint data) 
			{ 
				apu.Update(); 
				saw.WriteReg1( data, fixed ); 
			}
	
			void Vrc6::Sound::WriteSawReg2(uint data) 
			{ 
				apu.Update(); 
				saw.WriteReg2( data, fixed ); 
			}
	
			NES_POKE(Vrc6,9000) { sound.WriteSquareReg0 ( 0, data ); }
			NES_POKE(Vrc6,9001) { sound.WriteSquareReg1 ( 0, data ); }
			NES_POKE(Vrc6,9002) { sound.WriteSquareReg2 ( 0, data ); }
			NES_POKE(Vrc6,A000) { sound.WriteSquareReg0 ( 1, data ); }
			NES_POKE(Vrc6,A001) { sound.WriteSquareReg1 ( 1, data ); }
			NES_POKE(Vrc6,A002) { sound.WriteSquareReg2 ( 1, data ); }
			NES_POKE(Vrc6,B000) { sound.WriteSawReg0    (    data ); }
			NES_POKE(Vrc6,B001) { sound.WriteSawReg1    (    data ); }
			NES_POKE(Vrc6,B002) { sound.WriteSawReg2    (    data ); }
	
			NST_FORCE_INLINE dword Vrc6::Sound::Square::GetSample(const Cycle rate)
			{
				NST_VERIFY( bool(active) == CanOutput() && timer >= 0 );

				if (active)
				{
					dword sum = timer;
					timer -= iword(rate);
					
					if (timer >= 0)
					{
						return step < duty ? volume : 0;
					}
					else
					{
						if (step >= duty)
							sum = 0;

						do 
						{	
							step = (step + 1) & 0xF;

							if (step < duty)
								sum += NST_MIN(dword(-timer),frequency);

							timer += iword(frequency);
						} 
						while (timer < 0);

						return (sum * volume + (rate/2)) / rate;
					}
				}

				return 0;
			}
	
			NST_FORCE_INLINE dword Vrc6::Sound::Saw::GetSample(const Cycle rate)
			{
				NST_VERIFY( bool(active) == CanOutput() && timer >= 0 );
	
				if (active)
				{
					dword sum = timer;
					timer -= iword(rate);

					if (timer >= 0)
					{
						return (amp >> 3) * VOLUME;
					}
					else
					{
						sum *= amp;

						do
						{
							if (++step >= 0x7)
							{
								step = 0;
								amp = 0;
							}

							amp = (amp + phase) & 0xFF;
							sum += NST_MIN(dword(-timer),frequency) * amp;
							
							timer += iword(frequency);
						}
						while (timer < 0);

						return ((sum >> 3) * VOLUME + (rate / 2)) / rate;
					}
				}

				return 0;
			}
	
			Vrc6::Sound::Sample Vrc6::Sound::GetSample()
			{	
				if (outputVolume)
				{
					dword sample = 0;

					for (uint i=0; i < 2; ++i)
						sample += square[i].GetSample( rate );
	
					sample += saw.GetSample( rate );

					return dcBlocker.Apply( sample * outputVolume / DEFAULT_VOLUME );
				}
				else
				{
					return 0;
				}
			}
			
			NES_POKE(Vrc6,B003) 
			{
				static const uchar lut[4] =
				{
					Ppu::NMT_VERTICAL,
					Ppu::NMT_HORIZONTAL,
					Ppu::NMT_ZERO,
					Ppu::NMT_ONE
				};
	
				ppu.SetMirroring( lut[(data >> 2) & 0x3] );
			}
	
			NES_POKE(Vrc6,F000) 
			{ 
				irq.Update();
				irq.unit.latch = data;
			}
	
			NES_POKE(Vrc6,F001) 
			{ 
				irq.Toggle( data );
			}
	
			NES_POKE(Vrc6,F002) 
			{ 
				irq.Toggle();
			}

			void Vrc6::VSync()
			{
				irq.VSync();
			}
		}
	}
}
