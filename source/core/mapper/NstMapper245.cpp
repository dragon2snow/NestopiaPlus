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
#include "NstMapper245.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper245::Mapper245(Context& c)
		: Mmc3(c,WRAM_8K | (c.prgCrc == 0xD3A269DCUL ? CROM_NONE : 0)) {} // Dragon Quest II (J)

		void Mapper245::SubReset(const bool hard)
		{
			if (hard)
				exReg = 0;

			Mmc3::SubReset( hard );

			banks.prg[2] = 0x3E;
			banks.prg[3] = 0x3F;

			for (uint i=0x8001U; i < 0x9000; i += 0x2)
				Map( i, &Mapper245::Poke_8001 );

			Map( WRK_PEEK_POKE );
		}

		void Mapper245::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					exReg = (state.Read8() & 0x2) << 5;

				state.End();
			}
		}

		void Mapper245::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write8( exReg >> 5 ).End();
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Mapper245::UpdatePrg()
		{
			const uint i = (regs.ctrl0 & Regs::CTRL0_XOR_PRG) >> 5;

			prg.SwapBanks<SIZE_8K,0x0000U>
			(
				exReg | banks.prg[i],
				exReg | banks.prg[1],
				exReg | banks.prg[i^2],
				exReg | banks.prg[3]
			);
		}

		void Mapper245::UpdateChr() const
		{
			if (!chr.Source().Writable())
				Mmc3::UpdateChr();
		}

		NES_POKE(Mapper245,8001)
		{
			address = (regs.ctrl0 & Regs::CTRL0_MODE);

			if (address >= 6)
			{
				if (banks.prg[address - 6] != data)
				{
					banks.prg[address - 6] = data;
					Mapper245::UpdatePrg();
				}
			}
			else
			{
				if (address < 2)
				{
					if (address == 0)
					{
						const uint tmp = (data & 0x2) << 5;

						if (exReg != tmp)
						{
							exReg = tmp;
							Mapper245::UpdatePrg();
						}
					}

					data >>= 1;
				}

				if (banks.chr[address] != data)
				{
					banks.chr[address] = data;
					Mapper245::UpdateChr();
				}
			}
		}
	}
}
