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
#include "NstMapper078.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper78::Mapper78(Context& c)
		: Mapper(c,PROM_MAX_128K|CROM_MAX_128K|WRAM_DEFAULT)
		{
			if (mirroring == Ppu::NMT_FOURSCREEN)
			{
				nmt[0] = Ppu::NMT_FOURSCREEN;
				nmt[1] = Ppu::NMT_FOURSCREEN;
			}
			else if (c.attribute == ATR_HV_MIRRORING)
			{
				nmt[0] = Ppu::NMT_HORIZONTAL;
				nmt[1] = Ppu::NMT_VERTICAL;
			}
			else
			{
				nmt[0] = Ppu::NMT_ZERO;
				nmt[1] = Ppu::NMT_ONE;
			}
		}

		void Mapper78::SubReset(bool)
		{
			Map( 0x8000U, 0xFFFFU, &Mapper78::Poke_Prg );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE(Mapper78,Prg)
		{
			ppu.SetMirroring( nmt[data >> 3 & 0x1] );
			prg.SwapBank<SIZE_16K,0x0000>( data );
			chr.SwapBank<SIZE_8K,0x0000>( data >> 4 );
		}
	}
}
