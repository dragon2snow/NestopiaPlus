////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
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
#include "NstMapper167.hpp"
		 
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		Mapper167::Mapper167(Context& c)
		: 
		Mapper (c),
		patch  (c.pRomCrc == 0x82F1FB96UL) 
		{}

		void Mapper167::SubReset(bool)
		{
			Map( 0x8000U, 0xFFFFU, &Mapper167::Poke_Prg );

			regs[3] = regs[2] = regs[1] = regs[0] = 0;

			NES_CALL_POKE(Mapper167,Prg,0x8000U,0x00);
		}
	
		void Mapper167::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write( regs ).End();
		}

		void Mapper167::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					state.Read( regs );

				state.End();
			}
		}

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif

		NES_POKE(Mapper167,Prg)
		{
			regs[(address >> 13) & 0x3] = data;

			uint banks[2] =
			{
				((regs[0] ^ regs[1]) & 0x10) << 1,
				((regs[2] ^ regs[3]) & 0x1F) << 0
			};

			if (regs[1] & 0x8)
			{
				banks[1] = banks[0] = banks[0] + (banks[1] & 0xFE);
				banks[0] += patch ^ 1;
				banks[1] += patch ^ 0;
			}
			else if (regs[1] & 0x4)
			{
				banks[1] = banks[0] + banks[1];
				banks[0] = 0x1F;
			}
			else
			{
				banks[0] = banks[0] + banks[1];
				banks[1] = patch ? 0x07 : 0x20;
			}

			prg.SwapBanks<SIZE_16K,0x0000U>( banks[0], banks[1] );
		}
	}
}
