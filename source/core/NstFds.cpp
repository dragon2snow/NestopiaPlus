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
#include <new>
#include "NstLog.hpp"
#include "NstCrc32.hpp"
#include "NstState.hpp"
#include "NstFds.hpp"
#include "api/NstApiUser.hpp"
#include "api/NstApiInput.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		class Fds::Bios
		{
		public:

			NES_DECL_PEEK( Rom );
			NES_DECL_POKE( Nop );

		private:

			enum
			{
				FAMICOM_ID    = 0x5E607DCF,
				TWINSYSTEM_ID = 0x4DF24A6C
			};

			byte rom[SIZE_8K];
			bool available;

		public:

			Bios()
			: available(false)
			{
			}

			void Set(StdStream stdStream)
			{
				available = false;

				if (stdStream)
				{
					Stream::In(stdStream).Read( rom, SIZE_8K );
					available = true;

					if (Log::Available())
					{
						switch (Crc32::Compute( rom, SIZE_8K ))
						{
							case FAMICOM_ID:
							case TWINSYSTEM_ID:

								Log::Flush( "Fds: BIOS ROM ok" NST_LINEBREAK );
								break;

							default:

								Log::Flush( "Fds: warning, unknown BIOS ROM!" NST_LINEBREAK );
								break;
						}
					}
				}
			}

			Result Get(StdStream stream) const
			{
				if (available)
				{
					Stream::Out(stream).Write( rom, SIZE_8K );
					return RESULT_OK;
				}
				else
				{
					return RESULT_ERR_NOT_READY;
				}
			}

			bool Available() const
			{
				return available;
			}
		};

		Fds::Bios Fds::bios;

		const byte Fds::Sound::volumes[4] =
		{
			30 * 8,
			20 * 8,
			15 * 8,
			12 * 8
		};

		const byte Fds::Sound::Modulator::steps[8] =
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

		inline byte* Fds::Disks::Sides::operator [] (uint i) const
		{
			NST_ASSERT( i < count );
			return data + i * dword(SIDE_SIZE);
		}

		inline void Fds::Disks::Sides::Cache() const
		{
			if (!dirty)
				Load();
		}

		Fds::Disks::Sides::Sides(StdStream stdStream)
		: dirty(false)
		{
			Stream::In stream( stdStream );

			dword size;
			uint header;

			switch (stream.Read32())
			{
				case FDS_ID:
				{
					size = stream.Read8();
					stream.Seek( -5 );
					header = HEADER_SIZE;
					break;
				}

				case FDS_RAW_ID:
				{
					stream.Seek( -4 );

					for (size=0; size < 0xFF && !stream.Eof(); ++size)
						stream.Seek( SIDE_SIZE );

					stream.Seek( -idword(size * SIDE_SIZE) );
					header = 0;
					break;
				}

				default: throw RESULT_ERR_INVALID_FILE;
			}

			if (!size)
				throw RESULT_ERR_CORRUPT_FILE;

			count = size;
			size *= SIDE_SIZE;
			data  = new byte [HEADER_SIZE + size];
			std::memset( data, 0, HEADER_SIZE );
			data += HEADER_SIZE;

			try
			{
				stream.Read( data - header, header + size );
			}
			catch (...)
			{
				delete [] (data - HEADER_SIZE);
				throw;
			}
		}

		Fds::Disks::Sides::~Sides()
		{
			delete [] (data - HEADER_SIZE);
		}

		Fds::Disks::Disks(StdStream stream)
		:
		sides          (stream),
		crc            (Crc32::Compute( sides[0], sides.count * dword(SIDE_SIZE) )),
		id             (dword(sides[0][0x0F]) << 24 | dword(sides[0][0x10]) << 16 | uint(sides[0][0x11]) <<  8 | sides[0][0x12]),
		current        (EJECTED),
		mounting       (0),
		writeProtected (false)
		{
			if (Log::Available())
			{
				Log log;

				for (uint i=0; i < sides.count; ++i)
				{
					Api::Fds::DiskData data;

					if (NES_SUCCEEDED(Unit::Drive::Analyze( sides[i], data )))
					{
						dword disksize = 0;

						for (Api::Fds::DiskData::Files::const_iterator it(data.files.begin()), end(data.files.end()); it != end; ++it)
							disksize += it->data.size();

						log << "Fds: Disk "
							<< (1+i/2)
							<< (i % 2 ? " Side B: " : " Side A: ")
							<< (disksize / SIZE_1K)
							<< "k in "
							<< data.files.size()
							<< " files";

						if (const uint raw = data.raw.size())
							log << ", " << raw << "b trailing data";

						log << ".." NST_LINEBREAK;

						for (Api::Fds::DiskData::Files::const_iterator it(data.files.begin()), end(data.files.end()); it != end; ++it)
						{
							log << "Fds: file: \"" << it->name
								<< "\", id: "      << it->id
								<< ", size: "      << it->data.size()
								<< ", index: "     << it->index
								<< ", address: "   << Log::Hex( 16, it->address )
								<< ", type: "
								<<
								(
									it->type == Api::Fds::DiskData::File::TYPE_PRG ? "PRG" NST_LINEBREAK :
									it->type == Api::Fds::DiskData::File::TYPE_CHR ? "CHR" NST_LINEBREAK :
									it->type == Api::Fds::DiskData::File::TYPE_NMT ? "NMT" NST_LINEBREAK :
                                                                                     "unknown" NST_LINEBREAK
								);
						}
					}
				}
			}
		}

		Fds::Unit::Timer::Timer()
		{
			Reset();
		}

		Fds::Unit::Drive::Drive(const Disks::Sides& s)
		: dirty(false), sides(s)
		{
			Reset();
		}

		Fds::Unit::Unit(const Disks::Sides& s)
		: drive(s)
		{
			status = 0;
		}

		Fds::Adapter::Adapter(Cpu& c,const Disks::Sides& s)
		: Clock::M2<Unit>(c,s) {}

		Fds::Io::Io()
		: led(Api::Fds::MOTOR_OFF)
		{
			Reset();
		}

		Fds::Fds(Context& context)
		:
		Image   (DISK),
		disks   (context.stream),
		adapter (context.cpu,disks.sides),
		cpu     (context.cpu),
		ppu     (context.ppu),
		sound   (context.cpu)
		{
			if (!bios.Available())
				throw RESULT_ERR_MISSING_BIOS;

			ppu.GetChrMem().Source().Set( SIZE_8K, true, true );
		}

		void Fds::SetBios(StdStream stream)
		{
			bios.Set( stream );
		}

		Result Fds::GetBios(StdStream stream)
		{
			return bios.Get( stream );
		}

		bool Fds::HasBios()
		{
			return bios.Available();
		}

		Fds::~Fds()
		{
			EjectDisk();

			if (!disks.writeProtected)
				disks.sides.Save();
		}

		NST_NO_INLINE void Fds::Disks::Sides::Load() const
		{
			NST_ASSERT( !dirty );

			dirty = true;
			const uint header = HasHeader() ? HEADER_SIZE : 0;
			file.Load( data - header, header + count * dword(SIDE_SIZE) );
		}

		void Fds::Disks::Sides::Save() const
		{
			if (dirty)
			{
				try
				{
					const uint header = HasHeader() ? HEADER_SIZE : 0;
					file.Save( File::SAVE_FDS, data - header, header + count * dword(SIDE_SIZE), false );
				}
				catch (...)
				{
					NST_DEBUG_MSG("fds save failure!");
				}
			}
		}

		bool Fds::PowerOff() const
		{
			if (io.led != Api::Fds::MOTOR_OFF)
			{
				io.led = Api::Fds::MOTOR_OFF;
				Api::Fds::diskAccessLampCallback( Api::Fds::MOTOR_OFF );
			}

			return true;
		}

		uint Fds::GetDesiredController(const uint port) const
		{
			if (port == Api::Input::EXPANSION_PORT)
				return (disks.id == DOREMIKKO_ID) ? Api::Input::DOREMIKKOKEYBOARD : Api::Input::UNCONNECTED;
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

		inline void Fds::Adapter::Mount(byte* io,bool protect)
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

		void Fds::Adapter::Reset(byte* const io,bool protect)
		{
			Clock::M2<Unit>::Reset( true, true );

			unit.drive.Mount( io, protect );

			cpu.Map( 0x4020 ).Set( this, &Adapter::Peek_Nop,  &Adapter::Poke_4020 );
			cpu.Map( 0x4021 ).Set( this, &Adapter::Peek_Nop,  &Adapter::Poke_4021 );
			cpu.Map( 0x4022 ).Set( this, &Adapter::Peek_Nop,  &Adapter::Poke_4022 );
			cpu.Map( 0x4024 ).Set( this, &Adapter::Peek_Nop,  &Adapter::Poke_4024 );
			cpu.Map( 0x4030 ).Set( this, &Adapter::Peek_4030, &Adapter::Poke_Nop  );
			cpu.Map( 0x4032 ).Set( this, &Adapter::Peek_4032, &Adapter::Poke_Nop  );
		}

		void Fds::Reset(const bool hard)
		{
			disks.mounting = 0;

			adapter.Reset
			(
				disks.current == Disks::EJECTED ? NULL : disks.sides[disks.current],
				disks.writeProtected
			);

			if (hard)
			{
				ram.Reset();
				ppu.GetChrMem().Source().Fill( 0x00 );
				ppu.GetChrMem().SwapBank<SIZE_8K,0x0000>( 0 );
			}

			cpu.ClearIRQ();

			cpu.Map( 0x4023 ).Set( this, &Fds::Peek_Nop,  &Fds::Poke_4023 );
			cpu.Map( 0x4025 ).Set( this, &Fds::Peek_Nop,  &Fds::Poke_4025 );
			cpu.Map( 0x4026 ).Set( this, &Fds::Peek_Nop,  &Fds::Poke_4026 );
			cpu.Map( 0x4031 ).Set( this, &Fds::Peek_4031, &Fds::Poke_Nop  );
			cpu.Map( 0x4033 ).Set( this, &Fds::Peek_4033, &Fds::Poke_Nop  );

			cpu.Map( 0x6000, 0xDFFF ).Set( &ram, &Fds::Ram::Peek_Ram, &Fds::Ram::Poke_Ram );
			cpu.Map( 0xE000, 0xFFFF ).Set( &bios, &Bios::Peek_Rom, &Bios::Poke_Nop );
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
			cpu.Map( 0x4040, 0x407F ).Set( this, &Fds::Sound::Peek_4040, &Fds::Sound::Poke_4040 );

			cpu.Map( 0x4080 ).Set( this, &Fds::Sound::Peek_Nop,  &Fds::Sound::Poke_4080 );
			cpu.Map( 0x4082 ).Set( this, &Fds::Sound::Peek_Nop,  &Fds::Sound::Poke_4082 );
			cpu.Map( 0x4083 ).Set( this, &Fds::Sound::Peek_Nop,  &Fds::Sound::Poke_4083 );
			cpu.Map( 0x4084 ).Set( this, &Fds::Sound::Peek_Nop,  &Fds::Sound::Poke_4084 );
			cpu.Map( 0x4085 ).Set( this, &Fds::Sound::Peek_Nop,  &Fds::Sound::Poke_4085 );
			cpu.Map( 0x4086 ).Set( this, &Fds::Sound::Peek_Nop,  &Fds::Sound::Poke_4086 );
			cpu.Map( 0x4087 ).Set( this, &Fds::Sound::Peek_Nop,  &Fds::Sound::Poke_4087 );
			cpu.Map( 0x4088 ).Set( this, &Fds::Sound::Peek_Nop,  &Fds::Sound::Poke_4088 );
			cpu.Map( 0x4089 ).Set( this, &Fds::Sound::Peek_Nop,  &Fds::Sound::Poke_4089 );
			cpu.Map( 0x408A ).Set( this, &Fds::Sound::Peek_Nop,  &Fds::Sound::Poke_408A );
			cpu.Map( 0x4090 ).Set( this, &Fds::Sound::Peek_4090, &Fds::Sound::Poke_Nop  );
			cpu.Map( 0x4092 ).Set( this, &Fds::Sound::Peek_4092, &Fds::Sound::Poke_Nop  );

			ResetChannel();
			dcBlocker.Reset();

			cpu.GetApu().SetExternalClock( 0 );
		}

		void Fds::Sound::UpdateContext(uint,const byte (&outputLevels)[MAX_CHANNELS])
		{
			NST_VERIFY( fixed <= 0xFFFF && rate <= 0x7FFFF );

			outputVolume = outputLevels[Apu::CHANNEL_FDS] * 69U / DEFAULT_VOLUME;
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

		void Fds::Unit::Drive::Mount(byte* data,bool protect)
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

		Result Fds::Unit::Drive::Analyze(const byte* NST_RESTRICT src,Api::Fds::DiskData& dst)
		{
			try
			{
				idword i = SIDE_SIZE;

				for (uint block=~0U, files=0; i; )
				{
					const uint prev = block;
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

						Stream::In::AsciiToC( file.name, src+3, 8 );

						for (uint j=8; j < sizeof(array(file.name)); ++j)
							file.name[j] = '\0';

						file.address = src[11] | uint(src[12]) << 8;

						switch (src[15])
						{
							case 0:  file.type = Api::Fds::DiskData::File::TYPE_PRG;     break;
							case 1:  file.type = Api::Fds::DiskData::File::TYPE_CHR;     break;
							case 2:  file.type = Api::Fds::DiskData::File::TYPE_NMT;     break;
							default: file.type = Api::Fds::DiskData::File::TYPE_UNKNOWN; break;
						}

						file.data.resize( src[13] | uint(src[14]) << 8 );

						if (const dword size = file.data.size())
							std::memset( &file.data.front(), 0x00, size );

						src += LENGTH_HEADER+1;
					}
					else if (block == BLOCK_DATA)
					{
						if (prev != BLOCK_HEADER)
							break;

						Api::Fds::DiskData::Data& data = dst.files.back().data;
						const idword size = data.size();

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

				for (idword j=i; j-- > 0; )
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

		Result Fds::GetDiskData(uint side,Api::Fds::DiskData& data) const
		{
			if (side < disks.sides.count)
				return Unit::Drive::Analyze( disks.sides[side], data );

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
					case AsciiId<'I','O'>::V:
					{
						State::Loader::Data<4> data( state );

						io.ctrl = data[0];
						io.port = data[1];
						break;
					}

					case AsciiId<'R','A','M'>::V:

						state.Uncompress( ram.mem );
						break;

					case AsciiId<'C','H','R'>::V:

						state.Uncompress( ppu.GetChrMem().Source().Mem(), SIZE_8K );
						break;

					case AsciiId<'I','R','Q'>::V:
					case AsciiId<'D','R','V'>::V:

						adapter.LoadState( state, chunk, ppu );
						break;

					case AsciiId<'D','S','K'>::V:
					{
						State::Loader::Data<4> data( state );

						if (data[0] != disks.sides.count)
							throw RESULT_ERR_INVALID_FILE;

						saveDisks[0] = data[1];
						saveDisks[1] = data[2];
						saveDisks[2] = data[3];
						break;
					}

					case AsciiId<'S','N','D'>::V:

						sound.LoadState( state );
						break;

					default:

						for (uint i=0; i < disks.sides.count; ++i)
						{
							if (chunk == AsciiId<'D','0','A'>::R( 0, i / 2, i % 2 ))
							{
								disks.sides.Cache();

								byte* const data = disks.sides[i];
								state.Uncompress( data, SIDE_SIZE );

								for (uint j=0; j < SIDE_SIZE; ++j)
									data[j] ^= 0xFFU;

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
				disks.writeProtected = saveDisks[0] & 0x2U;

				if (saveDisks[0] & 0x1U)
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
				disks.current != Disks::EJECTED && !disks.mounting ? disks.sides[disks.current] : NULL,
				disks.writeProtected
			);
		}

		void Fds::SaveState(State::Saver& state,const dword id) const
		{
			state.Begin( id );

			{
				const byte data[4] =
				{
					io.ctrl,
					io.port,
					0,
					0
				};

				state.Begin( AsciiId<'I','O'>::V ).Write( data ).End();
			}

			adapter.SaveState( state );

			state.Begin( AsciiId<'R','A','M'>::V ).Compress( ram.mem ).End();
			state.Begin( AsciiId<'C','H','R'>::V ).Compress( ppu.GetChrMem().Source().Mem(), SIZE_8K ).End();

			{
				const byte data[4] =
				{
					disks.sides.count,
					(disks.current != Disks::EJECTED) | (disks.writeProtected ? 0x2U : 0x0U),
					disks.current != Disks::EJECTED ? disks.current : 0xFF,
					disks.current != Disks::EJECTED ? disks.mounting : 0
				};

				state.Begin( AsciiId<'D','S','K'>::V ).Write( data ).End();
			}

			if (adapter.Dirty() || !state.Internal())
			{
				struct Dst
				{
					byte* const NST_RESTRICT mem;

					Dst() : mem(new byte [SIDE_SIZE]) {}
					~Dst() { delete [] mem; }
				};

				Dst dst;

				for (uint i=0; i < disks.sides.count; ++i)
				{
					const byte* const NST_RESTRICT src = disks.sides[i];

					for (uint j=0; j < SIDE_SIZE; ++j)
						dst.mem[j] = src[j] ^ 0xFFU;

					state.Begin( AsciiId<'D','0','A'>::R( 0, i / 2, i % 2 ) ).Compress( dst.mem, SIDE_SIZE ).End();
				}
			}

			sound.SaveState( state, AsciiId<'S','N','D'>::V );

			state.End();
		}

		void Fds::Adapter::SaveState(State::Saver& state) const
		{
			{
				const byte data[7] =
				{
					unit.timer.ctrl,
					unit.status,
					unit.timer.latch & 0xFF,
					unit.timer.latch >> 8,
					unit.timer.count & 0xFF,
					unit.timer.count >> 8,
					0
				};

				state.Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End();
			}

			{
				const uint headPos = NST_MIN(unit.drive.headPos,SIDE_SIZE);

				const byte data[16] =
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

				state.Begin( AsciiId<'D','R','V'>::V ).Write( data ).End();
			}
		}

		void Fds::Adapter::LoadState(State::Loader& state,const dword chunk,Ppu& ppu)
		{
			switch (chunk)
			{
				case AsciiId<'I','R','Q'>::V:
				{
					State::Loader::Data<7> data( state );

					unit.timer.ctrl = data[0];
					unit.status = data[1] & (Unit::STATUS_PENDING_IRQ|Unit::STATUS_TRANSFERED);
					unit.timer.latch = data[2] | data[3] << 8;
					unit.timer.count = data[4] | data[5] << 8;

					break;
				}

				case AsciiId<'D','R','V'>::V:
				{
					State::Loader::Data<16> data( state );

					unit.drive.ctrl = data[0];
					unit.drive.status = data[1] & (Unit::Drive::STATUS_EJECTED|Unit::Drive::STATUS_UNREADY|Unit::Drive::STATUS_PROTECTED) | OPEN_BUS;
					unit.drive.in = data[2] | (data[15] << 8 & 0x100);
					unit.drive.out = data[3];
					unit.drive.headPos = data[4] | data[5] << 8;
					unit.drive.dataPos = data[6] | data[7] << 8;
					unit.drive.gap = data[8] | data[9] << 8;
					unit.drive.length = data[10] | data[11] << 8;
					unit.drive.count = data[12] | data[13] << 8 | dword(data[14]) << 16;

					if (unit.drive.dataPos > SIDE_SIZE)
						unit.drive.dataPos = SIDE_SIZE;

					if (unit.drive.headPos < unit.drive.dataPos)
						unit.drive.headPos = unit.drive.dataPos;

					ppu.SetMirroring( (unit.drive.ctrl & CTRL1_NMT_HORIZONTAL) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
					break;
				}
			}
		}

		void Fds::Sound::SaveState(State::Saver& state,const dword id) const
		{
			state.Begin( id );

			state.Begin( AsciiId<'M','A','S'>::V );
			{
				{
					byte data[6] =
					{
						((status & STATUS_OUTPUT_ENABLED) ? 0U : uint(REG3_OUTPUT_DISABLE)) |
						((status & STATUS_ENVELOPES_ENABLED) ? 0U : uint(REG3_ENVELOPE_DISABLE)),
						wave.writing ? REG9_WRITE_MODE : 0,
						wave.length & 0xFF,
						wave.length >> 8,
						envelopes.length,
						envelopes.counter - 1
					};

					for (uint i=0; i < sizeof(array(volumes)); ++i)
					{
						if (volume == volumes[i])
						{
							data[1] |= i;
							break;
						}
					}

					state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
				}
				{
					state.Begin( AsciiId<'W','A','V'>::V ).Compress( wave.table ).End();
				}
			}
			state.End();

			envelopes.units[VOLUME].SaveState( state, AsciiId<'V','O','L'>::V );
			envelopes.units[SWEEP].SaveState( state, AsciiId<'S','W','P'>::V );

			state.Begin( AsciiId<'M','O','D'>::V );
			{
				{
					const byte data[4] =
					{
						modulator.length & 0xFF,
						modulator.length >> 8 | (modulator.writing ? REG7_MOD_WRITE_MODE : 0),
						modulator.sweep,
						modulator.pos
					};

					state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
				}

				{
					byte data[Modulator::SIZE];

					for (uint i=0; i < Modulator::SIZE; ++i)
					{
						for (uint j=0; j < sizeof(array(Modulator::steps)); ++j)
						{
							if (modulator.table[i] == Modulator::steps[j])
							{
								data[i] = j;
								break;
							}
						}
					}

					state.Begin( AsciiId<'R','A','M'>::V ).Compress( data ).End();
				}
			}
			state.End();

			state.End();
		}

		void Fds::Sound::LoadState(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case AsciiId<'M','A','S'>::V:
					{
						while (const dword subchunk = state.Begin())
						{
							switch (subchunk)
							{
								case AsciiId<'R','E','G'>::V:
								{
									State::Loader::Data<6> data( state );

									status =
									(
										((data[0] & REG3_OUTPUT_DISABLE) ? 0U : uint(STATUS_OUTPUT_ENABLED)) |
										((data[0] & REG3_ENVELOPE_DISABLE) ? 0U : uint(STATUS_ENVELOPES_ENABLED))
									);

									volume = volumes[data[1] & REG9_VOLUME];
									wave.writing = data[1] & REG9_WRITE_MODE;
									wave.length = data[2] | (data[3] & REG3_WAVELENGTH_HIGH) << 8;
									envelopes.length = data[4];
									envelopes.counter = data[5] + 1;
									break;
								}

								case AsciiId<'W','A','V'>::V:

									state.Uncompress( wave.table );

									for (uint i=0; i < Wave::SIZE; ++i)
										wave.table[i] &= 0x3FU;

									break;
							}

							state.End();
						}
						break;
					}

					case AsciiId<'V','O','L'>::V:

						envelopes.units[VOLUME].LoadState( state );
						break;

					case AsciiId<'S','W','P'>::V:

						envelopes.units[SWEEP].LoadState( state );
						break;

					case AsciiId<'M','O','D'>::V:
					{
						while (const dword subchunk = state.Begin())
						{
							switch (subchunk)
							{
								case AsciiId<'R','E','G'>::V:
								{
									State::Loader::Data<4> data( state );

									modulator.length = data[0] | (data[1] & REG7_MOD_WAVELENGTH_HIGH) << 8;
									modulator.writing = data[1] & (REG7_MOD_WRITE_MODE);
									modulator.sweep = data[2] & (REG5_MOD_SWEEP|REG5_MOD_NEGATE);
									modulator.pos = data[3] & 0x3F;

									break;
								}

								case AsciiId<'R','A','M'>::V:
								{
									byte data[Modulator::SIZE];
									state.Uncompress( data );

									for (uint i=0; i < Modulator::SIZE; ++i)
										modulator.table[i] = Modulator::steps[data[i] & uint(REG8_MOD_DATA)];

									break;
								}
							}

							state.End();
						}
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

		void Fds::Sound::Envelope::SaveState(State::Saver& state,const dword id) const
		{
			const byte data[3] =
			{
				ctrl,
				counter - 1,
				gain
			};

			state.Begin( id ).Write( data ).End();
		}

		void Fds::Sound::Envelope::LoadState(State::Loader& state)
		{
			State::Loader::Data<3> data( state );

			ctrl = data[0];
			counter = (data[1] & CTRL_COUNT) + 1;
			gain = data[2] & CTRL_COUNT;
		}

		#ifdef NST_MSVC_OPTIMIZE
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
			return mem[address - 0x6000];
		}

		NES_POKE(Fds::Ram,Ram)
		{
			mem[address - 0x6000] = data;
		}

		NES_PEEK(Fds::Bios,Rom)
		{
			return rom[address - 0xE000];
		}

		NES_POKE(Fds::Bios,Nop)
		{
		}

		NES_POKE(Fds::Adapter,4020)
		{
			Update();
			unit.timer.latch = (unit.timer.latch & 0xFF00) | (data << 0);
		}

		NES_POKE(Fds::Adapter,4021)
		{
			Update();
			unit.timer.latch = (unit.timer.latch & 0x00FF) | (data << 8);
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
			else if (!(reg & CTRL_STOP | count) && io)
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
			return wave.table[address & 0x3F] | uint(OPEN_BUS);
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

			if (data & Envelope::CTRL_DISABLE && !wave.pos)
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

				if (envelopes.length && status & STATUS_ENVELOPES_ENABLED)
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

					if (pos & Modulator::TIMER_CARRY)
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
				for (modulator.timer -= modulator.length * rate; modulator.timer & Modulator::TIMER_CARRY; modulator.timer += fixed << 16)
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
				byte* stream = io + dataPos;
				count = CLK_BYTE;

				NST_VERIFY( ctrl & CTRL_READ_MODE || length != LENGTH_UNKNOWN );

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

							in = *stream | 0x100U;

							if (ctrl & CTRL_CRC)
								dataPos -= 2;
							else
								dataPos += 1;
						}
						else if (length-- > 2)
						{
							in = *stream;
							++dataPos;
						}
						else if (length == 1)
						{
							if (*stream <= 4)
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
							if (*stream <= 4)
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
							NST_VERIFY( *stream <= 4 );

							switch (stream[0])
							{
								case BLOCK_HEADER:

									length = LENGTH_HEADER + 3;
									break;

								case BLOCK_DATA:

									length = (uint(stream[-2]) << 8 | stream[-3]) + 3;
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
					NST_VERIFY( ctrl & CTRL_IO_MODE || !length );

					gap -= (gap > 0);

					const uint data = (ctrl & CTRL_IO_MODE) ? out : 0;

					if (length-- > 3)
					{
						++dataPos;

						dirty = true;
						sides.Cache();

						*stream = data;
					}
					else if (length == 2)
					{
					}
					else if (length == 1)
					{
						gap = BYTES_GAP_NEXT;
					}
					else
					{
						length = 0;

						if (data-1 <= 3)
						{
							NST_VERIFY( ctrl & CTRL_IO_MODE );

							++dataPos;

							dirty = true;
							sides.Cache();

							switch (*stream = data)
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

									length = (uint(stream[-2]) << 8 | stream[-3]) + 3;
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

		inline bool Fds::Unit::Drive::Clock()
		{
			return !count || --count;
		}

		inline bool Fds::Unit::Timer::Clock()
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
					Api::Fds::diskAccessLampCallback( static_cast<Api::Fds::Motor>(io.led) );
				}
			}
			else if (!--disks.mounting)
			{
				adapter.Mount( disks.sides[disks.current], disks.writeProtected );
			}
		}
	}
}
