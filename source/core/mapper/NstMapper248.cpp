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
#include "NstMapper248.h"
		
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER248::Reset()
{
	EnableIrqSync(IRQSYNC_PPU);

	cpu.SetPort( 0x6000, 0x7FFF, this, Peek_Nop, Poke_6000 );

	for (ULONG i=0x8000; i <= 0xFFFFU; ++i)
	{
		switch (i & 0xF001)
		{
			case 0x8000: cpu.SetPort( i, this, Peek_8000, Poke_8000 ); continue;
			case 0x8001: cpu.SetPort( i, this, Peek_8000, Poke_8001 ); continue;
			case 0xA000: cpu.SetPort( i, this, Peek_A000, Poke_A000 ); continue;
			case 0xC000: cpu.SetPort( i, this, Peek_C000, Poke_C000 ); continue;
			case 0xC001: cpu.SetPort( i, this, Peek_C000, Poke_C001 ); continue;
			case 0xE000: cpu.SetPort( i, this, Peek_E000, Poke_E000 ); continue;
			case 0xE001: cpu.SetPort( i, this, Peek_E000, Poke_E001 ); continue;
		}
	}

	command  = 0;
	status   = 0;
	banks[0] = 0;
	banks[1] = 1;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER248::SwapBanks()
{
	apu.Update();

	if (status & SELECT_PROM_16)
	{
		pRom.SwapBanks<n16k,0x0000>( status & SWAP_PROM );
	}
	else
	{
		pRom.SwapBanks<n8k,0x0000>( banks[0] & 0x1F );
		pRom.SwapBanks<n8k,0x2000>( banks[1] & 0x1F );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER248,6000)
{
	status = data;
	SwapBanks();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER248,8000)
{
	command = data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER248,8001)
{
	ppu.Update();

	switch (command & 0x7)
	{
     	case 0x0: cRom.SwapBanks<n2k,0x0000>( data >> 1 ); return;
     	case 0x1: cRom.SwapBanks<n2k,0x0800>( data >> 1 ); return;
       	case 0x2: cRom.SwapBanks<n1k,0x1000>( data >> 0 ); return;
     	case 0x3: cRom.SwapBanks<n1k,0x1400>( data >> 0 ); return;
     	case 0x4: cRom.SwapBanks<n1k,0x1800>( data >> 0 ); return;
     	case 0x5: cRom.SwapBanks<n1k,0x1C00>( data >> 0 ); return;
     	case 0x6: banks[0] = data; SwapBanks(); return;
     	case 0x7: banks[1] = data; SwapBanks(); return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER248,A000)
{
	if (mirroring != MIRROR_FOURSCREEN)
		ppu.SetMirroring( (data & 0x1) ? MIRROR_HORIZONTAL : MIRROR_VERTICAL );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER248,C000) { IrqLatch = data;     }
NES_POKE(MAPPER248,C001) { IrqCount = IrqLatch; }
NES_POKE(MAPPER248,E000) { SetIrqEnable(FALSE); }
NES_POKE(MAPPER248,E001) { SetIrqEnable(TRUE);  }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER248::IrqSync()
{ 
	if (--IrqCount < 0)
	{
		IrqCount = IrqLatch;
		cpu.DoIRQ();
	}
}

NES_NAMESPACE_END
