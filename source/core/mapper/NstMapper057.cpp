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
#include "NstMapper057.hpp"

namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper57::SubReset(const bool hard)
		{
			if (hard)
				reg = 0x00;

			Map( 0x8000U, 0x8003U, &Mapper57::Poke_8000 );
			Map( 0x8800U,          &Mapper57::Poke_8800 );
		}
	
		void Mapper57::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					reg = state.Read8();
	
				state.End();
			}
		}
	
		void Mapper57::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write8( reg ).End();
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper57,8000) 
		{
			if (data & 0x40)
			{
				ppu.Update();
	
				chr.SwapBank<NES_8K,0x0000U>
				( 
					((data & 0x03) >> 0) + 
					((reg  & 0x10) >> 1) +
					((reg  & 0x07) >> 0)
				);
			}
		}
	
		NES_POKE(Mapper57,8800) 
		{
			reg = data;
	
			ppu.SetMirroring( (data & 0x8) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
	
			if (data & 0x80)
				prg.SwapBank<NES_32K,0x0000U>( ((data & 0x40) >> 6) + 2 );
			else
				prg.SwapBanks<NES_16K,0x0000U>( (data & 0x60) >> 5, (data & 0x60) >> 5 );
	
			chr.SwapBank<NES_8K,0x0000U>( ((data & 0x07) >> 0) + ((data & 0x10) >> 1) );
		}
	}
}
