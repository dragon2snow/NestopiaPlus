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
#include "NstBrdKs7032.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			void Ks7032::SubReset(const bool hard)
			{
				if (hard)
					reg = 0;

				Map( WRK_PEEK );
				Map( 0xE000U, &Ks7032::Poke_E000 );
				Map( 0xF000U, &Ks7032::Poke_F000 );
			}

			void Ks7032::SubLoad(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					if (chunk == AsciiId<'R','E','G'>::V)
						reg = state.Read8();

					state.End();
				}
			}

			void Ks7032::SubSave(State::Saver& state) const
			{
				state.Begin( AsciiId<'R','E','G'>::V ).Write8( reg ).End();
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			NES_POKE(Ks7032,E000)
			{
				reg = data;
			}

			NES_POKE(Ks7032,F000)
			{
				address = reg & 0x7;

				switch (address)
				{
					case 1:
					case 2:
					case 3:

						prg.SwapBank<SIZE_8K>( (address-1) << 13, data );
						break;

					case 4:

						wrk.SwapBank<SIZE_8K,0x0000>( data );
						break;
				}
			}
		}
	}
}
