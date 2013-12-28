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

#include <cstring>
#include <new>
#include "NstLog.hpp"
#include "NstChecksumCrc32.hpp"
#include "NstState.hpp"
#include "NstPpu.hpp"
#include "NstFds.hpp"
#include "api/NstApiFds.hpp"
#include "api/NstApiUser.hpp"
#include "api/NstApiInput.hpp"
#include "NstSignedArithmetic.hpp"

namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif

		Fds::Bios::Instance::Instance()
		: loaded(false)	
		{
			std::memset( rom, 0x00, SIZE_8K );
		}

		Fds::Bios::Instance Fds::Bios::instance;

		Result Fds::Bios::Set(StdStream input)
		{
			instance.loaded = false;
	
			if (input)
			{
				Stream::In stream( input );

				try
				{
					if (stream.Length() < SIZE_8K)
						throw RESULT_ERR_CORRUPT_FILE;
	
					long offset = 0;
	
					if (stream.Read32() == 0x1A53454EUL)
					{
						// locate last bank
						const ulong romOffset = (stream.Read8() * SIZE_16K);
	
						if (romOffset < SIZE_8K) 
							throw RESULT_ERR_CORRUPT_FILE;
	
						stream.Seek( 1 );
	
						if (stream.Read8() & 0x4) // trainer (unlikely but just in case)
							offset = 512;
	
						offset += (16 - (4+1+1+1)) + (romOffset - SIZE_8K);
					}
					else
					{
						offset = -4;
					}
	
					stream.Seek( offset );
					stream.Read( instance.rom, SIZE_8K );
	
					instance.loaded = true;
	
					switch (Checksum::Crc32::Compute( instance.rom, SIZE_8K ))
					{
						case 0x5E607DCFUL: // standard
						case 0x4DF24A6CUL: // twinsys
				
							Log::Flush( "Fds: BIOS ROM ok" NST_LINEBREAK );
							break;
				
						default:
				
							Log::Flush( "Fds: warning, unknown BIOS ROM!" NST_LINEBREAK );
				     		break;
					}
				}
				catch (Result result)
				{
					return result;
				}
				catch (const std::bad_alloc&)
				{
					return RESULT_ERR_OUT_OF_MEMORY;
				}
				catch (...)
				{
					return RESULT_ERR_GENERIC;
				}
			}
			else
			{
				std::memset( instance.rom, 0x00, SIZE_8K );
			}
	
			return RESULT_OK;
		}
	
		Result Fds::Bios::Get(StdStream stream)
		{
			try
			{
				if (instance.loaded)
					Stream::Out(stream).Write( instance.rom, SIZE_8K );
				else
					return RESULT_ERR_NOT_READY;
			}
			catch (Result result)
			{
				return result;
			}
			catch (const std::bad_alloc&)
			{
				return RESULT_ERR_OUT_OF_MEMORY;
			}
			catch (...) 
			{
				return RESULT_ERR_GENERIC;
			}
	
			return RESULT_OK;
		}

		const u8 Fds::Sound::volumes[4] = 
		{
			30 * 8,
			20 * 8,
			15 * 8,
			12 * 8
		};

		const u8 Fds::Sound::Modulator::steps[8] = 
		{
			0x00,
			0x01,
			0x02,
			0x04,
			0x80,
			0xFC,
			0xFE,
			0xFF
		};

		Fds::Sound::Sound(Cpu& c,bool hook)
		: cpu(c), hooked(hook)
		{
			ResetChannel();

			if (hook)
				cpu.GetApu().HookChannel( this );
		}
	
		Fds::Sound::~Sound()
		{
			if (hooked)
				cpu.GetApu().ReleaseChannel();
		}

		const Fds::Disks::Sides Fds::Disks::Create(StdStream input)
		{
			Stream::In stream( input );

			Sides sides;

			switch (stream.Read32())
			{
				case 0x1A534446UL:
				
					sides.count = stream.Read8();

					if (sides.count && stream.Length() >= HEADER_RESERVED + (sides.count * SIDE_SIZE))
					{
						u8 header[HEADER_RESERVED];
						stream.Read( header, HEADER_RESERVED );
						sides.header = static_cast<const u8*>(std::memcpy( new u8 [HEADER_RESERVED], header, HEADER_RESERVED ));
					}
					else
					{
						throw RESULT_ERR_CORRUPT_FILE;
					}
					break;
			
				case 0x494E2A01UL:
				
					stream.Seek( -4 );
			
					if (ulong count = stream.Length())
					{
						if (count > SIDE_SIZE * 0xFF)
							count = SIDE_SIZE * 0xFF;

						sides.count = count / SIDE_SIZE;
						sides.header = NULL;
					}
					else
					{
						throw RESULT_ERR_CORRUPT_FILE;
					}
					break;
			
				default: throw RESULT_ERR_INVALID_FILE;
			}

			sides.data = NULL;

			try
			{
				sides.data = new u8 [sides.count][SIDE_SIZE];
				stream.Read( sides.data, sides.count * SIDE_SIZE );
			}
			catch (...)
			{
				delete [] sides.data;
				delete [] sides.header;
				throw;
			}

			sides.id =
			(
				( sides.data[0][0x0F] << 24 ) | 
				( sides.data[0][0x10] << 16 ) | 
				( sides.data[0][0x11] <<  8 ) | 
				( sides.data[0][0x12] <<  0 )
			);

			sides.crc = Checksum::Crc32::Compute( sides.data, sides.count * SIDE_SIZE );
			sides.checksum = Checksum::Md5::Compute( sides.data, sides.count * SIDE_SIZE );

			return sides;
		}

		void Fds::Disks::Sides::Save() const
		{
			if (!count || checksum == Checksum::Md5::Compute( data, count * SIDE_SIZE ))
				return;
	
			try
			{
				std::vector<u8> image( (header ? HEADER_SIZE : 0) + (count * SIDE_SIZE) );	
				u8* it = &image.front();
	
				if (header)
				{
					NST_COMPILE_ASSERT( (HEADER_SIZE - HEADER_RESERVED) == (4+1) );

					it[0] = 0x46;
					it[1] = 0x44;
					it[2] = 0x53;
					it[3] = 0x1A;
					it[4] = count;
	
					std::memcpy( it + 5, header, HEADER_RESERVED );
					it += HEADER_SIZE;
				}
	
				std::memcpy( it, data, count * SIDE_SIZE );
	
				Api::User::fileIoCallback( Api::User::FILE_SAVE_FDS, image );
			}
			catch (const std::bad_alloc&)
			{
			}
		}

		Fds::IrqClock::IrqClock(Cpu& cpu)
		: Clock::M2<Irq>(cpu) {}

		Fds::Disks::Disks(StdStream stream)
		: 
		data     (NULL),
		current  (EJECTED),
		mounting (0),
		sides    (Create(stream))
		{
		}

		Fds::Disks::~Disks()
		{
			delete [] sides.data;
			delete [] sides.header;
		}

		Fds::Fds(Context& context)
		: 
		Image (DISK),
		disks (context.stream),
		irq   (context.cpu),
		cpu   (context.cpu), 
		ppu   (context.ppu),
		sound (context.cpu)
		{
			if (!Bios::IsLoaded())
				throw RESULT_ERR_MISSING_BIOS;

			ppu.GetChrMem().Source().Set( SIZE_8K, true, true );
		}
	
		Fds::~Fds()
		{
			EjectDisk();
			disks.sides.Save();
		}

		uint Fds::GetDesiredController(const uint port) const
		{
			if (port == Api::Input::EXPANSION_PORT)
				return (disks.sides.id == 0xA4445245UL) ? Api::Input::DOREMIKKOKEYBOARD : Api::Input::UNCONNECTED;
			else
				return Image::GetDesiredController( port );
		}

		void Fds::Regs::Reset()
		{
			ctrl0 = 0;
			ctrl1 = 0;
			data = 0;
		}

		void Fds::Io::Reset()
		{
			pos = 0;
			skip = 0;
			port = 0;
			led = ~0U;
		}
		
		void Fds::Ram::Reset()
		{
			std::memset( mem, 0x00, sizeof(mem) );
		}

		void Fds::Irq::Reset(bool)
		{
			ctrl = 0;
			count = 0;
			latch = 0;
			status = 0;
			drive.count = 0;
			drive.notify = false;
		}

		void Fds::Reset(const bool hard)
		{
			irq.Reset(true,true);
			regs.Reset();
			io.Reset();

			if (hard)
			{
				if (disks.mounting)
				{
					disks.mounting = 0;
					disks.data = disks.sides.data[disks.current];
				}

				ram.Reset();
				ppu.GetChrMem().Source().Clear();
				ppu.GetChrMem().SwapBank<SIZE_8K,0x0000U>( 0 );
			}

			cpu.ClearIRQ(); 
	
			cpu.Map( 0x4020U ).Set( &irq, &Fds::IrqClock::Peek_Nop,  &Fds::IrqClock::Poke_4020 );
			cpu.Map( 0x4021U ).Set( &irq, &Fds::IrqClock::Peek_Nop,  &Fds::IrqClock::Poke_4021 );
			cpu.Map( 0x4022U ).Set( &irq, &Fds::IrqClock::Peek_Nop,  &Fds::IrqClock::Poke_4022 );
			cpu.Map( 0x4030U ).Set( &irq, &Fds::IrqClock::Peek_4030, &Fds::IrqClock::Poke_Nop  );

			cpu.Map( 0x4023U ).Set( this, &Fds::Peek_Nop,  &Fds::Poke_4023 );
			cpu.Map( 0x4024U ).Set( this, &Fds::Peek_Nop,  &Fds::Poke_4024 );
			cpu.Map( 0x4025U ).Set( this, &Fds::Peek_Nop,  &Fds::Poke_4025 );
			cpu.Map( 0x4026U ).Set( this, &Fds::Peek_Nop,  &Fds::Poke_4026 );
			cpu.Map( 0x4031U ).Set( this, &Fds::Peek_4031, &Fds::Poke_Nop  );
			cpu.Map( 0x4032U ).Set( this, &Fds::Peek_4032, &Fds::Poke_Nop  );
			cpu.Map( 0x4033U ).Set( this, &Fds::Peek_4033, &Fds::Poke_Nop  );

			cpu.Map( 0x6000U, 0xDFFFU ).Set( &ram, &Fds::Ram::Peek_Ram, &Fds::Ram::Poke_Ram );
			cpu.Map( 0xE000U, 0xFFFFU ).Set( &Bios::instance, &Bios::Instance::Peek_Rom, &Bios::Instance::Poke_Nop );
		
			Log() << "Fds: reset" NST_LINEBREAK "Fds: " << NumDisks() << " disk(s) present" NST_LINEBREAK;
		}
	
		void Fds::Sound::Envelope::Reset()
		{
			ctrl = 0;
			counter = 1;
			gain = GAIN_MIN;
		}

		void Fds::Sound::ResetChannel()
		{
			Apu::Channel::Reset();
	
			wave.writing = false;
			wave.length = 0;
			wave.pos = 0;
			wave.volume = 0;
	
			modulator.active = false;
			modulator.writing = false;
			modulator.pos = 0;
			modulator.length = 0;
			modulator.timer = 0;
			modulator.sweep = 0;
	
			envelopes.counter = 1;
			envelopes.length = 0;
	
			envelopes.units[VOLUME].Reset();
			envelopes.units[SWEEP].Reset();
	
			std::memset( wave.table, 0, Wave::SIZE );
			std::memset( modulator.table, 0x00, Modulator::SIZE );
	
			status = 0;
			volume = volumes[0];
			amp = 0;
		}
	
		void Fds::Sound::Reset()
		{
			cpu.Map( 0x4040U, 0x407FU ).Set( this, &Fds::Sound::Peek_4040, &Fds::Sound::Poke_4040 );
			
			cpu.Map( 0x4080U ).Set( this, &Fds::Sound::Peek_Nop,  &Fds::Sound::Poke_4080 );
			cpu.Map( 0x4082U ).Set( this, &Fds::Sound::Peek_Nop,  &Fds::Sound::Poke_4082 );
			cpu.Map( 0x4083U ).Set( this, &Fds::Sound::Peek_Nop,  &Fds::Sound::Poke_4083 );
			cpu.Map( 0x4084U ).Set( this, &Fds::Sound::Peek_Nop,  &Fds::Sound::Poke_4084 );
			cpu.Map( 0x4085U ).Set( this, &Fds::Sound::Peek_Nop,  &Fds::Sound::Poke_4085 );
			cpu.Map( 0x4086U ).Set( this, &Fds::Sound::Peek_Nop,  &Fds::Sound::Poke_4086 );
			cpu.Map( 0x4087U ).Set( this, &Fds::Sound::Peek_Nop,  &Fds::Sound::Poke_4087 );
			cpu.Map( 0x4088U ).Set( this, &Fds::Sound::Peek_Nop,  &Fds::Sound::Poke_4088 );
			cpu.Map( 0x4089U ).Set( this, &Fds::Sound::Peek_Nop,  &Fds::Sound::Poke_4089 );
			cpu.Map( 0x408AU ).Set( this, &Fds::Sound::Peek_Nop,  &Fds::Sound::Poke_408A );
			cpu.Map( 0x4090U ).Set( this, &Fds::Sound::Peek_4090, &Fds::Sound::Poke_Nop  );
			cpu.Map( 0x4092U ).Set( this, &Fds::Sound::Peek_4092, &Fds::Sound::Poke_Nop  );
	
			ResetChannel();
			dcBlocker.Reset();
	
			cpu.GetApu().SetExternalClock( 0 );
		}

		void Fds::Sound::UpdateContext(uint,const u8 (&volumes)[MAX_CHANNELS])
		{
			NST_VERIFY( fixed <= 0xFFFFU && rate <= 0x7FFFFUL );
			
			outputVolume = volumes[Apu::CHANNEL_FDS] * 69 / DEFAULT_VOLUME;
			amp = 0;
			dcBlocker.Reset();
			active = CanOutput();
		}

		Result Fds::InsertDisk(uint disk,const uint side)
		{
			NST_VERIFY( disks.sides.count );

			disk *= 2;

			if (side < 2 && disk < disks.sides.count)
			{
				disk += side;

				if (disks.current != disk)
				{
					const uint prev = disks.current;

					disks.current = disk;
					disks.data = NULL;
					disks.mounting = Disks::MOUNTING;

					if (prev != Disks::EJECTED)
						Api::Fds::diskChangeCallback( Api::Fds::DISK_EJECT, prev / 2, prev % 2 );

					Api::Fds::diskChangeCallback( Api::Fds::DISK_INSERT, disk / 2, disk % 2 );

					return RESULT_OK;
				}
			
				return RESULT_NOP;
			}
			
			return RESULT_ERR_INVALID_PARAM;
		}

		Result Fds::EjectDisk()
		{
			if (disks.current != Disks::EJECTED)
			{
				const uint prev = disks.current;

				disks.current = Disks::EJECTED;
				disks.data = NULL;
				disks.mounting = 0;

				Api::Fds::diskChangeCallback( Api::Fds::DISK_EJECT, prev / 2, prev % 2 );

				return RESULT_OK;
			}

			return RESULT_NOP;
		}

		inline bool Fds::Sound::CanModulate() const
		{
			return modulator.length && !modulator.writing;
		}

		void Fds::LoadState(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case NES_STATE_CHUNK_ID('R','E','G','\0'):
					{
						const State::Loader::Data<7> data( state );

						regs.ctrl0 = data[0];
						regs.ctrl1 = data[1];
						regs.data = data[2];
						io.port = data[3];
						io.pos = data[4] | (data[5] << 8);
						io.skip = data[6] & 0x3;

						if (io.pos > 65000U)
							io.pos = 65000U;

						ppu.SetMirroring( (regs.ctrl1 & Regs::CTRL1_MIRRORING_HORIZONTAL) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
						break;
					}

					case NES_STATE_CHUNK_ID('I','R','Q','\0'):
					{
						const State::Loader::Data<7> data( state );

						irq.unit.ctrl = data[0];
						irq.unit.status = data[1] & (Irq::PENDING_CTRL|Irq::PENDING_DRIVE);
						irq.unit.latch = data[2] | (data[3] << 8);
						irq.unit.count = data[4] | (data[5] << 8);
						irq.unit.drive.count = data[6];

						if (irq.unit.status)
							cpu.DoIRQ();
						else
							cpu.ClearIRQ();

						break;
					}

					case NES_STATE_CHUNK_ID('R','A','M','\0'):

						state.Uncompress( ram.mem );
						break;

					case NES_STATE_CHUNK_ID('C','H','R','\0'):

						state.Uncompress( ppu.GetChrMem().Source().Mem(), SIZE_8K );
						break;

					case NES_STATE_CHUNK_ID('D','S','K','\0'):
					{
						const State::Loader::Data<4> data( state );

						if (data[0] != disks.sides.count)
							throw RESULT_ERR_INVALID_FILE;

						if (data[1] & 0x1)
						{
							if (NES_FAILED(InsertDisk( data[2] / 2, data[2] % 2 )))
								throw RESULT_ERR_CORRUPT_FILE;

							disks.mounting = data[3];
							disks.data = disks.mounting ? NULL : disks.sides.data[disks.current];
						}
						else 
						{
							EjectDisk();

							disks.mounting = 0;
							disks.data = NULL;
						}
						break;
					}

					case NES_STATE_CHUNK_ID('S','N','D','\0'):

						sound.LoadState( State::Loader::Subset(state).Ref() );
						break;

					default:

						for (uint i=0; i < disks.sides.count; ++i)
						{
							if (chunk == NES_STATE_CHUNK_ID('D','0' + (i / 2),'A' + (i % 2),'\0'))
							{
								u8* const data = disks.sides.data[i];
								state.Uncompress( data, Disks::SIDE_SIZE );

								for (uint j=0; j < Disks::SIDE_SIZE; ++j)
									data[j] = ~data[j];

								break;
							}
						}
						break;
				}

				state.End();
			}

			io.led = ~0U;
			irq.unit.drive.notify = regs.ctrl1 & Regs::CTRL1_DISK_IRQ_ENABLED;
		}
	
		void Fds::SaveState(State::Saver& state) const
		{
			{
				const u8 data[7] =
				{
					regs.ctrl0,
					regs.ctrl1,
					regs.data,
					io.port,
					io.pos & 0xFF,
					io.pos >> 8,
					io.skip
				};

				state.Begin('R','E','G','\0').Write( data ).End();
			}

			{
				const u8 data[7] =
				{
					irq.unit.ctrl,
					irq.unit.status,
					irq.unit.latch & 0xFF,
					irq.unit.latch >> 8,
					irq.unit.count & 0xFF,
					irq.unit.count >> 8,
					irq.unit.drive.count
				};

				state.Begin('I','R','Q','\0').Write( data ).End();
			}

			{
				state.Begin('R','A','M','\0').Compress( ram.mem ).End();
			}

			{
				state.Begin('C','H','R','\0').Compress( ppu.GetChrMem().Source().Mem(), SIZE_8K ).End();
			}

			{
				const u8 data[4] =
				{
					disks.sides.count,
					disks.current != Disks::EJECTED,
					disks.current != Disks::EJECTED ? disks.current : 0xFF,
					disks.current != Disks::EJECTED ? disks.mounting : 0
				};

				state.Begin('D','S','K','\0').Write( data ).End();
			}

			{
				u8 dst[Disks::SIDE_SIZE];

				for (uint i=0; i < disks.sides.count; ++i)
				{
					const u8* const src = disks.sides.data[i];

					for (uint j=0; j < Disks::SIDE_SIZE; ++j)
						dst[j] = ~src[j];

					state.Begin('D','0' + (i / 2),'A' + (i % 2),'\0').Compress( dst ).End();
				}
			}

			{
				sound.SaveState( State::Saver::Subset(state,'S','N','D','\0').Ref() );
			}
		}
	
		void Fds::Sound::SaveState(State::Saver& state) const
		{
			state.Begin('M','A','S','\0');
			{
				{
					u8 data[6] =
					{
						((status & STATUS_OUTPUT_ENABLED) ? 0 : REG3_OUTPUT_DISABLE) |
						((status & STATUS_ENVELOPES_ENABLED) ? 0 : REG3_ENVELOPE_DISABLE),
						wave.writing ? REG9_WRITE_MODE : 0,
						wave.length & 0xFF,
						wave.length >> 8,
						envelopes.length,
						envelopes.counter - 1
					};

					for (uint i=0; i < NST_COUNT(volumes); ++i)
					{
						if (volume == volumes[i])
						{
							data[1] |= i;
							break;
						}
					}

					state.Begin('R','E','G','\0').Write( data ).End();
				}
				{
					state.Begin('W','A','V','\0').Compress( wave.table ).End();
				}
			}			
			state.End();

			envelopes.units[VOLUME].SaveState( State::Saver::Subset(state,'V','O','L','\0').Ref() );
			envelopes.units[SWEEP].SaveState( State::Saver::Subset(state,'S','W','P','\0').Ref() );

			state.Begin('M','O','D','\0');
			{
				{
					const u8 data[4] = 
					{
						modulator.length & 0xFF,
						(modulator.length >> 8) | (modulator.writing ? REG7_MOD_WRITE_MODE : 0),
						modulator.sweep,
						modulator.pos
					};

					state.Begin('R','E','G','\0').Write( data ).End();
				}

				{
					u8 data[Modulator::SIZE];

					for (uint i=0; i < Modulator::SIZE; ++i)
					{
						for (uint j=0; j < NST_COUNT(Modulator::steps); ++j)
						{
							if (modulator.table[i] == Modulator::steps[j])
							{
								data[i] = j;
								break;
							}
						}
					}

					state.Begin('R','A','M','\0').Compress( data ).End();
				}
			}
			state.End();
		}
	
		void Fds::Sound::LoadState(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case NES_STATE_CHUNK_ID('M','A','S','\0'):
					{
						state.DigIn();

						while (const dword subchunk = state.Begin())
						{
							switch (subchunk)
							{
								case NES_STATE_CHUNK_ID('R','E','G','\0'):
								{
									const State::Loader::Data<6> data( state );
					
									status = 
									(
										((data[0] & REG3_OUTPUT_DISABLE) ? 0 : STATUS_OUTPUT_ENABLED) |
										((data[0] & REG3_ENVELOPE_DISABLE) ? 0 : STATUS_ENVELOPES_ENABLED)
									);
					
									volume = volumes[data[1] & REG9_VOLUME];
									wave.writing = data[1] & REG9_WRITE_MODE;
									wave.length = data[2] | ((data[3] & REG3_WAVELENGTH_HIGH) << 8);
									envelopes.length = data[4];
									envelopes.counter = data[5] + 1;
									break;
								}
					
								case NES_STATE_CHUNK_ID('W','A','V','\0'):
								
									state.Uncompress( wave.table );
					
									for (uint i=0; i < Wave::SIZE; ++i)
										wave.table[i] &= 0x3F;
					
									break;
							}

							state.End();
						}

						state.DigOut();
						break;
					}

					case NES_STATE_CHUNK_ID('V','O','L','\0'):

						envelopes.units[VOLUME].LoadState( state );
						break;

					case NES_STATE_CHUNK_ID('S','W','P','\0'):

						envelopes.units[SWEEP].LoadState( state );
						break;

					case NES_STATE_CHUNK_ID('M','O','D','\0'):
					{
						state.DigIn();

						while (const dword subchunk = state.Begin())
						{
							switch (subchunk)
							{
								case NES_STATE_CHUNK_ID('R','E','G','\0'):
								{
									State::Loader::Data<4> data( state );

									modulator.length = data[0] | ((data[1] & REG7_MOD_WAVELENGTH_HIGH) << 8);
									modulator.writing = data[1] &  REG7_MOD_WRITE_MODE;
									modulator.sweep = data[2] & (REG5_MOD_SWEEP|REG5_MOD_NEGATE);
									modulator.pos = data[3] & 0x3F;

									break;
								}

								case NES_STATE_CHUNK_ID('R','A','M','\0'):
								{
									u8 data[Modulator::SIZE];
									state.Uncompress( data );

									for (uint i=0; i < Modulator::SIZE; ++i)
										modulator.table[i] = Modulator::steps[data[i] & REG8_MOD_DATA];

									break;
								}
							}

							state.End();
						}

						state.DigOut();
						break;
					}
				}

				state.End();
			}

			amp = 0;
			wave.pos = 0;
			wave.volume = envelopes.units[VOLUME].Output();
			modulator.timer = 0;
			modulator.active = CanModulate();
			active = CanOutput();
		}
	
		void Fds::Sound::Envelope::SaveState(State::Saver& state) const
		{
			const u8 data[3] =
			{
				ctrl,
				counter - 1,
				gain
			};

			state.Write( data );
		}

		void Fds::Sound::Envelope::LoadState(State::Loader& state)
		{
			const State::Loader::Data<3> data( state );

			ctrl = data[0];
			counter = (data[1] & CTRL_COUNT) + 1;
			gain = data[2] & CTRL_COUNT;
		}

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
        NES_PEEK(Fds,Nop)
		{ 
			return OPEN_BUS;
		}

		NES_PEEK(Fds::IrqClock,Nop)
		{ 
			return OPEN_BUS;
		}

		NES_POKE(Fds,Nop)  
		{
		}

		NES_POKE(Fds::Bios::Instance,Nop)
		{
		}

		NES_POKE(Fds::IrqClock,Nop)  
		{
		}

		NES_PEEK(Fds::Ram,Ram) 
		{ 
			return mem[address - 0x6000U]; 
		}

		NES_POKE(Fds::Ram,Ram) 
		{ 
			mem[address - 0x6000U] = data; 
		}

		NES_PEEK(Fds::Bios::Instance,Rom)
		{
			return rom[address - 0xE000U];
		}

		NES_POKE(Fds::IrqClock,4020) 
		{ 
			Update();
			unit.latch = (unit.latch & 0xFF00U) | (data << 0);
		}
	
		NES_POKE(Fds::IrqClock,4021) 
		{ 
			Update();
			unit.latch = (unit.latch & 0x00FFU) | (data << 8);
		}
	
		void Fds::IrqClock::Clear(const Irq::Flag flag)
		{
			Update();
			unit.status &= ~u8(flag);

			if (!unit.status)
				ClearIRQ();
		}

		NES_POKE(Fds::IrqClock,4022) 
		{ 
			Clear( Irq::PENDING_CTRL );
			unit.ctrl = data;
			unit.count = unit.latch;
		}
	
		NES_POKE(Fds,4023) 
		{
			regs.ctrl0 = data;
		}
	
		NES_POKE(Fds,4024) 
		{ 
			NST_VERIFY( regs.ctrl0 & Regs::CTRL0_DISK_ENABLED );

			if (!(regs.ctrl1 & Regs::CTRL1_READ_MODE) && io.pos < 65000U && disks.data)
			{
				if (io.skip)
				{
					--io.skip;
				}
				else if (io.pos >= 2)
				{
					disks.data[io.pos-2] = data;
				}
			}
		}
	
		NES_POKE(Fds,4025) 
		{
			irq.Clear( Irq::PENDING_DRIVE );
	
			irq.unit.drive.notify = data & Regs::CTRL1_DISK_IRQ_ENABLED;

			if (disks.data)
			{
				if (!(data & Regs::CTRL1_READ_MODE))
					io.skip = 2;

				if (data & Regs::CTRL1_TRANSFER_RESET)
				{
					io.pos = 0;
					irq.unit.drive.count = Irq::Drive::SLOW;
				}
				else if (data & Regs::CTRL1_DRIVE_READY)
				{
					irq.unit.drive.count = Irq::Drive::SLOW;
				}
				else if (((regs.ctrl1 & Regs::CTRL1_DRIVE_READY) | (data & Regs::CTRL1_CRC)) == Regs::CTRL1_DRIVE_READY)
				{
					io.pos = (io.pos > 2) ? io.pos - 2 : 0;
					irq.unit.drive.count = Irq::Drive::SLOW;
				}
			}
	
			regs.ctrl1 = data;
			ppu.SetMirroring( (data & Regs::CTRL1_MIRRORING_HORIZONTAL) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
		}

		NES_POKE(Fds,4026)
		{
			io.port = data;
		}
		
		NES_PEEK(Fds::IrqClock,4030) 
		{ 
			Update();

			const uint status = unit.status;
			unit.status = 0;

			ClearIRQ();

			return status;
		}

		NES_PEEK(Fds,4031) 
		{
			NST_VERIFY( regs.ctrl0 & Regs::CTRL0_DISK_ENABLED );

			irq.Clear( Irq::PENDING_DRIVE );

			if (disks.data)
			{
				regs.data = disks.data[io.pos];
	
				if (regs.ctrl1 & Regs::CTRL1_MOTOR)
				{
					io.pos = NST_MIN(io.pos+1,65000U);
					irq.unit.drive.count = Irq::Drive::FAST;
				}
			}
	
			return regs.data;
		}
	
		NES_PEEK(Fds,4032) 
		{ 
			if (disks.data == NULL)
			{
				return Regs::STATUS_LATCH|Regs::STATUS_DRIVE_NOT_READY|Regs::STATUS_DISK_EJECTED|Regs::STATUS_DISK_PROTECTED;
			}
			else if ((regs.ctrl1 & (Regs::CTRL1_MOTOR|Regs::CTRL1_TRANSFER_RESET)) != Regs::CTRL1_MOTOR)				
			{
				return Regs::STATUS_LATCH|Regs::STATUS_DRIVE_NOT_READY;
			}
			else
			{
				return Regs::STATUS_LATCH;
			}
		}
	
		NES_PEEK(Fds,4033) 
		{ 
			NST_VERIFY( io.port & Io::BATTERY_CHARGED );
			return io.port & Io::BATTERY_CHARGED;
		}
	
		NES_POKE(Fds::Sound,Nop) 
		{
		}
	
		NES_PEEK(Fds::Sound,Nop) 
		{ 
			return OPEN_BUS; 
		}
	
		NES_PEEK(Fds::Sound,4040) 
		{
			return wave.table[address & 0x3F] | OPEN_BUS; 
		}
	
		NES_POKE(Fds::Sound,4040) 
		{ 
			NST_VERIFY( wave.writing );

			if (wave.writing)
			{
				cpu.GetApu().Update(); 
				wave.table[address & 0x3F] = data & 0x3F;
			}
		}
	
		void Fds::Sound::Envelope::Write(const uint data)
		{
			ctrl = data;
			counter = (data & CTRL_COUNT) + 1;

			if (data & CTRL_DISABLE)
				gain = data & CTRL_COUNT;
		}

		NES_POKE(Fds::Sound,4080) 
		{ 
			cpu.GetApu().Update(); 
			envelopes.units[VOLUME].Write( data );
	
			if ((data & Envelope::CTRL_DISABLE) && !wave.pos)
				wave.volume = envelopes.units[VOLUME].Output();
		}
	
		bool Fds::Sound::CanOutput() const
		{
			return (status & STATUS_OUTPUT_ENABLED) && wave.length && !wave.writing && outputVolume;
		}

		NES_POKE(Fds::Sound,4082) 
		{ 
			cpu.GetApu().Update(); 
	
			wave.length &= uint(REG3_WAVELENGTH_HIGH) << 8;
			wave.length |= data;
	
			active = CanOutput();
		}
	
		NES_POKE(Fds::Sound,4083) 
		{ 
			cpu.GetApu().Update(); 
	
			wave.length &= REG2_WAVELENGTH_LOW;
			wave.length |= (data & REG3_WAVELENGTH_HIGH) << 8;
	
			status = ~data & (REG3_OUTPUT_DISABLE|REG3_ENVELOPE_DISABLE);
	
			if (data & REG3_OUTPUT_DISABLE) 
			{
				wave.pos = 0;
				wave.volume = envelopes.units[VOLUME].Output();
			}
	
			active = CanOutput();
		}
	
		NES_POKE(Fds::Sound,4084) 
		{ 
			cpu.GetApu().Update(); 
			envelopes.units[SWEEP].Write( data ); 
		}
	
		NES_POKE(Fds::Sound,4085) 
		{ 
			cpu.GetApu().Update(); 
	
			modulator.sweep = data & (REG5_MOD_SWEEP|REG5_MOD_NEGATE);
			modulator.pos = 0x00;
		}
	
		NES_POKE(Fds::Sound,4086) 
		{ 
			cpu.GetApu().Update(); 
	
			modulator.length &= uint(REG7_MOD_WAVELENGTH_HIGH) << 8;
			modulator.length |= data;
	
			modulator.active = CanModulate();
		}
	
		NES_POKE(Fds::Sound,4087) 
		{ 
			cpu.GetApu().Update(); 
	
			modulator.length &= REG6_MOD_WAVELENGTH_LOW;
			modulator.length |= (data & REG7_MOD_WAVELENGTH_HIGH) << 8;
			modulator.writing = data & REG7_MOD_WRITE_MODE;
	
			modulator.active = CanModulate();
		}
	
		NES_POKE(Fds::Sound,4088) 
		{ 
			NST_VERIFY( modulator.writing );

			if (modulator.writing)
			{
				cpu.GetApu().Update(); 
				std::memmove( modulator.table, modulator.table + 1, Modulator::SIZE-1 );
				modulator.table[Modulator::SIZE-1] = Modulator::steps[data & REG8_MOD_DATA];
			}
		}
	
		NES_POKE(Fds::Sound,4089) 
		{ 
			cpu.GetApu().Update(); 
	
			volume = volumes[data & REG9_VOLUME];
			wave.writing = data & REG9_WRITE_MODE;
	
			active = CanOutput();
		}
	
		NES_POKE(Fds::Sound,408A) 
		{ 
			cpu.GetApu().Update(); 
	
			envelopes.length = data;
		}
	
		NES_PEEK(Fds::Sound,4090) 
		{ 
			return envelopes.units[VOLUME].Gain() | OPEN_BUS; 
		}
	
		NES_PEEK(Fds::Sound,4092) 
		{ 
			return envelopes.units[SWEEP].Gain() | OPEN_BUS; 
		}

		NST_FORCE_INLINE void Fds::Sound::Envelope::Clock()
		{
			if (!(ctrl & CTRL_DISABLE) && !--counter)
			{
				counter = (ctrl & CTRL_COUNT) + 1;

				if (ctrl & CTRL_UP) gain += (gain < GAIN_MAX);
				else                gain -= (gain > GAIN_MIN);
			}
		}

		Cycle Fds::Sound::Clock()
		{
			if (!--envelopes.counter)
			{
				envelopes.counter = envelopes.length + 1;
	
				if (envelopes.length && (status & STATUS_ENVELOPES_ENABLED))
				{
					for (uint i=0; i < 2; ++i)
						envelopes.units[i].Clock();
				}
			}
	
			return Envelopes::PULSE * fixed;
		}
	
		NST_FORCE_INLINE dword Fds::Sound::GetModulation() const
		{
			if (modulator.active)
			{
				if (dword pos = envelopes.units[SWEEP].Gain())
				{
					pos = (pos * (((modulator.sweep & REG5_MOD_SWEEP) - (modulator.sweep & REG5_MOD_NEGATE)))) & 0xFFF;

					if (modulator.sweep & REG5_MOD_NEGATE)
					{
						pos >>= 4;

						if (pos >= 0xC0)
							pos = (pos & 0x7F) - (pos & 0x80);
					}
					else
					{
						pos = (pos >> 4) + ((pos & 0xF) ? 2 : 0);

						if (pos >= 0xC2)
						{
							pos -= 0x102;
							pos = (pos & 0x7F) - (pos & 0x80);
						}
					}

					pos *= wave.length;

					if (pos & (1UL << Modulator::TIMER_CARRY))
						pos = wave.length - (~(pos-1) >> 6);
					else
						pos = wave.length + (pos >> 6);

					return pos;
				}
			}

			return wave.length;
		}
	
		Fds::Sound::Sample Fds::Sound::GetSample()
		{
			NST_ASSERT( bool(modulator.active) == CanModulate() && bool(active) == CanOutput() );
	
			if (modulator.active)
			{
				for (modulator.timer -= modulator.length * rate; modulator.timer & (1UL << Modulator::TIMER_CARRY); modulator.timer += fixed << 16)
				{
					const uint value = modulator.pos >> 1;
					modulator.pos = (modulator.pos + 1) & 0x3F;
					modulator.sweep = (modulator.table[value] != 0x80) ? (modulator.sweep + modulator.table[value]) & 0x7F : 0x00;
				}
			}

			dword sample = 0;
	
			if (active)
			{
				static const dword clocks[2][2] =
				{			
					{ Cpu::MC_NTSC, Cpu::MC_DIV_NTSC * Cpu::CLK_NTSC_DIV * 0x10000UL },
					{ Cpu::MC_PAL,  Cpu::MC_DIV_PAL  * Cpu::CLK_PAL_DIV  * 0x10000UL }
				};

				const dword pos = wave.pos;	
				wave.pos = (wave.pos + dword(qword(GetModulation()) * clocks[mode][0] / clocks[mode][1]) + Wave::SIZE * cpu.GetApu().GetSampleRate()) % (Wave::SIZE * cpu.GetApu().GetSampleRate());
	
				if (wave.pos < pos)
					wave.volume = envelopes.units[VOLUME].Output();
	
				sample = wave.volume * volume * wave.table[(wave.pos / cpu.GetApu().GetSampleRate()) & 0x3F] / 30;
			} 
	
			amp = (amp * 2 + sample) / 3;

			return dcBlocker.Apply( amp * outputVolume / DEFAULT_VOLUME );
		}

		ibool Fds::Irq::Signal()
		{
			if ((ctrl & CTRL_ENABLED) && count && !--count)
			{
				if (ctrl & CTRL_REPEAT)
					count = latch;
				else
					ctrl &= CTRL_ENABLED^0xFF;

				status |= PENDING_CTRL;
			}

			if (drive.count && !--drive.count && drive.notify)
				status |= PENDING_DRIVE;

			return status;
		}

		void Fds::VSync()
		{
			irq.VSync();

			if (!disks.mounting)
			{
				const ibool led = (irq.unit.drive.count > 0);

				if (io.led != led)
				{
					io.led = led;
					Api::Fds::diskAccessLampCallback( led );
				}
			}
			else if (!--disks.mounting)
			{
				disks.data = disks.sides.data[disks.current];
			}
		}
	}
}
