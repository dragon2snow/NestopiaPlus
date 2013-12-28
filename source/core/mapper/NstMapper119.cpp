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
#include "../board/NstBrdMmc3.hpp"
#include "NstMapper119.hpp"

namespace Nes
{
	namespace Core
	{
		void Mapper119::UpdateChr() const
		{
			ppu.Update();

			const uint swap = (regs.ctrl0 & Regs::CTRL0_XOR_CHR) << 5;

			chr.Source( banks.chr[0] >> 5 & 0x1 ).SwapBank<SIZE_2K>( 0x0000 ^ swap, banks.chr[0] );
			chr.Source( banks.chr[1] >> 5 & 0x1 ).SwapBank<SIZE_2K>( 0x0800 ^ swap, banks.chr[1] );
			chr.Source( banks.chr[2] >> 6 & 0x1 ).SwapBank<SIZE_1K>( 0x1000 ^ swap, banks.chr[2] );
			chr.Source( banks.chr[3] >> 6 & 0x1 ).SwapBank<SIZE_1K>( 0x1400 ^ swap, banks.chr[3] );
			chr.Source( banks.chr[4] >> 6 & 0x1 ).SwapBank<SIZE_1K>( 0x1800 ^ swap, banks.chr[4] );
			chr.Source( banks.chr[5] >> 6 & 0x1 ).SwapBank<SIZE_1K>( 0x1C00 ^ swap, banks.chr[5] );
		}
	}
}
