////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003 Martin Freij
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

#include "NstMappers.h"
#include "NstMapper160.h"
			
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER160::Reset()
{
	cpu.SetPort( 0x8000,         this, Peek_8000, Poke_8000 );
	cpu.SetPort( 0x8001,         this, Peek_8000, Poke_8001 );
	cpu.SetPort( 0x8002,         this, Peek_8000, Poke_8002 );
	cpu.SetPort( 0x9000,         this, Peek_9000, Poke_9000 );
	cpu.SetPort( 0x9001,         this, Peek_9000, Poke_9001 );
	cpu.SetPort( 0x9002, 0x9007, this, Peek_9000, Poke_9000 );
	cpu.SetPort( 0xD000, 0xFFFF, this, Peek_pRom, Poke_D000 );

	mode = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER160,8000) { apu.Update(); pRom.SwapBanks<n8k,0x0000>(data); }
NES_POKE(MAPPER160,8001) { apu.Update(); pRom.SwapBanks<n8k,0x2000>(data); }
NES_POKE(MAPPER160,8002) { apu.Update(); pRom.SwapBanks<n8k,0x4000>(data); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER160,9000)
{
	if (mode == 1)
	{
		ppu.Update();
		cRom.SwapBanks<n1k>( (address & 0x7) * 1024, data );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER160,9001)
{
	ppu.Update();

	switch (mode)
	{
     	case 4:

			cRom.SwapBanks<n4k,0x0000>(data >> 1);
			cRom.SwapBanks<n4k,0x1000>(data >> 1);
			return;

		case 1:

			cRom.SwapBanks<n1k>( (0x9001 & 0x7) * 1024, data );
			return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER160,D000)
{
	mode = data;
}

NES_NAMESPACE_END
