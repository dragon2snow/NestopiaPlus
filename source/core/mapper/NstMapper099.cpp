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
#include "NstMapper099.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper99::SubReset(bool)
		{
			p4016 = cpu.Map( 0x4016 );
			cpu.Map( 0x4016 ).Set( this, &Mapper99::Peek_4016, &Mapper99::Poke_4016 );
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE(Mapper99,4016)
		{
			ppu.Update();
			chr.SwapBank<SIZE_8K,0x0000U>( data >> 2 );
			prg.SwapBank<SIZE_8K,0x0000U>( data & 0x4 );
			p4016.Poke( 0x4016, data );
		}

		NES_PEEK(Mapper99,4016)
		{
			return p4016.Peek( address );
		}
	}
}
