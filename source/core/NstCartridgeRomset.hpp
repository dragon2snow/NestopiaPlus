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

#ifndef NST_CARTRIDGE_ROMSET_H
#define NST_CARTRIDGE_ROMSET_H

#include <vector>

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Cartridge::Romset
		{
		public:

			static void Load
			(
				StdStream,
				StdStream,
				Ram&,
				Ram&,
				FavoredSystem,
				bool,
				Profile&,
				bool=false
			);

		private:

			typedef std::vector<Profile> Profiles;

			enum
			{
				DEFAULT_VERSION  = 10,
				MIN_PLAYERS      = 1,
				MAX_PLAYERS      = 255,
				MIN_CHIP_SIZE    = 1,
				MAX_CHIP_SIZE    = SIZE_16384K,
				MIN_IC_PINS      = 1,
				MAX_IC_PINS      = 127,
				MAX_CHIP_SAMPLES = 255,
				MAX_MAPPER       = 255
			};

			static void Collect(StdStream,FavoredSystem,Profiles&);
			static void ChooseProfile(FavoredSystem,bool,const Profiles&,Profile&);
			static void LoadRoms(Ram&,Ram&,Profile&,bool);
		};
	}
}

#endif
