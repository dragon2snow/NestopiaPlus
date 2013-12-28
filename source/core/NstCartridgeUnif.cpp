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

#include <cstdlib>
#include <cstring>
#include <algorithm>
#include "NstLog.hpp"
#include "NstCrc32.hpp"
#include "NstImageDatabase.hpp"
#include "NstCartridge.hpp"
#include "NstMapper.hpp"
#include "NstCartridgeUnif.hpp"
#include "api/NstApiUser.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		class Cartridge::Unif::Boards
		{
			Boards();

			struct Board
			{
				bool operator < (const Board&) const;
				bool operator == (const Board&) const;

				cstring name;
				word mapper;
				word wrkRam;
			};

			static Board table[];

		public:

			static bool Set(cstring,Info&);
		};

		Cartridge::Unif::Boards::Board Cartridge::Unif::Boards::table[] =
		{
			{"NROM",                   0,0},
			{"NROM-128",               0,0},
			{"NROM-256",               0,0},
			{"RROM",                   0,0},
			{"RROM-128",               0,0},
			{"SAROM",                  1,SIZE_8K},
			{"SBROM",                  1,0},
			{"SCROM",                  1,0},
			{"SC1ROM",                 1,0},
			{"SCEOROM",                1,0},
			{"SEROM",                  1,0},
			{"SFROM",                  1,0},
			{"SF1ROM",                 1,0},
			{"SFEOROM",                1,0},
			{"SFEXPROM",               1,0},
			{"SGROM",                  1,0},
			{"SHROM",                  1,0},
			{"SH1ROM",                 1,0},
			{"SJROM",                  1,SIZE_8K},
			{"SKROM",                  1,SIZE_8K},
			{"SLROM",                  1,0},
			{"SL1ROM",                 1,0},
			{"SL2ROM",                 1,0},
			{"SL3ROM",                 1,0},
			{"SLRROM",                 1,0},
			{"SL1RROM",                1,0},
			{"SNROM",                  1,SIZE_8K},
			{"SN1ROM",                 1,SIZE_8K},
			{"SOROM",                  1,SIZE_16K},
			{"SUROM",                  1,SIZE_8K},
			{"SVROM",                  1,SIZE_16K},
			{"SXROM",                  1,SIZE_32K},
			{"UNROM",                  2,0},
			{"UOROM",                  2,0},
			{"CNROM",                  3,0},
			{"TEROM",                  4,0},
			{"TFROM",                  4,0},
			{"TGROM",                  4,0},
			{"TKROM",                  4,SIZE_8K},
			{"TLROM",                  4,0},
			{"TL1ROM",                 4,0},
			{"TLSROM",                 4,0},
			{"TR1ROM",                 4,0},
			{"TSROM",                  4,SIZE_8K},
			{"TVROM",                  4,0},
			{"HKROM",                  4,SIZE_1K},
			{"B4",                     4,0},
			{"EKROM",                  5,SIZE_8K},
			{"ELROM",                  5,0},
			{"ETROM",                  5,SIZE_16K},
			{"EWROM",                  5,SIZE_32K},
			{"AMROM",                  7,0},
			{"ANROM",                  7,0},
			{"AOROM",                  7,0},
			{"PNROM",                  9,0},
			{"PEEOROM",                9,0},
			{"FJROM",                  10,0},
			{"CDREAMS",                11,0},
			{"AVENINA-07",             11,0},
			{"CPROM",                  13,0},
			{"SL1632",                 14,0},
			{"BNROM",                  34,0},
			{"AVENINA-01",             34,SIZE_8K},
			{"1991SUPERHIK7IN1",       45,0},
			{"SUPERHIK8IN1",           45,0},
			{"QJ",                     47,0},
			{"1992BALLGAMES11IN1",     51,0},
			{"MARIO7IN1",              52,0},
			{"SUPERVISION16IN1",       53,0},
			{"NOVELDIAMOND9999999IN1", 54,0},
			{"MARIO1-MALEE2",          55,0},
			{"STUDYGAME32IN1",         58,0},
			{"RESET4IN1",              60,0},
			{"D1038",                  60,0},
			{"TEN800032",              64,0},
			{"GNROM",                  66,0},
			{"MHROM",                  66,0},
			{"NTBROM",                 68,0},
			{"TEN800042",              68,0},
			{"BTR",                    69,SIZE_8K},
			{"CAMBF9093",              71,0},
			{"CAMALADDINNORMAL",       71,0},
			{"AVENINA-03",             79,0},
			{"AVENINA-06",             79,0},
			{"TEK90",                  90,0},
			{"TEN800037",              118,0},
			{"TLSROM",                 118,0},
			{"TKSROM",                 118,SIZE_8K},
			{"TQROM",                  119,0},
			{"H2288",                  123,0},
			{"22211",                  132,0},
			{"SA-72008",               133,0},
			{"SACHEN-8259D",           137,0},
			{"SACHEN-8259B",           138,0},
			{"SACHEN-8259C",           139,0},
			{"SACHEN-8259A",           141,0},
			{"SA-NROM",                143,0},
			{"SA-72007",               145,0},
			{"SA-016-1M",              146,0},
			{"TC-U01-1.5M",            147,0},
			{"SA-0037",                148,0},
			{"SA-0036",                149,0},
			{"SACHEN-74LS374N",        150,0},
			{"DEROM",                  206,0},
			{"DE1ROM",                 206,0},
			{"DRROM",                  206,0},
			{"TEN800030",              206,0},
			{"TEN800002",              206,0},
			{"TEN800004",              206,0},
			{"CAMBF9096",              232,0},
			{"CAMALADDINQUATTRO",      232,0},
			{"42IN1RESETSWITCH",       233,0},
			{"GOLDENGAME150IN1",       235,0},
			{"70IN1",                  236,0},
			{"70IN1B",                 236,0},
			{"SUPER24IN1SC03",         Mapper::EXT_SUPER24IN1,SIZE_8K},
			{"8157",                   Mapper::EXT_8157,0},
			{"8237",                   Mapper::EXT_8237,0},
			{"WS",                     Mapper::EXT_WS,0},
			{"DREAMTECH01",            Mapper::EXT_DREAMTECH01,0},
			{"CC-21",                  Mapper::EXT_CC21,0},
			{"KOF97",                  Mapper::EXT_KOF97,0},
			{"64IN1NOREPEAT",          Mapper::EXT_64IN1NR,0},
			{"SHERO",                  Mapper::EXT_STREETHEROES,0},
			{"T-262",                  Mapper::EXT_T262,0},
			{"FK23C",                  Mapper::EXT_FK23C,0},
			{"603-5052",               Mapper::EXT_6035052,0},
			{"A65AS",                  Mapper::EXT_A65AS,0},
			{"EDU2000",                Mapper::EXT_EDU2000,SIZE_32K},
			{"TF1201",                 Mapper::EXT_TF1201,0},
			{"KS7032",                 Mapper::EXT_KS7032,0},
			{"GS-2004",                Mapper::EXT_GS2004,0}
		};

		bool Cartridge::Unif::Boards::Board::operator < (const Board& board) const
		{
			return std::strcmp( name, board.name ) < 0;
		}

		bool Cartridge::Unif::Boards::Board::operator == (const Board& board) const
		{
			return std::strcmp( name, board.name ) == 0;
		}

		Cartridge::Unif::Boards::Boards()
		{
			std::sort( table, table + sizeof(array(table)) );
		}

		bool Cartridge::Unif::Boards::Set(cstring name,Info& info)
		{
			static Boards instance;

			const Board board = { name, 0, 0 };
			const Board* const result = std::lower_bound( table, table + sizeof(array(table)), board );

			if (result != table + sizeof(array(table)) && *result == board)
			{
				info.setup.mapper = result->mapper;
				info.setup.wrkRam = result->wrkRam;

				return true;
			}

			return false;
		}

		Cartridge::Unif::Rom::Rom()
		: crc(0), truncated(0) {}

		Cartridge::Unif::Unif
		(
			StdStream s,
			Ram& p,
			Ram& c,
			Ram& wrk,
			Api::Cartridge::Info& i,
			const ImageDatabase* const database,
			ImageDatabase::Handle& databaseHandle
		)
		:
		stream     (s),
		prg        (p),
		chr        (c),
		info       (i),
		knownBoard (false),
		result     (RESULT_OK)
		{
			info.Clear();

			ReadHeader();
			ReadChunks();

			CopyRom();

			info.setup.prgRom = prg.Size();
			info.setup.chrRom = chr.Size();

			if (info.setup.wrkRamBacked && info.setup.wrkRam)
			{
				info.setup.wrkRamBacked = info.setup.wrkRam;
				info.setup.wrkRam = 0;
			}

			CheckImageDatabase( database, databaseHandle );

			wrk.Set( info.setup.wrkRam + info.setup.wrkRamBacked );
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

		bool Cartridge::Unif::NewChunk(byte& processed,const dword id)
		{
			NST_VERIFY( !processed );

			if (!processed)
			{
				processed = true;
				return true;
			}
			else
			{
				char name[5];
				Log() << "Unif: warning, duplicate chunk: \"" << ChunkName(name,id) << "\" ignored" NST_LINEBREAK;
				return false;
			}
		}

		void Cartridge::Unif::ReadHeader()
		{
			if (stream.Read32() != AsciiId<'U','N','I','F'>::V)
				throw RESULT_ERR_INVALID_FILE;

			Log() << "Unif: revision " << stream.Read32() << NST_LINEBREAK;

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

		void Cartridge::Unif::ReadChunks()
		{
			byte chunks[80];
			std::memset( chunks, 0, sizeof(chunks) );

			while (!stream.Eof())
			{
				dword id = stream.Read32();
				const dword length = stream.Read32();
				NST_VERIFY( length <= SIZE_1K * 4096UL );

				switch (id)
				{
					case AsciiId<'N','A','M','E'>::V: id = (NewChunk( chunks[0], id ) ? ReadName()       : 0); break;
					case AsciiId<'R','E','A','D'>::V: id = (NewChunk( chunks[1], id ) ? ReadComment()    : 0); break;
					case AsciiId<'D','I','N','F'>::V: id = (NewChunk( chunks[2], id ) ? ReadDumper()     : 0); break;
					case AsciiId<'T','V','C','I'>::V: id = (NewChunk( chunks[3], id ) ? ReadSystem()     : 0); break;
					case AsciiId<'B','A','T','R'>::V: id = (NewChunk( chunks[4], id ) ? ReadBattery()    : 0); break;
					case AsciiId<'M','A','P','R'>::V: id = (NewChunk( chunks[5], id ) ? ReadMapper()     : 0); break;
					case AsciiId<'M','I','R','R'>::V: id = (NewChunk( chunks[6], id ) ? ReadMirroring()  : 0); break;
					case AsciiId<'C','T','R','L'>::V: id = (NewChunk( chunks[7], id ) ? ReadController() : 0); break;
					case AsciiId<'V','R','O','R'>::V: id = (NewChunk( chunks[8], id ) ? ReadChrRam()     : 0); break;

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
							else if (index >= Ascii<'a'>::V && index <= Ascii<'f'>::V)
							{
								index = index - Ascii<'a'>::V + 10;
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
									case AsciiId<'P','C','K'>::V: id = (NewChunk( chunks[9+0+index],  id ) ? ReadRomCrc  ( 0, index         ) : 0); break;
									case AsciiId<'C','C','K'>::V: id = (NewChunk( chunks[9+16+index], id ) ? ReadRomCrc  ( 1, index         ) : 0); break;
									case AsciiId<'P','R','G'>::V: id = (NewChunk( chunks[9+32+index], id ) ? ReadRomData ( 0, index, length ) : 0); break;
									case AsciiId<'C','H','R'>::V: id = (NewChunk( chunks[9+48+index], id ) ? ReadRomData ( 1, index, length ) : 0); break;
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
		}

		dword Cartridge::Unif::ReadString(cstring const logtext,Vector<char>* string=NULL)
		{
			Vector<char> tmp;

			if (string == NULL)
				string = &tmp;

			const dword count = stream.Read( *string );

			if (string->Size() > 1)
				Log() << logtext << string->Begin() << NST_LINEBREAK;

			return count;
		}

		dword Cartridge::Unif::ReadName()
		{
			return ReadString("Unif: name: ");
		}

		dword Cartridge::Unif::ReadComment()
		{
			return ReadString("Unif: comment: ");
		}

		dword Cartridge::Unif::ReadDumper()
		{
			Dump dump;

			stream.Read( dump.name, Dump::NAME_LENGTH );
			dump.name[Dump::NAME_LENGTH-1] = '\0';

			dump.day   = stream.Read8();
			dump.month = stream.Read8();
			dump.year  = stream.Read16();

			stream.Read( dump.agent, Dump::AGENT_LENGTH );
			dump.agent[Dump::AGENT_LENGTH-1] = '\0';

			Log log;

			if (*dump.name)
				log << "Unif: dumped by: " << dump.name << NST_LINEBREAK;

			log << "Unif: dump year: "  << dump.year << NST_LINEBREAK
                   "Unif: dump month: " << dump.month << NST_LINEBREAK
                   "Unif: dump day: "   << dump.day << NST_LINEBREAK;

			if (*dump.agent)
				log << "Unif: dumper agent: " << dump.agent << NST_LINEBREAK;

			return Dump::LENGTH;
		}

		dword Cartridge::Unif::ReadSystem()
		{
			switch (stream.Read8())
			{
				case 0:  info.setup.region = REGION_NTSC;  Log::Flush( "Unif: NTSC system"     NST_LINEBREAK ); break;
				case 1:  info.setup.region = REGION_PAL;   Log::Flush( "Unif: PAL system"      NST_LINEBREAK ); break;
				default: info.setup.region = REGION_BOTH;  Log::Flush( "Unif: NTSC/PAL system" NST_LINEBREAK ); break;
			}

			return 1;
		}

		dword Cartridge::Unif::ReadRomCrc(const uint type,const uint index)
		{
			NST_ASSERT( type < 2 && index < 16 );

			roms[type][index].crc = stream.Read32();

			Log() << "Unif: "
                  << (type ? "CHR-ROM " : "PRG-ROM ")
                  << char(index < 10 ? index + '0' : index-10 + 'A')
                  << " CRC: "
                  << Log::Hex( 32, roms[type][index].crc )
                  << NST_LINEBREAK;

			return 4;
		}

		dword Cartridge::Unif::ReadRomData(const uint type,const uint index,dword length)
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
				available += roms[type][i].rom.Size();

			available = MAX_ROM_SIZE - available;
			NST_VERIFY( length <= available );

			if (length > available)
			{
				roms[type][index].truncated = length - available;
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
				roms[type][index].rom.Set( length );
				stream.Read( roms[type][index].rom.Mem(), length );
			}

			return length;
		}

		dword Cartridge::Unif::ReadMapper()
		{
			Vector<char> buffer;
			const dword length = ReadString( "Unif: board: ", &buffer );

			if (buffer.Size() > 1)
			{
				for (char* it=buffer.Begin(); *it; ++it)
				{
					if (*it >= 'a' && *it <= 'z')
						*it = *it - 'a' + 'A';
				}

				char* board = buffer.Begin();

				if (buffer.Size() > 1+4)
				{
					static const char types[][4] =
					{
						{'N','E','S','-'},
						{'U','N','L','-'},
						{'H','V','C','-'},
						{'B','T','L','-'},
						{'B','M','C','-'}
					};

					for (uint i=0; i < sizeof(array(types)); ++i)
					{
						if (board[0] == types[i][0] && board[1] == types[i][1] && board[2] == types[i][2] && board[3] == types[i][3])
						{
							board += 4;
							break;
						}
					}
				}

				knownBoard = Boards::Set( board, info );
				info.board.assign( board, buffer.End() );
			}

			return length;
		}

		dword Cartridge::Unif::ReadBattery()
		{
			info.setup.wrkRamBacked = SIZE_8K;
			Log::Flush( "Unif: battery present" NST_LINEBREAK );
			return 0;
		}

		dword Cartridge::Unif::ReadMirroring()
		{
			switch (stream.Read8())
			{
				case 0:  info.setup.mirroring = Api::Cartridge::MIRROR_HORIZONTAL; Log::Flush( "Unif: horizontal mirroring"        NST_LINEBREAK ); break;
				case 1:  info.setup.mirroring = Api::Cartridge::MIRROR_VERTICAL;   Log::Flush( "Unif: vertical mirroring"          NST_LINEBREAK ); break;
				case 2:  info.setup.mirroring = Api::Cartridge::MIRROR_ZERO;       Log::Flush( "Unif: zero mirroring"              NST_LINEBREAK ); break;
				case 3:  info.setup.mirroring = Api::Cartridge::MIRROR_ONE;        Log::Flush( "Unif: one mirroring"               NST_LINEBREAK ); break;
				case 4:  info.setup.mirroring = Api::Cartridge::MIRROR_FOURSCREEN; Log::Flush( "Unif: four-screen mirroring"       NST_LINEBREAK ); break;
				default: info.setup.mirroring = Api::Cartridge::MIRROR_CONTROLLED; Log::Flush( "Unif: mapper controlled mirroring" NST_LINEBREAK ); break;
			}

			return 1;
		}

		dword Cartridge::Unif::ReadController()
		{
			Log log;
			log << "Unif: controllers: ";

			const uint controller = stream.Read8();
			NST_VERIFY( !(controller & (0x40|0x80)) );

			if (controller & (0x1|0x2|0x4|0x8|0x10|0x20))
			{
				if (controller & 0x01)
				{
					info.controllers[0] = Api::Input::PAD1;
					info.controllers[1] = Api::Input::PAD2;
					log << "standard joypad";
				}

				if (controller & 0x02)
				{
					info.controllers[1] = Api::Input::ZAPPER;

					cstring const zapper = ", zapper";
					log << (zapper + (controller & 0x1) ? 0 : 2);
				}

				if (controller & 0x04)
				{
					info.controllers[1] = Api::Input::ROB;

					cstring const rob = ", R.O.B";
					log << (rob + (controller & (0x1|0x2)) ? 0 : 2);
				}

				if (controller & 0x08)
				{
					info.controllers[0] = Api::Input::PADDLE;

					cstring const paddle = ", paddle";
					log << (paddle + (controller & (0x1|0x2|0x4)) ? 0 : 2);
				}

				if (controller & 0x10)
				{
					info.controllers[1] = Api::Input::POWERPAD;

					cstring const powerpad = ", power pad";
					log << (powerpad + (controller & (0x1|0x2|0x4|0x8)) ? 0 : 2);
				}

				if (controller & 0x20)
				{
					info.controllers[2] = Api::Input::PAD3;
					info.controllers[3] = Api::Input::PAD4;

					cstring const fourplayer = ", four player adapter";
					log << (fourplayer + (controller & (0x1|0x2|0x4|0x8|0x10)) ? 0 : 2);
				}

				log << NST_LINEBREAK;
			}
			else
			{
				log << ((controller & (0x40|0x80)) ? "unknown" NST_LINEBREAK : "unspecified" NST_LINEBREAK);
			}

			return 1;
		}

		dword Cartridge::Unif::ReadChrRam() const
		{
			Log::Flush( "Unif: CHR is writable" NST_LINEBREAK );
			return 0;
		}

		dword Cartridge::Unif::ReadUnknown(dword id) const
		{
			char name[5];
			Log() << "Unif: warning, skipping unknown chunk: \"" << ChunkName(name,id) << "\"" NST_LINEBREAK;
			NST_DEBUG_MSG("unknown unif chunk");
			return 0;
		}

		void Cartridge::Unif::CopyRom()
		{
			for (uint i=2; i--; )
			{
				Ram& dst = (i ? chr : prg);
				cstring const type = (i ? "Unif: CHR-ROM " : "Unif: PRG-ROM ");

				dst.Destroy();

				for (uint j=0; j < 16; ++j)
				{
					Rom& src = roms[i][j];

					if (const dword size = src.rom.Size())
					{
						if (src.crc && !src.truncated)
						{
							cstring msg;

							if (src.crc == Crc32::Compute( src.rom.Mem(), size ))
							{
								msg = " CRC check ok" NST_LINEBREAK;
							}
							else
							{
								msg = " CRC check failed" NST_LINEBREAK;
								info.condition = Api::Cartridge::DUMP_BAD;
								result = (i ? RESULT_WARN_BAD_CROM : RESULT_WARN_BAD_PROM);
							}

							Log() << type << j << msg;
						}

						const dword pos = dst.Size();
						dst.Set( pos + size );
						std::memcpy( dst.Mem(pos), src.rom.Mem(), size );
						src.rom.Destroy();
					}
				}
			}

			if (prg.Empty())
				throw RESULT_ERR_CORRUPT_FILE;
		}

		void Cartridge::Unif::CheckImageDatabase(const ImageDatabase* const database,ImageDatabase::Handle& databaseHandle)
		{
			if (database)
			{
				dword crc = Crc32::Compute( prg.Mem(), prg.Size() );

				if (chr.Size())
					crc = Crc32::Compute( chr.Mem(), chr.Size(), crc );

				if (const ImageDatabase::Handle handle = database->Search( crc ))
				{
					databaseHandle = handle;

					if (!knownBoard)
						info.setup.mapper = database->Mapper(handle);

					switch (database->GetSystem(handle))
					{
						case SYSTEM_VS:

							info.setup.system = SYSTEM_VS;
							info.setup.ppu = PPU_RP2C03B;
							break;

						case SYSTEM_PC10:

							info.setup.system = SYSTEM_PC10;
							info.setup.ppu = PPU_RP2C03B;
							break;
					}

					if (database->Enabled())
					{
						info.setup.mapper = database->Mapper(handle);

						if (const dword wrkRam = database->WrkRam(handle))
							info.setup.wrkRam = wrkRam;

						if (const dword wrkRamBacked = database->WrkRamBacked(handle))
							info.setup.wrkRamBacked = wrkRamBacked;
					}

					return;
				}
			}

			databaseHandle = NULL;

			if (!knownBoard)
			{
				std::string choice;

				Api::User::inputCallback
				(
					Api::User::INPUT_CHOOSE_MAPPER,
					info.board.empty() ? "unknown" : info.board.c_str(),
					choice
				);

				if (choice.length() > 0 && choice.length() < 4)
				{
					const int id = std::atoi( choice.c_str() );

					if (id >= 0 && id <= 255)
					{
						info.setup.mapper = id;
						return;
					}
				}

				throw RESULT_ERR_UNSUPPORTED_MAPPER;
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
