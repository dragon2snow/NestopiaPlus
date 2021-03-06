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

#include "NstBoard.hpp"
#include "NstBoardBmcSuperHiK300in1.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Bmc
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void SuperHiK300in1::SubReset(const bool hard)
				{
					Map( 0x8000U, 0xBFFFU, &SuperHiK300in1::Poke_8000 );
					Map( 0xC000U, 0xFFFFU, &SuperHiK300in1::Poke_C000 );

					if (hard)
						NES_DO_POKE(C000,0xFFFF,0x00);
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				void SuperHiK300in1::SwapGfx(const uint address) const
				{
					ppu.SetMirroring( (address & 0x8) ? Ppu::NMT_H : Ppu::NMT_V );
					chr.SwapBank<SIZE_8K,0x0000>( address );
				}

				NES_POKE_A(SuperHiK300in1,8000)
				{
					prg.SwapBanks<SIZE_16K,0x0000>( address, address );
					SwapGfx( address );
				}

				NES_POKE_A(SuperHiK300in1,C000)
				{
					prg.SwapBank<SIZE_32K,0x0000>( address >> 1 );
					SwapGfx( address );
				}
			}
		}
	}
}
