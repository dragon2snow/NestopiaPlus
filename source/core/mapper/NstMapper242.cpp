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
#include "NstMapper242.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper242::Mapper242(Context& c)
		:
		Mapper (c,(PROM_MAX_512K|CROM_MAX_8K) | (c.attribute == ATR_EXT_NMT ? NMT_VERTICAL : NMT_DEFAULT)),
		extNmt (c.attribute == ATR_EXT_NMT)
		{
		}

		void Mapper242::SubReset(const bool hard)
		{
			Map( 0x8000U, 0xFFFFU, &Mapper242::Poke_Prg );

			if (hard)
				NES_DO_POKE(Prg,0x8000,0x00);
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE_AD(Mapper242,Prg)
		{
			prg.SwapBank<SIZE_32K,0x0000>( address >> 3 & 0xF );

			if (extNmt)
				SetMirroringVH01( data );
		}
	}
}
