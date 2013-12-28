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
#include "NstMapper064.h"
		
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER64::Reset()
{
	EnableIrqSync(IRQSYNC_COMBINED);

	for (ULONG i=0x8000; i <= 0xFFFF; ++i)
	{
		switch (i & 0xF001)
		{
     		case 0x8000: cpu.SetPort( i, this, Peek_8000, Poke_8000 );	break;
     		case 0x8001: cpu.SetPort( i, this, Peek_8000, Poke_8001 );	break;
     		case 0xA000: cpu.SetPort( i, this, Peek_A000, Poke_A000 );	break;
    		case 0xC000: cpu.SetPort( i, this, Peek_C000, Poke_C000 );	break;
    		case 0xC001: cpu.SetPort( i, this, Peek_C000, Poke_C001 );	break;
    		case 0xE000: cpu.SetPort( i, this, Peek_E000, Poke_E000 );	break;
    		case 0xE001: cpu.SetPort( i, this, Peek_E000, Poke_E001 );	break;
		}
	}

	mode = 0;
	IrqOn = FALSE;
	IrqMode = 0;
	IrqNext = 0;
	command = 0;
	ScanLine = 0;

	for (UINT i=0; i < 8; ++i)
	{
		regs[0][i] = ~0x0;
		regs[1][i] = ~0x0;
	}

	UpdateBanks();

	SetIrqEnable(TRUE);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER64::UpdateBanks()
{
	if (command & 0x20)
	{
		cRom.SwapBanks<n1k,0x0000>( regs[0][0] );
		cRom.SwapBanks<n1k,0x0400>( regs[1][0] );
		cRom.SwapBanks<n1k,0x0800>( regs[0][1] );
		cRom.SwapBanks<n1k,0x0C00>( regs[1][1] );
	}
	else
	{
		cRom.SwapBanks<n2k,0x0000>( regs[0][0] >> 1 );
		cRom.SwapBanks<n2k,0x0800>( regs[0][1] >> 1 );
	}

	cRom.SwapBanks<n1k,0x1000>( regs[0][2] );
	cRom.SwapBanks<n1k,0x1400>( regs[0][3] );
	cRom.SwapBanks<n1k,0x1800>( regs[0][4] );
	cRom.SwapBanks<n1k,0x1C00>( regs[0][5] );

	pRom.SwapBanks<n8k,0x0000>( regs[0][6] );
	pRom.SwapBanks<n8k,0x2000>( regs[0][7] );
	pRom.SwapBanks<n8k,0x4000>( regs[1][7] );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER64,8000)
{
	command = data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER64,8001)
{
	apu.Update();
	ppu.Update();

	regs[(command & 0xF) < 0x8 ? 0 : 1][command & 0x7] = data;

	UpdateBanks();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER64,A000)
{
	ppu.SetMirroring( (data & 0x1) ? MIRROR_HORIZONTAL : MIRROR_VERTICAL );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER64,C000) 
{ 
	IrqLatch = data; 

	if (mode)
		IrqCount = data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER64,C001) 
{ 
	mode = 1;
	IrqNext = 0;
	IrqCount = IrqLatch;
	IrqMode = (data & 0x1);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER64,E000) 
{
	cpu.ClearIRQ();
	IrqOn = FALSE; 

	if (mode)
		IrqCount = IrqLatch; 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER64,E001) 
{ 
	IrqOn = TRUE;  

	if (mode)
		IrqCount = IrqLatch; 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER64::IrqSync(const UINT delta)
{
	if (IrqMode)
	{
		if ((IrqNext += delta) >= 4)
		{
			IrqNext -= 4;

			if (IrqCount >= 0 && --IrqCount < 0 && IrqOn)
				cpu.DoIRQ();
		}
	}
	else
	{
		if (ScanLine < 21 && ppu.IsBgEnabled() || ppu.IsSpEnabled())
		{
			const ULONG cycles = cpu.GetCycles<CPU::CYCLE_MASTER>();

			if (cycles >= ((NES_PPU_MCC_HSYNC_NTSC * 21UL)))
			{
				ScanLine = 21;
			}
			else if (cycles >= ((NES_PPU_MCC_HSYNC_NTSC * 20UL) + NES_PPU_TO_NTSC(52)))
			{
				ScanLine = 21;

				if (!IrqCount && IrqOn)
				{
					IrqCount = -1;
					mode = 1;
					cpu.DoIRQ();
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER64::IrqSync()
{
	if (!IrqMode)
	{
		if (IrqCount >= 0 && --IrqCount < 0 && IrqOn)
		{
			mode = 1;
			cpu.DoIRQ();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER64::EndFrame()
{
	ScanLine = 0;
}

NES_NAMESPACE_END
