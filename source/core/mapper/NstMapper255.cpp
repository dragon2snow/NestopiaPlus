////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2005 Martin Freij
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
#include "NstMapper255.hpp"
		 
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper255::SubReset(const bool hard)
		{
			if (hard)
				regs[3] = regs[2] = regs[1] = regs[0] = 0xF;

			Map( 0x5800U, 0x5FFFU, &Mapper255::Peek_5800, &Mapper255::Poke_5800 );
			Map( 0x8000U, 0xFFFFU, &Mapper255::Poke_Prg );
		}
	
		void Mapper255::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
				{
					const State::Loader::Data<2> data( state );
	
					regs[0] = data[0] & 0xF;
					regs[1] = data[0] >> 4;
					regs[2] = data[1] & 0xF;
					regs[3] = data[1] >> 4;
				}
	
				state.End();
			}
		}
	
		void Mapper255::SubSave(State::Saver& state) const
		{
			const u8 data[2] =
			{
				regs[0]	| (regs[1] << 4),
				regs[2] | (regs[3] << 4)
			};

			state.Begin('R','E','G','\0').Write( data ).End();
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper255,Prg) 
		{ 
			uint rBank = (address >> 14) & 0x01;
			uint pBank = ((address >> 7) & 0x1F) | (rBank << 5);
			uint cBank = ((address >> 0) & 0x3F) | (rBank << 6);
	
			if (address & 0x1000)
			{
				pBank = (pBank << 1) | ((address & 0x40) >> 6);
				prg.SwapBanks<NES_16K,0x0000U>( pBank, pBank );
			}
			else
			{
				prg.SwapBank<NES_32K,0x0000U>( pBank );
			}
	
			ppu.SetMirroring( (address & 0x2000U) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );	
			chr.SwapBank<NES_8K,0x0000U>( cBank ); 
		}
	
		NES_POKE(Mapper255,5800)
		{
			regs[address & 0x3] = data & 0xF;
		}
	
		NES_PEEK(Mapper255,5800)
		{
			return regs[address & 0x3];
		}
	}
}
