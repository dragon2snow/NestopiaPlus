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
#include "NstMapper012.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper12::SubReset(const bool hard)
		{
			if (hard)
				exRegs[0] = exRegs[1] = 0;

			Mmc3::SubReset( hard );

			Map( 0x4100U, 0x5FFFU, &Mapper12::Peek_4100, &Mapper12::Poke_4100 );
			Map( 0x6000U, 0x7FFFU, &Mapper12::Peek_4100 );
		}

		void Mapper12::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
				{
					const uint data = state.Read8();
					exRegs[0] = (data & 0x01) << 8;
					exRegs[1] = (data & 0x10) << 4;
				}

				state.End();
			}
		}

		void Mapper12::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write8( ((exRegs[0] >> 8) & 0x01) | ((exRegs[1] >> 4) & 0x10) ).End();
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE(Mapper12,4100)
		{
			exRegs[0] = (data & 0x01) << 8;
			exRegs[1] = (data & 0x10) << 4;

			Mapper12::UpdateChr();
		}

		NES_PEEK(Mapper12,4100)
		{
			return 0x01;
		}

		void Mapper12::UpdateChr() const
		{
			ppu.Update();

			const uint swap = (regs.ctrl0 & Regs::CTRL0_XOR_CHR) << 5;

			const uint base[2] =
			{
				exRegs[(regs.ctrl0 >> Regs::CTRL0_MODE) ^ 0],
				exRegs[(regs.ctrl0 >> Regs::CTRL0_MODE) ^ 1]
			};

			chr.SwapBanks<SIZE_2K>( 0x0000U ^ swap, base[0] + banks.chr[0], base[0] + banks.chr[1] );
			chr.SwapBanks<SIZE_1K>( 0x1000U ^ swap, base[1] + banks.chr[2], base[1] + banks.chr[3], base[1] + banks.chr[4], base[1] + banks.chr[5] );
		}
	}
}
