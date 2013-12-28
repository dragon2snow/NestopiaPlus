////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2007 Martin Freij
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
#include "NstMapper070.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper70::Mapper70(Context& c)
		:
		Mapper (c,PROM_MAX_256K|CROM_MAX_128K|WRAM_DEFAULT),
		useGun (c.attribute == ATR_LIGHTGUN)
		{}

		void Mapper70::SubReset(bool)
		{
			Map( 0x6000U, 0xFFFFU, &Mapper70::Poke_Prg );

			if (useGun)
			{
				p4016 = cpu.Map( 0x4016 );
				cpu.Map( 0x4016 ).Set( this, &Mapper70::Peek_SpaceShadow, &Mapper70::Poke_SpaceShadow );
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE_D(Mapper70,Prg)
		{
			ppu.Update();
			chr.SwapBank<SIZE_8K,0x0000>( data );
			prg.SwapBank<SIZE_16K,0x0000>( data >> 4 );
		}

		NES_PEEK(Mapper70,SpaceShadow)
		{
			const uint data = p4016.Peek( 0x4016 );
			return (data & 0xFE) | (data << 1 & 0x2);
		}

		NES_POKE_AD(Mapper70,SpaceShadow)
		{
			p4016.Poke( address, data );
		}
	}
}
