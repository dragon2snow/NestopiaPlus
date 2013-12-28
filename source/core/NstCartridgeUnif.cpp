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
#include <algorithm>
#include "NstStream.hpp"
#include "NstLog.hpp"
#include "NstChecksumCrc32.hpp"
#include "NstImageDatabase.hpp"
#include "NstCartridge.hpp"
#include "NstCartridgeUnif.hpp"
#include "NstMapper.hpp"
#include "api/NstApiUser.hpp"

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("s", on)
#endif

namespace Nes
{
	namespace Core
	{
		const char Cartridge::Unif::Rom::id[] =
		{
			'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
		};

		bool Cartridge::Unif::sorted = false;

		Cartridge::Unif::Board Cartridge::Unif::boards[] =
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
			{"EDU2000",                Mapper::EXT_EDU2000,SIZE_32K}
		};

		bool Cartridge::Unif::Board::operator < (const Board& board) const
		{
			return std::strcmp( name, board.name ) < 0;
		}

		Cartridge::Unif::Rom::Rom()
		: crc(0) {}

		Cartridge::Unif::Unif
		(
			StdStream s,
			Ram& p,
			Ram& c,
			Ram& w,
			Api::Cartridge::Info& i,
			const ImageDatabase* const r,
			ImageDatabase::Handle& h
		)
		:
		stream         (s),
		prg            (p),
		chr            (c),
		wrk            (w),
		info           (i),
		database       (r),
		databaseHandle (h),
		crc            (0),
		result         (RESULT_OK)
		{
			if (!sorted)
			{
				sorted = true;
				std::sort( boards, boards + NST_COUNT(boards) );
			}

			info.Clear();
			Import();
		}

		dword Cartridge::Unif::ComputeCrc() const
		{
			dword crc = Checksum::Crc32::Compute( prg.Mem(), prg.Size() );

			if (chr.Size())
				crc = Checksum::Crc32::Compute( chr.Mem(), chr.Size(), crc );

			return crc;
		}

		bool Cartridge::Unif::NewChunk(bool& index)
		{
			if (index)
			{
				log << "Unif: duplicate chunk, ignoring.." NST_LINEBREAK;
				return false;
			}

			index = true;
			return true;
		}

		#define NES_ID4(a_,b_,c_,d_) u32( (a_) | ((b_) << 8) | ((c_) << 16) | ((d_) << 24) )
		#define NES_ID3(a_,b_,c_)    u32( (a_) | ((b_) << 8) | ((c_) << 16) )

		void Cartridge::Unif::Import()
		{
			stream.Validate( NES_ID4('U','N','I','F') );

			log << "Unif: revision " << stream.Read32() << NST_LINEBREAK;

			stream.Seek( HEADER_RESERVED_LENGTH );

			info.setup.mapper = NO_MAPPER;

			bool chunks[9] = {false};

			while (!stream.Eof())
			{
				ulong id = stream.Read32();
				const ulong length = stream.Read32();

				switch (id)
				{
					case NES_ID4('N','A','M','E'): if (NewChunk( chunks[0] )) id = ReadName       (); break;
					case NES_ID4('R','E','A','D'): if (NewChunk( chunks[1] )) id = ReadComment    (); break;
					case NES_ID4('D','I','N','F'): if (NewChunk( chunks[2] )) id = ReadDumper     (); break;
					case NES_ID4('T','V','C','I'): if (NewChunk( chunks[3] )) id = ReadSystem     (); break;
					case NES_ID4('B','A','T','R'): if (NewChunk( chunks[4] )) id = ReadBattery    (); break;
					case NES_ID4('M','A','P','R'): if (NewChunk( chunks[5] )) id = ReadMapper     (); break;
					case NES_ID4('M','I','R','R'): if (NewChunk( chunks[6] )) id = ReadMirroring  (); break;
					case NES_ID4('C','T','R','L'): if (NewChunk( chunks[7] )) id = ReadController (); break;
					case NES_ID4('V','R','O','R'): if (NewChunk( chunks[8] )) id = ReadChrRam     (); break;

					default:
					{
						const uint id4 = id >> 24;

						switch (id & 0x00FFFFFFUL)
						{
							case NES_ID3('P','C','K'): id = ReadRomCrc  ( 0, id4         ); break;
							case NES_ID3('C','C','K'): id = ReadRomCrc  ( 1, id4         ); break;
							case NES_ID3('P','R','G'): id = ReadRomData ( 0, id4, length ); break;
							case NES_ID3('C','H','R'): id = ReadRomData ( 1, id4, length ); break;
							default: id = 0; break;
						}
					}
				}

				if (id == ULONG_MAX || id > length)
					throw RESULT_ERR_CORRUPT_FILE;

				if (length != id)
					stream.Seek( length - id );
			}

			CopyRom();

			if (info.setup.wrkRamBacked && info.setup.wrkRam)
			{
				info.setup.wrkRamBacked = info.setup.wrkRam;
				info.setup.wrkRam = 0;
			}

			info.setup.ppu = PPU_RP2C02;
			info.setup.prgRom = prg.Size();
			info.setup.chrRom = chr.Size();

			databaseHandle = NULL;

			if (database)
				CheckImageDatabase();

			if (!CheckMapper())
				throw RESULT_ERR_UNSUPPORTED_MAPPER;

			wrk.Set( info.setup.wrkRam + info.setup.wrkRamBacked );
		}

