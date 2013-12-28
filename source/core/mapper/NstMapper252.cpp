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
#include "NstMapper252.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER252::Reset()
{
	EnableCartridgeCRam();
	EnableIrqSync(IRQSYNC_COUNT);

	for (ULONG i=0x8000; i <= 0xFFFF; ++i)
	{
		switch (i & 0xF000)
		{
     		case 0x8000: cpu.SetPort( i, this, Peek_8000, Poke_8000 ); continue;
			case 0xA000: cpu.SetPort( i, this, Peek_A000, Poke_A000 ); continue;
		}

		switch (i & 0xF00C)
		{
			case 0xB000: cpu.SetPort( i, this, Peek_B000, Poke_B000 ); continue;
			case 0xB004: cpu.SetPort( i, this, Peek_B000, Poke_B004 ); continue;
			case 0xB008: cpu.SetPort( i, this, Peek_B000, Poke_B008 ); continue;
			case 0xB00C: cpu.SetPort( i, this, Peek_B000, Poke_B00C ); continue;
			case 0xC000: cpu.SetPort( i, this, Peek_C000, Poke_C000 ); continue;
			case 0xC004: cpu.SetPort( i, this, Peek_C000, Poke_C004 ); continue;
			case 0xC008: cpu.SetPort( i, this, Peek_C000, Poke_C008 ); continue;
			case 0xC00C: cpu.SetPort( i, this, Peek_C000, Poke_C00C ); continue;
			case 0xD000: cpu.SetPort( i, this, Peek_D000, Poke_D000 ); continue;
			case 0xD004: cpu.SetPort( i, this, Peek_D000, Poke_D004 ); continue;
			case 0xD008: cpu.SetPort( i, this, Peek_D000, Poke_D008 ); continue;
			case 0xD00C: cpu.SetPort( i, this, Peek_D000, Poke_D00C ); continue;
			case 0xE000: cpu.SetPort( i, this, Peek_E000, Poke_E000 ); continue;
			case 0xE004: cpu.SetPort( i, this, Peek_E000, Poke_E004 ); continue;
			case 0xE008: cpu.SetPort( i, this, Peek_E000, Poke_E008 ); continue;
			case 0xE00C: cpu.SetPort( i, this, Peek_E000, Poke_E00C ); continue;
			case 0xF000: cpu.SetPort( i, this, Peek_F000, Poke_F000 ); continue;
			case 0xF004: cpu.SetPort( i, this, Peek_F000, Poke_F004 ); continue;
			case 0xF008: cpu.SetPort( i, this, Peek_F000, Poke_F008 ); continue;
			case 0xF00C: cpu.SetPort( i, this, Peek_F000, Poke_F00C ); continue;
		}
	}

	IrqClock = 0;

	for (UINT i=0; i < 8; ++i)
		regs[0] = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER252,8000) { apu.Update(); pRom.SwapBanks<n8k,0x0000>( data ); }
