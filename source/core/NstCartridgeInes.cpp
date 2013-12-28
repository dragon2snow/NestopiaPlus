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

#include "NstStream.hpp"
#include "NstLog.hpp"
#include "NstChecksumCrc32.hpp"
#include "NstImageDatabase.hpp"
#include "NstCartridge.hpp"
#include "NstCartridgeInes.hpp"

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("s", on)
#endif

namespace Nes
{
	namespace Core
	{
		Cartridge::Ines::Ines
		(
			StdStream s,
			Ram& p,
			Ram& c,
			Ram& w,
			Api::Cartridge::Info& i,
			const ImageDatabase* const d
		)
		:
		result   (RESULT_OK),
		stream   (s),
		prg      (p),
		chr      (c),
		wrk      (w),
		info     (i),
		prgSkip  (0),
		chrSkip  (0),
		database (d)
		{
			info.Clear();
			Import();
		}

		void Cartridge::Ines::Import()
		{
			stream.Validate( (u32) 0x1A53454EUL );

			{
				Header header;

				header.num16kPrgBanks = stream.Read8();
				header.num8kChrBanks = stream.Read8();
				header.flags = stream.Read16();
				header.num8kWrkBanks = stream.Read8();
				header.pal = stream.Read8() & Header::PAL_BIT;
				stream.Read( header.reserved, Header::RESERVED_LENGTH );

				MessWithTheHeader( header );
			}

			if (database)
				TryDatabase();

			if (!info.pRom)
				throw RESULT_ERR_CORRUPT_FILE;

			if (!info.wRam && (info.battery || info.trained))
				info.wRam = SIZE_8K;

			wrk.Set( info.wRam );
			wrk.Fill( 0x00 );

			if (info.trained)
			{
				stream.Read( wrk.Mem(TRAINER_OFFSET), TRAINER_LENGTH );
				info.crc = Checksum::Crc32::Compute( wrk.Mem(TRAINER_OFFSET), TRAINER_LENGTH );
			}

			prg.Set( info.pRom );
			stream.Read( prg.Mem(), info.pRom );

			if (prgSkip)
				stream.Seek( prgSkip );

			info.pRomCrc = Checksum::Crc32::Compute( prg.Mem(), info.pRom );
			info.crc = info.crc ? Checksum::Crc32::Compute( prg.Mem(), info.pRom, info.crc ) : info.pRomCrc;

			chr.Set( info.cRom );

			if (info.cRom)
			{
				stream.Read( chr.Mem(), info.cRom );
				info.cRomCrc = Checksum::Crc32::Compute( chr.Mem(), info.cRom );
				info.crc = Checksum::Crc32::Compute( chr.Mem(), info.cRom, info.crc );
			}
		}

		void Cartridge::Ines::MessWithTheHeader(Header& header)
		{
			for (uint i=0; i < Header::RESERVED_LENGTH; ++i)
			{
				if (header.reserved[i])
				{
					header.flags &= 0x00FFU;
					header.num8kWrkBanks = 0;
					header.pal = false;
					result = RESULT_WARN_BAD_FILE_HEADER;
					log << "Ines: warning, header might be dirty!" NST_LINEBREAK;
					break;
				}
			}

			if (header.flags & FLAGS_FOURSCREEN)
			{
				info.mirroring = Api::Cartridge::MIRROR_FOURSCREEN;
				log << "Ines: four-screen mirroring set" NST_LINEBREAK;
			}
			else if (header.flags & FLAGS_VERTICAL)
			{
				info.mirroring = Api::Cartridge::MIRROR_VERTICAL;
				log << "Ines: vertical mirroring set" NST_LINEBREAK;
			}
			else
			{
				info.mirroring = Api::Cartridge::MIRROR_HORIZONTAL;
				log << "Ines: horizontal mirroring set" NST_LINEBREAK;
			}

			info.mapper =
			(
				((header.flags & FLAGS_MAPPER_LO) >> 4) +
				((header.flags & FLAGS_MAPPER_HI) >> 8)
			);

			log << "Ines: mapper " << info.mapper << " set";

			if (info.mapper == VS_MAPPER && !(header.flags & FLAGS_VS))
				log << ", forcing VS-System";

			log << NST_LINEBREAK;

			info.trained = bool(header.flags & FLAGS_TRAINER);

			if (info.trained)
				log << "Ines: trainer set" NST_LINEBREAK;

			info.battery = bool(header.flags & FLAGS_BATTERY);

			if (info.battery)
				log << "Ines: battery set" NST_LINEBREAK;

			if (header.pal)
				log << "Ines: PAL set" NST_LINEBREAK;

			if (header.flags & FLAGS_VS)
			{
				info.system = Api::Cartridge::SYSTEM_VS;
				log << "Ines: VS-System set" NST_LINEBREAK;
			}
			else if (info.mapper == VS_MAPPER)
			{
				info.system = Api::Cartridge::SYSTEM_VS;
			}
			else if (header.pal)
			{
				info.system = Api::Cartridge::SYSTEM_PAL;
			}
			else
			{
				info.system = Api::Cartridge::SYSTEM_NTSC;
			}

			info.pRom = SIZE_16K * header.num16kPrgBanks;
			info.cRom = SIZE_8K * header.num8kChrBanks;
			info.wRam = SIZE_8K * header.num8kWrkBanks;

			log << "Ines: " << (info.pRom / SIZE_1K) << "k PRG-ROM set" NST_LINEBREAK
                   "Ines: " << (info.cRom / SIZE_1K) << "k CHR-ROM set" NST_LINEBREAK
                   "Ines: " << (info.wRam / SIZE_1K) << "k WRAM set" NST_LINEBREAK;
		}

