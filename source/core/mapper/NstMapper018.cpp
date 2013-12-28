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
#include "NstMapper018.h"
		 
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
// reset
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER18::Reset()
{
	for (ULONG i=0x8000; i <= 0xFFFF; ++i)
	{
		switch (i & 0xF003)
		{
			case 0x8000: cpu->SetPort( i, this, Peek_8000, Poke_8000 );
			case 0x8001: cpu->SetPort( i, this, Peek_8000, Poke_8001 );
			case 0x8002: cpu->SetPort( i, this, Peek_8000, Poke_8002 );
			case 0x8003: cpu->SetPort( i, this, Peek_8000, Poke_8003 );
			case 0x9000: cpu->SetPort( i, this, Peek_9000, Poke_9000 );
			case 0x9001: cpu->SetPort( i, this, Peek_9000, Poke_9001 );
			case 0xA000: cpu->SetPort( i, this, Peek_A000, Poke_A000 );
			case 0xA001: cpu->SetPort( i, this, Peek_A000, Poke_A001 );
			case 0xA002: cpu->SetPort( i, this, Peek_A000, Poke_A002 );
			case 0xA003: cpu->SetPort( i, this, Peek_A000, Poke_A003 );
			case 0xB000: cpu->SetPort( i, this, Peek_B000, Poke_B000 );
			case 0xB001: cpu->SetPort( i, this, Peek_B000, Poke_B001 );
			case 0xB002: cpu->SetPort( i, this, Peek_B000, Poke_B002 );
			case 0xB003: cpu->SetPort( i, this, Peek_B000, Poke_B003 );
			case 0xC000: cpu->SetPort( i, this, Peek_C000, Poke_C000 );
			case 0xC001: cpu->SetPort( i, this, Peek_C000, Poke_C001 );
			case 0xC002: cpu->SetPort( i, this, Peek_C000, Poke_C002 );
			case 0xC003: cpu->SetPort( i, this, Peek_C000, Poke_C003 );
			case 0xD000: cpu->SetPort( i, this, Peek_D000, Poke_D000 );
			case 0xD001: cpu->SetPort( i, this, Peek_D000, Poke_D001 );
			case 0xD002: cpu->SetPort( i, this, Peek_D000, Poke_D002 );
			case 0xD003: cpu->SetPort( i, this, Peek_D000, Poke_D003 );
			case 0xE000: cpu->SetPort( i, this, Peek_E000, Poke_E000 );
			case 0xE001: cpu->SetPort( i, this, Peek_E000, Poke_E001 );
			case 0xE002: cpu->SetPort( i, this, Peek_E000, Poke_E002 );
			case 0xE003: cpu->SetPort( i, this, Peek_E000, Poke_E003 );
			case 0xF000: cpu->SetPort( i, this, Peek_F000, Poke_F000 );
			case 0xF001: cpu->SetPort( i, this, Peek_F000, Poke_F001 );
			case 0xF002: cpu->SetPort( i, this, Peek_F000, Poke_F002 );
		}
	}

	EnableIrqSync(IRQSYNC_COUNT);

	buffer[0x0] = 0;
	buffer[0x1] = 1;
	buffer[0x2] = pRom.NumBanks<n8k>() - 2;
	buffer[0x3] = pRom.NumBanks<n8k>() - 1;
	buffer[0x4] = 0;
	buffer[0x5] = 0;
	buffer[0x6] = 0;
	buffer[0x7] = 0;
	buffer[0x8] = 0;
	buffer[0x9] = 0;
	buffer[0xA] = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER18,8000) { buffer[0x0] = (buffer[0x0] & 0xF0) | ((data & 0x0F) << 0); apu->Update(); pRom.SwapBanks<n8k,0x0000>( buffer[0x0] ); } 