NES_POKE(MAPPER252,A000) { apu.Update(); pRom.SwapBanks<n8k,0x2000>( data ); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER252,B000) { ppu.Update(); regs[0] = (regs[0] & 0xF0) | ((data & 0xF) << 0); cRom.SwapBanks<n1k,0x0000>( regs[0] ); }
NES_POKE(MAPPER252,B004) { ppu.Update(); regs[0] = (regs[0] & 0x0F) | ((data & 0xF) << 4); cRom.SwapBanks<n1k,0x0000>( regs[0] ); }
NES_POKE(MAPPER252,B008) { ppu.Update(); regs[1] = (regs[1] & 0xF0) | ((data & 0xF) << 0); cRom.SwapBanks<n1k,0x0400>( regs[1] ); }
NES_POKE(MAPPER252,B00C) { ppu.Update(); regs[1] = (regs[1] & 0x0F) | ((data & 0xF) << 4); cRom.SwapBanks<n1k,0x0400>( regs[1] ); }
NES_POKE(MAPPER252,C000) { ppu.Update(); regs[2] = (regs[2] & 0xF0) | ((data & 0xF) << 0); cRom.SwapBanks<n1k,0x0800>( regs[2] ); }
NES_POKE(MAPPER252,C004) { ppu.Update(); regs[2] = (regs[2] & 0x0F) | ((data & 0xF) << 4); cRom.SwapBanks<n1k,0x0800>( regs[2] ); }
NES_POKE(MAPPER252,C008) { ppu.Update(); regs[3] = (regs[3] & 0xF0) | ((data & 0xF) << 0); cRom.SwapBanks<n1k,0x0C00>( regs[3] ); }
NES_POKE(MAPPER252,C00C) { ppu.Update(); regs[3] = (regs[3] & 0x0F) | ((data & 0xF) << 4); cRom.SwapBanks<n1k,0x0C00>( regs[3] ); }
NES_POKE(MAPPER252,D000) { ppu.Update(); regs[4] = (regs[4] & 0xF0) | ((data & 0xF) << 0); cRom.SwapBanks<n1k,0x1000>( regs[4] ); }
NES_POKE(MAPPER252,D004) { ppu.Update(); regs[4] = (regs[4] & 0x0F) | ((data & 0xF) << 4); cRom.SwapBanks<n1k,0x1000>( regs[4] ); }
NES_POKE(MAPPER252,D008) { ppu.Update(); regs[5] = (regs[5] & 0xF0) | ((data & 0xF) << 0); cRom.SwapBanks<n1k,0x1400>( regs[5] ); }
NES_POKE(MAPPER252,D00C) { ppu.Update(); regs[5] = (regs[5] & 0x0F) | ((data & 0xF) << 4); cRom.SwapBanks<n1k,0x1400>( regs[5] ); }
NES_POKE(MAPPER252,E000) { ppu.Update(); regs[6] = (regs[6] & 0xF0) | ((data & 0xF) << 0); cRom.SwapBanks<n1k,0x1800>( regs[6] ); }
NES_POKE(MAPPER252,E004) { ppu.Update(); regs[6] = (regs[6] & 0x0F) | ((data & 0xF) << 4); cRom.SwapBanks<n1k,0x1800>( regs[6] ); }
NES_POKE(MAPPER252,E008) { ppu.Update(); regs[7] = (regs[7] & 0xF0) | ((data & 0xF) << 0); cRom.SwapBanks<n1k,0x1C00>( regs[7] ); }
NES_POKE(MAPPER252,E00C) { ppu.Update(); regs[7] = (regs[7] & 0x0F) | ((data & 0xF) << 4); cRom.SwapBanks<n1k,0x1C00>( regs[7] ); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER252,F000) { IrqLatch = (IrqLatch & 0xF0) | ((data & 0xF) << 0); }
NES_POKE(MAPPER252,F004) { IrqLatch = (IrqLatch & 0x0F) | ((data & 0xF) << 4); }

NES_POKE(MAPPER252,F008) 
{
	cpu.ClearIRQ();

	IrqTmp = data & 0x3;

	if (SetIrqEnable( IrqTmp & 0x2 ))
	{
		IrqCount = IrqLatch;
		IrqClock = 0;
	}
}

NES_POKE(MAPPER252,F00C) 
{
	cpu.ClearIRQ();
	IrqTmp = (IrqTmp & 0x1) * 3;
	SetIrqEnable( IrqTmp & 0x2 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER252::IrqSync(const UINT delta)
{
	if ((IrqClock += delta) >= 114)
	{
		IrqClock -= 114;

		if (IrqCount++ == 0xFF)
		{
			IrqCount = IrqLatch;
			IrqTmp = (IrqTmp & 0x1) * 3;
			SetIrqEnable( IrqTmp & 0x2 );
			cpu.DoIRQ();
		}
	}
}

NES_NAMESPACE_END
