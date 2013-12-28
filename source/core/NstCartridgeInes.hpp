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

#ifndef NST_CARTRIDGE_INES_H
#define NST_CARTRIDGE_INES_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Ips;

		class Cartridge::Ines
		{
		public:

			typedef Api::Cartridge::NesHeader Header;

			static void Load
			(
				StdStream,
				StdStream,
				Ram&,
				Ram&,
				FavoredSystem,
				Profile&,
				ProfileEx&,
				const ImageDatabase*
			);

			static Result ReadHeader(Header&,const byte*,ulong);
			static Result WriteHeader(const Header&,byte*,ulong);

		private:

			enum TrainerSetup
			{
				TRAINER_NONE,
				TRAINER_IGNORE,
				TRAINER_READ
			};

			enum
			{
				VS_MAPPER_99 = 99,
				VS_MAPPER_151 = 151,
				FFE_MAPPER_6 = 6,
				FFE_MAPPER_8 = 8,
				FFE_MAPPER_17 = 17,
				TRAINER_LENGTH = 0x200,
				MIN_DB_SEARCH_STRIDE = SIZE_8K,
				MAX_DB_SEARCH_LENGTH = SIZE_16K * 0xFFFUL + SIZE_8K * 0xFFFUL
			};

			static TrainerSetup Collect(FavoredSystem,Profile&,ProfileEx&,StdStream,const Ips&);
			static ImageDatabase::Entry SearchDatabase(const ImageDatabase&,StdStream,dword,TrainerSetup,FavoredSystem);
		};
	}
}

#endif
