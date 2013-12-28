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
#include "NstMapper185.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper185::SubReset(bool)
		{
			Map( 0x8000U, 0xFFFFU, &Mapper185::Poke_Prg );
		}

		void Mapper185::SubSave(State::Saver& state) const
		{
			state.Begin('O','P','B','\0').Write8( chr.SameComponent(0,this) ? 0x1 : 0x0 ).End();
		}

		void Mapper185::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('O','P','B','\0'))
				{
					if (state.Read8() & 0x1)
						chr.SetAccessors( this, &Mapper185::Access_Chr, &Mapper185::Access_Chr );
					else
						chr.ResetAccessors();
				}

				state.End();
			}
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_ACCESSOR(Mapper185,Chr)
		{
			return 0xFF;
		}

		NES_POKE(Mapper185,Prg)
		{
			ppu.Update();

			chr.SwapBank<SIZE_8K,0x0000U>( data );

			if (chr.SameComponent( 0, this ))
				chr.ResetAccessors();
			else
				chr.SetAccessors( this, &Mapper185::Access_Chr, &Mapper185::Access_Chr );
		}
	}
}