		void Cartridge::Unif::CopyRom()
		{
			for (uint i=2; i--; )
			{
				Ram& dst = (i ? chr : prg);
				cstring const type = (i ? "Unif: CHR-ROM " : "Unif: PRG-ROM ");

				dst.Destroy();

				for (uint j=0; j < sizeof(Rom::id); ++j)
				{
					Rom& src = roms[i][j];

					if (const ulong size = src.rom.Size())
					{
						if (src.crc)
						{
							cstring msg;

							if (src.crc == Checksum::Crc32::Compute( src.rom.Mem(), size ))
							{
								msg = " CRC check ok" NST_LINEBREAK;
							}
							else
							{
								msg = " CRC check failed" NST_LINEBREAK;
								info.condition = Api::Cartridge::DUMP_BAD;
								result = (i ? RESULT_WARN_BAD_CROM : RESULT_WARN_BAD_PROM);
							}

							log << type << j << msg;
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

		bool Cartridge::Unif::CheckMapper()
		{
			if (info.setup.mapper != NO_MAPPER)
				return true;

			if (database)
			{
				if (crc == 0)
					crc = ComputeCrc();

				if (ImageDatabase::Handle handle = database->Search( crc ))
				{
					info.setup.mapper = database->Mapper( handle );
					return true;
				}
			}

			std::string choice;

			Api::User::inputCallback
			(
				Api::User::INPUT_CHOOSE_MAPPER,
				(info.board.empty() ? "unknown" : info.board.c_str()),
				choice
			);

			if (!choice.empty() && choice.length() <= 3)
			{
				const int id = std::atoi( choice.c_str() );

				if (id >= 0 && id <= 255)
				{
					info.setup.mapper = id;
					return true;
				}
			}

			return false;
		}

		ulong Cartridge::Unif::ReadName()
		{
			std::string text;
			return ReadString( "Unif: name: ", text ) + 1;
		}

		ulong Cartridge::Unif::ReadComment()
		{
			std::string text;
			return ReadString( "Unif: comments: ", text ) + 1;
		}

		ulong Cartridge::Unif::ReadString(cstring const logtext,std::string& text)
		{
			if (const size_t length = stream.ReadString( &text ))
			{
				log << logtext << text.c_str() << NST_LINEBREAK;
				return length;
			}

			return 0;
		}

		ulong Cartridge::Unif::ReadDumper()
		{
			Dump dump;

			stream.Read( dump.name, Dump::NAME_LENGTH );

			dump.day   = stream.Read8();
			dump.month = stream.Read8();
			dump.year  = stream.Read16();

			stream.Read( dump.agent, Dump::AGENT_LENGTH );

			if (*dump.name)
				log << "Unif: dumper name: " << dump.name << NST_LINEBREAK;

			log << "Unif: dump year: "  << dump.year << NST_LINEBREAK
                   "Unif: dump month: " << dump.month << NST_LINEBREAK
                   "Unif: dump day: "   << dump.day << NST_LINEBREAK;

			if (*dump.agent)
				log << "Unif: dumper agent: " << dump.agent << NST_LINEBREAK;

			return Dump::LENGTH;
		}

		ulong Cartridge::Unif::ReadSystem()
		{
			cstring msg;

			switch (stream.Read8())
			{
				case 0:  info.setup.region = REGION_NTSC;  msg = "Unif: NTSC system"     NST_LINEBREAK; break;
				case 1:  info.setup.region = REGION_PAL;   msg = "Unif: PAL system"      NST_LINEBREAK; break;
				default: info.setup.region = REGION_BOTH;  msg = "Unif: NTSC/PAL system" NST_LINEBREAK; break;
			}

			log << msg;

			return 1;
		}

		ulong Cartridge::Unif::ReadRomCrc(const uint type,const uint id)
		{
			NST_ASSERT( type <= 1 );

			for (uint i=0; i < sizeof(Rom::id); ++i)
			{
				if (char(id) == Rom::id[i])
				{
					roms[type][i].crc = stream.Read32();

					log << (type ? "Unif: CHR-ROM " : "Unif: PRG-ROM ")
						<< char(id)
						<< " crc - "
						<< Log::Hex( (u32) roms[type][i].crc )
						<< NST_LINEBREAK;

					return 4;
				}
			}

			return 0;
		}

		ulong Cartridge::Unif::ReadRomData(const uint type,const uint id,const ulong length)
		{
			NST_ASSERT( type <= 1 );

			for (uint i=0; i < sizeof(Rom::id); ++i)
			{
				if (char(id) == Rom::id[i])
				{
					cstring const name = (type ? "Unif: CHR-ROM " : "Unif: PRG-ROM ");
					log << name << char(id) << " data, " << (length / SIZE_1K) << "k" NST_LINEBREAK;

					Ram& rom = roms[type][i].rom;

					if (rom.Size())
					{
						log << "Unif: duplicate chunk, ";

						if (!length)
						{
							log << "length is zero! keeping old data.." NST_LINEBREAK;
							return 0;
						}

						log << "refreshing data.." NST_LINEBREAK;
						rom.Destroy();
					}
					else if (!length)
					{
						return 0;
					}

					rom.Set( length );
					stream.Read( rom.Mem(), length );

					return length;
				}
			}

			return 0;
		}

		ulong Cartridge::Unif::ReadMapper()
		{
			const ulong length = ReadString( "Unif: board: ", info.board );

			if (length)
			{
				for (std::string::iterator it(info.board.begin()); it != info.board.end(); ++it)
				{
					if (*it >= 0x61 && *it <= 0x7A)
						*it -= 0x20;
				}

				if (length > 4)
				{
					static const ulong types[5] =
					{
						NES_ID4('N','E','S','-'),
						NES_ID4('U','N','L','-'),
						NES_ID4('H','V','C','-'),
						NES_ID4('B','T','L','-'),
						NES_ID4('B','M','C','-')
					};

					if (std::find( types, types + 5, NES_ID4(info.board[0],info.board[1],info.board[2],info.board[3]) ) != types + 5)
						info.board.erase( 0, 4 );
				}

				{
					const Board* begin = boards;
					const Board* const end = boards + NST_COUNT(boards);
					const Board board = { info.board.c_str(), 0 };

					begin = std::lower_bound( begin, end, board );

					if (begin != end && info.board == begin->name)
					{
						info.setup.mapper = begin->mapper;
						info.setup.wrkRam = begin->wrkRam;
					}
				}
			}

			return length + 1;
		}

		#undef NES_ID3
		#undef NES_ID4

		ulong Cartridge::Unif::ReadBattery()
		{
			info.setup.wrkRamBacked = SIZE_8K;
			log << "Unif: battery present" NST_LINEBREAK;
			return 0;
		}

		ulong Cartridge::Unif::ReadMirroring()
		{
			cstring text;

			switch (stream.Read8())
			{
				case 0:  info.setup.mirroring = Api::Cartridge::MIRROR_HORIZONTAL; text = "Unif: horizontal mirroring"                    NST_LINEBREAK; break;
				case 1:  info.setup.mirroring = Api::Cartridge::MIRROR_VERTICAL;   text = "Unif: vertical mirroring"                      NST_LINEBREAK; break;
				case 2:  info.setup.mirroring = Api::Cartridge::MIRROR_ZERO;       text = "Unif: zero mirroring"                          NST_LINEBREAK; break;
				case 3:  info.setup.mirroring = Api::Cartridge::MIRROR_ONE;        text = "Unif: one mirroring"                           NST_LINEBREAK; break;
				case 4:  info.setup.mirroring = Api::Cartridge::MIRROR_FOURSCREEN; text = "Unif: four-screen mirroring"                   NST_LINEBREAK; break;
				default: info.setup.mirroring = Api::Cartridge::MIRROR_CONTROLLED; text = "Unif: mirroring controlled by mapper hardware" NST_LINEBREAK; break;
			}

			log << text;

			return 1;
		}

		ulong Cartridge::Unif::ReadController()
		{
			std::string string("Unif: controller(s): ");

			const uint controller = stream.Read8();

			if (controller & 0x01)
			{
				info.controllers[0] = Api::Input::PAD1;
				info.controllers[1] = Api::Input::PAD2;
				string += "standard joypad";
			}

			cstring offset;

			if (controller & 0x02)
			{
				info.controllers[1] = Api::Input::ZAPPER;

				offset = ", zapper";

				if (string[string.length() - 1] == ' ')
					offset += 2;

				string += offset;
			}

			if (controller & 0x04)
			{
				offset = ", R.O.B";

				if (string[string.length() - 1] == ' ')
					offset += 2;

				string += offset;
			}

			if (controller & 0x08)
			{
				info.controllers[0] = Api::Input::PADDLE;

				offset = ", paddle";

				if (string[string.length() - 1] == ' ')
					offset += 2;

				string += offset;
			}

			if (controller & 0x10)
			{
				info.controllers[1] = Api::Input::POWERPAD;

				offset = ", power pad";

				if (string[string.length() - 1] == ' ')
					offset += 2;

				string += offset;
			}

			if (controller & 0x20)
			{
				info.controllers[2] = Api::Input::PAD3;
				info.controllers[3] = Api::Input::PAD4;

				offset = ", four-score adapter";

				if (string[string.length() - 1] == ' ')
					offset += 2;

				string += offset;
			}

			if (string[string.length() - 1] == ' ')
				string += "not defined";

			log << string.c_str() << NST_LINEBREAK;

			return 1;
		}

		ulong Cartridge::Unif::ReadChrRam()
		{
			log << "Unif: CHR is writable" NST_LINEBREAK;
			return 0;
		}

		void Cartridge::Unif::CheckImageDatabase()
		{
			NST_ASSERT( database );

			if (crc == 0)
				crc = ComputeCrc();

			if (const ImageDatabase::Handle handle = database->Search( crc ))
			{
				databaseHandle = handle;

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
			}
		}
	}
}

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif
