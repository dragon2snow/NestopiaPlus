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
#include "NstBrdStreetHeroes.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			void StreetHeroes::SubReset(const bool hard)
			{
				if (hard)
				{
					exRegs[0] = 0x00;
					exRegs[1] = 0x00;
				}
				else
				{
					exRegs[0] ^= 0xFF;
				}

				Mmc3::SubReset( hard );

				Map( 0x4100U, &StreetHeroes::Peek_4100, &StreetHeroes::Poke_4100 );
			}

			void StreetHeroes::SubLoad(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					{
						const State::Loader::Data<2> data( state );

						exRegs[0] = (data[0] & 0x1) ? 0xFF : 0x00;
						exRegs[1] = data[1];
					}

					state.End();
				}
			}

			void StreetHeroes::SubSave(State::Saver& state) const
			{
				const u8 data[2] = {exRegs[0] ? 0x1 : 0x0, exRegs[1]};
				state.Begin('R','E','G','\0').Write( data ).End();
			}

			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void StreetHeroes::UpdateChr() const
			{
				ppu.Update();

				if (exRegs[1] & 0x40)
				{
					chr.Source(1).SwapBank<SIZE_8K,0x0000U>(0);
				}
				else
				{
					const uint swap = (regs.ctrl0 & Regs::CTRL0_XOR_CHR) << 5;

					chr.SwapBanks<SIZE_2K>
					(
						0x0000U ^ swap,
						banks.chr[0] | (exRegs[1] << (swap ? 7 : 4) & 0x80),
						banks.chr[1] | (exRegs[1] << (swap ? 6 : 5) & 0x80)
					);

					chr.SwapBanks<SIZE_1K>
					(
						0x1000U ^ swap,
						banks.chr[2] | (exRegs[1] << (swap ? 5 : 8) & 0x100),
						banks.chr[3] | (exRegs[1] << (swap ? 5 : 8) & 0x100),
						banks.chr[4] | (exRegs[1] << (swap ? 6 : 7) & 0x100),
						banks.chr[5] | (exRegs[1] << (swap ? 6 : 7) & 0x100)
					);
				}
			}

			NES_POKE(StreetHeroes,4100)
			{
				if (exRegs[1] != data)
				{
					exRegs[1] = data;
					StreetHeroes::UpdateChr();
				}
			}

			NES_PEEK(StreetHeroes,4100)
			{
				return exRegs[0];
			}
		}
	}
}
