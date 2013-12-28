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
#include "NstBrdMmc3.hpp"
#include "NstBrdKof97.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			void Kof97::SubReset(const bool hard)
			{
				Mmc3::SubReset( hard );

				for (uint i=0x0000U; i < 0x2000U; i += 0x2)
				{
					Map( 0x8000U + i, &Kof97::Poke_8000 );
					Map( 0x8001U + i, &Kof97::Poke_8001 );
					Map( 0xC000U + i, &Kof97::Poke_C000 );
					Map( 0xC001U + i, &Kof97::Poke_C001 );
				}

				Map( 0x9000U, &Kof97::Poke_8001 );
				Map( 0xA000U, &Kof97::Poke_8000 );
				Map( 0xD000U, &Kof97::Poke_C001 );

				for (uint i=0x0000U; i < 0x1000U; i += 0x2)
				{
					Map( 0xE000U + i, &Kof97::Poke_E000 );
					Map( 0xE001U + i, &Kof97::Poke_E001 );
				}

				Map( 0xF000U, &Kof97::Poke_E001 );
			}

			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("", on)
			#endif

			uint Kof97::Unscramble(uint data)
			{
				return
				(
					(data >> 1 & 0x01) |
					(data >> 4 & 0x02) |
					(data << 2 & 0x04) |
					(data >> 0 & 0xD8) |
					(data << 3 & 0x20)
				);
			}

			NES_POKE(Kof97,8000)
			{
				NES_CALL_POKE(Mmc3,8000,0x8000U,Unscramble(data));
			}

			NES_POKE(Kof97,8001)
			{
				NES_CALL_POKE(Mmc3,8001,0x8001U,Unscramble(data));
			}

			NES_POKE(Kof97,C000)
			{
				NES_CALL_POKE(Mmc3,C000,0xC000U,Unscramble(data));
			}

			NES_POKE(Kof97,C001)
			{
				NES_CALL_POKE(Mmc3,C001,0xC001U,Unscramble(data));
			}

			NES_POKE(Kof97,E000)
			{
				NES_CALL_POKE(Mmc3,E000,0xE000U,Unscramble(data));
			}

			NES_POKE(Kof97,E001)
			{
				NES_CALL_POKE(Mmc3,E001,0xE001U,Unscramble(data));
			}
		}
	}
}
