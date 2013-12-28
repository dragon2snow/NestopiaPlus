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
#include "NstMapper230.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper230::SubReset(const bool hard)
		{
			if (hard)
				romSwitch = 0;
			else
				romSwitch ^= 1;

			prg.SwapBanks<SIZE_16K,0x0000U>( romSwitch ? 0 : 8, romSwitch ? 7 : 39 );

			Map( 0x8000U, 0xFFFFU, &Mapper230::Poke_Prg );

			// for the soft reset triggering feature
			cpu.Poke( 0x2000, 0x00 );
		}

		void Mapper230::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					romSwitch = state.Read8() & 0x1;

				state.End();
			}
		}

		void Mapper230::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write8( romSwitch ).End();
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE(Mapper230,Prg)
		{
			if (romSwitch)
			{
				prg.SwapBank<SIZE_16K,0x0000U>( data & 0x7 );
			}
			else
			{
				ppu.SetMirroring( (data & 0x40) ? Ppu::NMT_VERTICAL : Ppu::NMT_HORIZONTAL );

				if (data & 0x20)
					prg.SwapBanks<SIZE_16K,0x0000U>( 8 + (data & 0x1F), 8 + (data & 0x1F) );
				else
					prg.SwapBank<SIZE_32K,0x0000U>( 4 + (data >> 1 & 0xF) );
			}
		}
	}
}
