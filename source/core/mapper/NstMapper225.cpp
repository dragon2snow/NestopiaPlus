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
#include "NstMapper225.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper225::SubReset(bool)
		{
			Map( 0x8000U, 0xFFFFU, &Mapper225::Poke_Prg );
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE(Mapper225,Prg)
		{
			ppu.SetMirroring( (address & 0x2000) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
			chr.SwapBank<SIZE_8K,0x0000U>( address & 0x3F );

			uint bank = address >> 7 & 0x1F;

			if (address & 0x1000)
			{
				bank = (bank << 1) | (address >> 6 & 0x1);
				prg.SwapBanks<SIZE_16K,0x0000U>( bank, bank );
			}
			else
			{
				prg.SwapBank<SIZE_32K,0x0000U>( bank );
			}
		}
	}
}
