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
#include "api/NstApiUser.hpp"
#include "api/NstApiInput.hpp"

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

			Log log;

			for (uint i=0, n=sides.count; i < n; ++i)
			{
				Api::Fds::DiskData data;

				if (NES_SUCCEEDED(Unit::Drive::Analyze( sides.data[i], data )))
				{
					dword size = 0;

					for (Api::Fds::DiskData::Files::const_iterator it(data.files.begin()), end(data.files.end()); it != end; ++it)
						size += it->data.size();

					log << "Fds: Disk "
						<< (1+i/2)
						<< (i % 2 ? " Side B: " : " Side A: ")
						<< (size / 1024)
						<< "k in "
						<< data.files.size()
						<< " files";

					if (const uint size = data.raw.size())
						log << ", " << size << "b trailing data";

					log << ".." NST_LINEBREAK;

					for (Api::Fds::DiskData::Files::const_iterator it(data.files.begin()), end(data.files.end()); it != end; ++it)
					{
						size += it->data.size();

						log << "Fds: file: \"" << it->name
							<< "\", id: "      << it->id
							<< ", size: "      << it->data.size()
							<< ", index: "     << it->index
							<< ", address: "   << Log::Hex( (u16) it->address )
							<< ", type: "
							<<
							(
								it->type == Api::Fds::DiskData::File::TYPE_PRG ? "PRG" :
								it->type == Api::Fds::DiskData::File::TYPE_CHR ? "CHR" :
								it->type == Api::Fds::DiskData::File::TYPE_NMT ? "NMT" :
                                                                                 "unknown"
							)
							<< NST_LINEBREAK;
					}
				}
			}

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

		Fds::Disks::Disks(StdStream stream)
		:
		current        (EJECTED),
		mounting       (0),
		writeProtected (false),
		sides          (Create(stream))
		{
		}

		Fds::Disks::~Disks()
		{
			delete [] sides.data;
			delete [] sides.header;
		}

		Fds::Unit::Timer::Timer()
		{
			Reset();
		}

		Fds::Unit::Drive::Drive()
		: dirty(false)
		{
			Reset();
		}

		Fds::Unit::Unit()
		{
			status = 0;
		}

		Fds::Adapter::Adapter(Cpu& c)
		: Clock::M2<Unit>(c) {}

		Fds::Io::Io()
		{
			Reset();
		}

		Fds::Fds(Context& context)
		:
		Image   (DISK),
		disks   (context.stream),
		adapter (context.cpu),
		cpu     (context.cpu),
		ppu     (context.ppu),
		sound   (context.cpu)
		{
			if (!Bios::IsLoaded())
				throw RESULT_ERR_MISSING_BIOS;

			ppu.GetChrMem().Source().Set( SIZE_8K, true, true );
		}

		Fds::~Fds()
		{
			EjectDisk();

			if (!disks.writeProtected)
				disks.sides.Save();
		}

		Result Fds::Flush(bool,bool) const
		{
			if (io.led != Api::Fds::MOTOR_OFF)
			{
				io.led = Api::Fds::MOTOR_OFF;
				Api::Fds::diskAccessLampCallback( Api::Fds::MOTOR_OFF );
			}

			return RESULT_OK;
		}

		uint Fds::GetDesiredController(const uint port) const
		{
			if (port == Api::Input::EXPANSION_PORT)
				return (disks.sides.id == 0xA4445245UL) ? Api::Input::DOREMIKKOKEYBOARD : Api::Input::UNCONNECTED;
			else
				return Image::GetDesiredController( port );
		}

		uint Fds::GetDesiredAdapter() const
		{
			return Api::Input::ADAPTER_FAMICOM;
		}

		void Fds::Io::Reset()
		{
			ctrl = 0;
			port = 0;
		}

		void Fds::Ram::Reset()
		{
			std::memset( mem, 0x00, sizeof(mem) );
		}

		void Fds::Unit::Timer::Reset()
		{
			ctrl = 0;
			count = 0;
			latch = 0;
		}

		void Fds::Unit::Drive::Reset()
		{
			count = 0;
			headPos = 0;
			dataPos = 0;
			gap = 0;
			io = NULL;
			ctrl = 0;
			length = 0;
			in = 0;
			out = 0;
			status = STATUS_EJECTED|STATUS_UNREADY|STATUS_PROTECTED|OPEN_BUS;
		}

		void Fds::Unit::Reset(bool)
		{
			timer.Reset();
			drive.Reset();

			status = 0;
		}

		inline void Fds::Adapter::Mount(u8* io,ibool protect)
		{
			unit.drive.Mount( io, protect );
		}

		inline ibool Fds::Adapter::Dirty() const
		{
			ibool dirty = unit.drive.dirty;
			unit.drive.dirty = false;
			return dirty;
		}

		inline uint Fds::Adapter::Activity() const
		{
			return unit.drive.count ? (unit.drive.ctrl & Unit::Drive::CTRL_READ_MODE) ? Api::Fds::MOTOR_READ : Api::Fds::MOTOR_WRITE : Api::Fds::MOTOR_OFF;
		}

		inline void Fds::Adapter::WriteProtect()
		{
			unit.drive.status |= Unit::Drive::STATUS_PROTECTED;
		}

		void Fds::Adapter::Reset(u8* const io,ibool protect)
		{
			Clock::M2<Unit>::Reset( true, true );

			unit.drive.Mount( io, protect );

			cpu.Map( 0x4020U ).Set( this, &Adapter::Peek_Nop,  &Adapter::Poke_4020 );
			cpu.Map( 0x4021U ).Set( this, &Adapter::Peek_Nop,  &Adapter::Poke_4021 );
			cpu.Map( 0x4022U ).Set( this, &Adapter::Peek_Nop,  &Adapter::Poke_4022 );
			cpu.Map( 0x4024U ).Set( this, &Adapter::Peek_Nop,  &Adapter::Poke_4024 );
			cpu.Map( 0x4030U ).Set( this, &Adapter::Peek_4030, &Adapter::Poke_Nop  );
			cpu.Map( 0x4032U ).Set( this, &Adapter::Peek_4032, &Adapter::Poke_Nop  );
		}

		void Fds::Reset(const bool hard)
		{
			disks.mounting = 0;

			adapter.Reset
			(
				disks.current == Disks::EJECTED ? NULL : disks.sides.data[disks.current],
				disks.writeProtected
			);

			if (hard)
			{
				ram.Reset();
				ppu.GetChrMem().Source().Fill( 0x00 );
				ppu.GetChrMem().SwapBank<SIZE_8K,0x0000U>( 0 );
			}

			cpu.ClearIRQ();

			cpu.Map( 0x4023U ).Set( this, &Fds::Peek_Nop,  &Fds::Poke_4023 );
			cpu.Map( 0x4025U ).Set( this, &Fds::Peek_Nop,  &Fds::Poke_4025 );
			cpu.Map( 0x4026U ).Set( this, &Fds::Peek_Nop,  &Fds::Poke_4026 );
			cpu.Map( 0x4031U ).Set( this, &Fds::Peek_4031, &Fds::Poke_Nop  );
			cpu.Map( 0x4033U ).Set( this, &Fds::Peek_4033, &Fds::Poke_Nop  );

			cpu.Map( 0x6000U, 0xDFFFU ).Set( &ram, &Fds::Ram::Peek_Ram, &Fds::Ram::Poke_Ram );
			cpu.Map( 0xE000U, 0xFFFFU ).Set( &Bios::instance, &Bios::Instance::Peek_Rom, &Bios::Instance::Poke_Nop );

			Log() << "Fds: reset" NST_LINEBREAK;
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

			if (side < 2)
			{
				disk = (disk * 2) + side;

				if (disk < disks.sides.count)
				{
					if (disks.current != disk)
					{
						const uint prev = disks.current;

						disks.current = disk;
						disks.mounting = Disks::MOUNTING;

						adapter.Mount( NULL );

						if (prev != Disks::EJECTED)
							Api::Fds::diskChangeCallback( Api::Fds::DISK_EJECT, prev / 2, prev % 2 );

						Api::Fds::diskChangeCallback( Api::Fds::DISK_INSERT, disk / 2, disk % 2 );

						return RESULT_OK;
					}

					return RESULT_NOP;
				}
			}

			return RESULT_ERR_INVALID_PARAM;
		}

		Result Fds::EjectDisk()
		{
			if (disks.current != Disks::EJECTED)
			{
				const uint prev = disks.current;

				disks.current = Disks::EJECTED;
				disks.mounting = 0;

				adapter.Mount( NULL );

				Api::Fds::diskChangeCallback( Api::Fds::DISK_EJECT, prev / 2, prev % 2 );

				return RESULT_OK;
			}

			return RESULT_NOP;
		}

		void Fds::Unit::Drive::Mount(u8* data,ibool protect)
		{
			io = data;

			if (data)
			{
				status &= ~uint(STATUS_EJECTED|STATUS_PROTECTED);

				if (protect)
					status |= STATUS_PROTECTED;
			}
			else
			{
				count = 0;
				status |= uint(STATUS_EJECTED|STATUS_PROTECTED|STATUS_UNREADY);
			}
		}

		Result Fds::Unit::Drive::Analyze(const u8* NST_RESTRICT src,Api::Fds::DiskData& dst)
		{
			try
			{
				iword i = SIDE_SIZE;

				for (uint block=~0U, files=0; i; )
				{
					const iword prev = block;
					block = src[0];

					if (block == BLOCK_VOLUME)
					{
						i -= LENGTH_VOLUME+1;

						if (i < 0 || prev != ~0U)
							break;

						src += LENGTH_VOLUME+1;
					}
					else if (block == BLOCK_COUNT)
					{
						i -= LENGTH_COUNT+1;

						if (i < 0 || prev != BLOCK_VOLUME)
							break;

						files = src[1];
						src += LENGTH_COUNT+1;
					}
					else if (block == BLOCK_HEADER)
					{
						i -= LENGTH_HEADER+1;

						if (i < 0 || (prev != BLOCK_DATA && prev != BLOCK_COUNT) || !files)
							break;

						dst.files.push_back( Api::Fds::DiskData::File() );
						Api::Fds::DiskData::File& file = dst.files.back();

						file.index = src[1];
						file.id = src[2];

						std::memcpy( file.name, src+3, 8 );
						file.name[9] = '\0';

						for (uint j=8; j--; )
						{
							if (file.name[j] == ' ')
								file.name[j] = '\0';
						}

						file.address = src[11] | src[12] << 8;

						switch (src[15])
						{
							case 0:  file.type = Api::Fds::DiskData::File::TYPE_PRG;     break;
							case 1:  file.type = Api::Fds::DiskData::File::TYPE_CHR;     break;
							case 2:  file.type = Api::Fds::DiskData::File::TYPE_NMT;     break;
							default: file.type = Api::Fds::DiskData::File::TYPE_UNKNOWN; break;
						}

						file.data.resize( src[13] | src[14] << 8 );

						if (const uint size = file.data.size())
							std::memset( &file.data.front(), 0x00, size );

						src += LENGTH_HEADER+1;
					}
					else if (block == BLOCK_DATA)
					{
						if (prev != BLOCK_HEADER)
							break;

						Api::Fds::DiskData::Data& data = dst.files.back().data;
						const iword size = data.size();

						i -= size+1;

						if (i < 0)
							break;

						++src;

						if (size)
						{
							std::memcpy( &data.front(), src, size );
							src += size;
						}

						NST_ASSERT( files );

						if (!--files)
							break;
					}
					else
					{
						break;
					}
				}

				for (iword j=i; j-- > 0; )
				{
					if (src[j])
					{
						dst.raw.assign( src, src+j+1 );
						break;
					}
				}

				return i >= 0 ? RESULT_OK : RESULT_WARN_BAD_DUMP;
			}
			catch (std::bad_alloc&)
			{
				return RESULT_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				return RESULT_ERR_GENERIC;
			}
		}

		Result Fds::GetDiskData(uint side,Api::Fds::DiskData& data)
		{
			if (side < disks.sides.count)
				return Unit::Drive::Analyze( disks.sides.data[side], data );

			return RESULT_ERR_INVALID_PARAM;
		}

		inline bool Fds::Sound::CanModulate() const
		{
			return modulator.length && !modulator.writing;
		}

		void Fds::LoadState(State::Loader& state)
		{
			uint saveDisks[3] = {~0U,~0U,~0U};

			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case NES_STATE_CHUNK_ID('I','O','\0','\0'):
					{
						const State::Loader::Data<4> data( state );

						io.ctrl = data[0];
						io.port = data[1];
						break;
					}

					case NES_STATE_CHUNK_ID('R','A','M','\0'):

						state.Uncompress( ram.mem );
						break;

					case NES_STATE_CHUNK_ID('C','H','R','\0'):

						state.Uncompress( ppu.GetChrMem().Source().Mem(), SIZE_8K );
						break;

					case NES_STATE_CHUNK_ID('I','R','Q','\0'):
					case NES_STATE_CHUNK_ID('D','R','V','\0'):

						adapter.LoadState( state, chunk );
						break;

					case NES_STATE_CHUNK_ID('D','S','K','\0'):
					{
						const State::Loader::Data<4> data( state );

						if (data[0] != disks.sides.count)
							throw RESULT_ERR_INVALID_FILE;

						saveDisks[0] = data[1];
						saveDisks[1] = data[2];
						saveDisks[2] = data[3];
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
								state.Uncompress( data, SIDE_SIZE );

								for (uint j=0; j < SIDE_SIZE; ++j)
									data[j] = ~data[j];

								break;
							}
						}
						break;
				}

				state.End();
			}

			disks.mounting = 0;

			if (saveDisks[0] != ~0U)
			{
				disks.writeProtected = saveDisks[0] & 0x2;

				if (saveDisks[0] & 0x1)
				{
					if (NES_FAILED(InsertDisk( saveDisks[1] / 2, saveDisks[1] % 2 )))
						throw RESULT_ERR_CORRUPT_FILE;

					disks.mounting = saveDisks[2];
				}
				else
				{
					EjectDisk();
				}
			}

			adapter.Mount
			(
				disks.current != Disks::EJECTED && !disks.mounting ? disks.sides.data[disks.current] : NULL,
				disks.writeProtected
			);
		}

		void Fds::SaveState(State::Saver& state) const
		{
			{
				const u8 data[4] =
				{
					io.ctrl,
					io.port,
					0,
					0
				};

				state.Begin('I','O','\0','\0').Write( data ).End();
			}

			adapter.SaveState( state );

			state.Begin('R','A','M','\0').Compress( ram.mem ).End();
			state.Begin('C','H','R','\0').Compress( ppu.GetChrMem().Source().Mem(), SIZE_8K ).End();

			{
				const u8 data[4] =
				{
					disks.sides.count,
					(disks.current != Disks::EJECTED) | (disks.writeProtected ? 0x2 : 0x0),
					disks.current != Disks::EJECTED ? disks.current : 0xFF,
					disks.current != Disks::EJECTED ? disks.mounting : 0
				};

				state.Begin('D','S','K','\0').Write( data ).End();
			}

			if (adapter.Dirty() || !state.Internal())
			{
				struct Dst
				{
					u8* const NST_RESTRICT mem;

					Dst() : mem(new u8 [SIDE_SIZE]) {}
					~Dst() { delete [] mem; }
				};

				Dst dst;

				for (uint i=0; i < disks.sides.count; ++i)
				{
					const u8* const NST_RESTRICT src = disks.sides.data[i];

					for (uint j=0; j < SIDE_SIZE; ++j)
						dst.mem[j] = ~src[j];

					state.Begin('D','0' + (i / 2),'A' + (i % 2),'\0').Compress( dst.mem, SIDE_SIZE ).End();
				}
			}

			sound.SaveState( State::Saver::Subset(state,'S','N','D','\0').Ref() );
		}

		void Fds::Adapter::SaveState(State::Saver& state) const
		{
			{
				const u8 data[7] =
				{
					unit.timer.ctrl,
					unit.status,
					unit.timer.latch & 0xFF,
					unit.timer.latch >> 8,
					unit.timer.count & 0xFF,
					unit.timer.count >> 8,
					0
				};

				state.Begin('I','R','Q','\0').Write( data ).End();
			}

			{
				const uint headPos = NST_MIN(unit.drive.headPos,SIDE_SIZE);

				const u8 data[16] =
				{
					unit.drive.ctrl,
					unit.drive.status,
					unit.drive.in & 0xFF,
					unit.drive.out,
					unit.drive.count ? headPos & 0xFF            : 0,
					unit.drive.count ? headPos >> 8              : 0,
					unit.drive.count ? unit.drive.dataPos & 0xFF : 0,
					unit.drive.count ? unit.drive.dataPos >> 8   : 0,
					unit.drive.count ? unit.drive.gap & 0xFF     : 0,
					unit.drive.count ? unit.drive.gap >> 8       : 0,
					unit.drive.count ? unit.drive.length & 0xFF  : 0,
					unit.drive.count ? unit.drive.length >> 8    : 0,
					unit.drive.count >> 0 & 0xFF,
					unit.drive.count >> 8 & 0xFF,
					unit.drive.count >> 16,
					unit.drive.in >> 8
				};

				state.Begin('D','R','V','\0').Write( data ).End();
			}
		}

		void Fds::Adapter::LoadState(State::Loader& state,const dword chunk)
		{
			switch (chunk)
			{
				case NES_STATE_CHUNK_ID('I','R','Q','\0'):
				{
					const State::Loader::Data<7> data( state );

					unit.timer.ctrl = data[0];
					unit.status = data[1] & (Unit::STATUS_PENDING_IRQ|Unit::STATUS_TRANSFERED);
					unit.timer.latch = data[2] | data[3] << 8;
					unit.timer.count = data[4] | data[5] << 8;

					break;
				}

				case NES_STATE_CHUNK_ID('D','R','V','\0'):
				{
					const State::Loader::Data<16> data( state );

					unit.drive.ctrl = data[0];
					unit.drive.status = (data[1] & (Unit::Drive::STATUS_EJECTED|Unit::Drive::STATUS_UNREADY|Unit::Drive::STATUS_PROTECTED)) | OPEN_BUS;
					unit.drive.in = data[2] | (data[15] << 8 & 0x100);
					unit.drive.out = data[3];
					unit.drive.headPos = data[4] | data[5] << 8;
					unit.drive.dataPos = data[6] | data[7] << 8;
					unit.drive.gap = data[8] | data[9] << 8;
					unit.drive.length = data[10] | data[11] << 8;
					unit.drive.count = data[12] | data[13] << 8 | data[14] << 16;

					if (unit.drive.dataPos > SIDE_SIZE)
						unit.drive.dataPos = SIDE_SIZE;

					if (unit.drive.headPos < unit.drive.dataPos)
						unit.drive.headPos = unit.drive.dataPos;

					break;
				}
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

		NES_POKE(Fds,Nop)
		{
		}

		NES_PEEK(Fds::Adapter,Nop)
		{
			return OPEN_BUS;
		}

		NES_POKE(Fds::Adapter,Nop)
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

		NES_POKE(Fds::Bios::Instance,Nop)
		{
		}

		NES_POKE(Fds::Adapter,4020)
		{
			Update();
			unit.timer.latch = (unit.timer.latch & 0xFF00U) | (data << 0);
		}

		NES_POKE(Fds::Adapter,4021)
		{
			Update();
			unit.timer.latch = (unit.timer.latch & 0x00FFU) | (data << 8);
		}

		NES_POKE(Fds::Adapter,4022)
		{
			Update();

			unit.timer.ctrl = data;
			unit.timer.count = unit.timer.latch;
			unit.status &= Unit::STATUS_TRANSFERED;

			if (!unit.status)
				ClearIRQ();
		}

		NES_POKE(Fds,4023)
		{
			io.ctrl = data;
		}

		NES_POKE(Fds::Adapter,4024)
		{
			Update();

			unit.drive.out = data;
			unit.status &= Unit::STATUS_PENDING_IRQ;

			if (!unit.status)
				ClearIRQ();
		}

		NST_FORCE_INLINE void Fds::Unit::Drive::Write(uint reg)
		{
			ctrl = reg;

			if (!(reg & CTRL_ON))
			{
				count = 0;
				status |= STATUS_UNREADY;
			}
			else if (!(count | (reg & CTRL_STOP)) && io)
			{
				count = CLK_MOTOR;
				headPos = 0;
			}
		}

		NST_FORCE_INLINE void Fds::Adapter::Write(uint reg)
		{
			Update();

			unit.status &= (reg >> 6 & Unit::STATUS_TRANSFERED) | Unit::STATUS_PENDING_IRQ;

			if (!unit.status)
				cpu.ClearIRQ();

			unit.drive.Write( reg );
		}

		NES_POKE(Fds,4025)
		{
			adapter.Write( data );
			ppu.SetMirroring( (data & CTRL1_NMT_HORIZONTAL) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
		}

		NES_POKE(Fds,4026)
		{
			io.port = data;
		}

		NES_PEEK(Fds::Adapter,4030)
		{
			Update();

			const uint status = unit.status;
			unit.status = 0;

			ClearIRQ();

			return status;
		}

		NST_FORCE_INLINE uint Fds::Adapter::Read()
		{
			Update();

			unit.status &= Unit::STATUS_PENDING_IRQ;

			if (!unit.status)
				ClearIRQ();

			return unit.drive.in;
		}

		NES_PEEK(Fds,4031)
		{
			const uint data = adapter.Read();

			if (data <= 0xFF)
				return data;

			if (!disks.writeProtected)
			{
				disks.writeProtected = true;
				adapter.WriteProtect();
				Api::User::eventCallback( Api::User::EVENT_NONSTANDARD_DISK );
			}

			return data & 0xFF;
		}

		NES_PEEK(Fds::Adapter,4032)
		{
			Update();

			NST_ASSERT( unit.drive.status & OPEN_BUS );
			return unit.drive.status | (unit.drive.ctrl & Unit::Drive::CTRL_STOP);
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

		ibool Fds::Unit::Drive::Advance(uint& timer)
		{
			NST_ASSERT( io && !count );

			if (headPos-1U < MAX_SIDE_SIZE && dataPos < SIDE_SIZE)
			{
				NST_VERIFY( !(status & STATUS_UNREADY) );

				++headPos;
				u8* NST_RESTRICT stream = io + dataPos;
				count = CLK_BYTE;

				NST_VERIFY( (ctrl & CTRL_READ_MODE) || length != LENGTH_UNKNOWN );

				if (ctrl & CTRL_READ_MODE)
				{
					if (!gap)
					{
						if (length == LENGTH_UNKNOWN)
						{
							// Non-standard file layout which cannot accurately
							// be emulated within the FDS file format since it
							// removes the CRC value at the end of each block.
							// No choice but to fall back on the BIOS.

							in = *stream | 0x100;
							dataPos += (ctrl & CTRL_CRC) ? -2 : +1;
						}
						else if (length-- > 2U)
						{
							in = *stream;
							++dataPos;
						}
						else if (length == 1U)
						{
							if (*stream <= 4U)
							{
								in = 0x91;
							}
							else
							{
								in = *stream;
								++dataPos;
							}
						}
						else
						{
							if (*stream <= 4U)
							{
								in = 0x88;
								length = 0;
								gap = BYTES_GAP_NEXT;
							}
							else
							{
								in = *stream;
								length = LENGTH_UNKNOWN;
								++dataPos;
							}
						}
					}
					else
					{
						if (!--gap)
						{
							NST_VERIFY( *stream <= 4U );

							switch (stream[0])
							{
								case BLOCK_HEADER:

									length = LENGTH_HEADER + 3;
									break;

								case BLOCK_DATA:

									length = (stream[-2] << 8 | stream[-3]) + 3;
									NST_VERIFY( length > 3 );
									break;

								case BLOCK_VOLUME:

									length = LENGTH_VOLUME + 3;
									break;

								case BLOCK_COUNT:

									length = LENGTH_COUNT + 3;
									break;

								default:

									gap = 1;
									break;
							}
						}

						if (ctrl & CTRL_IO_MODE)
							return false;

						NST_VERIFY( !(ctrl & CTRL_GEN_IRQ) );

						in = 0;
					}
				}
				else if (!(status & STATUS_PROTECTED) && length != LENGTH_UNKNOWN)
				{
					NST_VERIFY( (ctrl & CTRL_IO_MODE) || !length );

					gap -= (gap > 0);

					const uint data = (ctrl & CTRL_IO_MODE) ? out : 0;

					if (length-- > 3U)
					{
						++dataPos;
						*stream = data;
					}
					else if (length == 2U)
					{
					}
					else if (length == 1U)
					{
						gap = BYTES_GAP_NEXT;
					}
					else
					{
						length = 0;

						if (data-1U <= 3U)
						{
							NST_VERIFY( ctrl & CTRL_IO_MODE );

							++dataPos;
							dirty = true;

							switch (*stream=data)
							{
								case BLOCK_VOLUME:

									length = LENGTH_VOLUME + 3;
									break;

								case BLOCK_COUNT:

									length = LENGTH_COUNT + 3;
									break;

								case BLOCK_HEADER:

									length = LENGTH_HEADER + 3;
									break;

								case BLOCK_DATA:

									length = (stream[-2] << 8 | stream[-3]) + 3;
									NST_VERIFY( length > 3 );
									break;

								NST_UNREACHABLE
							}
						}
					}
				}

				uint irq = ctrl & CTRL_GEN_IRQ;
				timer |= irq >> 6;
				return irq;
			}
			else if (headPos)
			{
				count = CLK_REWIND;
				headPos = 0;
				status |= STATUS_UNREADY;
			}
			else if (!(ctrl & CTRL_STOP))
			{
				count = CLK_BYTE;
				headPos = 1;
				dataPos = 0;
				length = 0;
				gap = BYTES_GAP_INIT + BYTES_GAP_NEXT;
				status &= ~uint(STATUS_UNREADY);
			}

			return false;
		}

		void Fds::Unit::Timer::Advance(uint& timer)
		{
			timer |= STATUS_PENDING_IRQ;

			if (ctrl & CTRL_REPEAT)
				count = latch;
			else
				ctrl &= ~uint(CTRL_ENABLED);
		}

		inline ibool Fds::Unit::Drive::Clock()
		{
			return !count || --count;
		}

		inline ibool Fds::Unit::Timer::Clock()
		{
			return !(ctrl & CTRL_ENABLED) || !count || --count;
		}

		ibool Fds::Unit::Signal()
		{
			return
			(
				(timer.Clock() ? 0 : (timer.Advance(status), 1)) |
				(drive.Clock() ? 0 : drive.Advance(status))
			);
		}

		void Fds::VSync()
		{
			adapter.VSync();

			if (!disks.mounting)
			{
				const uint led = adapter.Activity();

				if (io.led != led && (io.led != Api::Fds::MOTOR_WRITE || led != Api::Fds::MOTOR_READ))
				{
					io.led = led;
					Api::Fds::diskAccessLampCallback( (Api::Fds::Motor) io.led );
				}
			}
			else if (!--disks.mounting)
			{
				adapter.Mount( disks.sides.data[disks.current], disks.writeProtected );
			}
		}
	}
}
