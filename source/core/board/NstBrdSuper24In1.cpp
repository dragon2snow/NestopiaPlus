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
#include "NstBrdMmc3.hpp"
#include "NstBrdSuper24In1.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			void Super24In1::SubReset(const bool hard)
			{
				if (hard)
				{
					exRegs[0] = EXREG_USE_CRAM | 0x4;
					exRegs[1] = 0x9FU << 1;
					exRegs[2] = 0x00U;
				}

				Mmc3::SubReset( hard );

				Map( 0x5FF0U, &Super24In1::Poke_5FF0 );
				Map( 0x5FF1U, &Super24In1::Poke_5FF1 );
				Map( 0x5FF2U, &Super24In1::Poke_5FF2 );
			}

			void Super24In1::SubLoad(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					{
						const State::Loader::Data<3> data( state );

						exRegs[0] = data[0] << 0;
						exRegs[1] = data[1] << 1;
						exRegs[2] = data[2] << 2;
					}

					state.End();
				}
			}

			void Super24In1::SubSave(State::Saver& state) const
			{
				const u8 data[3] =
				{
					exRegs[0] >> 0,
					exRegs[1] >> 1,
					exRegs[2] >> 2
				};

				state.Begin('R','E','G','\0').Write( data ).End();
			}

			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("", on)
			#endif

			NES_POKE(Super24In1,5FF0)
			{
				if (exRegs[0] != data)
				{
					exRegs[0] = data;
					Super24In1::UpdatePrg();
					Super24In1::UpdateChr();
				}
			}

			NES_POKE(Super24In1,5FF1)
			{
				if (exRegs[1] != (data << 1))
				{
					exRegs[1] = data << 1;
					Super24In1::UpdatePrg();
				}
			}

			NES_POKE(Super24In1,5FF2)
			{
				if (exRegs[2] != (data << 2))
				{
					exRegs[2] = data << 2;
					Super24In1::UpdateChr();
				}
			}

			void Super24In1::UpdatePrg()
			{
				static const uchar masks[8] = {0x3F,0x1F,0xF,0x1,0x3,0x0,0x0,0x0};

				const uint m = masks[exRegs[0] & EXREG_PRG_BANKS];
				const uint i = (regs.ctrl0 & Regs::CTRL0_XOR_PRG) >> 5;

				prg.SwapBanks<SIZE_8K,0x0000U>
				(
					exRegs[1] | (banks.prg[i]   & m),
					exRegs[1] | (banks.prg[1]   & m),
					exRegs[1] | (banks.prg[i^2] & m),
					exRegs[1] | (banks.prg[3]   & m)
				);
			}

			void Super24In1::UpdateChr() const
			{
				ppu.Update();

				if (exRegs[0] & EXREG_USE_CRAM)
				{
					chr.Source(1).SwapBank<SIZE_8K,0x0000U>(0);
				}
				else
				{
					const uint swap = (regs.ctrl0 & Regs::CTRL0_XOR_CHR) << 5;

					chr.SwapBanks<SIZE_2K>
					(
						0x0000U ^ swap,
						exRegs[2] + banks.chr[0],
						exRegs[2] + banks.chr[1]
					);

					chr.SwapBanks<SIZE_1K>
					(
						0x1000U ^ swap,
						(exRegs[2] << 1) + banks.chr[2],
						(exRegs[2] << 1) + banks.chr[3],
						(exRegs[2] << 1) + banks.chr[4],
						(exRegs[2] << 1) + banks.chr[5]
					);
				}
			}
		}
	}
}
