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
#include "NstMapper181.hpp"
	  
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif

		void Mapper181::SubReset(bool)
		{
			openChrBus = 0x00;

			for (uint i=0; i < 2; ++i)
				chr.SetAccessor( i, this, &Mapper181::Access_Chr );

			Map( 0x8000U, 0xFFFFU, &Mapper181::Poke_Prg );
		}
	
		void Mapper181::SubSave(State::Saver& state) const
		{
			state.Begin('O','P','B','\0').Write8( openChrBus ? 0x1 : 0x0 ).End();
		}

		void Mapper181::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('O','P','B','\0'))
					openChrBus = (state.Read8() & 0x1) ? 0xFF : 0x00;

				state.End();
			}
		}

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif

		NES_ACCESSOR(Mapper181,Chr)
		{
			return chr.Peek( address ) | openChrBus;
		}

		NES_POKE(Mapper181,Prg)
		{
			ppu.Update();	
			openChrBus = (data & 0x01) ? 0xFF : 0x00;
		}
	}
}
