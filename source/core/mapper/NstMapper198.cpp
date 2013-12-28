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

#include <cstring>
#include "../NstMapper.hpp"
#include "NstMapper198.hpp"
		 
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper198::SubReset(const bool hard)
		{
			if (hard)
			{
				regs.ctrl = 0;
				regs.prg[0] = 0;
				regs.prg[1] = 1;

				std::memset( exRam, 0x00, sizeof(exRam) );
			}
	
			Map( 0x5000U, 0x5FFFU, &Mapper198::Peek_ExRam, &Mapper198::Poke_ExRam );
			Map( 0x6000U, 0x7FFFU, &Mapper198::Peek_wRam,  &Mapper198::Poke_wRam  );
	
			for (uint i=0x8000U; i < 0xA000U; i += 0x2)
			{
				Map( i + 0x0, &Mapper198::Poke_8000 );
				Map( i + 0x1, &Mapper198::Poke_8001 );
			}
	
			for (uint i=0xA000U; i < 0xC000U; i += 0x2)			
				Map( i, NMT_SWAP_HV );
		}
	
		void Mapper198::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case NES_STATE_CHUNK_ID('R','E','G','\0'):
					{
						const State::Loader::Data<3> data( state );
	
						regs.ctrl = data[0];
						regs.prg[0] = data[1];
						regs.prg[1] = data[2];
	
						break;
					}
				
					case NES_STATE_CHUNK_ID('R','A','M','\0'):
	
						state.Uncompress( exRam );
						break;
				}
	
				state.End();
			}
		}
	
		void Mapper198::SubSave(State::Saver& state) const
		{
			const u8 data[3] =
			{
				regs.ctrl,
				regs.prg[0],
				regs.prg[1]
			};

			state.Begin('R','E','G','\0').Write( data ).End();
			state.Begin('R','A','M','\0').Compress( exRam ).End();
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_PEEK(Mapper198,ExRam) 
		{
			return exRam[address - 0x5000U]; 
		}
	
		NES_POKE(Mapper198,ExRam) 
		{
			exRam[address - 0x5000U] = data; 
		}
	
		NES_PEEK(Mapper198,wRam) 
		{
			return wrk[0][address & 0xFFFU];
		}
	
		NES_POKE(Mapper198,wRam)
		{
			wrk[0][address & 0xFFFU] = data;
		}
	
		void Mapper198::UpdatePrg()
		{
			const uint swap = (regs.ctrl & 0x40) << 8;

			prg.SwapBank<SIZE_8K>( 0x0000U ^ swap, regs.prg[0] );
			prg.SwapBank<SIZE_8K>( 0x2000U,        regs.prg[1] );
			prg.SwapBank<SIZE_8K>( 0x4000U ^ swap, ~1U         );
		}

		NES_POKE(Mapper198,8000) 
		{ 
			regs.ctrl = data;
			UpdatePrg();
		}
	
		NES_POKE(Mapper198,8001) 
		{ 
			switch (regs.ctrl & 0x7)
			{
				case 0x6:
		
					regs.prg[0] = (data >= 0x50) ? (data & 0x4F) : data;
					UpdatePrg();
					break;
		
				case 0x7:
		
					regs.prg[1] = data;
					UpdatePrg();
					break;
			}
		}
	}
}
