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
#include "NstMapper067.h"
		  
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER67::Reset()
{
	EnableIrqSync(IRQSYNC_COUNT);

	for (ULONG i=0x8000; i <= 0xFFFFU; ++i)
	{
		switch (i & 0xF800)
		{
     		case 0x8800: cpu->SetPort( i, this, Peek_8000, Poke_8800 ); continue;
     		case 0x9800: cpu->SetPort( i, this, Peek_9000, Poke_9800 ); continue;
       		case 0xA800: cpu->SetPort( i, this, Peek_A000, Poke_A800 ); continue;
     		case 0xB800: cpu->SetPort( i, this, Peek_B000, Poke_B800 ); continue;
			case 0xC000:							
     		case 0xC800: cpu->SetPort( i, this, Peek_C000, Poke_C800 ); continue;
     		case 0xD800: cpu->SetPort( i, this, Peek_D000, Poke_D800 ); continue;
       		case 0xE800: cpu->SetPort( i, this, Peek_E000, Poke_E800 ); continue;
     		case 0xF800: cpu->SetPort( i, this, Peek_F000, Poke_F800 ); continue;
		}
	}

	cRom.SwapBanks<n1k,0x1000>( cRom.NumBanks<n8k>() - 4 );
	cRom.SwapBanks<n1k,0x1400>( cRom.NumBanks<n8k>() - 3 );
	cRom.SwapBanks<n1k,0x1800>( cRom.NumBanks<n8k>() - 2 );
	cRom.SwapBanks<n1k,0x1C00>( cRom.NumBanks<n8k>() - 1 );

	FlipFlop = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER67,8800) { ppu->Update(); cRom.SwapBanks<n2k,0x0000>(data); }
NES_POKE(MAPPER67,9800) { ppu->Update(); cRom.SwapBanks<n2k,0x0800>(data); }
NES_POKE(MAPPER67,A800) { ppu->Update(); cRom.SwapBanks<n2k,0x1000>(data); }
NES_POKE(MAPPER67,B800) { ppu->Update(); cRom.SwapBanks<n2k,0x1800>(data); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER67,C800)
{
	if (FlipFlop ^= 1) IrqCount = (IrqCount & 0x00FF) | (data << 8);
	else               IrqCount = (IrqCount & 0xFF00) | (data << 0);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER67,D800) 
{
	FlipFlop = 0;
	SetIrqEnable(data & 0x10);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER67,E800) 
{  
	static const UCHAR select[4][4] =
	{
		{0,1,0,1},
		{0,0,1,1},
		{0,0,0,0},
		{1,1,1,1}
	};

	const UCHAR* const index = select[data & 0x3];

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

NES_POKE(MAPPER67,F800) 
{
	apu->Update(); 
	pRom.SwapBanks<n16k,0x0000>(data);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER67::IrqSync(const UINT delta)
{
	IrqCount -= delta;

	if (IrqCount <= 0)
	{
		SetIrqEnable(FALSE);
		IrqCount = 0xFFFF;
		cpu->TryIRQ();
	}
}

NES_NAMESPACE_END

