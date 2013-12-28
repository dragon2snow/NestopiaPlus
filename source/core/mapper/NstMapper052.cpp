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
#include "NstMapper052.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper52::SubReset(const bool hard)
		{
			if (hard)
			{
				exRegs[0] = 0;
				exRegs[1] = 0;
			}

			Mmc3::SubReset( hard );

			Map( 0x6000U, 0x7FFFU, &Mapper52::Poke_Wrk );
		}

		void Mapper52::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == AsciiId<'R','E','G'>::V)
				{
					State::Loader::Data<2> data( state );

					exRegs[0] = data[0];
					exRegs[1] = data[1];
				}

				state.End();
			}
		}

		void Mapper52::SubSave(State::Saver& state) const
		{
			const byte data[2] =
			{
				exRegs[0],
				exRegs[1]
			};

			state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE_AD(Mapper52,Wrk)
		{
			if (exRegs[1])
			{
				NST_VERIFY( wrk.Writable(0) );

				if (wrk.Writable(0))
					wrk[0][address - 0x6000] = data;
			}
			else
			{
				exRegs[1] = 1;
				exRegs[0] = data;

				Mapper52::UpdatePrg();
				Mapper52::UpdateChr();
			}
		}

		void Mapper52::UpdatePrg()
		{
			const uint r[2] =
			{
				(exRegs[0] << 1 & 0x10) ^ 0x1F,
				((exRegs[0] & 0x6) | (exRegs[0] >> 3 & exRegs[0] & 0x1)) << 4
			};

			const uint i = (regs.ctrl0 & Regs::CTRL0_XOR_PRG) >> 5;

			prg.SwapBanks<SIZE_8K,0x0000>
			(
				(banks.prg[i]   & r[0]) | r[1],
				(banks.prg[1]   & r[0]) | r[1],
				(banks.prg[i^2] & r[0]) | r[1],
				(banks.prg[3]   & r[0]) | r[1]
			);
		}

		void Mapper52::UpdateChr() const
		{
			ppu.Update();

			const uint r[2] =
			{
				((exRegs[0] & 0x40) << 1) ^ 0xFF,
				((exRegs[0] >> 3 & 0x4) | (exRegs[0] >> 1 & 0x2) | ((exRegs[0] >> 6) & (exRegs[0] >> 4) & 0x1)) << 7
			};

			const uint swap = (regs.ctrl0 & Regs::CTRL0_XOR_CHR) << 5;

			chr.SwapBanks<SIZE_2K>
			(
				0x0000 ^ swap,
				(banks.chr[0] & r[0] >> 1) | (r[1] >> 1),
				(banks.chr[1] & r[0] >> 1) | (r[1] >> 1)
			);

			chr.SwapBanks<SIZE_1K>
			(
				0x1000 ^ swap,
				(banks.chr[2] & r[0]) | r[1],
				(banks.chr[3] & r[0]) | r[1],
				(banks.chr[4] & r[0]) | r[1],
				(banks.chr[5] & r[0]) | r[1]
			);
		}
	}
}
