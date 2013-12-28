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
#include "NstMapper182.h"
		 
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER182::Reset()
{
	EnableIrqSync(IRQSYNC_PPU);

	for (ULONG i=0x8000; i <= 0xFFFF; ++i)
	{
		switch (i & 0xF003)
		{
     		case 0x8001: cpu.SetPort( i, this, Peek_8000, Poke_8001 ); continue;
    		case 0xA000: cpu.SetPort( i, this, Peek_A000, Poke_A000 ); continue;
     		case 0xC000: cpu.SetPort( i, this, Peek_C000, Poke_C000 ); continue;
     		case 0xE003: cpu.SetPort( i, this, Peek_E000, Poke_E003 ); continue;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER182,8001)
{
	ppu.SetMirroring( (data & 0x1) ? MIRROR_HORIZONTAL : MIRROR_VERTICAL );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER182,A000)
{
	command = data & 0x7;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER182,C000)
{
	apu.Update();
	ppu.Update();

	switch (command)
	{
     	case 0x0: cRom.SwapBanks<n2k,0x0000>(data >> 1); return;
     	case 0x1: cRom.SwapBanks<n1k,0x1400>(data);      return;
     	case 0x2: cRom.SwapBanks<n2k,0x0800>(data >> 1); return;
     	case 0x3: cRom.SwapBanks<n1k,0x1C00>(data);      return;
       	case 0x4: pRom.SwapBanks<n8k,0x0000>(data);      return;
		case 0x5: pRom.SwapBanks<n8k,0x2000>(data);      return;
     	case 0x6: cRom.SwapBanks<n1k,0x1000>(data);      return;
       	case 0x7: cRom.SwapBanks<n1k,0x1800>(data);	     return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER182,E003)
{
	cpu.ClearIRQ();
	IrqCount = data;
	SetIrqEnable(data);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER182::IrqSync()
{
	if (IrqCount-- <= 0)
	{
		IrqCount = 0;
		SetIrqEnable(FALSE);
		cpu.DoIRQ();
	}
}

NES_NAMESPACE_END
