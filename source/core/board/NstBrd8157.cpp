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
#include "NstBrd8157.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			void Unl8157::SubReset(const bool hard)
			{
				if (hard)
					menu = 0x100;
				else
					menu ^= 0x100;

				trash = 0;

				Map( 0x8000U, 0xFFFFU, &Unl8157::Peek_Prg, &Unl8157::Poke_Prg );
			}

			void Unl8157::SubSave(State::Saver& state) const
			{
				state.Begin('R','E','G','\0').Write8( (menu >> 8) | (trash >> 7) ).End();
			}

			void Unl8157::SubLoad(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					{
						trash = state.Read8();
						menu = trash << 8 & 0x100;
						trash = trash << 7 & 0x100;
					}

					state.End();
				}
			}

			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("", on)
			#endif

			NES_PEEK(Unl8157,Prg)
			{
				return !trash ? prg.Peek( address - 0x8000U ) : 0xFF;
			}

			NES_POKE(Unl8157,Prg)
			{
				trash = address & menu;

				prg.SwapBanks<SIZE_16K,0x0000U>
				(
					(address >> 2 & 0x18) | (address >> 2 & 0x7),
					(address >> 2 & 0x18) | ((address & 0x200) ? 0x7 : 0x0)
				);

				ppu.SetMirroring( (address & 0x2) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
			}
		}
	}
}
