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

#include "NstStream.hpp"
#include "NstMemory.hpp"
#include "NstLog.hpp"
#include "NstCrc32.hpp"
#include "NstImageDatabase.hpp"
#include "NstInes.hpp"

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("s", on)
#endif

namespace Nes
{
	namespace Core
	{
		Ines::Ines
		(
         	StdStream s,
			LinearMemory& p,
			LinearMemory& c,
			LinearMemory& w,
			Api::Cartridge::Info& i,
			const ImageDatabase* const d,
			Result& r
		)
		: 
		result   (r),
		stream   (s),
		pRom     (p),
		cRom     (c),
		wRam     (w),
		info     (i),
		database (d)
		{
			info.Clear();
			Import();
		}

		void Ines::Import()
		{
			stream.Validate( (u32) 0x1A53454EUL );

			{
				Header header;

				header.num16kPRomBanks = stream.Read8();
				header.num8kCRomBanks = stream.Read8();		
				header.flags = stream.Read16();		
				header.num8kWRamBanks = stream.Read8();
				header.pal = stream.Read8() & Header::PAL_BIT;
				stream.Read( header.reserved, Header::RESERVED_LENGTH );

				MessWithTheHeader( header );
			}

			info.crc = Crc32::Compute( stream );

			if (database && database->Enabled())
				TryDatabase();

			if (!info.pRom || stream.Length() < info.pRom)
				throw RESULT_ERR_CORRUPT_FILE;

			wRam.Set( info.wRam ? info.wRam : info.trained ? SIZE_8K : 0 );

			if (info.trained)
				stream.Read( wRam.Mem(TRAINER_OFFSET), TRAINER_LENGTH );

			pRom.Set( info.pRom );
			stream.Read( pRom.Mem(), info.pRom );

			info.pRomCrc = Crc32::Compute( pRom.Mem(), info.pRom );

			cRom.Set( info.cRom );

			if (info.cRom)
			{
				stream.Read( cRom.Mem(), info.cRom );
				info.cRomCrc = Crc32::Compute( cRom.Mem(), info.cRom );
			}
		}

		void Ines::MessWithTheHeader(Header& header)
		{
			for (uint i=0; i < Header::RESERVED_LENGTH; ++i)
			{
				if (header.reserved[i])
				{
					header.flags &= 0x00FFU;
					header.num8kWRamBanks = 0;
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

			log << "Ines: mapper " << info.mapper << " set" NST_LINEBREAK;

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
			else if (header.pal)
			{
				info.system = Api::Cartridge::SYSTEM_PAL;
			}
			else
			{
				info.system = Api::Cartridge::SYSTEM_NTSC;
			}

			info.pRom = SIZE_16K * header.num16kPRomBanks;
			info.cRom = SIZE_8K * header.num8kCRomBanks;
			info.wRam = SIZE_8K * header.num8kWRamBanks;

			log << "Ines: " << (info.pRom / SIZE_1K) << "k PRG-ROM set" NST_LINEBREAK
			       "Ines: " << (info.cRom / SIZE_1K) << "k CHR-ROM set" NST_LINEBREAK
			       "Ines: " << (info.wRam / SIZE_1K) << "k WRAM set" NST_LINEBREAK;   
		}

		void Ines::TryDatabase()
		{
			NST_ASSERT( database && database->Enabled() );

			dword crc = info.crc;
			ImageDatabase::Handle handle = database->GetHandle( info.crc );

			if (!handle)
			{
				if (stream.Length() >= info.pRom + info.cRom)
				{
					crc = Crc32::Compute( stream, info.pRom + info.cRom );

					if (crc)
						handle = database->GetHandle( crc );
				}

				if (!handle)
				{
					if (info.pRom == 0) // hack
						info.pRom = SIZE_16K * 256;

					return;
				}
			}

			info.crc = crc;

			const dword pRom = database->pRomSize( handle );
			NST_VERIFY( pRom );

			if (!pRom)
				return;

			static const char title[] = "Ines Database: warning, ";

			const uint trainer = database->HasTrainer( handle ) ? TRAINER_LENGTH : 0;

			if (info.pRom != pRom)
			{	
				const ulong total = trainer + pRom;

				log << title;

				if (info.pRom > pRom || stream.Length() >= total)
				{
					log << "changed PRG-ROM size: "
						<< (info.pRom / SIZE_1K)
						<< "k to: "
						<< (pRom / SIZE_1K)
						<< "k" NST_LINEBREAK;

					info.pRom = pRom;
				}
				else
				{
					log << "wanted to change PRG-ROM size: "
						<< (info.pRom / SIZE_1K)
						<< "k to: "
						<< (pRom / SIZE_1K)
						<< "k but couldn't" NST_LINEBREAK;
				}

				info.condition = Api::Cartridge::NO;

				if (result == RESULT_OK)
					result = RESULT_WARN_BAD_PROM;
			}

			const dword cRom = database->cRomSize( handle );

			if (info.cRom != cRom)
			{
				const ulong total = trainer + info.pRom + cRom;

				log << title;

				if (info.cRom > cRom || stream.Length() >= total)
				{
					log << "changed CHR-ROM size: "
						<< (info.cRom / SIZE_1K)
						<< "k to: "
						<< (cRom / SIZE_1K)
						<< "k" NST_LINEBREAK;

					info.cRom = cRom;
				}
				else
				{
					log << "wanted to change CHR-ROM size: "
						<< (info.cRom / SIZE_1K)
						<< "k to: "
						<< (cRom / SIZE_1K)
						<< "k but couldn't" NST_LINEBREAK;
				}

				info.condition = Api::Cartridge::NO;

				if (result == RESULT_OK)
					result = RESULT_WARN_BAD_CROM;
			}

			const dword wRam = database->wRamSize( handle );

			if (info.wRam != wRam)
			{
				log << title
					<< "changed WRAM size: "
					<< (info.wRam / SIZE_1K)
					<< "k to: "
				  	<< (wRam / SIZE_1K)
					<< "k" NST_LINEBREAK;

				info.wRam = wRam;
			}

			const uint mapper = database->Mapper( handle );

			if (info.mapper != mapper)
			{
				log << title
					<< "changed mapper "
					<< info.mapper
					<< " to "
					<< mapper
					<< NST_LINEBREAK;

				info.mapper = mapper;
			}

			const bool battery = database->HasBattery( handle );

			if (bool(info.battery) != battery)
			{
				info.battery = battery;
				log << title << (battery ? "enabled battery-backed RAM" NST_LINEBREAK : "ignored battery-backed RAM" NST_LINEBREAK);
			}

			const Api::Cartridge::Mirroring mirroring = database->GetMirroring( handle );

			if (info.mirroring != mirroring)
			{
				NST_COMPILE_ASSERT
				(
					Api::Cartridge::MIRROR_HORIZONTAL < 6 && 
					Api::Cartridge::MIRROR_VERTICAL < 6 && 
					Api::Cartridge::MIRROR_FOURSCREEN < 6 &&
					Api::Cartridge::MIRROR_ZERO < 6 &&
					Api::Cartridge::MIRROR_ONE < 6 &&
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
					<< types[info.mirroring]
					<< " to "
					<< types[mirroring]
					<< NST_LINEBREAK;

				info.mirroring = mirroring;
			}

			if (bool(info.trained) != bool(trainer))
			{
				info.trained = bool(trainer);
				log << title << (trainer ? "enabled trainer" NST_LINEBREAK : "ignored trainer" NST_LINEBREAK);
			}

			info.system = database->GetSystem( handle );
		}
	}
}

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif
