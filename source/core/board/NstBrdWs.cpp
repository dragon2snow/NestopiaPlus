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
#include "NstBrdWs.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			void Ws::SubReset(const bool)
			{
				reg = 0x00;

				for (uint i=0x6000U; i < 0x7000U; i += 0x2)
				{
					Map( i + 0x0, &Ws::Poke_6000 );
					Map( i + 0x1, &Ws::Poke_6001 );
				}
			}

			void Ws::SubLoad(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
						reg = state.Read8() & 0x20;

					state.End();
				}
			}

			void Ws::SubSave(State::Saver& state) const
			{
				state.Begin('R','E','G','\0').Write8( reg ).End();
			}

			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("", on)
			#endif

			NES_POKE(Ws,6000)
			{
				if (!reg)
				{
					reg = data & 0x20;
					prg.SwapBanks<SIZE_16K,0x0000U>( data & ~(~data >> 3 & 0x1), data | (~data >> 3 & 0x1) );
					ppu.SetMirroring( (data & 0x10) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
				}
			}

			NES_POKE(Ws,6001)
			{
				if (!reg)
				{
					ppu.Update();
					chr.SwapBank<SIZE_8K,0x0000U>( data );
				}
			}
		}
	}
}
