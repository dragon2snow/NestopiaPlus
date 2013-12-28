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
#include "NstMapper236.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper236::SubReset(const bool hard)
		{
			mode = 0x0;

			if (hard)
				title = 0x6;
			else
				title = (title + 1) & 0xF;

			Map( 0x8000U, 0xFFFFU, &Mapper236::Peek_Prg, &Mapper236::Poke_Prg );

			NES_CALL_POKE(Mapper236,Prg,0x8000,0x00);
			NES_CALL_POKE(Mapper236,Prg,0xC000,0x00);
		}

		void Mapper236::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
				{
					mode = state.Read8();
					title = mode & 0xF;
					mode = mode >> 4 & 0x1;
				}

				state.End();
			}
		}

		void Mapper236::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write8( (mode << 4) | title ).End();
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_PEEK(Mapper236,Prg)
		{
			return prg.Peek( mode ? (address & 0x7FF0) | title : address - 0x8000U );
		}

		NES_POKE(Mapper236,Prg)
		{
			uint banks[2] =
			{
				prg.GetBank<SIZE_16K,0x0000U>(),
				prg.GetBank<SIZE_16K,0x4000U>()
			};

			if (address < 0xC000U)
			{
				ppu.SetMirroring( (address & 0x20) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );

				if (chr.Source().IsWritable())
				{
					banks[0] = (banks[0] & 0x7) | (address << 3 & 0x38);
					banks[1] = (banks[1] & 0x7) | (address << 3 & 0x38);
				}
				else
				{
					chr.SwapBank<SIZE_8K,0x0000U>( address & 0x7 );
					return;
				}
			}
			else switch (address & 0x30)
			{
				case 0x00: mode = 0x0; banks[0] = (banks[0] & 0x38) | (address & 0x7); banks[1] = banks[0] | 0x7; break;
				case 0x10: mode = 0x1; banks[0] = (banks[0] & 0x38) | (address & 0x7); banks[1] = banks[0] | 0x7; break;
				case 0x20: mode = 0x0; banks[0] = (banks[0] & 0x38) | (address & 0x6); banks[1] = banks[0] | 0x1; break;
				case 0x30: mode = 0x0; banks[0] = (banks[0] & 0x38) | (address & 0x7); banks[1] = banks[0] | 0x0; break;
			}

			prg.SwapBanks<SIZE_16K,0x0000U>( banks[0], banks[1] );
		}
	}
}
