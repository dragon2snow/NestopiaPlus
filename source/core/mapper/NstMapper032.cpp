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
#include "NstMapper032.h"
		   
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER32::Reset()
{
	cpu.SetPort( 0x8000, 0x8FFF, this, Peek_8000, Poke_8000 );
	cpu.SetPort( 0x9000, 0x9FFF, this, Peek_9000, Poke_9000 );
	cpu.SetPort( 0xA000, 0xAFFF, this, Peek_A000, Poke_A000 );

	for (UINT i=0xB000; i <= 0xBFFF; i += 0x8)
	{
		cpu.SetPort( i + 0x0, this, Peek_B000, Poke_B000 );
		cpu.SetPort( i + 0x1, this, Peek_B000, Poke_B001 );
		cpu.SetPort( i + 0x2, this, Peek_B000, Poke_B002 );
		cpu.SetPort( i + 0x3, this, Peek_B000, Poke_B003 );
		cpu.SetPort( i + 0x4, this, Peek_B000, Poke_B004 );
		cpu.SetPort( i + 0x5, this, Peek_B000, Poke_B005 );
		cpu.SetPort( i + 0x6, this, Peek_B000, Poke_B006 );
		cpu.SetPort( i + 0x7, this, Peek_B000, Poke_B007 );
	}

	pRomOffset = 0x0000;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER32,8000)
{
	apu.Update(); 
	pRom.SwapBanks<n8k>( pRomOffset, data );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER32,9000)
{
	ppu.SetMirroring( (data & 0x1) ? MIRROR_HORIZONTAL : MIRROR_VERTICAL );
	pRomOffset = (data & 0x2) ? 0x4000 : 0x0000;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER32,A000)
{
	apu.Update(); 
	pRom.SwapBanks<n8k,0x2000>(data);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER32,B000) { ppu.Update(); cRom.SwapBanks<n1k,0x0000>(data); }
NES_POKE(MAPPER32,B001) { ppu.Update(); cRom.SwapBanks<n1k,0x0400>(data); }
NES_POKE(MAPPER32,B002) { ppu.Update(); cRom.SwapBanks<n1k,0x0800>(data); }
NES_POKE(MAPPER32,B003) { ppu.Update(); cRom.SwapBanks<n1k,0x0C00>(data); }
NES_POKE(MAPPER32,B004) { ppu.Update(); cRom.SwapBanks<n1k,0x1000>(data); }
NES_POKE(MAPPER32,B005) { ppu.Update(); cRom.SwapBanks<n1k,0x1400>(data); }
NES_POKE(MAPPER32,B006) { ppu.Update(); cRom.SwapBanks<n1k,0x1800>(data); }
NES_POKE(MAPPER32,B007) { ppu.Update(); cRom.SwapBanks<n1k,0x1C00>(data); }

NES_NAMESPACE_END
