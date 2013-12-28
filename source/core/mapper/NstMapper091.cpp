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
#include "NstMapper091.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper91::Mapper91(Context& c)
		:
		Mapper (c,CROM_MAX_512K|WRAM_NONE),
		irq    (c.cpu,c.ppu)
		{}

		void Mapper91::SubReset(const bool hard)
		{
			irq.Reset( hard, hard || irq.IsLineEnabled() );

			for (uint i=0x0000U; i < 0x1000U; i += 0x4)
			{
				Map( 0x6000U + i, CHR_SWAP_2K_0 );
				Map( 0x6001U + i, CHR_SWAP_2K_1 );
				Map( 0x6002U + i, CHR_SWAP_2K_2 );
				Map( 0x6003U + i, CHR_SWAP_2K_3 );
				Map( 0x7000U + i, PRG_SWAP_8K_0 );
				Map( 0x7001U + i, PRG_SWAP_8K_1 );
				Map( 0x7002U + i, &Mapper91::Poke_7002 );
				Map( 0x7003U + i, &Mapper91::Poke_7003 );
			}
		}

		void Mapper91::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('I','R','Q','\0'))
					irq.unit.LoadState( state );

				state.End();
			}
		}

		void Mapper91::SubSave(State::Saver& state) const
		{
			irq.unit.SaveState( State::Saver::Subset(state,'I','R','Q','\0').Ref() );
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE(Mapper91,7002)
		{
			irq.Update();
			irq.unit.Disable( cpu );
			irq.unit.SetLatch( 0 );
		}

		NES_POKE(Mapper91,7003)
		{
			irq.Update();
			irq.unit.Enable();
			irq.unit.SetLatch( 7 );
		}

		void Mapper91::VSync()
		{
			irq.VSync();
		}
	}
}
