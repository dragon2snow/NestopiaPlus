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
#include "NstMapper113.hpp"
		  
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		Mapper113::Mapper113(Context& c)
		: 
		Mapper  (c,WRAM_NONE), 
		hes6in1 (c.pRomCrc == 0xA75AEDE5UL) // HES 6-in-1 
		{}

		void Mapper113::SubReset(const bool)
		{
			Map( 0x4100U, hes6in1 ? &Mapper113::Poke_4100 : &Mapper113::Poke_8008 );
			Map( 0x4111U, hes6in1 ? &Mapper113::Poke_4100 : &Mapper113::Poke_8008 );
			Map( 0x4120U, hes6in1 ? &Mapper113::Poke_4100 : &Mapper113::Poke_8008 );
			Map( 0x4194U, hes6in1 ? &Mapper113::Poke_4100 : &Mapper113::Poke_8008 );
			Map( 0x4195U, hes6in1 ? &Mapper113::Poke_4100 : &Mapper113::Poke_8008 );
			Map( 0x4900U, hes6in1 ? &Mapper113::Poke_4100 : &Mapper113::Poke_8008 );

			Map( 0x8008U, 0x8009U, &Mapper113::Poke_8008 );
			Map( 0x8E66U, 0x8E67U, &Mapper113::Poke_8E66 );
			Map( 0xE00AU,          &Mapper113::Poke_E00A );
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper113,4100)
		{
			ppu.SetMirroring( (data & 0x80) ? Ppu::NMT_VERTICAL : Ppu::NMT_HORIZONTAL );
			NES_CALL_POKE( Mapper113, 8008, address, data );
		}

		NES_POKE(Mapper113,8008)
		{
			ppu.Update();
			prg.SwapBank<SIZE_32K,0x0000U>( data >> 3 );
			chr.SwapBank<SIZE_8K,0x0000U>( (data >> 3 & 0x8) | (data & 0x7) );
		}

		NES_POKE(Mapper113,8E66)
		{
			ppu.Update();
			chr.SwapBank<SIZE_8K,0x0000U>( (data & 0x7) == 0 );
		}

		NES_POKE(Mapper113,E00A) 
		{
			ppu.SetMirroring( Ppu::NMT_ZERO );
		}
	}
}
