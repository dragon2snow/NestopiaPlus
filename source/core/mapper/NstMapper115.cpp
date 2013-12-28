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
#include "../board/NstBrdMmc3.hpp"
#include "NstMapper115.hpp"

namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper115::SubReset(const bool hard)
		{
			if (hard)
				exReg = 0;

			Mmc3::SubReset( hard );
	
			Map( 0x6000U,          &Mapper115::Poke_6000 );
			Map( 0x6001U, 0x7FFFU, &Mapper115::Poke_6001 );
		}
	
		void Mapper115::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					exReg = state.Read8();
	
				state.End();
			}
		}
	
		void Mapper115::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write8( exReg ).End();
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper115,6000)
		{
			exReg = data;
			Mapper115::UpdatePrg();
		}
	
		NES_POKE(Mapper115,6001)
		{
			Mapper115::UpdatePrg();
		}
	
		void Mapper115::UpdatePrg()
		{
			Mmc3::UpdatePrg();
	
			if (exReg & 0x80)
				prg.SwapBank<NES_16K,0x0000U>(exReg & 0x7);
		}
	}
}
