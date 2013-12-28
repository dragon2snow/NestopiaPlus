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
#include "../board/NstBrdMmc3.hpp"
#include "NstMapper196.hpp"

namespace Nes
{
	namespace Core
	{
		void Mapper196::SubReset(bool hard)
		{
			Mmc3::SubReset( hard );

			for (uint i=0x0000; i < 0x2000; i += 0x8)
			{
				Map( 0x8000 + i, 0x8003 + i, &Mapper196::Poke_8000 );
				Map( 0x8004 + i, 0x8007 + i, &Mapper196::Poke_8001 );
				Map( 0xA000 + i, 0xA003 + i, NMT_SWAP_HV           );
				Map( 0xA004 + i, 0xA007 + i, &Mapper196::Poke_A001 );
				Map( 0xC000 + i, 0xC003 + i, &Mapper196::Poke_C000 );
				Map( 0xC004 + i, 0xC007 + i, &Mapper196::Poke_C001 );
				Map( 0xE000 + i, 0xE003 + i, &Mapper196::Poke_E000 );
				Map( 0xE004 + i, 0xE007 + i, &Mapper196::Poke_E001 );
			}
		}
	}
}
