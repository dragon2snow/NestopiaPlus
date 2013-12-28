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
#include "NstMapper121.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper121::SubReset(const bool hard)
		{
			exMode = 0;

			Mmc3::SubReset( hard );

			Map( 0x5000U, 0x5FFFU, &Mapper121::Peek_5000 );
			Map( 0x8000U, 0x9FFFU, &Mapper121::Poke_8000 );
		}

		void Mapper121::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == AsciiId<'R','E','G'>::V)
				{
					exMode = state.Read8();

					if (exMode != 0xAB && exMode != 0xFF)
						exMode = 0;
				}

				state.End();
			}
		}

		void Mapper121::SubSave(State::Saver& state) const
		{
			state.Begin( AsciiId<'R','E','G'>::V ).Write8( exMode ).End();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Mapper121::UpdatePrg()
		{
			const uint i = (regs.ctrl0 & Regs::CTRL0_XOR_PRG) >> 5;

			prg.SwapBanks<SIZE_8K,0x0000>
			(
				banks.prg[i],
				banks.prg[1],
				banks.prg[i^2],
				exMode == 0xAB ? 7 : exMode == 0xFF ? 9 : banks.prg[3]
			);
		}

		NES_PEEK(Mapper121,5000)
		{
			return 0x9F;
		}

		NES_POKE_AD(Mapper121,8000)
		{
			if ((address & 0xF003) == 0x8003 && (data == 0xAB || data == 0xFF))
			{
				exMode = data;
			}
			else
			{
				exMode = 0;

				if (address & 0x1)
					Mmc3::NES_DO_POKE(8001,address,data);
				else
					Mmc3::NES_DO_POKE(8000,address,data);
			}

			Mapper121::UpdatePrg();
		}
	}
}