NES_POKE(MAPPER18,8001) { buffer[0x0] = (buffer[0x0] & 0x0F) | ((data & 0x0F) << 4); apu->Update(); pRom.SwapBanks<n8k,0x0000>( buffer[0x0] ); }
NES_POKE(MAPPER18,8002) { buffer[0x1] = (buffer[0x1] & 0xF0) | ((data & 0x0F) << 0); apu->Update(); pRom.SwapBanks<n8k,0x2000>( buffer[0x1] ); }
NES_POKE(MAPPER18,8003) { buffer[0x1] = (buffer[0x1] & 0x0F) | ((data & 0x0F) << 4); apu->Update(); pRom.SwapBanks<n8k,0x2000>( buffer[0x1] ); }
NES_POKE(MAPPER18,9000) { buffer[0x2] = (buffer[0x2] & 0xF0) | ((data & 0x0F) << 0); apu->Update(); pRom.SwapBanks<n8k,0x4000>( buffer[0x2] ); }
NES_POKE(MAPPER18,9001) { buffer[0x2] = (buffer[0x2] & 0x0F) | ((data & 0x0F) << 4); apu->Update(); pRom.SwapBanks<n8k,0x4000>( buffer[0x2] ); }
NES_POKE(MAPPER18,A000) { buffer[0x3] = (buffer[0x3] & 0xF0) | ((data & 0x0F) << 0); ppu->Update(); cRom.SwapBanks<n1k,0x0000>( buffer[0x3] ); }
NES_POKE(MAPPER18,A001) { buffer[0x3] = (buffer[0x3] & 0x0F) | ((data & 0x0F) << 4); ppu->Update(); cRom.SwapBanks<n1k,0x0000>( buffer[0x3] ); }
NES_POKE(MAPPER18,A002) { buffer[0x4] = (buffer[0x4] & 0xF0) | ((data & 0x0F) << 0); ppu->Update(); cRom.SwapBanks<n1k,0x0400>( buffer[0x4] ); }
NES_POKE(MAPPER18,A003) { buffer[0x4] = (buffer[0x4] & 0x0F) | ((data & 0x0F) << 4); ppu->Update(); cRom.SwapBanks<n1k,0x0400>( buffer[0x4] ); }
NES_POKE(MAPPER18,B000) { buffer[0x5] = (buffer[0x5] & 0xF0) | ((data & 0x0F) << 0); ppu->Update(); cRom.SwapBanks<n1k,0x0800>( buffer[0x5] ); }
NES_POKE(MAPPER18,B001) { buffer[0x5] = (buffer[0x5] & 0x0F) | ((data & 0x0F) << 4); ppu->Update(); cRom.SwapBanks<n1k,0x0800>( buffer[0x5] ); }
NES_POKE(MAPPER18,B002) { buffer[0x6] = (buffer[0x6] & 0xF0) | ((data & 0x0F) << 0); ppu->Update(); cRom.SwapBanks<n1k,0x0C00>( buffer[0x6] ); }
NES_POKE(MAPPER18,B003) { buffer[0x6] = (buffer[0x6] & 0x0F) | ((data & 0x0F) << 4); ppu->Update(); cRom.SwapBanks<n1k,0x0C00>( buffer[0x6] ); }
NES_POKE(MAPPER18,C000) { buffer[0x7] = (buffer[0x7] & 0xF0) | ((data & 0x0F) << 0); ppu->Update(); cRom.SwapBanks<n1k,0x1000>( buffer[0x7] ); }
NES_POKE(MAPPER18,C001) { buffer[0x7] = (buffer[0x7] & 0x0F) | ((data & 0x0F) << 4); ppu->Update(); cRom.SwapBanks<n1k,0x1000>( buffer[0x7] ); }
NES_POKE(MAPPER18,C002) { buffer[0x8] = (buffer[0x8] & 0xF0) | ((data & 0x0F) << 0); ppu->Update(); cRom.SwapBanks<n1k,0x1400>( buffer[0x8] ); }
NES_POKE(MAPPER18,C003) { buffer[0x8] = (buffer[0x8] & 0x0F) | ((data & 0x0F) << 4); ppu->Update(); cRom.SwapBanks<n1k,0x1400>( buffer[0x8] ); }
NES_POKE(MAPPER18,D000) { buffer[0x9] = (buffer[0x9] & 0xF0) | ((data & 0x0F) << 0); ppu->Update(); cRom.SwapBanks<n1k,0x1800>( buffer[0x9] ); }
NES_POKE(MAPPER18,D001) { buffer[0x9] = (buffer[0x9] & 0x0F) | ((data & 0x0F) << 4); ppu->Update(); cRom.SwapBanks<n1k,0x1800>( buffer[0x9] ); }
NES_POKE(MAPPER18,D002) { buffer[0xA] = (buffer[0xA] & 0xF0) | ((data & 0x0F) << 0); ppu->Update(); cRom.SwapBanks<n1k,0x1C00>( buffer[0xA] ); }
NES_POKE(MAPPER18,D003) { buffer[0xA] = (buffer[0xA] & 0x0F) | ((data & 0x0F) << 4); ppu->Update(); cRom.SwapBanks<n1k,0x1C00>( buffer[0xA] ); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER18,E000) { IrqLatch = (IrqLatch & 0xFFF0) | ((data & 0xF) << 0x0); }
NES_POKE(MAPPER18,E001) { IrqLatch = (IrqLatch & 0xFF0F) | ((data & 0xF) << 0x4); }
NES_POKE(MAPPER18,E002) { IrqLatch = (IrqLatch & 0xF0FF) | ((data & 0xF) << 0x8); }
NES_POKE(MAPPER18,E003) { IrqLatch = (IrqLatch & 0x0FFF) | ((data & 0xF) << 0xC); }
NES_POKE(MAPPER18,F000) { IrqCount = IrqLatch; }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER18,F001) 
{
	cpu->ClearIRQ();
	SetIrqEnable(data & 0x1);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER18,F002) 
{
	static const UCHAR select[3][4] =
	{
		{0,0,1,1},
		{0,1,0,1},
		{0,0,0,0},
	};

	const UCHAR* const index = select[data & 0x2];

	ppu->SetMirroring
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

VOID MAPPER18::IrqSync(const UINT delta)
{
	if ((IrqCount -= delta) <= 0)
	{
		IrqCount = 0;
		SetIrqEnable(FALSE);
		cpu->DoIRQ();
	}
}

NES_NAMESPACE_END
