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
#include "NstBrdMmc3.hpp"
#include "NstBrd6035052.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			void Unl6035052::SubReset(const bool hard)
			{
				security = 0x0;

				Mmc3::SubReset( hard );

				Map( 0x4020U, 0x7FFFU, &Unl6035052::Peek_4020, &Unl6035052::Poke_4020 );
			}

			void Unl6035052::SubLoad(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
						security = state.Read8() & 0x3;

					state.End();
				}
			}

			void Unl6035052::SubSave(State::Saver& state) const
			{
				state.Begin('R','E','G','\0').Write8( security ).End();
			}

			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("", on)
			#endif

			NES_PEEK(Unl6035052,4020)
			{
				return security;
			}

			NES_POKE(Unl6035052,4020)
			{
				security = data & 0x3;

				if (security == 0x1)
					security = 0x2;
			}
		}
	}
}
