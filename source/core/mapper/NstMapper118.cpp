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
#include "NstMapper118.hpp"

namespace Nes
{
	namespace Core
	{
		void Mapper118::UpdateChr() const
		{
			Mmc3::UpdateChr();

			nmt.SwapBanks<SIZE_1K,0x0000>
			(
				((regs.ctrl0 & Regs::CTRL0_XOR_CHR) ? banks.chr[2] >> 7 : banks.chr[0] >> 6) ^ 1,
				((regs.ctrl0 & Regs::CTRL0_XOR_CHR) ? banks.chr[3] >> 7 : banks.chr[0] >> 6) ^ 1,
				((regs.ctrl0 & Regs::CTRL0_XOR_CHR) ? banks.chr[4] >> 7 : banks.chr[1] >> 6) ^ 1,
				((regs.ctrl0 & Regs::CTRL0_XOR_CHR) ? banks.chr[5] >> 7 : banks.chr[1] >> 6) ^ 1
			);
		}
	}
}
