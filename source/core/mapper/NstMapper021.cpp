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
#include "NstMapper021.h"
			   
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER21::Reset()
{
	for (ULONG i=0x8000; i <= 0xFFFF; ++i)
	{
		switch (i & 0xF0CF)
		{
     		case 0x8000: cpu.SetPort( i, this, Peek_8000, Poke_8000 ); continue;
			case 0x9000: cpu.SetPort( i, this, Peek_9000, Poke_9000 ); continue;
			case 0xA000: cpu.SetPort( i, this, Peek_A000, Poke_A000 ); continue;
			case 0xB000: cpu.SetPort( i, this, Peek_B000, Poke_B000 ); continue;
			case 0xC000: cpu.SetPort( i, this, Peek_C000, Poke_C000 ); continue;
			case 0xD000: cpu.SetPort( i, this, Peek_D000, Poke_D000 ); continue;
			case 0xE000: cpu.SetPort( i, this, Peek_E000, Poke_E000 ); continue;
			case 0xF000: cpu.SetPort( i, this, Peek_F000, Poke_F000 ); continue;

			case 0x9002: case 0x9080: cpu.SetPort( i, this, Peek_9000, Poke_9002 ); continue;
			case 0xB002: case 0xB040: cpu.SetPort( i, this, Peek_B000, Poke_B002 ); continue;
			case 0xC002: case 0xC040: cpu.SetPort( i, this, Peek_C000, Poke_C002 ); continue;
			case 0xD002: case 0xD040: cpu.SetPort( i, this, Peek_D000, Poke_D002 ); continue;
			case 0xE002: case 0xE040: cpu.SetPort( i, this, Peek_E000, Poke_E002 ); continue;
			case 0xF002: case 0xF040: cpu.SetPort( i, this, Peek_F000, Poke_F002 ); continue;
			case 0xF003: case 0xF0C0: cpu.SetPort( i, this, Peek_F000, Poke_F003 ); continue;
			case 0xF004: case 0xF080: cpu.SetPort( i, this, Peek_F000, Poke_F004 ); continue;

			case 0xB001: case 0xB004: case 0xB080: cpu.SetPort( i, this, Peek_B000, Poke_B004 ); continue;
			case 0xB003: case 0xB006: case 0xB0C0: cpu.SetPort( i, this, Peek_B000, Poke_B006 ); continue;
			case 0xC001: case 0xC004: case 0xC080: cpu.SetPort( i, this, Peek_C000, Poke_C004 ); continue;
			case 0xC003: case 0xC006: case 0xC0C0: cpu.SetPort( i, this, Peek_C000, Poke_C006 ); continue;
			case 0xD001: case 0xD004: case 0xD080: cpu.SetPort( i, this, Peek_D000, Poke_D004 ); continue;
			case 0xD003: case 0xD006: case 0xD0C0: cpu.SetPort( i, this, Peek_D000, Poke_D006 ); continue;
			case 0xE001: case 0xE004: case 0xE080: cpu.SetPort( i, this, Peek_E000, Poke_E004 ); continue;
			case 0xE003: case 0xE006: case 0xE0C0: cpu.SetPort( i, this, Peek_E000, Poke_E006 ); continue;
		}
	}

	EnableIrqSync(IRQSYNC_COUNT);

	pRomSelect = 0x0000;

	cRomSelect[0].b = 0;
	cRomSelect[1].b = 1;
	cRomSelect[2].b = 2;
	cRomSelect[3].b = 3;
	cRomSelect[4].b = 4;
	cRomSelect[5].b = 5;
	cRomSelect[6].b = 6;
	cRomSelect[7].b = 7;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER21,8000) 
{ 
	apu.Update(); 
	pRom.SwapBanks<n8k>( pRomSelect, data );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER21,9000) 
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

