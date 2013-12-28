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
#include "NstMapper033.h"
	 
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER33::Reset()
{
	EnableIrqSync(IRQSYNC_PPU);

	for (ULONG i=0x8000; i <= 0xFFFFU; ++i)
	{
		switch (i & 0xF003)
		{
     		case 0x8000: cpu->SetPort( i, this, Peek_8000, Poke_8000 ); continue;
			case 0x8001: cpu->SetPort( i, this, Peek_8000, Poke_8001 ); continue;
			case 0x8002: cpu->SetPort( i, this, Peek_8000, Poke_8002 ); continue;
			case 0x8003: cpu->SetPort( i, this, Peek_8000, Poke_8003 ); continue;
			case 0xA000: cpu->SetPort( i, this, Peek_A000, Poke_A000 ); continue;
			case 0xA001: cpu->SetPort( i, this, Peek_A000, Poke_A001 ); continue;
			case 0xA002: cpu->SetPort( i, this, Peek_A000, Poke_A002 ); continue;
			case 0xA003: cpu->SetPort( i, this, Peek_A000, Poke_A003 ); continue;
			case 0xC000: cpu->SetPort( i, this, Peek_C000, Poke_C000 ); continue;
			case 0xC001: cpu->SetPort( i, this, Peek_C000, Poke_C001 ); continue;
			case 0xC002: cpu->SetPort( i, this, Peek_C000, Poke_C002 ); continue;
			case 0xC003: cpu->SetPort( i, this, Peek_C000, Poke_C003 ); continue;
			case 0xE000: cpu->SetPort( i, this, Peek_E000, Poke_E000 ); continue;
		}
	}

	Mirror8000 = TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER33,8000) 
{
	if (Mirror8000)
		ppu->SetMirroring( (data & 0x40) ? MIRROR_HORIZONTAL : MIRROR_VERTICAL );

	apu->Update(); 
	pRom.SwapBanks<n8k,0x0000>(data);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER33,8001) 
{ 
	apu->Update(); 
	pRom.SwapBanks<n8k,0x2000>(data); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER33,8002) { ppu->Update(); cRom.SwapBanks<n2k,0x0000>(data); }
NES_POKE(MAPPER33,8003) { ppu->Update(); cRom.SwapBanks<n2k,0x0800>(data); }
NES_POKE(MAPPER33,A000) { ppu->Update(); cRom.SwapBanks<n1k,0x1000>(data); }
NES_POKE(MAPPER33,A001) { ppu->Update(); cRom.SwapBanks<n1k,0x1400>(data); }
NES_POKE(MAPPER33,A002) { ppu->Update(); cRom.SwapBanks<n1k,0x1800>(data); }
NES_POKE(MAPPER33,A003) { ppu->Update(); cRom.SwapBanks<n1k,0x1C00>(data); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER33,C000) { IrqLatch = data;       }
NES_POKE(MAPPER33,C001) { IrqCount = IrqLatch; }
NES_POKE(MAPPER33,C002) { SetIrqEnable(FALSE);   }
NES_POKE(MAPPER33,C003) { SetIrqEnable(TRUE);    }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER33,E000) 
{
	Mirror8000 = FALSE;
	ppu->SetMirroring( (data & 0x40) ? MIRROR_HORIZONTAL : MIRROR_VERTICAL );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER33::IrqSync()
{
	if (++IrqCount == 0x100)
	{
		SetIrqEnable(FALSE);
		cpu->TryIRQ();
	}
}

NES_NAMESPACE_END

