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
#include "NstMapper027.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper27::Mapper27(Context& c)
		:
		Mapper (c,WRAM_DEFAULT),
		irq    (c.cpu)
		{}

		void Mapper27::SubReset(const bool hard)
		{
			if (hard)
				prgSwap = 0;

			irq.Reset( hard, hard ? false : irq.Connected() );

			for (dword i=0x8000; i <= 0xFFFF; ++i)
			{
				switch (i & 0xF0CF)
				{
					case 0x8000: Map( i, &Mapper27::Poke_8  ); break;
					case 0x9000: Map( i, NMT_SWAP_VH01      ); break;
					case 0x9002:
					case 0x9080: Map( i, &Mapper27::Poke_9  ); break;
					case 0xA000: Map( i, PRG_SWAP_8K_1      ); break;
					case 0xB000: Map( i, &Mapper27::Poke_B0 ); break;
					case 0xB001: Map( i, &Mapper27::Poke_B1 ); break;
					case 0xB002: Map( i, &Mapper27::Poke_B2 ); break;
					case 0xB003: Map( i, &Mapper27::Poke_B3 ); break;
					case 0xC000: Map( i, &Mapper27::Poke_C0 ); break;
					case 0xC001: Map( i, &Mapper27::Poke_C1 ); break;
					case 0xC002: Map( i, &Mapper27::Poke_C2 ); break;
					case 0xC003: Map( i, &Mapper27::Poke_C3 ); break;
					case 0xD000: Map( i, &Mapper27::Poke_D0 ); break;
					case 0xD001: Map( i, &Mapper27::Poke_D1 ); break;
					case 0xD002: Map( i, &Mapper27::Poke_D2 ); break;
					case 0xD003: Map( i, &Mapper27::Poke_D3 ); break;
					case 0xE000: Map( i, &Mapper27::Poke_E0 ); break;
					case 0xE001: Map( i, &Mapper27::Poke_E1 ); break;
					case 0xE002: Map( i, &Mapper27::Poke_E2 ); break;
					case 0xE003: Map( i, &Mapper27::Poke_E3 ); break;
					case 0xF000: Map( i, &Mapper27::Poke_F0 ); break;
					case 0xF001: Map( i, &Mapper27::Poke_F1 ); break;
					case 0xF002: Map( i, &Mapper27::Poke_F2 ); break;
					case 0xF003: Map( i, &Mapper27::Poke_F3 ); break;
				}
			}
		}

		void Mapper27::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case AsciiId<'R','E','G'>::V:

						prgSwap = state.Read8() & 0x2;
						break;

					case AsciiId<'I','R','Q'>::V:

						irq.LoadState( state );
						break;
				}

				state.End();
			}
		}

		void Mapper27::SubSave(State::Saver& state) const
		{
			state.Begin( AsciiId<'R','E','G'>::V ).Write8( prgSwap ).End();
			irq.SaveState( state, AsciiId<'I','R','Q'>::V );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE_D(Mapper27,8)
		{
			prg.SwapBank<SIZE_8K>( prgSwap << 13, data );
		}

		NES_POKE_D(Mapper27,9)
		{
			data &= 0x2;

			if (prgSwap != data)
			{
				prgSwap = data;

				prg.SwapBanks<SIZE_8K,0x0000>
				(
					prg.GetBank<SIZE_8K,0x4000>(),
					prg.GetBank<SIZE_8K,0x0000>()
				);
			}
		}

		template<uint MASK,uint BITS,uint SHIFT>
		void Mapper27::SwapChr(const uint address,const uint data) const
		{
			ppu.Update();
			chr.SwapBank<SIZE_1K>( address, (chr.GetBank<SIZE_1K>(address) & MASK) | (data & BITS) << SHIFT );
		}

		NES_POKE_D(Mapper27,B0) { SwapChr<0xFF0,0x0F,0>( 0x0000, data ); }
		NES_POKE_D(Mapper27,B1) { SwapChr<0x00F,0xFF,4>( 0x0000, data ); }
		NES_POKE_D(Mapper27,B2) { SwapChr<0xFF0,0x0F,0>( 0x0400, data ); }
		NES_POKE_D(Mapper27,B3) { SwapChr<0x00F,0xFF,4>( 0x0400, data ); }
		NES_POKE_D(Mapper27,C0) { SwapChr<0xFF0,0x0F,0>( 0x0800, data ); }
		NES_POKE_D(Mapper27,C1) { SwapChr<0x00F,0xFF,4>( 0x0800, data ); }
		NES_POKE_D(Mapper27,C2) { SwapChr<0xFF0,0x0F,0>( 0x0C00, data ); }
		NES_POKE_D(Mapper27,C3) { SwapChr<0x00F,0xFF,4>( 0x0C00, data ); }
		NES_POKE_D(Mapper27,D0) { SwapChr<0xFF0,0x0F,0>( 0x1000, data ); }
		NES_POKE_D(Mapper27,D1) { SwapChr<0x00F,0xFF,4>( 0x1000, data ); }
		NES_POKE_D(Mapper27,D2) { SwapChr<0xFF0,0x0F,0>( 0x1400, data ); }
		NES_POKE_D(Mapper27,D3) { SwapChr<0x00F,0xFF,4>( 0x1400, data ); }
		NES_POKE_D(Mapper27,E0) { SwapChr<0xFF0,0x0F,0>( 0x1800, data ); }
		NES_POKE_D(Mapper27,E1) { SwapChr<0x00F,0xFF,4>( 0x1800, data ); }
		NES_POKE_D(Mapper27,E2) { SwapChr<0xFF0,0x0F,0>( 0x1C00, data ); }
		NES_POKE_D(Mapper27,E3) { SwapChr<0x00F,0xFF,4>( 0x1C00, data ); }

		NES_POKE_D(Mapper27,F0)
		{
			irq.WriteLatch0( data );
		}

		NES_POKE_D(Mapper27,F1)
		{
			irq.WriteLatch1( data );
		}

		NES_POKE_D(Mapper27,F2)
		{
			irq.Toggle( data );
		}

		NES_POKE(Mapper27,F3)
		{
			irq.Toggle();
		}

		void Mapper27::Sync(Event event,Input::Controllers*)
		{
			if (event == EVENT_END_FRAME)
				irq.VSync();
		}
	}
}
