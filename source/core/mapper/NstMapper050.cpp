////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2005 Martin Freij
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
#include "NstMapper050.hpp"

namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		Mapper50::Mapper50(Context& c)
		: Mapper(c,WRAM_NONE|CROM_NONE), irq(c.cpu) {}

		void Mapper50::Irq::Reset(const bool hard)
		{
			if (hard)
				count = 0;
		}

		void Mapper50::SubReset(const bool hard)
		{
			if (hard)
				prg.SwapBanks<NES_8K,0x0000U>( 8, 9, 0, 11 );

			irq.Reset( hard, hard ? false : irq.IsLineEnabled() );

			for (uint i=0x4020U; i < 0x6000U; ++i)
			{
				if ((i & 0xE060U) == 0x4020U)
					Map( i, (i & 0x0100U) ? &Mapper50::Poke_4120 : &Mapper50::Poke_4020 );
			}
	
			Map( 0x6000U, 0x7FFFU, &Mapper50::Peek_wRom );
		}
	
		void Mapper50::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('I','R','Q','\0'))
				{
					const State::Loader::Data<3> data( state );
					irq.EnableLine( data[0] & 0x1 );
					irq.unit.count = data[1] | (data[2] << 8);
				}
	
				state.End();
			}
		}
	
		void Mapper50::SubSave(State::Saver& state) const
		{
			const u8 data[3] =
			{
				irq.IsLineEnabled() ? 0x1 : 0x0,
				irq.unit.count & 0xFF,
				irq.unit.count >> 8
			};

			state.Begin('I','R','Q','\0').Write( data ).End();
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_PEEK(Mapper50,wRom)
		{
			return *prg.Source().Mem( (0x1E000UL - 0x6000U) + address );
		}
	
		NES_POKE(Mapper50,4020) 
		{
			prg.SwapBank<NES_8K,0x4000U>
			( 
				((data & 0x8) << 0) |
				((data & 0x1) << 2)	|
				((data & 0x6) >> 1)
			);
		}
	
		NES_POKE(Mapper50,4120) 
		{
			irq.Update();
			irq.EnableLine( data & 0x1 );
			irq.ClearIRQ();
		}

		ibool Mapper50::Irq::Signal()
		{
			return ++count == 0x1000U;
		}

		void Mapper50::VSync()
		{
			irq.unit.count = 0;
			irq.VSync();
		}
	}
}
