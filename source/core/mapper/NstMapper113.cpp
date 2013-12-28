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
#include "NstMapper113.h"
		  
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER113::Reset()
{
	cpu.SetPort( 0x4020, 0x7FFF, this, Peek_Nop,  Poke_4020 );
	cpu.SetPort( 0x8008, 0x8009, this, Peek_pRom, Poke_4020 );
	cpu.SetPort( 0x8E66, 0x8E67, this, Peek_pRom, Poke_8E66 );
	cpu.SetPort( 0xE00A,         this, Peek_pRom, Poke_E00A );
	
	pRom.SwapBanks<n32k,0x0000>(0);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER113,4020) 
{
	apu.Update();
	ppu.Update();
	pRom.SwapBanks<n32k,0x0000>(data >> 3);
	cRom.SwapBanks<n8k,0x0000>(((data >> 3) & 0x8) + (data & 0x7));
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER113,8E66) 
{
	ppu.Update();
	cRom.SwapBanks<n8k,0x0000>( (data & 0x7) ? 0x0 : 0x1 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER113,E00A) 
{
	apu.Update();
	ppu.SetMirroring( MIRROR_ZERO );
}

NES_NAMESPACE_END
