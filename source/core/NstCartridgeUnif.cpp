////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
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
#include "NstLog.hpp"
#include "NstIps.hpp"
#include "NstStream.hpp"
#include "NstChecksum.hpp"
#include "NstImageDatabase.hpp"
#include "NstCartridge.hpp"
#include "NstCartridgeUnif.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Cartridge::Unif::Context::Rom::Rom()
		: truncated(0)
		{
			for (uint i=0; i < sizeof(crc); ++i)
				crc[i] = '\0';
		}

		Cartridge::Unif::Context::Context()
		: system(SYSTEM_NTSC)
		{
			std::memset( chunks, 0, sizeof(chunks) );
		}

		void Cartridge::Unif::Load
		(
			StdStream const stdStreamImage,
			StdStream const stdStreamIps,
			Ram& prg,
			Ram& chr,
			const FavoredSystem favoredSystem,
			Profile& profile,
			ProfileEx& profileEx,
			const ImageDatabase* const database
		)
		{
			NST_ASSERT( prg.Empty() && chr.Empty() );

			profile = Profile();
			profileEx = ProfileEx();

			ReadHeader( stdStreamImage );
			ReadChunks( stdStreamImage, prg, chr, favoredSystem, profile, profileEx );

			Ips ips;

			if (stdStreamIps && !ips.Load( stdStreamIps ))
				Log::Flush( "Unif: Warning, invalid or corrupt IPS file" NST_LINEBREAK );

			if (ips.Empty() && database && database->Enabled())
			{
				Checksum checksum;

				checksum.Compute( prg.Mem(), prg.Size() );
				checksum.Compute( chr.Mem(), chr.Size() );

				if (const ImageDatabase::Entry entry = database->Search( Profile::Hash(checksum.GetSha1(),checksum.GetCrc()), favoredSystem ))
					entry.Fill( profile );
			}

			if (ips.Patch( prg.Mem(), prg.Size(), 16 ))
				Log::Flush( "Unif: PRG-ROM was IPS patched" NST_LINEBREAK );

			if (ips.Patch( chr.Mem(), chr.Size(), 16 + prg.Size() ))
				Log::Flush( "Unif: CHR-ROM was IPS patched" NST_LINEBREAK );
		}

		bool Cartridge::Unif::Context::operator () (const uint id,const dword chunk)
		{
			NST_VERIFY( chunks[id] == 0 );

			if (chunks[id] == 0)
			{
				chunks[id] = 1;
				return true;
			}
			else
			{
				char name[5];
				Log() << "Unif: warning, duplicate chunk: \"" << ChunkName(name,chunk) << "\" ignored" NST_LINEBREAK;

				return false;
			}
		}

		void Cartridge::Unif::ReadHeader(StdStream stdStream)
		{
			Stream::In stream( stdStream );

			if (stream.Read32() != AsciiId<'U','N','I','F'>::V)
				throw RESULT_ERR_INVALID_FILE;

			dword version = stream.Read32();

			Log() << "Unif: revision " << version << NST_LINEBREAK;

			byte reserved[HEADER_RESERVED_LENGTH];
			stream.Read( reserved );

			for (uint i=0; i < HEADER_RESERVED_LENGTH; ++i)
			{
				NST_VERIFY( !reserved[i] );

				if (reserved[i])
				{
					Log() << "Unif: warning, unknown header data" NST_LINEBREAK;
					break;
				}
			}
		}

		void Cartridge::Unif::ReadChunks(StdStream stdStream,Ram& prg,Ram& chr,const FavoredSystem favoredSystem,Profile& profile,ProfileEx& profileEx)
		{
			Context context;

			for (Stream::In stream(stdStream); !stream.Eof(); )
			{
				dword id = stream.Read32();
				const dword length = stream.Read32();
				NST_VERIFY( length <= SIZE_1K * 4096UL );

				switch (id)
				{
					case AsciiId<'N','A','M','E'>::V: id = (context( 0, id ) ? ReadName       ( stdStream, profile.game.title       ) : 0); break;
					case AsciiId<'R','E','A','D'>::V: id = (context( 1, id ) ? ReadComment    ( stdStream                           ) : 0); break;
					case AsciiId<'D','I','N','F'>::V: id = (context( 2, id ) ? ReadDumper     ( stdStream                           ) : 0); break;
					case AsciiId<'T','V','C','I'>::V: id = (context( 3, id ) ? ReadSystem     ( stdStream, context                  ) : 0); break;
					case AsciiId<'B','A','T','R'>::V: id = (context( 4, id ) ? ReadBattery    ( profileEx                           ) : 0); break;
					case AsciiId<'M','A','P','R'>::V: id = (context( 5, id ) ? ReadBoard      ( stdStream, profile.board            ) : 0); break;
					case AsciiId<'M','I','R','R'>::V: id = (context( 6, id ) ? ReadMirroring  ( stdStream, profileEx                ) : 0); break;
					case AsciiId<'C','T','R','L'>::V: id = (context( 7, id ) ? ReadController ( stdStream, profile.game.controllers ) : 0); break;
					case AsciiId<'V','R','O','R'>::V: id = (context( 8, id ) ? ReadChrRam     (                                     ) : 0); break;

					default: switch (id & 0x00FFFFFF)
					{
						case AsciiId<'P','C','K'>::V:
						case AsciiId<'C','C','K'>::V:
						case AsciiId<'P','R','G'>::V:
						case AsciiId<'C','H','R'>::V:
						{
							uint index = id >> 24 & 0xFF;

							if (index >= Ascii<'0'>::V && index <= Ascii<'9'>::V)
							{
								index -= Ascii<'0'>::V;
							}
							else if (index >= Ascii<'A'>::V && index <= Ascii<'F'>::V)
							{
								index = index - Ascii<'A'>::V + 10;
							}
							else
							{
								index = ~0U;
							}

							if (index < 16)
							{
								switch (id & 0x00FFFFFF)
								{
									case AsciiId<'P','C','K'>::V: id = (context( 9+0+index,  id ) ? ReadChecksum ( stdStream, 0, index, context.roms[0][index]  ) : 0); break;
									case AsciiId<'C','C','K'>::V: id = (context( 9+16+index, id ) ? ReadChecksum ( stdStream, 1, index, context.roms[1][index]  ) : 0); break;
									case AsciiId<'P','R','G'>::V: id = (context( 9+32+index, id ) ? ReadRom      ( stdStream, 0, index, length, context.roms[0] ) : 0); break;
									case AsciiId<'C','H','R'>::V: id = (context( 9+48+index, id ) ? ReadRom      ( stdStream, 1, index, length, context.roms[1] ) : 0); break;
								}

								break;
							}
						}

						default:

							id = ReadUnknown( id );
							break;
					}
				}

				if (id < length)
				{
					for (id = length - id; id > 0x7FFFFFFF; id -= 0x7FFFFFFF)
						stream.Seek( 0x7FFFFFFF );

					if (id)
						stream.Seek( id );
				}
				else if (id > length)
				{
					throw RESULT_ERR_CORRUPT_FILE;
				}
			}

			for (uint i=0; i < 2; ++i)
			{
				uint count = 0;
				dword size = 0;

				for (uint j=0; j < 16; ++j)
				{
					if (const dword n=context.roms[i][j].data.Size())
					{
						count++;
						size += n;
					}
				}

				if (count)
				{
					Profile::Board::Roms& rom = (i ? profile.board.chr : profile.board.prg);
					rom.resize( count );

					Ram& dst = (i ? chr : prg);
					dst.Set( size );

					if (!rom.empty())
					{
						for (Profile::Board::Pins::const_iterator it(rom.front().pins.begin()), end(rom.front().pins.end()); it != end; ++it)
							dst.Pin(it->number) = it->function.c_str();
					}

					size = 0;

					for (uint j=0, k=0; j < 16; ++j)
					{
						const Context::Rom& src = context.roms[i][j];

						if (src.data.Size())
						{
							rom[k].id = k;
							rom[k].size = src.data.Size();
							rom[k].hash.Assign( NULL, src.crc );
							k++;

							std::memcpy( dst.Mem(size), src.data.Mem(), src.data.Size() );
							size += src.data.Size();
						}
					}
				}
			}

			if (profileEx.nmt == ProfileEx::NMT_HORIZONTAL)
			{
				profile.board.solderPads = Profile::Board::SOLDERPAD_V;
			}
			else if (profileEx.nmt == ProfileEx::NMT_HORIZONTAL)
			{
				profile.board.solderPads = Profile::Board::SOLDERPAD_H;
			}

			switch (context.system)
			{
				case Context::SYSTEM_NTSC:

					if (favoredSystem == FAVORED_FAMICOM)
						profile.system.type = Profile::System::FAMICOM;
					else
						profile.system.type = Profile::System::NES_NTSC;
					break;

				default:

					profile.multiRegion = true;

					if (favoredSystem == FAVORED_FAMICOM)
					{
						profile.system.type = Profile::System::FAMICOM;
						break;
					}
					else if (favoredSystem != FAVORED_NES_PAL)
					{
						profile.system.type = Profile::System::NES_NTSC;
						break;
					}

				case Context::SYSTEM_PAL:

					profile.system.type = Profile::System::NES_PAL;
					profile.system.cpu = Profile::System::CPU_RP2A07;
					profile.system.ppu = Profile::System::PPU_RP2C07;
					break;
			}
		}

		cstring Cartridge::Unif::ChunkName(char (&name)[5],const dword id)
		{
			const byte bytes[] =
			{
				id >>  0 & 0xFF,
				id >>  8 & 0xFF,
				id >> 16 & 0xFF,
				id >> 24 & 0xFF,
				0
			};

			Stream::In::AsciiToC( name, bytes, 5 );

			return name;
		}

		dword Cartridge::Unif::ReadString(StdStream stream,cstring const logtext,Vector<char>* string)
		{
			Vector<char> tmp;

			if (string == NULL)
				string = &tmp;

			const dword count = Stream::In(stream).Read( *string );

			if (string->Size() > 1)
				Log() << logtext << string->Begin() << NST_LINEBREAK;

			return count;
		}

		dword Cartridge::Unif::ReadName(StdStream stream,std::wstring& name)
		{
			Vector<char> buffer;
			const dword length = ReadString( stream, "Unif: name: ", &buffer );

			if (length)
			{
				const std::wstring tmp( buffer.Begin(), buffer.End() );
				name = tmp;
			}

			return length;
		}

		dword Cartridge::Unif::ReadComment(StdStream stream)
		{
			return ReadString( stream, "Unif: comment: ", NULL );
		}

		dword Cartridge::Unif::ReadDumper(StdStream stdStream)
		{
			Stream::In stream( stdStream );

			struct Dumper
			{
				enum
				{
					NAME_LENGTH = 100,
					AGENT_LENGTH = 100,
					LENGTH = NAME_LENGTH + 4 + AGENT_LENGTH
				};

				char name[NAME_LENGTH];
				byte day;
				byte month;
				word year;
				char agent[AGENT_LENGTH];
			};

			Dumper dumper;

			stream.Read( dumper.name, Dumper::NAME_LENGTH );
			dumper.name[Dumper::NAME_LENGTH-1] = '\0';

			dumper.day   = stream.Read8();
			dumper.month = stream.Read8();
			dumper.year  = stream.Read16();

			stream.Read( dumper.agent, Dumper::AGENT_LENGTH );
			dumper.agent[Dumper::AGENT_LENGTH-1] = '\0';

			Log log;

			if (*dumper.name)
				log << "Unif: dumped by: " << dumper.name << NST_LINEBREAK;

			log << "Unif: dump year: "  << dumper.year << NST_LINEBREAK
                   "Unif: dump month: " << dumper.month << NST_LINEBREAK
                   "Unif: dump day: "   << dumper.day << NST_LINEBREAK;

			if (*dumper.agent)
				log << "Unif: dumper agent: " << dumper.agent << NST_LINEBREAK;

			return Dumper::LENGTH;
		}

		dword Cartridge::Unif::ReadSystem(StdStream stream,Context& context)
		{
			switch (Stream::In(stream).Read8())
			{
				case 0:

					context.system = Context::SYSTEM_NTSC;
					Log::Flush( "Unif: NTSC system" NST_LINEBREAK );
					break;

				case 1:

					context.system = Context::SYSTEM_PAL;
					Log::Flush( "Unif: PAL system" NST_LINEBREAK );
					break;

				default:

					context.system = Context::SYSTEM_BOTH;
					Log::Flush( "Unif: dual system" NST_LINEBREAK );
					break;
			}

			return 1;
		}

		dword Cartridge::Unif::ReadChecksum
		(
			StdStream stream,
			const uint type,
			const uint index,
			Context::Rom& rom
		)
		{
			NST_ASSERT( type < 2 && index < 16 );

			for (dword crc=Stream::In(stream).Read32(), i=0; i < 8; ++i)
			{
				uint c = crc >> (i*4) & 0xF;
				rom.crc[i] = (c < 0xA ? '0' + c : 'A' + (c - 0xA) );
			}

			Log() << "Unif: "
                  << (type ? "CHR-ROM " : "PRG-ROM ")
                  << char(index < 10 ? index + '0' : index-10 + 'A')
                  << " CRC: "
                  << rom.crc
                  << NST_LINEBREAK;

			return 4;
		}

		dword Cartridge::Unif::ReadRom
		(
			StdStream stream,
			const uint type,
			const uint index,
			dword length,
			Context::Rom* const roms
		)
		{
			NST_ASSERT( type < 2 && index < 16 );

			Log() << "Unif: "
                  << (type ? "CHR-ROM " : "PRG-ROM ")
                  << char(index < 10 ? index + '0' : index-10 + 'A')
                  << " size: "
                  << (length / SIZE_1K)
                  << "k" NST_LINEBREAK;

			dword available = 0;

			for (uint i=0; i < 16; ++i)
				available += roms[i].data.Size();

			available = MAX_ROM_SIZE - available;
			NST_VERIFY( length <= available );

			if (length > available)
			{
				roms[index].truncated = length - available;
				length = available;

				Log() << "Unif: warning, "
                      << (type ? "CHR-ROM " : "PRG-ROM ")
                      << char(index < 10 ? index + '0' : index-10 + 'A')
                      << " truncated to: "
                      << (length / SIZE_1K)
                      << "k" NST_LINEBREAK;
			}

			if (length)
			{
				roms[index].data.Set( length );
				Stream::In(stream).Read( roms[index].data.Mem(), length );
			}

			return length;
		}

		dword Cartridge::Unif::ReadBoard(StdStream stream,Profile::Board& board)
		{
			Vector<char> buffer;
			const dword length = ReadString( stream, "Unif: board: ", &buffer );

			if (length && *buffer.Begin())
				board.type.assign( buffer.Begin(), buffer.End() );

			return length;
		}

		dword Cartridge::Unif::ReadBattery(ProfileEx& profileEx)
		{
			profileEx.battery = true;
			Log::Flush( "Unif: battery present" NST_LINEBREAK );
			return 0;
		}

		dword Cartridge::Unif::ReadMirroring(StdStream stream,ProfileEx& profileEx)
		{
			switch (Stream::In(stream).Read8())
			{
				case 0: profileEx.nmt = ProfileEx::NMT_HORIZONTAL;   Log::Flush( "Unif: horizontal mirroring"        NST_LINEBREAK ); break;
				case 1: profileEx.nmt = ProfileEx::NMT_VERTICAL;     Log::Flush( "Unif: vertical mirroring"          NST_LINEBREAK ); break;
				case 2:
				case 3: profileEx.nmt = ProfileEx::NMT_SINGLESCREEN; Log::Flush( "Unif: single-screen mirroring"     NST_LINEBREAK ); break;
				case 4: profileEx.nmt = ProfileEx::NMT_FOURSCREEN;   Log::Flush( "Unif: four-screen mirroring"       NST_LINEBREAK ); break;
				case 5: profileEx.nmt = ProfileEx::NMT_CONTROLLED;   Log::Flush( "Unif: mapper controlled mirroring" NST_LINEBREAK ); break;
			}

			return 1;
		}

		dword Cartridge::Unif::ReadController(StdStream stream,Api::Input::Type (&controllers)[5])
		{
			Log log;

			log << "Unif: controllers: ";

			const uint controller = Stream::In(stream).Read8();
			NST_VERIFY( !(controller & (0x40|0x80)) );

			if (controller & (0x1|0x2|0x4|0x8|0x10|0x20))
			{
				if (controller & 0x01)
				{
					controllers[0] = Api::Input::PAD1;
					controllers[1] = Api::Input::PAD2;

					log << "standard joypad";
				}

				if (controller & 0x02)
				{
					controllers[1] = Api::Input::ZAPPER;

					cstring const zapper = ", zapper";
					log << (zapper + ((controller & 0x1) ? 0 : 2));
				}

				if (controller & 0x04)
				{
					controllers[1] = Api::Input::ROB;

					cstring const rob = ", R.O.B";
					log << (rob + ((controller & (0x1|0x2)) ? 0 : 2));
				}

				if (controller & 0x08)
				{
					controllers[0] = Api::Input::PADDLE;

					cstring const paddle = ", paddle";
					log << (paddle + ((controller & (0x1|0x2|0x4)) ? 0 : 2));
				}

				if (controller & 0x10)
				{
					controllers[1] = Api::Input::POWERPAD;

					cstring const powerpad = ", power pad";
					log << (powerpad + ((controller & (0x1|0x2|0x4|0x8)) ? 0 : 2));
				}

				if (controller & 0x20)
				{
					controllers[2] = Api::Input::PAD3;
					controllers[3] = Api::Input::PAD4;

					cstring const fourplayer = ", four player adapter";
					log << (fourplayer + ((controller & (0x1|0x2|0x4|0x8|0x10)) ? 0 : 2));
				}

				log << NST_LINEBREAK;
			}
			else
			{
				log << ((controller & (0x40|0x80)) ? "unknown" NST_LINEBREAK : "unspecified" NST_LINEBREAK);
			}

			return 1;
		}

		dword Cartridge::Unif::ReadChrRam()
		{
			Log::Flush( "Unif: CHR is writable" NST_LINEBREAK );
			return 0;
		}

		dword Cartridge::Unif::ReadUnknown(dword id)
		{
			NST_DEBUG_MSG("unknown unif chunk");

			char name[5];
			Log() << "Unif: warning, skipping unknown chunk: \"" << ChunkName(name,id) << "\"" NST_LINEBREAK;

			return 0;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
