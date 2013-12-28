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
#include "NstMapper202.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER202::Reset()
{
	cpu.SetPort( 0x4020, 0x7FFF, this, Peek_Nop,  Poke_8000 );
	cpu.SetPort( 0x8000, 0xFFFF, this, Peek_pRom, Poke_8000 );

	pRom.SwapBanks<n16k,0x0000>(6);
	pRom.SwapBanks<n16k,0x4000>(7);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER202,8000)
{
	apu.Update();
	ppu.SetMirroring( (address & 0x1) ? MIRROR_HORIZONTAL : MIRROR_VERTICAL );

	const UINT bank = (address >> 1) & 0x7;

	pRom.SwapBanks<n16k,0x0000>( bank );
	pRom.SwapBanks<n16k,0x4000>( bank + ((address & 0xC) == 0xC ? 1 : 0) );
	cRom.SwapBanks<n8k,0x0000>( bank );
}

NES_NAMESPACE_END
