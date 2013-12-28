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
#include "NstMapper091.h"
	
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER91::Reset()
{
	EnableIrqSync(IRQSYNC_PPU);

	for (UINT i=0x6000; i <= 0x7FFF; ++i)
	{
		switch (i & 0xF003)
		{
     		case 0x6000: cpu.SetPort( i, this, Peek_Nop, Poke_6000 ); continue; 
			case 0x6001: cpu.SetPort( i, this, Peek_Nop, Poke_6001 ); continue;
			case 0x6002: cpu.SetPort( i, this, Peek_Nop, Poke_6002 ); continue;
			case 0x6003: cpu.SetPort( i, this, Peek_Nop, Poke_6003 ); continue;
			case 0x7000: cpu.SetPort( i, this, Peek_Nop, Poke_7000 ); continue;
			case 0x7001: cpu.SetPort( i, this, Peek_Nop, Poke_7001 ); continue;
			case 0x7002: cpu.SetPort( i, this, Peek_Nop, Poke_7002 ); continue;
			case 0x7003: cpu.SetPort( i, this, Peek_Nop, Poke_7003 ); continue;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER91,6000) { ppu.Update(); cRom.SwapBanks<n2k,0x0000>(data); }
NES_POKE(MAPPER91,6001) { ppu.Update(); cRom.SwapBanks<n2k,0x0800>(data); }
NES_POKE(MAPPER91,6002) { ppu.Update(); cRom.SwapBanks<n2k,0x1000>(data); }
NES_POKE(MAPPER91,6003) { ppu.Update(); cRom.SwapBanks<n2k,0x1800>(data); }
NES_POKE(MAPPER91,7000) { apu.Update(); pRom.SwapBanks<n8k,0x0000>(data); }
NES_POKE(MAPPER91,7001) { apu.Update(); pRom.SwapBanks<n8k,0x2000>(data); }
NES_POKE(MAPPER91,7002) { SetIrqEnable(FALSE); cpu.ClearIRQ(); IrqCount = 0; }
NES_POKE(MAPPER91,7003) { SetIrqEnable(TRUE); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER91::IrqSync()
{
	if (++IrqCount >= 8)
		cpu.DoIRQ();
}

NES_NAMESPACE_END

