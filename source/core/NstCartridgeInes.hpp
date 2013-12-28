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

#ifndef NST_CARTRIDGE_INES_H
#define NST_CARTRIDGE_INES_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "api/NstApiCartridge.hpp"

namespace Nes
{
	namespace Core
	{
		class Cartridge::Ines
		{
		public:

			Ines
			(
				StdStream,
				LinearMemory&,
				LinearMemory&,
				LinearMemory&,
				Api::Cartridge::Info&,
				const ImageDatabase*,
				Result&
			);

		private:

			enum
			{
				TRAINER_OFFSET = 0x1000,
				TRAINER_LENGTH = 0x0200
			};

			enum
			{
				FLAGS_VERTICAL   = 0x0001,
				FLAGS_BATTERY    = 0x0002,
				FLAGS_TRAINER    = 0x0004,
				FLAGS_FOURSCREEN = 0x0008,
				FLAGS_MAPPER_LO  = 0x00F0,
				FLAGS_VS         = 0x0100,
				FLAGS_MAPPER_HI  = 0xF000
			};

			struct Header
			{
				enum
				{
					PAL_BIT = 0x01,
					RESERVED_LENGTH = 6,
					LENGTH = 6 + RESERVED_LENGTH
				};

				uint num16kPRomBanks;
				uint num8kCRomBanks;
				uint flags;
				uint num8kWRamBanks;
				ibool pal;
				u8 reserved[RESERVED_LENGTH];
			};

			enum
			{
				CRC_OFFSET = 4 + Header::LENGTH
			};

			void Import();
			void MessWithTheHeader(Header&);
			void TryDatabase();

			Result& result;

			Stream::In stream;
			Log log;

			LinearMemory& pRom;
			LinearMemory& cRom;
			LinearMemory& wRam;

			Api::Cartridge::Info& info;

			const ImageDatabase* const database;
		};
	}
}

#endif
