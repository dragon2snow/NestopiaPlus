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
#include "NstMapper182.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper182::Mapper182(Context& c)
		:
		Mapper (c),
		irq    (c.cpu,c.ppu)
		{}

		void Mapper182::SubReset(const bool hard)
		{
			if (hard)
				command = 0;

			irq.Reset( hard, hard || irq.IsLineEnabled() );

			for (uint i=0x0000U; i < 0x1000U; i += 0x4)
			{
				Map( 0x8001U + i, NMT_SWAP_HV );
				Map( 0xA000U + i, &Mapper182::Poke_A000 );
				Map( 0xC000U + i, &Mapper182::Poke_C000 );
				Map( 0xE003U + i, &Mapper182::Poke_E003 );
			}
		}

		void Mapper182::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case NES_STATE_CHUNK_ID('R','E','G','\0'):

						command = state.Read8();
						break;

					case NES_STATE_CHUNK_ID('I','R','Q','\0'):

						irq.unit.LoadState( state );
						break;
				}

				state.End();
			}
		}

		void Mapper182::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write8( command ).End();
			irq.unit.SaveState( State::Saver::Subset(state,'I','R','Q','\0').Ref() );
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE(Mapper182,A000)
		{
			command = data;
		}

		NES_POKE(Mapper182,C000)
		{
			ppu.Update();

			switch (command & 0x7)
			{
				case 0x0: chr.SwapBank<SIZE_2K,0x0000U>(data >> 1);  break;
				case 0x1: chr.SwapBank<SIZE_1K,0x1400U>(data);       break;
				case 0x2: chr.SwapBank<SIZE_2K,0x0800U>(data >> 1);  break;
				case 0x3: chr.SwapBank<SIZE_1K,0x1C00U>(data);       break;
				case 0x4: prg.SwapBank<SIZE_8K,0x0000U>(data);       break;
				case 0x5: prg.SwapBank<SIZE_8K,0x2000U>(data);       break;
				case 0x6: chr.SwapBank<SIZE_1K,0x1000U>(data);       break;
				case 0x7: chr.SwapBank<SIZE_1K,0x1800U>(data);      break;
			}
		}

		NES_POKE(Mapper182,E003)
		{
			irq.Update();

			if (data)
			{
				irq.ClearIRQ();
				irq.unit.Enable();
				irq.unit.SetLatch( data );
				irq.unit.Reload();
			}
			else
			{
				irq.unit.Disable( cpu );
			}
		}

		void Mapper182::VSync()
		{
			irq.VSync();
		}
	}
}
