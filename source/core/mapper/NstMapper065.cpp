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
#include "NstMapper065.h"
		  
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER65::Reset()
{
	EnableIrqSync(IRQSYNC_COUNT);

	if (pRomCrc == 0xE30B7F64UL)
	{
		another = TRUE;
		cpu.SetPort( 0x9001, this, Peek_9000, Poke_9001 );
	}
	else
	{
		another = FALSE;
		cpu.SetPort( 0x9000, this, Peek_9000, Poke_9000 );
		cpu.SetPort( 0x9003, this, Peek_9000, Poke_9003 );
		cpu.SetPort( 0x9004, this, Peek_9000, Poke_9004 );
	}

	cpu.SetPort( 0x8000, this, Peek_8000, Poke_8000 );
	cpu.SetPort( 0x9005, this, Peek_9000, Poke_9005 );
	cpu.SetPort( 0x9006, this, Peek_9000, Poke_9006 );
	cpu.SetPort( 0xB000, this, Peek_B000, Poke_B000 );
	cpu.SetPort( 0xB001, this, Peek_B000, Poke_B001 );
	cpu.SetPort( 0xB002, this, Peek_B000, Poke_B002 );
	cpu.SetPort( 0xB003, this, Peek_B000, Poke_B003 );
	cpu.SetPort( 0xB004, this, Peek_B000, Poke_B004 );
	cpu.SetPort( 0xB005, this, Peek_B000, Poke_B005 );
	cpu.SetPort( 0xB006, this, Peek_B000, Poke_B006 );
	cpu.SetPort( 0xB007, this, Peek_B000, Poke_B007 );
	cpu.SetPort( 0xA000, this, Peek_A000, Poke_A000 );
	cpu.SetPort( 0xC000, this, Peek_C000, Poke_C000 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER65,8000) 
{ 
	apu.Update(); 
	pRom.SwapBanks<n8k,0x0000>(data); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER65,9000) 
{ 
	ppu.SetMirroring( (data & 0x40) ? MIRROR_VERTICAL : MIRROR_HORIZONTAL ); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER65,9001) 
{ 
	ppu.SetMirroring( (data & 0x80) ? MIRROR_VERTICAL : MIRROR_HORIZONTAL ); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER65,9003) 
{ 
	SetIrqEnable(data & 0x80);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER65,9004) 
{ 
	IrqCount = IrqLatch;  
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER65,9005) 
{
	if (another)
	{
		SetIrqEnable(data);
		IrqCount = data << 1;
	}
	else
	{
		IrqLatch = (IrqLatch & 0x00FF) || (data << 8);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER65,9006) 
{
	if (another)
	{
		SetIrqEnable(TRUE);
	}
	else
	{
		IrqLatch = (IrqLatch & 0xFF00) | data;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER65,B000) { ppu.Update(); cRom.SwapBanks<n1k,0x0000>(data); }
NES_POKE(MAPPER65,B001) { ppu.Update(); cRom.SwapBanks<n1k,0x0400>(data); }
NES_POKE(MAPPER65,B002) { ppu.Update(); cRom.SwapBanks<n1k,0x0800>(data); }
NES_POKE(MAPPER65,B003) { ppu.Update(); cRom.SwapBanks<n1k,0x0C00>(data); }
NES_POKE(MAPPER65,B004) { ppu.Update(); cRom.SwapBanks<n1k,0x1000>(data); }
NES_POKE(MAPPER65,B005) { ppu.Update(); cRom.SwapBanks<n1k,0x1400>(data); }
NES_POKE(MAPPER65,B006) { ppu.Update(); cRom.SwapBanks<n1k,0x1800>(data); }
NES_POKE(MAPPER65,B007) { ppu.Update(); cRom.SwapBanks<n1k,0x1C00>(data); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER65,A000) { apu.Update(); pRom.SwapBanks<n8k,0x2000>(data); }
NES_POKE(MAPPER65,C000) { apu.Update(); pRom.SwapBanks<n8k,0x4000>(data); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER65::IrqSync(const UINT delta)
{
	IrqCount -= delta;

	if (IrqCount <= 0)
	{
		SetIrqEnable(FALSE);

		if (another)
			IrqCount = 0xFFFF;

		cpu.TryIRQ();
	}
}

NES_NAMESPACE_END

