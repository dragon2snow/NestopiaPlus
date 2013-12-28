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
#include "NstMapper085.h"
	   
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER85::Reset()
{
	IrqState = 0;
	counter = 0;

	if (!cRom.Size())
		EnableCartridgeCRam();

	EnableIrqSync(IRQSYNC_COUNT);

	for (ULONG i=0x8000; i <= 0xFFFF; ++i)
	{
		switch (i & 0xF038)
		{
			case 0x8000: cpu.SetPort( i, this, Peek_8000, Poke_8000 ); continue;
			case 0x8008: 
			case 0x8010: cpu.SetPort( i, this, Peek_8000, Poke_8008 ); continue;
			case 0x9000: cpu.SetPort( i, this, Peek_9000, Poke_9000 ); continue;
			case 0x9010: 
			case 0x9030: cpu.SetPort( i, this, Peek_9000, Poke_9010 ); continue;
			case 0xA000: cpu.SetPort( i, this, Peek_A000, Poke_A000 ); continue;
			case 0xA008: 
			case 0xA010: cpu.SetPort( i, this, Peek_A000, Poke_A008 ); continue;
			case 0xB000: cpu.SetPort( i, this, Peek_B000, Poke_B000 ); continue;
			case 0xB008: 
			case 0xB010: cpu.SetPort( i, this, Peek_B000, Poke_B008 ); continue;
			case 0xC000: cpu.SetPort( i, this, Peek_C000, Poke_C000 ); continue;
			case 0xC008: 
			case 0xC010: cpu.SetPort( i, this, Peek_C000, Poke_C008 ); continue;
			case 0xD000: cpu.SetPort( i, this, Peek_D000, Poke_D000 ); continue;
			case 0xD008: 
			case 0xD010: cpu.SetPort( i, this, Peek_D000, Poke_D008 ); continue;
			case 0xE000: cpu.SetPort( i, this, Peek_E000, Poke_E000 ); continue;
			case 0xE008: 
			case 0xE010: cpu.SetPort( i, this, Peek_E000, Poke_E008 ); continue;
			case 0xF000: cpu.SetPort( i, this, Peek_F000, Poke_F000 ); continue;
			case 0xF008: 
			case 0xF010: cpu.SetPort( i, this, Peek_F000, Poke_F008 ); continue;
		}											   
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER85,8000) { apu.Update(); pRom.SwapBanks<n8k,0x0000>(data); }
NES_POKE(MAPPER85,8008) { apu.Update(); pRom.SwapBanks<n8k,0x2000>(data); }
NES_POKE(MAPPER85,9000) { apu.Update(); pRom.SwapBanks<n8k,0x4000>(data); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER85,9010) 
{
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER85,A000) { ppu.Update(); cRom.SwapBanks<n1k,0x0000>(data); }
NES_POKE(MAPPER85,A008) { ppu.Update(); cRom.SwapBanks<n1k,0x0400>(data); }
NES_POKE(MAPPER85,B000) { ppu.Update(); cRom.SwapBanks<n1k,0x0800>(data); }
NES_POKE(MAPPER85,B008) { ppu.Update(); cRom.SwapBanks<n1k,0x0C00>(data); }
NES_POKE(MAPPER85,C000) { ppu.Update(); cRom.SwapBanks<n1k,0x1000>(data); }
NES_POKE(MAPPER85,C008) { ppu.Update(); cRom.SwapBanks<n1k,0x1400>(data); }
NES_POKE(MAPPER85,D000) { ppu.Update(); cRom.SwapBanks<n1k,0x1800>(data); }
NES_POKE(MAPPER85,D008) { ppu.Update(); cRom.SwapBanks<n1k,0x1C00>(data); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER85,E000) 
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

NES_POKE(MAPPER85,E008) 
{ 
	IrqLatch = data; 
}

NES_POKE(MAPPER85,F000) 
{
	if ((IrqState = (data & 0x3)) & 0x2)
	{
		IrqCount = IrqLatch;
		counter = 0;
	}

	SetIrqEnable( IrqState & 0x2 );
	cpu.ClearIRQ();
}

NES_POKE(MAPPER85,F008) 
{ 
	SetIrqEnable( IrqState );
	cpu.ClearIRQ();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER85::IrqSync(const UINT delta)
{
	if ((counter += delta) >= 114)
	{
		counter -= 114;

		if (++IrqCount >= 0xFF)
		{
			IrqCount = IrqLatch;
			cpu.DoIRQ();
		}
	}
}

NES_NAMESPACE_END
