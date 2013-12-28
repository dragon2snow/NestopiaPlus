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
#include "NstMapper071.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper71::Mapper71(Context& c)
		:
		Mapper  (c,c.attribute == ATR_BF9097 ? (CROM_MAX_8K|WRAM_NONE|NMT_ZERO) : c.attribute == ATR_BF9093 ? (CROM_MAX_8K|WRAM_NONE) : CROM_MAX_8K),
		nmtCtrl (c.attribute == ATR_BF9097)
		{}

		void Mapper71::SubReset(bool)
		{
			if (nmtCtrl)
				Map( 0x8000U, 0x9FFFU, &Mapper71::Poke_8000 );

			Map( 0xC000U, 0xFFFFU, PRG_SWAP_16K_0 );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE_D(Mapper71,8000)
		{
			ppu.SetMirroring( (data & 0x10) ? Ppu::NMT_ONE : Ppu::NMT_ZERO );
		}
	}
}
