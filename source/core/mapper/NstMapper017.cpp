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
#include "NstMapper017.h"
		   
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
// reset
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER17::Reset()
{
	EnableIrqSync(IRQSYNC_COUNT);

	switch (pRomCrc)
	{
		case 0x57BAF095UL: // Doki! Doki! Yuuenchi
		case 0xE64138ECUL: // Dragon Ball Z 2 - Gekishin Freeza!!
		case 0xC7A4583EUL: // Dragon Ball Z 3 - Ressen Jinzou Ningen
		case 0xCB7E529DUL: // SD Gundam Gaiden - Knight Gundam Monogatari 2 - Hikari no Kishi
		case 0x8F3F8B1FUL: // Spartan X2
		case 0xA3047263UL: // -||-
		case 0xC529C604UL: // -||-

			IrqNum = 0x10000UL;
			break;

		default:

			IrqNum = 0xD000UL;
			break;
	}

	cpu.SetPort( 0x42FE, this, Peek_Nop, Poke_42FE );
	cpu.SetPort( 0x42FF, this, Peek_Nop, Poke_42FF );
	cpu.SetPort( 0x4501, this, Peek_Nop, Poke_4501 );
	cpu.SetPort( 0x4502, this, Peek_Nop, Poke_4502 );
	cpu.SetPort( 0x4503, this, Peek_Nop, Poke_4503 );
	cpu.SetPort( 0x4504, this, Peek_Nop, Poke_4504 );
	cpu.SetPort( 0x4505, this, Peek_Nop, Poke_4505 );
	cpu.SetPort( 0x4506, this, Peek_Nop, Poke_4506 );
	cpu.SetPort( 0x4507, this, Peek_Nop, Poke_4507 );
	cpu.SetPort( 0x4510, this, Peek_Nop, Poke_4510 );
	cpu.SetPort( 0x4511, this, Peek_Nop, Poke_4511 );
	cpu.SetPort( 0x4512, this, Peek_Nop, Poke_4512 );
	cpu.SetPort( 0x4513, this, Peek_Nop, Poke_4513 );
	cpu.SetPort( 0x4514, this, Peek_Nop, Poke_4514 );
	cpu.SetPort( 0x4515, this, Peek_Nop, Poke_4515 );
	cpu.SetPort( 0x4516, this, Peek_Nop, Poke_4516 );
	cpu.SetPort( 0x4517, this, Peek_Nop, Poke_4517 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER17,42FE)  
{
	const UINT m = (data & 0x10) ? 1 : 0;
	ppu.SetMirroring(m,m,m,m);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER17,42FF)  
{
	ppu.SetMirroring( (data & 0x10) ? MIRROR_HORIZONTAL : MIRROR_VERTICAL);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER17,4501)  
{ 
	SetIrqEnable(data & 0x1);
	cpu.ClearIRQ();
}

NES_POKE(MAPPER17,4502)  
{ 
	IrqCount = (IrqCount & 0xFF00) | (data << 0); 
}

NES_POKE(MAPPER17,4503)  
{ 
	IrqCount = (IrqCount & 0x00FF) | (data << 8); 
	SetIrqEnable(TRUE);
	cpu.ClearIRQ();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER17,4504) { apu.Update(); pRom.SwapBanks<n8k,0x0000>( data ); }
NES_POKE(MAPPER17,4505) { apu.Update(); pRom.SwapBanks<n8k,0x2000>( data ); }
NES_POKE(MAPPER17,4506) { apu.Update(); pRom.SwapBanks<n8k,0x4000>( data ); }
NES_POKE(MAPPER17,4507) { apu.Update(); pRom.SwapBanks<n8k,0x6000>( data ); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER17,4510) { ppu.Update(); cRom.SwapBanks<n1k,0x0000>( data ); }
NES_POKE(MAPPER17,4511) { ppu.Update(); cRom.SwapBanks<n1k,0x0400>( data ); }
NES_POKE(MAPPER17,4512) { ppu.Update(); cRom.SwapBanks<n1k,0x0800>( data ); }
NES_POKE(MAPPER17,4513) { ppu.Update(); cRom.SwapBanks<n1k,0x0C00>( data ); }
NES_POKE(MAPPER17,4514) { ppu.Update(); cRom.SwapBanks<n1k,0x1000>( data ); }
NES_POKE(MAPPER17,4515) { ppu.Update(); cRom.SwapBanks<n1k,0x1400>( data ); }
NES_POKE(MAPPER17,4516) { ppu.Update(); cRom.SwapBanks<n1k,0x1800>( data ); }
NES_POKE(MAPPER17,4517) { ppu.Update(); cRom.SwapBanks<n1k,0x1C00>( data ); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER17::IrqSync(const UINT delta)
{
	if ((IrqCount += delta) >= IrqNum)
	{
		IrqCount = 0;
		SetIrqEnable(FALSE);
		cpu.DoIRQ();
	}
}

NES_NAMESPACE_END

