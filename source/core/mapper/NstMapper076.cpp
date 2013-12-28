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
#include "NstMapper076.hpp"
			  
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper76::SubReset(const bool hard)
		{
			if (hard)
				command = 0;

			for (uint i=0x0000U; i < 0x2000U; i += 0x2)
			{
				Map( 0x8000U + i, &Mapper76::Poke_8000 );
				Map( 0x8001U + i, &Mapper76::Poke_8001 );
				Map( 0xA000U + i, NMT_SWAP_HV          );
			}
		}
	
		void Mapper76::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					command = state.Read8();
	
				state.End();
			}
		}
	
		void Mapper76::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write8( command ).End();
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper76,8000) 
		{
			command = data;
		}
	
		NES_POKE(Mapper76,8001) 
		{
			switch (const uint index = (command & 0x7))
			{
				case 0x2:
				case 0x3:
				case 0x4:
				case 0x5:

					ppu.Update();
					chr.SwapBank<SIZE_2K>( (index - 0x2) << 11, data );
					break;

				case 0x6:
				case 0x7:

					prg.SwapBank<SIZE_8K>( (index == 0x6) ? (command & 0x40) << 8 : 0x2000U, data );
					break;
			}
		}
	}
}
