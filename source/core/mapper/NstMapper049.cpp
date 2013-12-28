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
#include "NstMapper049.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper49::SubReset(const bool hard)
		{
			if (hard)
				exReg = 0x00;

			Mmc3::SubReset( hard );

			Map( 0x6000U, 0x7FFFU, &Mapper49::Poke_6000 );
		}

		void Mapper49::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == AsciiId<'R','E','G'>::V)
					exReg = state.Read8();

				state.End();
			}
		}

		void Mapper49::SubSave(State::Saver& state) const
		{
			state.Begin( AsciiId<'R','E','G'>::V ).Write8( exReg ).End();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE_D(Mapper49,6000)
		{
			if (regs.ctrl1 & 0x80)
			{
				exReg = data;
				Mapper49::UpdatePrg();
				Mapper49::UpdateChr();
			}
		}

		void Mapper49::UpdatePrg()
		{
			if (exReg & 0x1)
			{
				const uint r = exReg >> 2 & 0x30;
				const uint i = (regs.ctrl0 & Regs::CTRL0_XOR_PRG) >> 5;

				prg.SwapBanks<SIZE_8K,0x0000>
				(
					(banks.prg[i]   & 0x0F) | r,
					(banks.prg[1]   & 0x0F) | r,
					(banks.prg[i^2] & 0x0F) | r,
					(banks.prg[3]   & 0x0F) | r
				);
			}
			else
			{
				prg.SwapBank<SIZE_32K,0x0000>( exReg >> 4 & 0x3 );
			}
		}

		void Mapper49::UpdateChr() const
		{
			ppu.Update();

			uint base = exReg & 0xC0;
			const uint swap = (regs.ctrl0 & Regs::CTRL0_XOR_CHR) << 5;

			chr.SwapBanks<SIZE_2K>
			(
				0x0000 ^ swap,
				(banks.chr[0] & 0x3F) | base,
				(banks.chr[1] & 0x3F) | base
			);

			base <<= 1;

			chr.SwapBanks<SIZE_1K>
			(
				0x1000 ^ swap,
				(banks.chr[2] & 0x7F) | base,
				(banks.chr[3] & 0x7F) | base,
				(banks.chr[4] & 0x7F) | base,
				(banks.chr[5] & 0x7F) | base
			);
		}
	}
}
