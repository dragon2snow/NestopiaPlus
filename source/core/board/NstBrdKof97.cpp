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

#include "../NstMapper.hpp"
#include "NstBrdMmc3.hpp"
#include "NstBrdKof97.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			void Kof97::SubReset(const bool hard)
			{
				Mmc3::SubReset( hard );

				for (uint i=0x0000; i < 0x2000; i += 0x2)
				{
					Map( 0x8000 + i, &Kof97::Poke_8000 );
					Map( 0x8001 + i, &Kof97::Poke_8001 );
					Map( 0xC000 + i, &Kof97::Poke_C000 );
					Map( 0xC001 + i, &Kof97::Poke_C001 );
				}

				Map( 0x9000U, &Kof97::Poke_8001 );
				Map( 0xA000U, &Kof97::Poke_8000 );
				Map( 0xD000U, &Kof97::Poke_C001 );

				for (uint i=0x0000; i < 0x1000; i += 0x2)
				{
					Map( 0xE000 + i, &Kof97::Poke_E000 );
					Map( 0xE001 + i, &Kof97::Poke_E001 );
				}

				Map( 0xF000U, &Kof97::Poke_E001 );
			}

			#ifdef NST_MSVC_OPTIMIZE
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
				Mmc3::NES_DO_POKE(8000,0x8000,Unscramble(data));
			}

			NES_POKE(Kof97,8001)
			{
				Mmc3::NES_DO_POKE(8001,0x8001,Unscramble(data));
			}

			NES_POKE(Kof97,C000)
			{
				Mmc3::NES_DO_POKE(C000,0xC000,Unscramble(data));
			}

			NES_POKE(Kof97,C001)
			{
				Mmc3::NES_DO_POKE(C001,0xC001,Unscramble(data));
			}

			NES_POKE(Kof97,E000)
			{
				Mmc3::NES_DO_POKE(E000,0xE000,Unscramble(data));
			}

			NES_POKE(Kof97,E001)
			{
				Mmc3::NES_DO_POKE(E001,0xE001,Unscramble(data));
			}
		}
	}
}
