////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2007 Martin Freij
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

#include <cstring>
#include "../NstMapper.hpp"
#include "../NstClock.hpp"
#include "NstBrdN106.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			struct N106::Chips
			{
				struct Irq
				{
					void Reset(bool);
					bool Clock();

					uint count;
				};

				ClockUnits::M2<Irq> irq;
				Sound sound;

				Chips(Cpu& c,Apu& a)
				: irq(c), sound(a) {}
			};

			N106::Sound::Sound(Apu& a,bool connect)
			: Channel(a)
			{
				Reset();
				bool audible = UpdateSettings();

				if (connect)
					Connect( audible );
			}

			N106::N106(Context& c,const Type type)
			:
			Mapper (c,PROM_MAX_512K|CROM_MAX_256K|CRAM_8K),
			chips  (type == TYPE_ADD_ONS ? new Chips(c.cpu,c.apu) : NULL)
			{
			}

			N106::~N106()
			{
				delete chips;
			}

			void N106::Sound::BaseChannel::Reset()
			{
				enabled = false;
				active = false;
				timer = 0;
				frequency = 0;
				phase = 0;
				waveLength = 0;
				waveOffset = 0;
				volume = 0;
			}

			void N106::Sound::Reset()
			{
				exAddress = 0x00;
				exIncrease = 0x01;
				startChannel = NUM_CHANNELS;
				frequency = 0;

				std::memset( wave, 0, sizeof(wave) );
				std::memset( exRam, 0, sizeof(exRam) );

				for (uint i=0; i < NUM_CHANNELS; ++i)
					channels[i].Reset();

				dcBlocker.Reset();
			}

			void N106::Chips::Irq::Reset(const bool hard)
			{
				if (hard)
					count = 0;
			}

			void N106::SubReset(const bool hard)
			{
				if (hard)
					reg = 0;

				Map( 0x8000U, 0x87FFU, &N106::Poke_8000 );
				Map( 0x8800U, 0x8FFFU, &N106::Poke_8800 );
				Map( 0x9000U, 0x97FFU, &N106::Poke_9000 );
				Map( 0x9800U, 0x9FFFU, &N106::Poke_9800 );
				Map( 0xA000U, 0xA7FFU, &N106::Poke_A000 );
				Map( 0xA800U, 0xAFFFU, &N106::Poke_A800 );
				Map( 0xB000U, 0xB7FFU, &N106::Poke_B000 );
				Map( 0xB800U, 0xBFFFU, &N106::Poke_B800 );
				Map( 0xE000U, 0xE7FFU, &N106::Poke_E000 );
				Map( 0xE800U, 0xEFFFU, &N106::Poke_E800 );
				Map( 0xF000U, 0xF7FFU, &N106::Poke_F000 );

				if (!chr.Source().Internal())
					chr.SwapBank<SIZE_8K,0x0000>( ~0U );

				if (chips)
				{
					chips->irq.Reset( hard, hard || chips->irq.Connected() );

					Map( 0x4800U, 0x4FFFU, &N106::Peek_4800, &N106::Poke_4800 );
					Map( 0x5000U, 0x57FFU, &N106::Peek_5000, &N106::Poke_5000 );
					Map( 0x5800U, 0x5FFFU, &N106::Peek_5800, &N106::Poke_5800 );
					Map( 0xC000U, 0xC7FFU, &N106::Poke_C000 );
					Map( 0xC800U, 0xCFFFU, &N106::Poke_C800 );
					Map( 0xD000U, 0xD7FFU, &N106::Poke_D000 );
					Map( 0xD800U, 0xDFFFU, &N106::Poke_D800 );
					Map( 0xF800U, 0xFFFFU, &N106::Poke_F800 );
				}
			}

			void N106::Sound::SaveState(State::Saver& state,const dword baseChunk) const
			{
				state.Begin( baseChunk );
				state.Begin( AsciiId<'R','E','G'>::V ).Write8( exAddress | (exIncrease << 7) ).End();
				state.Begin( AsciiId<'R','A','M'>::V ).Compress( exRam ).End();
				state.End();
			}

			void N106::Sound::LoadState(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					switch (chunk)
					{
						case AsciiId<'R','E','G'>::V:
						{
							const uint data = state.Read8();
							exAddress = data & 0x7F;
							exIncrease = data >> 7;
							break;
						}

						case AsciiId<'R','A','M'>::V:

							state.Uncompress( exRam );

							for (uint i=0; i < sizeof(exRam); ++i)
							{
								wave[i*2+0] = (exRam[i] & 0xFU) << 2;
								wave[i*2+1] = (exRam[i] >>   4) << 2;
							}

							for (uint i=64; i < sizeof(exRam); i += 8)
							{
								BaseChannel& channel = channels[(i - 64) >> 3];

								channel.Reset();

								channel.SetFrequency  ( FetchFrequency(i) );
								channel.SetWaveLength ( exRam[i+4] );
								channel.SetWaveOffset ( exRam[i+6] );
								channel.SetVolume     ( exRam[i+7] );

								channel.Validate();
							}

							SetChannelState( exRam[0x7F] );
							break;
					}

					state.End();
				}
			}

			void N106::BaseLoad(State::Loader& state,const dword baseChunk)
			{
				NST_VERIFY( baseChunk == (AsciiId<'N','M','6'>::V) );

				if (baseChunk == AsciiId<'N','M','6'>::V)
				{
					while (const dword chunk = state.Begin())
					{
						switch (chunk)
						{
							case AsciiId<'R','E','G'>::V:

								reg = state.Read8();
								break;

							case AsciiId<'I','R','Q'>::V:

								NST_VERIFY( chips );

								if (chips)
								{
									State::Loader::Data<3> data( state );
									chips->irq.unit.count = data[1] | (data[2] << 8 & 0x7F00) | (data[0] << 15 & 0x8000);
								}
								break;

							case AsciiId<'S','N','D'>::V:

								NST_VERIFY( chips );

								if (chips)
									chips->sound.LoadState( state );

								break;
						}

						state.End();
					}
				}
			}

			void N106::BaseSave(State::Saver& state) const
			{
				state.Begin( AsciiId<'N','M','6'>::V );
				state.Begin( AsciiId<'R','E','G'>::V ).Write8( reg ).End();

				if (chips)
				{
					const byte data[3] =
					{
						chips->irq.unit.count >> 15,
						chips->irq.unit.count >> 0 & 0xFF,
						chips->irq.unit.count >> 8 & 0x7F
					};

					state.Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End();
					chips->sound.SaveState( state, AsciiId<'S','N','D'>::V );
				}

				state.End();
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			bool N106::Chips::Irq::Clock()
			{
				return (count - 0x8000 < 0x7FFF) && (++count == 0xFFFF);
			}

			inline bool N106::Sound::BaseChannel::CanOutput() const
			{
				return volume && frequency && enabled;
			}

			inline void N106::Sound::BaseChannel::Validate()
			{
				active = CanOutput();
			}

			inline void N106::Sound::BaseChannel::SetFrequency(const uint f)
			{
				frequency = f;
			}

			inline void N106::Sound::BaseChannel::SetWaveLength(const uint data)
			{
				const dword length = (0x20UL - (data & REG_WAVELENGTH)) << PHASE_SHIFT;

				if (waveLength != length)
				{
					waveLength = length;
					phase = 0;
				}

				enabled = data >> REG_ENABLE_SHIFT;
			}

			inline void N106::Sound::BaseChannel::SetWaveOffset(const uint data)
			{
				waveOffset = data;
			}

			inline void N106::Sound::BaseChannel::SetVolume(const uint data)
			{
				volume = (data & REG_VOLUME) * VOLUME;
			}

			inline dword N106::Sound::BaseChannel::GetSample
			(
				const Cycle rate,
				const Cycle factor,
				const byte (&wave)[0x100]
			)
			{
				NST_VERIFY( bool(active) == CanOutput() );

				if (active)
				{
					phase = (phase + (timer + rate) / factor * frequency) % waveLength;
					timer = (timer + rate) % factor;

					return wave[(waveOffset + (phase >> PHASE_SHIFT)) & 0xFF] * dword(volume);
				}

				return 0;
			}

			N106::Sound::Sample N106::Sound::GetSample()
			{
				if (output)
				{
					dword sample = 0;

					for (BaseChannel* channel = channels+startChannel; channel != channels+NUM_CHANNELS; ++channel)
						sample += channel->GetSample( rate, frequency, wave );

					return dcBlocker.Apply( sample * output / DEFAULT_VOLUME );
				}
				else
				{
					return 0;
				}
			}

			bool N106::Sound::UpdateSettings()
			{
				output = GetVolume(EXT_N106) * 68U / DEFAULT_VOLUME;

				rate =
				(
					qword(GetRegion() == Region::NTSC ? Clocks::NTSC_CLK : Clocks::PAL_CLK) * (1UL << SPEED_SHIFT) /
					(GetSampleRate() * 45UL * (GetRegion() == Region::NTSC ? Clocks::NTSC_DIV : Clocks::PAL_DIV))
				);

				dcBlocker.Reset();

				return output;
			}

			inline void N106::Sound::SetChannelState(uint data)
			{
				data = (data >> 4 & 0x7) + 1;
				frequency = dword(data) << SPEED_SHIFT;
				startChannel = NUM_CHANNELS - data;
			}

			inline dword N106::Sound::FetchFrequency(uint address) const
			{
				address &= 0x78;

				return
				(
					(dword(exRam[address+0x0]       ) <<  0) |
					(dword(exRam[address+0x2]       ) <<  8) |
					(dword(exRam[address+0x4] & 0x3U) << 16)
				);
			}

			inline void N106::Sound::WriteWave(const uint data)
			{
				const uint index = exAddress << 1;
				wave[index+0] = (data & 0xF) << 2;
				wave[index+1] = (data >>  4) << 2;
			}

			uint N106::Sound::ReadData()
			{
				const uint data = exRam[exAddress];
				exAddress = (exAddress + exIncrease) & 0x7F;
				return data;
			}

			NES_PEEK(N106,4800)
			{
				return chips->sound.ReadData();
			}

			void N106::Sound::WriteData(const uint data)
			{
				Update();

				WriteWave( data );
				exRam[exAddress] = data;

				if (exAddress >= 0x40)
				{
					BaseChannel& channel = channels[(exAddress - 0x40) >> 3];

					switch (exAddress & 0x7)
					{
						case 0x4:

							channel.SetWaveLength( data );

						case 0x0:
						case 0x2:

							channel.SetFrequency( FetchFrequency(exAddress) );
							break;

						case 0x6:

							channel.SetWaveOffset( data );
							break;

						case 0x7:

							channel.SetVolume( data );

							if (exAddress == 0x7F)
								SetChannelState( data );

							break;
					}

					channel.Validate();
				}

				exAddress = (exAddress + exIncrease) & 0x7F;
			}

			NES_POKE_D(N106,4800)
			{
				chips->sound.WriteData( data );
			}

			NES_PEEK(N106,5000)
			{
				chips->irq.Update();
				return chips->irq.unit.count & 0xFF;
			}

			NES_POKE_D(N106,5000)
			{
				chips->irq.Update();
				chips->irq.unit.count = (chips->irq.unit.count & 0xFF00) | data;
				chips->irq.ClearIRQ();
			}

			NES_PEEK(N106,5800)
			{
				chips->irq.Update();
				return chips->irq.unit.count >> 8;
			}

			NES_POKE_D(N106,5800)
			{
				chips->irq.Update();
				chips->irq.unit.count = (chips->irq.unit.count & 0x00FF) | (data << 8);
				chips->irq.ClearIRQ();
			}

			void N106::SwapChr(const uint address,const uint data,uint chrBits) const
			{
				ppu.Update();
				chr.Source( data >= 0xE0 && chrBits == 0x00 ).SwapBank<SIZE_1K>( address, data );
			}

			NES_POKE_D(N106,8000) { SwapChr( 0x0000, data, reg & 0x40 ); }
			NES_POKE_D(N106,8800) { SwapChr( 0x0400, data, reg & 0x40 ); }
			NES_POKE_D(N106,9000) { SwapChr( 0x0800, data, reg & 0x40 ); }
			NES_POKE_D(N106,9800) { SwapChr( 0x0C00, data, reg & 0x40 ); }
			NES_POKE_D(N106,A000) { SwapChr( 0x1000, data, reg & 0x80 ); }
			NES_POKE_D(N106,A800) { SwapChr( 0x1400, data, reg & 0x80 ); }
			NES_POKE_D(N106,B000) { SwapChr( 0x1800, data, reg & 0x80 ); }
			NES_POKE_D(N106,B800) { SwapChr( 0x1C00, data, reg & 0x80 ); }

			void N106::SwapNmt(const uint address,const uint data) const
			{
				ppu.Update();
				nmt.Source( data < 0xE0 ).SwapBank<SIZE_1K>( address, data & (data >= 0xE0 ? 0x01 : 0xFF) );
			}

			NES_POKE_D(N106,C000) { SwapNmt( 0x0000, data ); }
			NES_POKE_D(N106,C800) { SwapNmt( 0x0400, data ); }
			NES_POKE_D(N106,D000) { SwapNmt( 0x0800, data ); }
			NES_POKE_D(N106,D800) { SwapNmt( 0x0C00, data ); }

			NES_POKE_D(N106,E000)
			{
				prg.SwapBank<SIZE_8K,0x0000>( data );
			}

			NES_POKE_D(N106,E800)
			{
				reg = data;
				prg.SwapBank<SIZE_8K,0x2000>( data );
			}

			NES_POKE_D(N106,F000)
			{
				prg.SwapBank<SIZE_8K,0x4000>( data );
			}

			void N106::Sound::WriteAddress(const uint data)
			{
				NST_COMPILE_ASSERT( EXRAM_INC == 0x80 );

				exAddress = data & 0x7F;
				exIncrease = data >> 7;
			}

			NES_POKE_D(N106,F800)
			{
				chips->sound.WriteAddress( data );
			}

			void N106::Sync(Event event,Input::Controllers*)
			{
				if (event == EVENT_END_FRAME)
				{
					if (chips)
						chips->irq.VSync();
				}
			}
		}
	}
}
