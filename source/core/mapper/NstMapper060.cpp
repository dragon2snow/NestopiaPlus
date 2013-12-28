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
#include "NstMapper060.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper60::Mapper60(Context& c)
		:
		Mapper       (c,PROM_MAX_128K|CROM_MAX_64K|WRAM_DEFAULT),
		resetTrigger (c.attribute == ATR_RESET_TRIGGER),
		menu         (0)
		{}

		void Mapper60::SubReset(const bool hard)
		{
			latch = 0;

			if (resetTrigger)
			{
				if (hard)
					menu = 0;
				else
					menu = (menu + 1) & 0x3;

				chr.SwapBank<SIZE_8K,0x0000U>( menu );
				prg.SwapBanks<SIZE_16K,0x0000U>( menu, menu );
			}
			else
			{
				Map( 0x8000U, 0xFFFFU, &Mapper60::Peek_Prg, &Mapper60::Poke_Prg );
				NES_CALL_POKE(Mapper60,Prg,0x8000U,0x00);
			}
		}

		void Mapper60::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
				{
					latch = state.Read8();
					menu = latch & 0x3;
					latch = latch << 1 & 0x100;
				}

				state.End();
			}
		}

		void Mapper60::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write8( latch >> 1 | menu ).End();
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_PEEK(Mapper60,Prg)
		{
			return !latch ? prg.Peek( address - 0x8000U ) : menu;
		}

		NES_POKE(Mapper60,Prg)
		{
			latch = address & 0x100;
			ppu.SetMirroring( (address & 0x8) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
			prg.SwapBanks<SIZE_16K,0x0000U>( (address >> 4) & ~(~address >> 7 & 0x1), (address >> 4) | (~address >> 7 & 0x1) );
			chr.SwapBank<SIZE_8K,0x0000U>( address );
		}
	}
}
