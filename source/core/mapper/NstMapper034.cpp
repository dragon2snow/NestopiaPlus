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
#include "NstMapper034.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper34::SubReset(const bool hard)
		{
			if (chr.Source().Writable())
			{
				// BxROM

				Map( 0x8000U, 0xFFFFU, PRG_SWAP_32K );
			}
			else
			{
				// NINA-1

				Map( 0x7FFDU, PRG_SWAP_32K  );
				Map( 0x7FFEU, CHR_SWAP_4K_0 );
				Map( 0x7FFFU, CHR_SWAP_4K_1 );
			}

			if (hard)
				prg.SwapBank<SIZE_32K,0x0000>(0);
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
