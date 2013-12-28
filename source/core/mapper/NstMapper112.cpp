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
#include "NstMapper112.h"
		 
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER112::Reset()
{
	cpu.SetPort( 0x8000, this, Peek_8000, Poke_8000 );
	cpu.SetPort( 0xA000, this, Peek_A000, Poke_A000 );
	cpu.SetPort( 0xE000, this, Peek_E000, Poke_E000 );

	command = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER112,8000)
{
	command = data & 0x7;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER112,A000)
{
	apu.Update();
	ppu.Update();

	switch (command)
	{
		case 0x0: pRom.SwapBanks<n8k,0x0000>( data      ); return;
		case 0x1: pRom.SwapBanks<n8k,0x2000>( data      ); return;
		case 0x2: cRom.SwapBanks<n2k,0x0000>( data >> 1 ); return;
		case 0x3: cRom.SwapBanks<n2k,0x0800>( data >> 1 ); return;
		case 0x4: cRom.SwapBanks<n1k,0x1000>( data      ); return;
		case 0x5: cRom.SwapBanks<n1k,0x1400>( data      ); return;
		case 0x6: cRom.SwapBanks<n1k,0x1800>( data      ); return;
		case 0x7: cRom.SwapBanks<n1k,0x1C00>( data      ); return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER112,E000)
{
	ppu.SetMirroring( (data & 0x1) ? MIRROR_HORIZONTAL : MIRROR_VERTICAL );
}

NES_NAMESPACE_END
