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
#include "../NstClock.hpp"
#include "../board/NstBrdVrc4.hpp"
#include "NstMapper023.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper23::Mapper23(Context& c)
		: Vrc4(c,TYPE_VRC4_352396,c.chr.Size() ? (CROM_MAX_256K|NMT_VERTICAL) : (PROM_MAX_512K|NMT_VERTICAL)) {}

		void Mapper23::SubReset(const bool hard)
		{
			Vrc4::SubReset( hard );

			if (chr.Source().Writable())
			{
				Map( 0x8000U, 0x8FFFU, &Mapper23::Poke_8000 );
				Map( 0xA000U, 0xAFFFU, &Mapper23::Poke_A000 );
				Map( 0xB000U, 0xEFFFU, &Mapper23::Poke_B000 );
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE_D(Mapper23,8000)
		{
			prg.SwapBank<SIZE_8K>( prgSwap << 13, (data & 0x1F) | (prg.GetBank<SIZE_8K>(prgSwap << 13) & 0x20) );
		}

		NES_POKE_D(Mapper23,A000)
		{
			prg.SwapBank<SIZE_8K,0x2000>( (data & 0x1F) | (prg.GetBank<SIZE_8K,0x2000>() & 0x20) );
		}

		NES_POKE_D(Mapper23,B000)
		{
			data = data << 2 & 0x20;
			prg.SwapBank<SIZE_8K>( prgSwap << 13, (prg.GetBank<SIZE_8K>(prgSwap << 13) & 0x1F) | data );
			prg.SwapBank<SIZE_8K,0x2000>( (prg.GetBank<SIZE_8K,0x2000>() & 0x1F) | data );
		}
	}
}
