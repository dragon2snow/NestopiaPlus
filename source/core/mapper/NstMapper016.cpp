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
#include "NstMapper016.h"
	   
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER16::Reset()
{
	EnableIrqSync(IRQSYNC_COUNT);

	for (ULONG i=0x6000; i <= 0xFFFF; ++i)
	{
		switch (i & 0x000F)
		{
     		case 0x0000: cpu.SetPort( i, this, i > 0x7FFF ? Peek_pRom : Peek_Nop, Poke_x000 ); continue;
     		case 0x0001: cpu.SetPort( i, this, i > 0x7FFF ? Peek_pRom : Peek_Nop, Poke_x001 ); continue;
     		case 0x0002: cpu.SetPort( i, this, i > 0x7FFF ? Peek_pRom : Peek_Nop, Poke_x002 ); continue;
    		case 0x0003: cpu.SetPort( i, this, i > 0x7FFF ? Peek_pRom : Peek_Nop, Poke_x003 ); continue;
     		case 0x0004: cpu.SetPort( i, this, i > 0x7FFF ? Peek_pRom : Peek_Nop, Poke_x004 ); continue;
     		case 0x0005: cpu.SetPort( i, this, i > 0x7FFF ? Peek_pRom : Peek_Nop, Poke_x005 ); continue;
       		case 0x0006: cpu.SetPort( i, this, i > 0x7FFF ? Peek_pRom : Peek_Nop, Poke_x006 ); continue;
     		case 0x0007: cpu.SetPort( i, this, i > 0x7FFF ? Peek_pRom : Peek_Nop, Poke_x007 ); continue;
     		case 0x0008: cpu.SetPort( i, this, i > 0x7FFF ? Peek_pRom : Peek_Nop, Poke_x008 ); continue;
     		case 0x0009: cpu.SetPort( i, this, i > 0x7FFF ? Peek_pRom : Peek_Nop, Poke_x009 ); continue;
     		case 0x000A: cpu.SetPort( i, this, i > 0x7FFF ? Peek_pRom : Peek_Nop, Poke_x00A ); continue;
     		case 0x000B: cpu.SetPort( i, this, i > 0x7FFF ? Peek_pRom : Peek_Nop, Poke_x00B ); continue;
   	    	case 0x000C: cpu.SetPort( i, this, i > 0x7FFF ? Peek_pRom : Peek_Nop, Poke_x00C ); continue;
    		case 0x000D: cpu.SetPort( i, this, i > 0x7FFF ? Peek_pRom : Peek_Nop, Poke_x00D ); continue;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER16,pRom) 
{ 
	return pRom[address - 0x8000]; 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER16,x000) { ppu.Update(); cRom.SwapBanks<n1k,0x0000>(data); }
NES_POKE(MAPPER16,x001) { ppu.Update(); cRom.SwapBanks<n1k,0x0400>(data); }
NES_POKE(MAPPER16,x002) { ppu.Update(); cRom.SwapBanks<n1k,0x0800>(data); }
NES_POKE(MAPPER16,x003) { ppu.Update(); cRom.SwapBanks<n1k,0x0C00>(data); }
NES_POKE(MAPPER16,x004) { ppu.Update(); cRom.SwapBanks<n1k,0x1000>(data); }
NES_POKE(MAPPER16,x005) { ppu.Update(); cRom.SwapBanks<n1k,0x1400>(data); }
NES_POKE(MAPPER16,x006) { ppu.Update(); cRom.SwapBanks<n1k,0x1800>(data); }
NES_POKE(MAPPER16,x007) { ppu.Update(); cRom.SwapBanks<n1k,0x1C00>(data); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER16,x008) 
{ 
	apu.Update(); 
	pRom.SwapBanks<n16k,0x0000>(data); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER16,x009) 
{
	static const UCHAR select[4][4] =
	{
		{0,1,0,1},
		{0,0,1,1},
		{0,0,0,0},
		{1,1,1,1}
	};

	const UCHAR* const index = select[data & 0x3];

	ppu.SetMirroring
	(
	    index[0],
		index[1],
		index[2],
		index[3]
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER16,x00A) 
{ 
	cpu.ClearIRQ(); 
	SetIrqEnable(data & 0x1);
	IrqCount = IrqLatch; 
}

NES_POKE(MAPPER16,x00B) { IrqLatch = (IrqLatch & 0xFF00) | (data << 0);  }
NES_POKE(MAPPER16,x00C) { IrqLatch = (IrqLatch & 0x00FF) | (data << 8);  }

////////////////////////////////////////////////////////////////////////////////////////
// EEPROM I/O
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER16,x00D)
{
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER16::IrqSync(const UINT cycles)
{
	if ((IrqCount -= cycles) < 0)
	{
		IrqCount = 0xFFFF;
		SetIrqEnable(FALSE);
		cpu.DoIRQ();
	}
}

NES_NAMESPACE_END
