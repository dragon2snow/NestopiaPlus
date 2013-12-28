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

#include "../NstMapper.hpp"
#include "../board/NstBrdMmc5.hpp"
#include "NstMapper005.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		uint Mapper5::GetWrkSize(const Context& c)
		{
			switch (c.prgCrc)
			{
				case 0xF4120E58UL: // Aoki Ookami to Shiroki Mejika - Genchou Hishi (J)
				case 0x286613D8UL: // Nobunaga no Yabou - Bushou Fuuun Roku (J)
				case 0x11EAAD26UL: // Romance of the Three Kingdoms 2 (U)
				case 0x95BA5733UL: // Sangokushi 2 (J)

					return WRAM_32K;

				case 0x2B548D75UL: // Bandit Kings of Ancient China (U)
				case 0xF4CD4998UL: // Dai Koukai Jidai (J)
				case 0x8FA95456UL: // Ishin no Arashi (J)
				case 0x98C8E090UL: // Nobunaga no Yabou - Sengoku Gunyuu Den (U)
				case 0x57E3218BUL: // L'Empereur (J)
				case 0x2F50BD38UL: // L'Empereur (U)
				case 0x8E9A5E2FUL: // L'Empereur (U) alt
				case 0xB56958D1UL: // Nobunaga's Ambition 2 (U)
				case 0xE6C28C5FUL: // Suikoden - Tenmei no Chikai (J)
				case 0xCD35E2E9UL: // Uncharted Waters (U)

					return WRAM_16K;

				case 0xE7C72DBBUL: // Gemfire (U)
				case 0x57F33F70UL: // Royal Blood (J)
				case 0x5D9D9891UL: // Just Breed (J)
				case 0xE91548D8UL: // Shin 4 Nin Uchi Mahjong (J)

					return WRAM_8K;

				case 0x637E366AUL: // Castlevania 3 (E)
				case 0x95CA9EC7UL: // Castlevania 3 (U)
				case 0x255B129CUL: // Gun Sight (J)
				case 0x51D2112FUL: // Laser Invasion (U)

					return WRAM_NONE;

				default:
				{
					dword size = c.wrk.Size();

					if ( size > SIZE_40K ) return WRAM_64K;
					if ( size > SIZE_32K ) return WRAM_40K;
					if ( size > SIZE_16K ) return WRAM_32K;
					if ( size > SIZE_8K  ) return WRAM_16K;
				}
			}

			return WRAM_8K;
		}

		Mapper5::Mapper5(Context& c)
		: Mmc5(c,GetWrkSize(c))
		{
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
