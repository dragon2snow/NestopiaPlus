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
#include "NstMapper022.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper22::SubReset(bool)
		{
			Map( 0x8000U, 0x8FFFU, PRG_SWAP_8K_0       );
			Map( 0x9000U, 0x9FFFU, NMT_SWAP_VH01       );
			Map( 0xA000U, 0xAFFFU, PRG_SWAP_8K_1       );
			Map( 0xB000U, 0xEFFFU, &Mapper22::Poke_Prg );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE_AD(Mapper22,Prg)
		{
			ppu.Update();

			const uint part = address << 1 & 0x4;
			address = ((address - 0xB000) >> 1 & 0x1800) | (address << 10 & 0x0400);

			chr.SwapBank<SIZE_1K>( address, ((chr.GetBank<SIZE_1K>(address) & (0xF0U >> part)) | ((data >> 1 & 0x0F) << part)) );
		}
	}
}
