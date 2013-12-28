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
#include "NstMapper079.h"
		
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER79::Reset()
{
	cpu.SetPort( 0x4020, 0x40FF, this, Peek_Nop,  Poke_8000 );
	cpu.SetPort( 0x4100,         this, Peek_Nop,  Poke_4100 );
	cpu.SetPort( 0x4101, 0x5FFF, this, Peek_Nop,  Poke_8000 );
	cpu.SetPort( 0x8000, 0xFFFF, this, Peek_pRom, Poke_8000 );

	pRom.SwapBanks<n32k,0x0000>(0);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER79,8000)
{
	ppu.Update();
	cRom.SwapBanks<n8k,0x0000>(data);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER79,4100)
{
	ppu.Update();
	apu.Update(); 
	pRom.SwapBanks<n32k,0x0000>( (data >> 3) & 0x1 );
	cRom.SwapBanks<n8k,0x0000>(data);
}

NES_NAMESPACE_END
