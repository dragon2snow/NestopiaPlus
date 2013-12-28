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
#include "NstMapper037.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper37::SubReset(const bool hard)
		{
			if (hard)
				exReg = 0x0;

			Mmc3::SubReset( hard );

			Map( 0x6000U, 0x7FFFU, &Mapper37::Poke_6000 );
		}

		void Mapper37::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == AsciiId<'R','E','G'>::V)
					exReg = state.Read8() & 0x3;

				state.End();
			}
		}

		void Mapper37::SubSave(State::Saver& state) const
		{
			state.Begin( AsciiId<'R','E','G'>::V ).Write8( exReg ).End();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE(Mapper37,6000)
		{
			data = data >> 1 & 0x3;

			if (exReg != data)
			{
				exReg = data;
				Mapper37::UpdatePrg();
				Mapper37::UpdateChr();
			}
		}

		void Mapper37::UpdatePrg()
		{
			const uint base = exReg << 3;
			const uint swap = (regs.ctrl0 & Regs::CTRL0_XOR_PRG) << 8;

			prg.SwapBank<SIZE_8K>( 0x0000 ^ swap, base | (banks.prg[0] & 0xF) );
			prg.SwapBank<SIZE_8K>( 0x2000,        base | (banks.prg[1] & 0xF) );
			prg.SwapBank<SIZE_8K>( 0x4000 ^ swap, base | (base == 0x10 ? 0xE : 0x6) );
			prg.SwapBank<SIZE_8K>( 0x6000,        base | (base == 0x10 ? 0xF : 0x7) );
		}

		void Mapper37::UpdateChr() const
		{
			ppu.Update();

			const uint base = exReg << 6 & 0x80;
			const uint swap = (regs.ctrl0 & Regs::CTRL0_XOR_CHR) << 5;

			chr.SwapBanks<SIZE_2K>
			(
				0x0000 ^ swap,
				base >> 1 | (banks.chr[0] & 0x3F),
				base >> 1 | (banks.chr[1] & 0x3F)
			);

			chr.SwapBanks<SIZE_1K>
			(
				0x1000 ^ swap,
				base | (banks.chr[2] & 0x7F),
				base | (banks.chr[3] & 0x7F),
				base | (banks.chr[4] & 0x7F),
				base | (banks.chr[5] & 0x7F)
			);
		}
	}
}
