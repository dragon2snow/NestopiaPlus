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
#include "NstBrdMmc3China.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Mmc3China::Mmc3China(Context& c,uint flags)
			: Mmc3(c,flags|WRAM_8K) {}

			void Mmc3China::SubReset(bool hard)
			{
				Mmc3::SubReset( hard );

				if (hard)
					std::memset( exRam, 0, sizeof(exRam) );

				Map( 0x5000U, 0x5FFFU, &Mmc3China::Peek_ExRam, &Mmc3China::Poke_ExRam );
				Map( WRK_PEEK_POKE );

				for (uint i=0x0000U; i < 0x2000U; i += 0x2)
					Map( i + 0xA000U, &Mmc3China::Poke_A000 );
			}

			void Mmc3China::BaseLoad(State::Loader& state,const dword id)
			{
				NST_VERIFY( id == NES_STATE_CHUNK_ID('T','M','3','\0') );

				if (id == NES_STATE_CHUNK_ID('T','M','3','\0'))
				{
					while (const dword chunk = state.Begin())
					{
						if (chunk == NES_STATE_CHUNK_ID('R','A','M','\0'))
							state.Uncompress( exRam );

						state.End();
					}
				}
			}

			void Mmc3China::BaseSave(State::Saver& state) const
			{
				state.Begin('T','M','3','\0').Begin('R','A','M','\0').Compress( exRam ).End().End();
			}

			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void Mmc3China::SwapChr(uint address,uint bank) const
			{
				chr.Source( GetChrSource(bank) ).SwapBank<SIZE_1K>( address, bank );
			}

			void Mmc3China::UpdateChr() const
			{
				ppu.Update();

				const uint swap = (regs.ctrl0 & Regs::CTRL0_XOR_CHR) << 5;

				SwapChr( 0x0000U ^ swap, (banks.chr[0] << 1) | 0 );
				SwapChr( 0x0400U ^ swap, (banks.chr[0] << 1) | 1 );
				SwapChr( 0x0800U ^ swap, (banks.chr[1] << 1) | 0 );
				SwapChr( 0x0C00U ^ swap, (banks.chr[1] << 1) | 1 );
				SwapChr( 0x1000U ^ swap,  banks.chr[2]           );
				SwapChr( 0x1400U ^ swap,  banks.chr[3]           );
				SwapChr( 0x1800U ^ swap,  banks.chr[4]           );
				SwapChr( 0x1C00U ^ swap,  banks.chr[5]           );
			}

			NES_POKE(Mmc3China,A000)
			{
				static const uchar nmt[4][4] =
				{
					{0,1,0,1},
					{0,0,1,1},
					{0,0,0,0},
					{1,1,1,1}
				};

				ppu.SetMirroring( nmt[data & 0x3] );
			}

			NES_PEEK(Mmc3China,ExRam)
			{
				return exRam[address-0x5000];
			}

			NES_POKE(Mmc3China,ExRam)
			{
				exRam[address-0x5000] = data;
			}
		}
	}
}
