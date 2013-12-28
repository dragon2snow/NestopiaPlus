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
#include "../NstClock.hpp"
#include "../board/NstBrdVrc4.hpp"
#include "NstMapper252.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper252::Mapper252(Context& c)
		:
		Mapper (c,CROM_MAX_256K),
		irq    (c.cpu)
		{}

		void Mapper252::SubReset(const bool hard)
		{
			irq.Reset( hard, hard ? false : irq.Connected() );

			Map( 0x8000U, 0x8FFFU, PRG_SWAP_8K_0 );
			Map( 0xA000U, 0xAFFFU, PRG_SWAP_8K_1 );

			for (uint i=0x0000; i < 0x1000; i += 0x10)
			{
				Map( 0xB000 + i, 0xB003 + i, &Mapper252::Poke_B000 );
				Map( 0xB004 + i, 0xB007 + i, &Mapper252::Poke_B004 );
				Map( 0xB008 + i, 0xB00B + i, &Mapper252::Poke_B008 );
				Map( 0xB00C + i, 0xB00F + i, &Mapper252::Poke_B00C );
				Map( 0xC000 + i, 0xC003 + i, &Mapper252::Poke_C000 );
				Map( 0xC004 + i, 0xC007 + i, &Mapper252::Poke_C004 );
				Map( 0xC008 + i, 0xC00B + i, &Mapper252::Poke_C008 );
				Map( 0xC00C + i, 0xC00F + i, &Mapper252::Poke_C00C );
				Map( 0xD000 + i, 0xD003 + i, &Mapper252::Poke_D000 );
				Map( 0xD004 + i, 0xD007 + i, &Mapper252::Poke_D004 );
				Map( 0xD008 + i, 0xD00B + i, &Mapper252::Poke_D008 );
				Map( 0xD00C + i, 0xD00F + i, &Mapper252::Poke_D00C );
				Map( 0xE000 + i, 0xE003 + i, &Mapper252::Poke_E000 );
				Map( 0xE004 + i, 0xE007 + i, &Mapper252::Poke_E004 );
				Map( 0xE008 + i, 0xE00B + i, &Mapper252::Poke_E008 );
				Map( 0xE00C + i, 0xE00F + i, &Mapper252::Poke_E00C );
				Map( 0xF000 + i, 0xF003 + i, &Mapper252::Poke_F000 );
				Map( 0xF004 + i, 0xF007 + i, &Mapper252::Poke_F004 );
				Map( 0xF008 + i, 0xF00B + i, &Mapper252::Poke_F008 );
				Map( 0xF00C + i, 0xF00F + i, &Mapper252::Poke_F00C );
			}

			// hack
			chr.Source().WriteEnable( true );
		}

		void Mapper252::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == AsciiId<'I','R','Q'>::V)
					irq.LoadState( state );

				state.End();
			}
		}

		void Mapper252::SubSave(State::Saver& state) const
		{
			irq.SaveState( state, AsciiId<'I','R','Q'>::V );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		template<uint MASK,uint SHIFT>
		void Mapper252::SwapChr(const uint address,const uint data) const
		{
			ppu.Update();
			chr.SwapBank<SIZE_1K>( address, (chr.GetBank<SIZE_1K>(address) & MASK) | (data & 0xF) << SHIFT );
		}

		NES_POKE_D(Mapper252,B000) { SwapChr<0xF0,0>( 0x0000, data ); }
		NES_POKE_D(Mapper252,B004) { SwapChr<0x0F,4>( 0x0000, data ); }
		NES_POKE_D(Mapper252,B008) { SwapChr<0xF0,0>( 0x0400, data ); }
		NES_POKE_D(Mapper252,B00C) { SwapChr<0x0F,4>( 0x0400, data ); }
		NES_POKE_D(Mapper252,C000) { SwapChr<0xF0,0>( 0x0800, data ); }
		NES_POKE_D(Mapper252,C004) { SwapChr<0x0F,4>( 0x0800, data ); }
		NES_POKE_D(Mapper252,C008) { SwapChr<0xF0,0>( 0x0C00, data ); }
		NES_POKE_D(Mapper252,C00C) { SwapChr<0x0F,4>( 0x0C00, data ); }
		NES_POKE_D(Mapper252,D000) { SwapChr<0xF0,0>( 0x1000, data ); }
		NES_POKE_D(Mapper252,D004) { SwapChr<0x0F,4>( 0x1000, data ); }
		NES_POKE_D(Mapper252,D008) { SwapChr<0xF0,0>( 0x1400, data ); }
		NES_POKE_D(Mapper252,D00C) { SwapChr<0x0F,4>( 0x1400, data ); }
		NES_POKE_D(Mapper252,E000) { SwapChr<0xF0,0>( 0x1800, data ); }
		NES_POKE_D(Mapper252,E004) { SwapChr<0x0F,4>( 0x1800, data ); }
		NES_POKE_D(Mapper252,E008) { SwapChr<0xF0,0>( 0x1C00, data ); }
		NES_POKE_D(Mapper252,E00C) { SwapChr<0x0F,4>( 0x1C00, data ); }

		NES_POKE_D(Mapper252,F000)
		{
			irq.Update();
			irq.unit.latch = (irq.unit.latch & 0xF0) | (data & 0x0F);
		}

		NES_POKE_D(Mapper252,F004)
		{
			irq.Update();
			irq.unit.latch = (irq.unit.latch & 0x0F) | (data & 0x0F) << 4;
		}

		NES_POKE_D(Mapper252,F008)
		{
			irq.Toggle( data );
		}

		NES_POKE(Mapper252,F00C)
		{
			irq.Toggle();
		}

		void Mapper252::Sync(Event event,Input::Controllers*)
		{
			if (event == EVENT_END_FRAME)
				irq.VSync();
		}
	}
}
