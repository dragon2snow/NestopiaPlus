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
#include "NstMapper163.hpp"
		 
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper163::SubReset(const bool hard)
		{
			if (hard)
				regs[1] = regs[0] = 0;

			ppu.SetSpHook( Hook(this,&Mapper163::Hook_Ppu) );

			for (uint i=0x5000U; i < 0x6000U; i += 0x200)
				Map( i + 0x00, i + 0xFF, &Mapper163::Poke_5000 );
		}

		void Mapper163::SubSave(State::Saver& state) const
		{
			const u8 data[2] = { regs[0], regs[1] };
			state.Begin('R','E','G','\0').Write( data ).End();
		}

		void Mapper163::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
				{
					const State::Loader::Data<2> data( state );

					regs[0] = data[0];
					regs[1] = data[1];
				}

				state.End();
			}
		}

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif

		NES_POKE(Mapper163,5000)
		{
			regs[address >> 9 & 0x1] = data;	
			prg.SwapBank<SIZE_32K,0x0000U>( (regs[0] & 0xF) | (regs[1] << 4 & 0x30) );
			
			ppu.Update();
			chr.SwapBank<SIZE_4K,0x0000U>(0);
		}

		NES_HOOK(Mapper163,Ppu)
		{
			if ((regs[0] & 0x80) && ppu.GetScanline() == 127)
				chr.SwapBank<SIZE_4K,0x0000U>(1);
		}
	}
}
