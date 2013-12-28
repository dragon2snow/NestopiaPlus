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
#include "NstMapper170.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper170::SubReset(bool)
		{
			reg = 0x00;

			Map( 0x6502U, &Mapper170::Poke_7000 );
			Map( 0x7000U, &Mapper170::Poke_7000 );
			Map( 0x7001U, &Mapper170::Peek_7001 );
			Map( 0x7777U, &Mapper170::Peek_7001 );
		}

		void Mapper170::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					reg = state.Read8() & 0x80;

				state.End();
			}
		}

		void Mapper170::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write8( reg ).End();
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE(Mapper170,7000)
		{
			reg = data << 1 & 0x80;
		}

		NES_PEEK(Mapper170,7001)
		{
			return reg | (address >> 8 & 0x7F);
		}
	}
}
