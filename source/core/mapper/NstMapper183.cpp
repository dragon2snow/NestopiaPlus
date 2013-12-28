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
#include "../NstClock.hpp"
#include "NstMapper183.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper183::Mapper183(Context& c)
		:
		Mapper (c,CROM_MAX_256K|WRAM_NONE),
		irq    (c.cpu)
		{}

		void Mapper183::Irq::Reset(const bool hard)
		{
			if (hard)
			{
				enabled = false;
				count[0] = 0;
				count[1] = 0;
			}
		}

		void Mapper183::SubReset(const bool hard)
		{
			irq.Reset( hard, true );

			Map( WRK_PEEK );

			for (uint i=0x0000; i < 0x0800; i += 0x10)
			{
				Map( 0x8800U + i, 0x8803U + i, PRG_SWAP_8K_0 );
				Map( 0x9800U + i, 0x9803U + i, NMT_SWAP_VH01 );
				Map( 0xA000U + i, 0xA003U + i, PRG_SWAP_8K_2 );
				Map( 0xA800U + i, 0xA803U + i, PRG_SWAP_8K_1 );
			}

			for (uint i=0x000; i < 0x1000; i += 0x10)
			{
				Map( 0xB000U + i, 0xB003U + i, &Mapper183::Poke_B000 );
				Map( 0xB004U + i, 0xB007U + i, &Mapper183::Poke_B004 );
				Map( 0xB008U + i, 0xB00BU + i, &Mapper183::Poke_B008 );
				Map( 0xB00CU + i, 0xB00FU + i, &Mapper183::Poke_B00C );
				Map( 0xC000U + i, 0xC003U + i, &Mapper183::Poke_C000 );
				Map( 0xC004U + i, 0xC007U + i, &Mapper183::Poke_C004 );
				Map( 0xC008U + i, 0xC00BU + i, &Mapper183::Poke_C008 );
				Map( 0xC00CU + i, 0xC00FU + i, &Mapper183::Poke_C00C );
				Map( 0xD000U + i, 0xD003U + i, &Mapper183::Poke_D000 );
				Map( 0xD004U + i, 0xD007U + i, &Mapper183::Poke_D004 );
				Map( 0xD008U + i, 0xD00BU + i, &Mapper183::Poke_D008 );
				Map( 0xD00CU + i, 0xD00FU + i, &Mapper183::Poke_D00C );
				Map( 0xE000U + i, 0xE003U + i, &Mapper183::Poke_E000 );
				Map( 0xE004U + i, 0xE007U + i, &Mapper183::Poke_E004 );
				Map( 0xE008U + i, 0xE00BU + i, &Mapper183::Poke_E008 );
				Map( 0xE00CU + i, 0xE00FU + i, &Mapper183::Poke_E00C );
				Map( 0xF000U + i, 0xF003U + i, &Mapper183::Poke_F000 );
				Map( 0xF004U + i, 0xF007U + i, &Mapper183::Poke_F004 );
				Map( 0xF008U + i, 0xF00BU + i, &Mapper183::Poke_F008 );
				Map( 0xF00CU + i, 0xF00FU + i, &Mapper183::Poke_F00C );
			}
		}

		void Mapper183::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('I','R','Q','\0'))
				{
					const State::Loader::Data<2> data( state );

					irq.unit.enabled = data[0] & 0x1;
					irq.unit.count[1] = data[1];
				}

				state.End();
			}
		}

		void Mapper183::SubSave(State::Saver& state) const
		{
			state.Begin('I','R','Q','\0').Write16( (irq.unit.enabled != false) | ((irq.unit.count[1] & 0xFF) << 8) ).End();
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		ibool Mapper183::Irq::Signal()
		{
			if (++count[0] < 114)
				return false;

			count[0] = 0;
			return enabled && (++count[1] & 0xFF) == 0;
		}

		NES_POKE(Mapper183,B000) { ppu.Update(); chr.SwapBank<SIZE_1K,0x0000U>( (chr.GetBank<SIZE_1K,0x0000U>() & 0xF0) | ((data & 0xF) << 0) ); }
		NES_POKE(Mapper183,B004) { ppu.Update(); chr.SwapBank<SIZE_1K,0x0000U>( (chr.GetBank<SIZE_1K,0x0000U>() & 0x0F) | ((data & 0xF) << 4) ); }
		NES_POKE(Mapper183,B008) { ppu.Update(); chr.SwapBank<SIZE_1K,0x0400U>( (chr.GetBank<SIZE_1K,0x0400U>() & 0xF0) | ((data & 0xF) << 0) ); }
		NES_POKE(Mapper183,B00C) { ppu.Update(); chr.SwapBank<SIZE_1K,0x0400U>( (chr.GetBank<SIZE_1K,0x0400U>() & 0x0F) | ((data & 0xF) << 4) ); }
		NES_POKE(Mapper183,C000) { ppu.Update(); chr.SwapBank<SIZE_1K,0x0800U>( (chr.GetBank<SIZE_1K,0x0800U>() & 0xF0) | ((data & 0xF) << 0) ); }
		NES_POKE(Mapper183,C004) { ppu.Update(); chr.SwapBank<SIZE_1K,0x0800U>( (chr.GetBank<SIZE_1K,0x0800U>() & 0x0F) | ((data & 0xF) << 4) ); }
		NES_POKE(Mapper183,C008) { ppu.Update(); chr.SwapBank<SIZE_1K,0x0C00U>( (chr.GetBank<SIZE_1K,0x0C00U>() & 0xF0) | ((data & 0xF) << 0) ); }
		NES_POKE(Mapper183,C00C) { ppu.Update(); chr.SwapBank<SIZE_1K,0x0C00U>( (chr.GetBank<SIZE_1K,0x0C00U>() & 0x0F) | ((data & 0xF) << 4) ); }
		NES_POKE(Mapper183,D000) { ppu.Update(); chr.SwapBank<SIZE_1K,0x1000U>( (chr.GetBank<SIZE_1K,0x1000U>() & 0xF0) | ((data & 0xF) << 0) ); }
		NES_POKE(Mapper183,D004) { ppu.Update(); chr.SwapBank<SIZE_1K,0x1000U>( (chr.GetBank<SIZE_1K,0x1000U>() & 0x0F) | ((data & 0xF) << 4) ); }
		NES_POKE(Mapper183,D008) { ppu.Update(); chr.SwapBank<SIZE_1K,0x1400U>( (chr.GetBank<SIZE_1K,0x1400U>() & 0xF0) | ((data & 0xF) << 0) ); }
		NES_POKE(Mapper183,D00C) { ppu.Update(); chr.SwapBank<SIZE_1K,0x1400U>( (chr.GetBank<SIZE_1K,0x1400U>() & 0x0F) | ((data & 0xF) << 4) ); }
		NES_POKE(Mapper183,E000) { ppu.Update(); chr.SwapBank<SIZE_1K,0x1800U>( (chr.GetBank<SIZE_1K,0x1800U>() & 0xF0) | ((data & 0xF) << 0) ); }
		NES_POKE(Mapper183,E004) { ppu.Update(); chr.SwapBank<SIZE_1K,0x1800U>( (chr.GetBank<SIZE_1K,0x1800U>() & 0x0F) | ((data & 0xF) << 4) ); }
		NES_POKE(Mapper183,E008) { ppu.Update(); chr.SwapBank<SIZE_1K,0x1C00U>( (chr.GetBank<SIZE_1K,0x1C00U>() & 0xF0) | ((data & 0xF) << 0) ); }
		NES_POKE(Mapper183,E00C) { ppu.Update(); chr.SwapBank<SIZE_1K,0x1C00U>( (chr.GetBank<SIZE_1K,0x1C00U>() & 0x0F) | ((data & 0xF) << 4) ); }

		NES_POKE(Mapper183,F000)
		{
			irq.Update();
			irq.unit.count[1] = (irq.unit.count[1] & 0xF0) | (data << 0 & 0x0F);
		}

		NES_POKE(Mapper183,F004)
		{
			irq.Update();
			irq.unit.count[1] = (irq.unit.count[1] & 0x0F) | (data << 4 & 0xF0);
		}

		NES_POKE(Mapper183,F008)
		{
			irq.Update();
			irq.unit.enabled = data;

			if (!data)
				irq.ClearIRQ();
		}

		NES_POKE(Mapper183,F00C)
		{
		}

		void Mapper183::VSync()
		{
			irq.VSync();
			irq.unit.count[0] = 0;
		}
	}
}
