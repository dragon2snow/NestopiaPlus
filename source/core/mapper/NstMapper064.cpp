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
	EnableIrqSync(IRQSYNC_PPU);

	for (ULONG i=0x8000; i <= 0xFFFFU; ++i)
	{
		switch (i & 0xF003)
		{
     		case 0x8000: cpu->SetPort( 0x8000, this, Peek_8000, Poke_8000 );	break;
     		case 0x8001: cpu->SetPort( 0x8001, this, Peek_8000, Poke_8001 );	break;
     		case 0xA000: cpu->SetPort( 0xA000, this, Peek_A000, Poke_A000 );	break;
    		case 0xC000: cpu->SetPort( 0xC000, this, Peek_C000, Poke_C000 );	break;
    		case 0xC001: cpu->SetPort( 0xC001, this, Peek_C000, Poke_C001 );	break;
    		case 0xE000: cpu->SetPort( 0xE000, this, Peek_E000, Poke_E000 );	break;
    		case 0xE001: cpu->SetPort( 0xE001, this, Peek_E000, Poke_E001 );	break;
		}
	}

	pRom.SwapBanks<n8k,0x0000>( pRom.NumBanks<n8k>() - 1 );
	pRom.SwapBanks<n8k,0x2000>( pRom.NumBanks<n8k>() - 1 );
	pRom.SwapBanks<n8k,0x4000>( pRom.NumBanks<n8k>() - 1 );
	pRom.SwapBanks<n8k,0x6000>( pRom.NumBanks<n8k>() - 1 );

	regs[0] = 0;
	regs[1] = 0;
	regs[2] = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER64,8000)
{
	regs[0] = data & 0x0F;
	regs[1] = data & 0x40;
	regs[2] = data & 0x80;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER64,8001)
{
	apu->Update();
	ppu->Update();

	switch (regs[0])
	{
       	case 0x0:
		
			if (regs[2]) cRom.SwapBanks<n2k,0x1000>(data >> 1);
			else         cRom.SwapBanks<n2k,0x0000>(data >> 1);
			break;

       	case 0x1:
		
      		if (regs[2]) cRom.SwapBanks<n2k,0x1800>(data >> 1);
     		else         cRom.SwapBanks<n2k,0x0800>(data >> 1);
     		break;

       	case 0x02:

     		if (regs[2]) cRom.SwapBanks<n1k,0x0000>(data);
     		else         cRom.SwapBanks<n1k,0x1000>(data);
     		break;

     	case 0x3:

     		if (regs[2]) cRom.SwapBanks<n1k,0x0400>(data);
     		else         cRom.SwapBanks<n1k,0x1400>(data);
     		break;

     	case 0x4:

    		if (regs[2]) cRom.SwapBanks<n1k,0x0800>(data);
     		else         cRom.SwapBanks<n1k,0x1800>(data);
    		break;

     	case 0x5:

       		if (regs[2]) cRom.SwapBanks<n1k,0x0C00>(data);
     		else         cRom.SwapBanks<n1k,0x1C00>(data);
     		break;

     	case 0x6:

			if (regs[1]) pRom.SwapBanks<n8k,0x2000>(data);
			else         pRom.SwapBanks<n8k,0x0000>(data);
			break;

     	case 0x7:

     		if (regs[1]) pRom.SwapBanks<n8k,0x4000>(data);
     		else         pRom.SwapBanks<n8k,0x2000>(data);
     		break;

    	case 0x8:

     		cRom.SwapBanks<n1k,0x0000>(data);
     		break;

     	case 0x9:

			cRom.SwapBanks<n1k,0x0800>(data);
			break;

     	case 0xF:

     		if (regs[1]) pRom.SwapBanks<n8k,0x0000>(data);
     		else         pRom.SwapBanks<n8k,0x4000>(data);
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER64,A000)
{
	ppu->SetMirroring( (data & 0x1) ? MIRROR_HORIZONTAL : MIRROR_VERTICAL );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER64,C000) { IrqCount = IrqLatch = data;               }
NES_POKE(MAPPER64,C001) { IrqCount = IrqLatch;                      }
NES_POKE(MAPPER64,E000) { SetIrqEnable(FALSE); IrqCount = IrqLatch; }
NES_POKE(MAPPER64,E001) { SetIrqEnable(TRUE);  IrqCount = IrqLatch; }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER64::IrqSync()
{
	if (IrqCount-- <= 0)
	{
		IrqCount = IrqLatch;
		cpu->DoIRQ();
	}
}

NES_NAMESPACE_END

