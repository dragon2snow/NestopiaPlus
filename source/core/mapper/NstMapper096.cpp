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
#include "NstMapper096.hpp"

namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper96::SubReset(const bool hard)
		{
			nmt.SetAccessor( 0, 0, this, &Mapper96::Access_Name_2000 );
			nmt.SetAccessor( 1, 0, this, &Mapper96::Access_Name_2400 );
			nmt.SetAccessor( 2, 0, this, &Mapper96::Access_Name_2800 );
			nmt.SetAccessor( 3, 0, this, &Mapper96::Access_Name_2C00 );
	
			Map( 0x8000U, 0xFFFFU, &Mapper96::Poke_Prg );
	
			p2006 = cpu.Map( 0x2006U );
	
			for (uint i=0x2006U; i < 0x4000U; i += 0x8)
				cpu.Map( i ).Set( this, &Mapper96::Peek_2006, &Mapper96::Poke_2006 );
		}

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		void Mapper96::UpdateLatch(const uint bank) const
		{
			chr.SwapBank<SIZE_4K,0x0000U>( (chr.GetBank<SIZE_4K,0x0000U>() & 0x4) | (bank >> 8) );
		}
	
		NES_ACCESSOR(Mapper96,Name_2000)
		{
			UpdateLatch( address );
			return nmt[0][address];
		}
	
		NES_ACCESSOR(Mapper96,Name_2400)
		{
			UpdateLatch( address );
			return nmt[1][address];
		}
	
		NES_ACCESSOR(Mapper96,Name_2800)
		{
			UpdateLatch( address );
			return nmt[2][address];
		}
	
		NES_ACCESSOR(Mapper96,Name_2C00)
		{
			UpdateLatch( address );
			return nmt[3][address];
		}
	
		NES_PEEK(Mapper96,2006)
		{
			return p2006.Peek( address );
		}
	
		NES_POKE(Mapper96,2006)
		{
			p2006.Poke( 0x2006, data );
	
			address = ppu.GetVRamAddress();
	
			if ((address & 0x3000) == 0x2000)
				UpdateLatch( address & 0x0300U );
		}
	
		NES_POKE(Mapper96,Prg)
		{
			ppu.Update();
			prg.SwapBank<SIZE_32K,0x0000U>( data & 0x3 );
			chr.SwapBanks<SIZE_4K,0x0000U>( (data & 0x4) | (chr.GetBank<SIZE_4K,0x0000U>() & 0x3), (data & 0x4) | 0x3 );
		}
	}
}
