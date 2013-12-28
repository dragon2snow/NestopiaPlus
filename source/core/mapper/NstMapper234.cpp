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
#include "NstMapper234.hpp"
		 
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper234::SubReset(const bool hard)
		{
			if (hard)
			{
				banks[0] = 0;
				banks[1] = 0;
			}

			Map( 0xFF80U, 0xFF9FU, &Mapper234::Peek_FF80, &Mapper234::Poke_FF80 );
			Map( 0xFFE8U, 0xFFF7U, &Mapper234::Peek_FFE8, &Mapper234::Poke_FFE8 );
		}
	
		void Mapper234::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
				{
					const State::Loader::Data<2> data( state );
					banks[0] = data[0];
					banks[1] = data[1];
				}
	
				state.End();
			}
		}
	
		void Mapper234::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write16( banks[0] | (banks[0] << 8) ).End();
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		void Mapper234::UpdateBanks()
		{
			ppu.Update();
	
			if (banks[0] & 0x40)
			{
				prg.SwapBank<SIZE_32K,0x0000U>( (banks[0] & 0xE) | (banks[1] & 0x1) );
				chr.SwapBank<SIZE_8K,0x0000U> ( (banks[0] << 2 & 0x38) | (banks[1] >> 4 & 0x7) );
			}
			else
			{
				prg.SwapBank<SIZE_32K,0x0000U>( banks[0] & 0xF );
				chr.SwapBank<SIZE_8K,0x0000U> ( (banks[0] << 2 & 0x3C) | (banks[1] >> 4 & 0x3) );
			}
		}
	
		void Mapper234::UpdateBank0(const uint data)
		{
			if (!banks[0])
			{
				banks[0] = data;	
				ppu.SetMirroring( (data & 0x80) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );	
				UpdateBanks();
			}
		}
	
		void Mapper234::UpdateBank1(const uint data)
		{
			banks[1] = data;
			UpdateBanks();
		}
	
		NES_PEEK(Mapper234,FF80) 
		{ 
			const uint data = prg.Peek(address - 0x8000U);
			UpdateBank0( data );
			return data;
		}
	
		NES_POKE(Mapper234,FF80) 
		{ 
			UpdateBank0( data );
		}
	
		NES_PEEK(Mapper234,FFE8) 
		{ 
			const uint data = prg.Peek(address - 0x8000U);
			UpdateBank1( data );
			return data;
		}
	
		NES_POKE(Mapper234,FFE8) 
		{ 
			UpdateBank1( data );
		}
	}
}
