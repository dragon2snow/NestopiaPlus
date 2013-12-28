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
#include "../board/NstBrdMmc3.hpp"
#include "NstMapper197.hpp"

namespace Nes
{
	namespace Core
	{
		void Mapper197::UpdateChr() const
		{
			ppu.Update();

			const uint swap = regs.ctrl0 & Regs::CTRL0_XOR_CHR;

			chr.SwapBank<SIZE_4K,0x0000U>( banks.chr[swap ? 2 : 0] );
			chr.SwapBanks<SIZE_2K,0x1000U>( banks.chr[swap ? 0 : 2], banks.chr[swap ? 1 : 3] );
		}
	}
}