NES_POKE(MAPPER21,9002)
{
	pRomSelect = (data & 0x2) ? 0x4000 : 0x0000;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER21,A000) 
{ 
	apu.Update(); 
	pRom.SwapBanks<n8k,0x2000>( data ); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER21,B000) { ppu.Update(); cRomSelect[0].l = data; cRom.SwapBanks<n1k,0x0000>( cRomSelect[0].b ); }
NES_POKE(MAPPER21,B002) { ppu.Update(); cRomSelect[0].h = data; cRom.SwapBanks<n1k,0x0000>( cRomSelect[0].b ); }
NES_POKE(MAPPER21,B004) { ppu.Update(); cRomSelect[1].l = data; cRom.SwapBanks<n1k,0x0400>( cRomSelect[1].b ); }
NES_POKE(MAPPER21,B006) { ppu.Update(); cRomSelect[1].h = data; cRom.SwapBanks<n1k,0x0400>( cRomSelect[1].b ); }
NES_POKE(MAPPER21,C000) { ppu.Update(); cRomSelect[2].l = data; cRom.SwapBanks<n1k,0x0800>( cRomSelect[2].b ); }
NES_POKE(MAPPER21,C002) { ppu.Update(); cRomSelect[2].h = data; cRom.SwapBanks<n1k,0x0800>( cRomSelect[2].b ); }
NES_POKE(MAPPER21,C004) { ppu.Update(); cRomSelect[3].l = data; cRom.SwapBanks<n1k,0x0C00>( cRomSelect[3].b ); }
NES_POKE(MAPPER21,C006) { ppu.Update(); cRomSelect[3].h = data; cRom.SwapBanks<n1k,0x0C00>( cRomSelect[3].b ); }
NES_POKE(MAPPER21,D000) { ppu.Update(); cRomSelect[4].l = data; cRom.SwapBanks<n1k,0x1000>( cRomSelect[4].b ); }
NES_POKE(MAPPER21,D002) { ppu.Update(); cRomSelect[4].h = data; cRom.SwapBanks<n1k,0x1000>( cRomSelect[4].b ); }
NES_POKE(MAPPER21,D004) { ppu.Update(); cRomSelect[5].l = data; cRom.SwapBanks<n1k,0x1400>( cRomSelect[5].b ); }
NES_POKE(MAPPER21,D006) { ppu.Update(); cRomSelect[5].h = data; cRom.SwapBanks<n1k,0x1400>( cRomSelect[5].b ); }
NES_POKE(MAPPER21,E000) { ppu.Update(); cRomSelect[6].l = data; cRom.SwapBanks<n1k,0x1800>( cRomSelect[6].b ); }
NES_POKE(MAPPER21,E002) { ppu.Update(); cRomSelect[6].h = data; cRom.SwapBanks<n1k,0x1800>( cRomSelect[6].b ); }
NES_POKE(MAPPER21,E004) { ppu.Update(); cRomSelect[7].l = data; cRom.SwapBanks<n1k,0x1C00>( cRomSelect[7].b ); }
NES_POKE(MAPPER21,E006) { ppu.Update(); cRomSelect[7].h = data; cRom.SwapBanks<n1k,0x1C00>( cRomSelect[7].b ); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER21,F000) { IrqLatch = (IrqLatch & 0xF0) | ((data & 0x0F) << 0); }
NES_POKE(MAPPER21,F002) { IrqLatch = (IrqLatch & 0x0F) | ((data & 0x0F) << 4); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER21,F003)
{
	SetIrqEnable(IrqTmp);
	cpu.ClearIRQ();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER21,F004)
{
	IrqCount = (0x100 - IrqLatch) * 114;
	IrqTmp = data & 0x1;
	SetIrqEnable(data & 0x2);
	cpu.ClearIRQ();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER21::IrqSync(const UINT delta)
{
	if ((IrqCount -= delta) <= 0)
	{
		IrqCount = (0x100 - IrqLatch) * 114;
		cpu.DoIRQ();
	}
}

NES_NAMESPACE_END