		void Cartridge::Ines::TryDatabase()
		{
			NST_ASSERT( database );

			ImageDatabase::Handle handle;

			ulong length = stream.Length();
			length = length - (length % TRAINER_LENGTH);
			handle = database->GetHandle( Checksum::Crc32::Compute(stream,length) );

			if (!handle)
			{
				if (length > info.pRom + info.cRom)
					handle = database->GetHandle( Checksum::Crc32::Compute(stream,info.pRom+info.cRom) );

				if (!handle)
				{
					if (info.pRom == 0 && database->Enabled()) // hack
						info.pRom = SIZE_16K * 256;

					return;
				}
			}

			if (database->IsBad(handle) || (!database->Enabled() && database->MustFix(handle)))
			{
				info.condition = Api::Cartridge::NO;

				if (result == RESULT_OK)
					result = RESULT_WARN_BAD_DUMP;
			}

			NST_VERIFY( database->PrgSize(handle) );

			if (!database->Enabled() || !database->PrgSize(handle))
				return;

			if (!database->IsBad(handle))
				info.condition = Api::Cartridge::YES;

			prgSkip = database->PrgSkip(handle);
			chrSkip = database->ChrSkip(handle);

			if (result == RESULT_OK)
			{
				if
				(
					(info.pRom != database->PrgSize(handle)+prgSkip && (info.pRom != database->PrgSize(handle) || database->ChrSize(handle))) ||
					(info.cRom != database->ChrSize(handle)+chrSkip && (info.cRom != database->ChrSize(handle)))
				)
					result = RESULT_WARN_INCORRECT_FILE_HEADER;
			}

			static const char title[] = "Ines Database: warning, ";

			if (info.pRom != database->PrgSize(handle))
			{
				const dword pRom = info.pRom;
				info.pRom = database->PrgSize(handle);

				log << title
					<< "changed PRG-ROM size: "
					<< (pRom / SIZE_1K)
					<< "k to: "
					<< (info.pRom / SIZE_1K)
					<< "k" NST_LINEBREAK;
			}

			if (info.cRom != database->ChrSize(handle))
			{
				const dword cRom = info.cRom;
				info.cRom = database->ChrSize(handle);

				log << title;

				if (chrSkip == SIZE_8K && info.cRom+chrSkip == cRom && database->GetSystem(handle) == Api::Cartridge::SYSTEM_PC10)
				{
					log << "ignored last 8k CHR-ROM" NST_LINEBREAK;
				}
				else
				{
					log << "changed CHR-ROM size: "
						<< (cRom / SIZE_1K)
						<< "k to: "
						<< (info.cRom / SIZE_1K)
						<< "k" NST_LINEBREAK;
				}
			}

			if (info.wRam < database->WrkSize(handle) || info.wRam > NST_MAX(1,database->WrkSize(handle)))
			{
				const dword wRam = info.wRam;
				info.wRam = database->WrkSize(handle);

				if (result == RESULT_OK && NST_MAX(1,wRam) != NST_MAX(1,info.wRam))
					result = RESULT_WARN_INCORRECT_FILE_HEADER;

				log << title
					<< "changed WRAM size: "
					<< (wRam / SIZE_1K)
					<< "k to: "
					<< (info.wRam / SIZE_1K)
					<< "k" NST_LINEBREAK;
			}

			if (info.mapper != database->Mapper(handle))
			{
				const uint mapper = info.mapper;
				info.mapper = database->Mapper(handle);

				if (result == RESULT_OK)
					result = RESULT_WARN_INCORRECT_FILE_HEADER;

				log << title
					<< "changed mapper "
					<< mapper
					<< " to "
					<< info.mapper
					<< NST_LINEBREAK;
			}

			if (bool(info.battery) != bool(database->HasBattery(handle)))
			{
				if (result == RESULT_OK)
					result = RESULT_WARN_INCORRECT_FILE_HEADER;

				info.battery = bool(database->HasBattery(handle));
				log << title << (info.battery ? "enabled battery-backed RAM" NST_LINEBREAK : "ignored battery-backed RAM" NST_LINEBREAK);
			}

			if (info.mirroring != database->GetMirroring(handle))
			{
				const Api::Cartridge::Mirroring mirroring = info.mirroring;
				info.mirroring = database->GetMirroring(handle);

				if (result == RESULT_OK)
				{
					switch (info.mirroring)
					{
						case Api::Cartridge::MIRROR_HORIZONTAL:
						case Api::Cartridge::MIRROR_VERTICAL:
						case Api::Cartridge::MIRROR_FOURSCREEN:

							result = RESULT_WARN_INCORRECT_FILE_HEADER;
							break;
					}
				}

				NST_COMPILE_ASSERT
				(
					Api::Cartridge::MIRROR_HORIZONTAL < 6 &&
					Api::Cartridge::MIRROR_VERTICAL   < 6 &&
					Api::Cartridge::MIRROR_FOURSCREEN < 6 &&
					Api::Cartridge::MIRROR_ZERO       < 6 &&
					Api::Cartridge::MIRROR_ONE        < 6 &&
					Api::Cartridge::MIRROR_CONTROLLED < 6
				);

				cstring types[6];

				types[ Api::Cartridge::MIRROR_HORIZONTAL ] = "horizontal mirroring";
				types[ Api::Cartridge::MIRROR_VERTICAL   ] = "vertical mirroring";
				types[ Api::Cartridge::MIRROR_FOURSCREEN ] = "four-screen mirroring";
				types[ Api::Cartridge::MIRROR_ZERO       ] = "$2000 mirroring";
				types[ Api::Cartridge::MIRROR_ONE        ] = "$2400 mirroring";
				types[ Api::Cartridge::MIRROR_CONTROLLED ] = "mapper controlled mirroring";

				log << title
					<< "changed "
					<< types[mirroring]
					<< " to "
					<< types[info.mirroring]
					<< NST_LINEBREAK;
			}

			if (bool(info.trained) != bool(database->HasTrainer(handle)))
			{
				if (result == RESULT_OK)
					result = RESULT_WARN_INCORRECT_FILE_HEADER;

				info.trained = bool(database->HasTrainer(handle));
				log << title << (info.trained ? "enabled trainer" NST_LINEBREAK : "ignored trainer" NST_LINEBREAK);
			}

			if (info.system != database->GetSystem(handle))
			{
				const Api::Cartridge::System system = info.system;
				info.system = database->GetSystem(handle);

				if (result == RESULT_OK)
				{
					if (system != Api::Cartridge::SYSTEM_NTSC || (info.system != Api::Cartridge::SYSTEM_NTSC_PAL && info.system != Api::Cartridge::SYSTEM_PC10))
						result = RESULT_WARN_INCORRECT_FILE_HEADER;
				}

				NST_COMPILE_ASSERT
				(
					Api::Cartridge::SYSTEM_NTSC     < 5 &&
					Api::Cartridge::SYSTEM_PAL      < 5 &&
					Api::Cartridge::SYSTEM_NTSC_PAL < 5 &&
					Api::Cartridge::SYSTEM_VS       < 5 &&
					Api::Cartridge::SYSTEM_PC10     < 5
				);

				cstring types[5];

				types[ Api::Cartridge::SYSTEM_NTSC     ] = "NTSC";
				types[ Api::Cartridge::SYSTEM_PAL      ] = "PAL";
				types[ Api::Cartridge::SYSTEM_NTSC_PAL ] = "NTSC/PAL";
				types[ Api::Cartridge::SYSTEM_VS       ] = "VS-System";
				types[ Api::Cartridge::SYSTEM_PC10     ] = "Playchoice 10";

				log << title
					<< "changed "
					<< types[system]
					<< " to "
					<< types[info.system]
					<< NST_LINEBREAK;
			}
		}
	}
}

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif
