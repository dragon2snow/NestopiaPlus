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
#include "NstMapper117.h"
	  
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER117::Reset()
{
	EnableIrqSync(IRQSYNC_PPU);

	cpu->SetPort( 0x8000, this, Peek_8000, Poke_8000 );
	cpu->SetPort( 0x8001, this, Peek_8000, Poke_8001 );
	cpu->SetPort( 0x8002, this, Peek_8000, Poke_8002 );
	cpu->SetPort( 0x8003, this, Peek_8000, Poke_8003 );
	cpu->SetPort( 0xA000, this, Peek_A000, Poke_A000 );
	cpu->SetPort( 0xA001, this, Peek_A000, Poke_A001 );
	cpu->SetPort( 0xA002, this, Peek_A000, Poke_A002 );
	cpu->SetPort( 0xA003, this, Peek_A000, Poke_A003 );
	cpu->SetPort( 0xA004, this, Peek_A000, Poke_A004 );
	cpu->SetPort( 0xA005, this, Peek_A000, Poke_A005 );
	cpu->SetPort( 0xA006, this, Peek_A000, Poke_A006 );
	cpu->SetPort( 0xA007, this, Peek_A000, Poke_A007 );
	cpu->SetPort( 0xC001, this, Peek_C000, Poke_C001 );
	cpu->SetPort( 0xC002, this, Peek_C000, Poke_C002 );
	cpu->SetPort( 0xC003, this, Peek_C000, Poke_C003 );
	cpu->SetPort( 0xE000, this, Peek_E000, Poke_E000 );

	IrqLine = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER117,8000) { apu->Update(); pRom.SwapBanks<n8k,0x0000>(data); }
NES_POKE(MAPPER117,8001) { apu->Update(); pRom.SwapBanks<n8k,0x2000>(data); }
NES_POKE(MAPPER117,8002) { apu->Update(); pRom.SwapBanks<n8k,0x4000>(data); }
NES_POKE(MAPPER117,8003) { apu->Update(); pRom.SwapBanks<n8k,0x6000>(data); }
NES_POKE(MAPPER117,A000) { ppu->Update(); cRom.SwapBanks<n1k,0x0000>(data); }
NES_POKE(MAPPER117,A001) { ppu->Update(); cRom.SwapBanks<n1k,0x0400>(data); }
NES_POKE(MAPPER117,A002) { ppu->Update(); cRom.SwapBanks<n1k,0x0800>(data); }
NES_POKE(MAPPER117,A003) { ppu->Update(); cRom.SwapBanks<n1k,0x0C00>(data); }
NES_POKE(MAPPER117,A004) { ppu->Update(); cRom.SwapBanks<n1k,0x1000>(data); }
NES_POKE(MAPPER117,A005) { ppu->Update(); cRom.SwapBanks<n1k,0x1400>(data); }
NES_POKE(MAPPER117,A006) { ppu->Update(); cRom.SwapBanks<n1k,0x1800>(data); }
NES_POKE(MAPPER117,A007) { ppu->Update(); cRom.SwapBanks<n1k,0x1C00>(data); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER117,C001) 
{ 
	IrqLatch = data; 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER117,C002) 
{ 
	cpu->ClearIRQ();  
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER117,C003) 
{ 
	IrqCount = IrqLatch; 
	IrqLine |= IRQ_LINE_2; 
	SetIrqEnable(IrqLine == IRQ_LINE_1_2); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER117,E000) 
{ 
	cpu->ClearIRQ(); 
	IrqLine = (IrqLine & ~IRQ_LINE_1) | (data & IRQ_LINE_1);
	SetIrqEnable(IrqLine == IRQ_LINE_1_2);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER117::IrqSync()
{
	if (--IrqCount <= 0)
	{
		IrqLine &= IRQ_LINE_1;
		SetIrqEnable(FALSE);
		cpu->DoIRQ();
	}
}

NES_NAMESPACE_END
