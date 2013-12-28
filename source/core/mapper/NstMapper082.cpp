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
#include "NstMapper082.h"
	 
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER82::Reset()
{
	cpu.SetPort( 0x7EF0, this, Peek_Nop, Poke_7EF0 );
	cpu.SetPort( 0x7EF1, this, Peek_Nop, Poke_7EF1 );
	cpu.SetPort( 0x7EF2, this, Peek_Nop, Poke_7EF2 );
	cpu.SetPort( 0x7EF3, this, Peek_Nop, Poke_7EF3 );
	cpu.SetPort( 0x7EF4, this, Peek_Nop, Poke_7EF4 );
	cpu.SetPort( 0x7EF5, this, Peek_Nop, Poke_7EF5 );
	cpu.SetPort( 0x7EF6, this, Peek_Nop, Poke_7EF6 );
	cpu.SetPort( 0x7EFA, this, Peek_Nop, Poke_7EFA );
	cpu.SetPort( 0x7EFB, this, Peek_Nop, Poke_7EFB );
	cpu.SetPort( 0x7EFC, this, Peek_Nop, Poke_7EFC );

	ppu.SetMirroring(MIRROR_VERTICAL);
	SwapLow = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER82,7EF0)
{
	ppu.Update();

	if (SwapLow) cRom.SwapBanks<n2k,0x0000>( (data & 0xFE) >> 1 );
	else         cRom.SwapBanks<n2k,0x1000>( (data & 0xFE) >> 1 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER82,7EF1)
{
	ppu.Update();

	if (SwapLow) cRom.SwapBanks<n2k,0x0800>( (data & 0xFE) >> 1 );
	else         cRom.SwapBanks<n2k,0x1800>( (data & 0xFE) >> 1 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER82,7EF2) 
{ 
	ppu.Update(); 
	
	if (SwapLow) cRom.SwapBanks<n1k,0x0000>(data); 
	else         cRom.SwapBanks<n1k,0x1000>(data);  
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER82,7EF3) 
{ 
	ppu.Update(); 
	
	if (SwapLow) cRom.SwapBanks<n1k,0x0400>(data); 
	else         cRom.SwapBanks<n1k,0x1400>(data);  
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER82,7EF4) 
{ 
	ppu.Update(); 
	
	if (SwapLow) cRom.SwapBanks<n1k,0x0800>(data); 
	else         cRom.SwapBanks<n1k,0x1800>(data);  
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER82,7EF5) 
{ 
	ppu.Update(); 
	
	if (SwapLow) cRom.SwapBanks<n1k,0x0C00>(data); 
	else         cRom.SwapBanks<n1k,0x1C00>(data);  
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER82,7EF6)
{
	SwapLow = data & SWAP_LOW;
	ppu.SetMirroring( (data & SELECT_MIRRORING) ? MIRROR_VERTICAL : MIRROR_HORIZONTAL );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER82,7EFA) { apu.Update(); pRom.SwapBanks<n8k,0x0000>(data >> 2); }
NES_POKE(MAPPER82,7EFB) { apu.Update(); pRom.SwapBanks<n8k,0x2000>(data >> 2); }
NES_POKE(MAPPER82,7EFC) { apu.Update(); pRom.SwapBanks<n8k,0x4000>(data >> 2); }

NES_NAMESPACE_END
