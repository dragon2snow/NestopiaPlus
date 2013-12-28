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
#include "NstBrdTaitoX.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			TaitoX::TaitoX(Context& c,Type t)
			:
			Mapper (c,CROM_MAX_128K | (type == TYPE_A ? NMT_DEFAULT : NMT_HORIZONTAL)),
			type   (t)
			{}

			void TaitoX::SubReset(bool)
			{
				Map( 0x7EF0U, type == TYPE_A ? &TaitoX::Poke_7EF0_0 : &TaitoX::Poke_7EF0_1 );
				Map( 0x7EF1U, type == TYPE_A ? &TaitoX::Poke_7EF1_0 : &TaitoX::Poke_7EF1_1 );

				Map( 0x7EF2U, CHR_SWAP_1K_4 );
				Map( 0x7EF3U, CHR_SWAP_1K_5 );
				Map( 0x7EF4U, CHR_SWAP_1K_6 );
				Map( 0x7EF5U, CHR_SWAP_1K_7 );
				Map( 0x7EF6U, NMT_SWAP_VH   );

				Map( 0x7EFAU, 0x7EFBU, PRG_SWAP_8K_0 );
				Map( 0x7EFCU, 0x7EFDU, PRG_SWAP_8K_1 );
				Map( 0x7EFEU, 0x7EFFU, PRG_SWAP_8K_2 );
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void TaitoX::SwapChr(const uint address,const uint data) const
			{
				ppu.Update();
				chr.SwapBank<SIZE_2K>( address, data >> 1 );
			}

			NES_POKE_D(TaitoX,7EF0_0)
			{
				SwapChr( 0x0000, data );
			}

			NES_POKE_D(TaitoX,7EF0_1)
			{
				SwapChr( 0x0000, data );
				nmt.SwapBanks<SIZE_1K,0x0000>( data >> 7, data >> 7 );
			}

			NES_POKE_D(TaitoX,7EF1_0)
			{
				SwapChr( 0x0800U, data );
			}

			NES_POKE_D(TaitoX,7EF1_1)
			{
				SwapChr( 0x0800, data );
				nmt.SwapBanks<SIZE_1K,0x0800>( data >> 7, data >> 7 );
			}
		}
	}
}
