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
			{"NROM",                   0},
			{"NROM-128",               0},
			{"NROM-256",               0},
			{"RROM",                   0},
			{"RROM-128",               0},
			{"SNROM",                  1},  // MMC1
			{"SKROM",                  1},
			{"SAROM",                  1},
			{"SBROM",                  1},
			{"SCEOROM",                1},
			{"SC1ROM",                 1},
			{"SEROM",                  1},
			{"SFROM",                  1},
			{"SGROM",                  1},
			{"SHROM",                  1},
			{"SJROM",                  1},
			{"SLROM",                  1},
			{"SLRROM",                 1},
			{"SL2ROM",                 1},
			{"SL3ROM",                 1},
			{"SN1-ROM",                1},
			{"SOROM",                  1},
			{"SVROM",                  1},
			{"SUROM",                  1},
			{"351258",                 2},  // UNROM
			{"351298",                 2},
			{"351908",                 2},
			{"UNROM",                  2},
			{"CNROM",                  3},
			{"51555",                  4},  // MMC3/MMC6
			{"DRROM",                  4},
			{"EKROM",                  4},
			{"SL1ROM",                 4},
			{"SL2ROM",                 4},
			{"SL3ROM",                 4},
			{"TEROM",                  4},
			{"TFROM",                  4},
			{"TGROM",                  4},
			{"TKROM",                  4},
			{"TLROM",                  4},
			{"TQROM",                  4},
			{"TSROM",                  4},
			{"TVROM",                  4},
			{"TL1ROM",                 4},
			{"TLSROM",                 4},
			{"B4",                     4},
			{"HKROM",                  4},  // MMC6B
			{"ELROM",                  5},  // MMC5
			{"ETROM",                  5},
			{"EWROM",                  5},
			{"AOROM",                  7},
			{"PNROM",                  9},  // MMC2
			{"CPROM",                  13},
			{"BNROM",                  34},
			{"1991SUPERHIK7IN1",       45},
			{"SUPERHIK8IN1",           45},
			{"1992BALLGAMES11IN1",     51},
			{"MARIO7IN1",              52},
			{"SUPERVISION16IN1",       53},
			{"NOVELDIAMOND9999999IN1", 54},
			{"MARIO1-MALEE2",          55},
			{"STUDYGAME32IN1",         58},
			{"RESET4IN1",              60},
			{"D1038",                  60},
			{"GNROM",                  66},
			{"NTBROM",                 68},
			{"TEK90",                  90},
			{"22211",                  132},
			{"SA-72008",               133},
			{"SACHEN-8259D",           137},
			{"SACHEN-8259B",           138},
			{"SACHEN-8259C",           139},
			{"SACHEN-8259A",           141},
			{"SA-NROM",                143},
			{"SA-72007",               145},
			{"SA-016-1M",              146},
			{"TC-U01-1.5M",            147},
			{"SA-0037",                148},
			{"SA-0036",                149},
			{"SACHEN-74LS374N",        150},
			{"DEIROM",                 206},
			{"42IN1RESETSWITCH",       233},
			{"GOLDENGAME150IN1",       235},
			{"70IN1",                  236},
			{"70IN1B",                 236},
			{"SUPER24IN1SC03",         Mapper::EXT_SUPER24IN1},
			{"8157",                   Mapper::EXT_8157},
			{"8237",                   Mapper::EXT_8237},
			{"WS",                     Mapper::EXT_WS},
			{"DREAMTECH01",            Mapper::EXT_DREAMTECH01},
			{"H2288",                  Mapper::EXT_H2288},
			{"CC-21",                  Mapper::EXT_CC21},
			{"KOF97",                  Mapper::EXT_KOF97},
			{"64IN1NOREPEAT",          Mapper::EXT_64IN1NR},
			{"SHERO",                  Mapper::EXT_STREETHEROES},
			{"T-262",                  Mapper::EXT_T262},
			{"FK23C",                  Mapper::EXT_FK23C},
			{"603-5052",               Mapper::EXT_6035052},
			{"A65AS",                  Mapper::EXT_A65AS},
			{"EDU2000",                Mapper::EXT_EDU2000}
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
			LinearMemory& p,
			LinearMemory& c,
			LinearMemory& w,
			Api::Cartridge::Info& i,
			const ImageDatabase* const r,
			Result& result
		)
		:
		stream   (s),
		pRom     (p),
		cRom     (c),
		wRam     (w),
		info     (i),
		database (r)
		{
			if (!sorted)
			{
				sorted = true;
				std::sort( boards, boards + NST_COUNT(boards) );
			}

			info.Clear();
			Import( result );
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

		void Cartridge::Unif::Import(Result& result)
		{
			stream.Validate( NES_ID4('U','N','I','F') );

			log << "Unif: revision " << stream.Read32() << NST_LINEBREAK;

			stream.Seek( HEADER_RESERVED_LENGTH );

			info.mapper = UINT_MAX;

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

			result = CopyRom();

			info.crc = info.pRomCrc = Checksum::Crc32::Compute( pRom.Mem(), pRom.Size() );

			if (cRom.Size())
			{
				info.cRomCrc = Checksum::Crc32::Compute( cRom.Mem(), cRom.Size() );
				info.crc = Checksum::Crc32::Iterate( cRom.Mem(), cRom.Size(), info.crc );
			}

			if (database && database->Enabled())
				CheckImageDatabase();

			if (!CheckMapper())
				throw RESULT_ERR_UNSUPPORTED_MAPPER;

			info.pRom = pRom.Size();
			info.cRom = cRom.Size();
			info.wRam = wRam.Size();
		}

		Result Cartridge::Unif::CopyRom()
		{
			Result result = RESULT_OK;

			for (uint i=0; i < 2; ++i)
			{
				LinearMemory& dst = (i ? cRom : pRom);
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
								msg = " crc check ok" NST_LINEBREAK;
							}
							else
							{
								msg = " crc check failed" NST_LINEBREAK;
								info.condition = Api::Cartridge::NO;
								result = (i ? RESULT_WARN_BAD_CROM : RESULT_WARN_BAD_PROM);
							}

							log << type << j << msg;
						}

						dst += src.rom;
						src.rom.Destroy();
					}
				}
			}

			if (pRom.Empty())
				throw RESULT_ERR_CORRUPT_FILE;

			return result;
		}

		bool Cartridge::Unif::CheckMapper()
		{
			if (info.mapper != UINT_MAX)
				return true;

			if (info.crc && database)
			{
				if (ImageDatabase::Handle handle = database->GetHandle( info.crc ))
				{
					info.mapper = database->Mapper( handle );
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
					info.mapper = id;
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
				log << "Unif: dumper Name: " << dump.name << NST_LINEBREAK;

			log << "Unif: dump Year: "  << dump.year << NST_LINEBREAK
                   "Unif: dump Month: " << dump.month << NST_LINEBREAK
                   "Unif: dump Day: "   << dump.day << NST_LINEBREAK;

			if (*dump.agent)
				log << "Unif: dumper Agent: " << dump.agent << NST_LINEBREAK;

			return Dump::LENGTH;
		}

		ulong Cartridge::Unif::ReadSystem()
		{
			cstring msg;

			switch (stream.Read8())
			{
				case 0:  info.system = Api::Cartridge::SYSTEM_NTSC;     msg = "Unif: NTSC system"     NST_LINEBREAK; break;
				case 1:  info.system = Api::Cartridge::SYSTEM_PAL;      msg = "Unif: PAL system"      NST_LINEBREAK; break;
				default: info.system = Api::Cartridge::SYSTEM_NTSC_PAL; msg = "Unif: NTSC/PAL system" NST_LINEBREAK; break;
			}

			log << msg;

			return 1;
		}

		ulong Cartridge::Unif::ReadRomCrc(const uint type,const uint id)
		{
			NST_ASSERT( type == 0 || type == 1 );

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
			NST_ASSERT( type == 0 || type == 1 );

			for (uint i=0; i < sizeof(Rom::id); ++i)
			{
				if (char(id) == Rom::id[i])
				{
					cstring const name = (type ? "Unif: CHR-ROM " : "Unif: PRG-ROM ");
					log << name << char(id) << " data, " << (length / SIZE_1K) << "k" NST_LINEBREAK;

					LinearMemory& rom = roms[type][i].rom;

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
						info.mapper = begin->mapper;
				}
			}

			return length + 1;
		}

		#undef NES_ID3
		#undef NES_ID4

		ulong Cartridge::Unif::ReadBattery()
		{
			info.battery = true;
			log << "Unif: battery present" NST_LINEBREAK;
			return 0;
		}

		ulong Cartridge::Unif::ReadMirroring()
		{
			cstring text;

			switch (stream.Read8())
			{
				case 0:  info.mirroring = Api::Cartridge::MIRROR_HORIZONTAL; text = "Unif: horizontal mirroring"                    NST_LINEBREAK; break;
				case 1:  info.mirroring = Api::Cartridge::MIRROR_VERTICAL;   text = "Unif: vertical mirroring"                      NST_LINEBREAK; break;
				case 2:  info.mirroring = Api::Cartridge::MIRROR_ZERO;       text = "Unif: zero mirroring"                          NST_LINEBREAK; break;
				case 3:  info.mirroring = Api::Cartridge::MIRROR_ONE;        text = "Unif: one mirroring"                           NST_LINEBREAK; break;
				case 4:  info.mirroring = Api::Cartridge::MIRROR_FOURSCREEN; text = "Unif: four-screen mirroring"                   NST_LINEBREAK; break;
				default: info.mirroring = Api::Cartridge::MIRROR_CONTROLLED; text = "Unif: mirroring controlled by mapper hardware" NST_LINEBREAK; break;
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
			info.isCRam = true;
			log << "Unif: CHR is writable" NST_LINEBREAK;
			return 0;
		}

		void Cartridge::Unif::CheckImageDatabase()
		{
			NST_ASSERT( database && database->Enabled() );

			if (!info.crc)
				return;

			ImageDatabase::Handle handle = database->GetHandle( info.crc );

			if (!handle)
				return;

			if (info.condition == Api::Cartridge::UNKNOWN)
				info.condition = Api::Cartridge::YES;

			info.mapper = database->Mapper( handle );

			switch (database->GetSystem( handle ))
			{
				case Api::Cartridge::SYSTEM_VS:

					info.system = Api::Cartridge::SYSTEM_VS;
					break;

				case Api::Cartridge::SYSTEM_PC10:

					info.system = Api::Cartridge::SYSTEM_PC10;
					break;
			}

			const ulong wRamSize = database->wRamSize( handle );

			if (wRam.Size() < wRamSize)
				wRam.Set( wRamSize );
		}
	}
}

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif
