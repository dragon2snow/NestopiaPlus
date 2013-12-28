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
#include "NstMapper083.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER83::Reset()
{
	regs[0] = 0;
	regs[1] = 0;
	regs[2] = 0;

	EnableIrqSync(IRQSYNC_COUNT);

	for (UINT i=0x5100; i <= 0x5FFF; ++i)
	{
		if ((i & 0x5100) == 0x5100)
			cpu->SetPort( 0x5101, this, Peek_5100, Poke_Nop );
	}

	cpu->SetPort( 0x5101, this, Peek_Nop,  Poke_5101 );
	cpu->SetPort( 0x5102, this, Peek_Nop,  Poke_5101 );
	cpu->SetPort( 0x5103, this, Peek_Nop,  Poke_5101 );
	cpu->SetPort( 0x8000, this, Peek_8000, Poke_8000 );
	cpu->SetPort( 0x8100, this, Peek_8000, Poke_8100 );
	cpu->SetPort( 0x8200, this, Peek_8000, Poke_8200 );
	cpu->SetPort( 0x8201, this, Peek_8000, Poke_8201 );
	cpu->SetPort( 0x8300, this, Peek_8000, Poke_8300 );
	cpu->SetPort( 0x8301, this, Peek_8000, Poke_8301 );
	cpu->SetPort( 0x8302, this, Peek_8000, Poke_8302 );
	cpu->SetPort( 0x8310, this, Peek_8000, Poke_8310 );
	cpu->SetPort( 0x8311, this, Peek_8000, Poke_8311 );
	cpu->SetPort( 0x8312, this, Peek_8000, Poke_8312 );
	cpu->SetPort( 0x8313, this, Peek_8000, Poke_8313 );
	cpu->SetPort( 0x8314, this, Peek_8000, Poke_8314 );
	cpu->SetPort( 0x8315, this, Peek_8000, Poke_8315 );
	cpu->SetPort( 0x8316, this, Peek_8000, Poke_8316 );
	cpu->SetPort( 0x8317, this, Peek_8000, Poke_8317 );
	cpu->SetPort( 0x8318, this, Peek_8000, Poke_8318 );
	cpu->SetPort( 0xB000, this, Peek_B000, Poke_8000 );
	cpu->SetPort( 0xB0FF, this, Peek_B000, Poke_8000 );
	cpu->SetPort( 0xB1FF, this, Peek_B000, Poke_8000 );

	if (pRom.NumBanks<n8k>() >= 32)
	{
		regs[1] = 0x30;
		pRom.SwapBanks<n16k,0x0000>(0x0);
		pRom.SwapBanks<n16k,0x4000>(0xF);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER83,5100) { return regs[2]; }
NES_POKE(MAPPER83,5101) { regs[2] = data; }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER83,8000) 
{
	apu->Update();

	regs[0] = data;
	pRom.SwapBanks<n16k,0x0000>((data & 0x3F) | 0x0);
	pRom.SwapBanks<n16k,0x4000>((data & 0x3F) | 0xF);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER83,8100) 
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

NES_POKE(MAPPER83,8200) 
{
	IrqCount = (IrqCount & 0xFF00) | (data << 0);
}

NES_POKE(MAPPER83,8201) 
{
	IrqCount = (IrqCount & 0x00FF) | (data << 8);
	SetIrqEnable(data);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER83,8300) { apu->Update(); pRom.SwapBanks<n8k,0x0000>(data); }
NES_POKE(MAPPER83,8301) { apu->Update(); pRom.SwapBanks<n8k,0x4000>(data); }
NES_POKE(MAPPER83,8302) { apu->Update(); pRom.SwapBanks<n8k,0x6000>(data); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER83,8310) { ppu->Update(); cRom.SwapBanks<n1k,0x0000>( ((regs[0] & 0x30) << 4) | data ); }
NES_POKE(MAPPER83,8311) { ppu->Update(); cRom.SwapBanks<n1k,0x0400>( ((regs[0] & 0x30) << 4) | data ); }
NES_POKE(MAPPER83,8312) { ppu->Update(); cRom.SwapBanks<n1k,0x0800>( ((regs[0] & 0x30) << 4) | data ); }
NES_POKE(MAPPER83,8313) { ppu->Update(); cRom.SwapBanks<n1k,0x0C00>( ((regs[0] & 0x30) << 4) | data ); }
NES_POKE(MAPPER83,8314) { ppu->Update(); cRom.SwapBanks<n1k,0x1000>( ((regs[0] & 0x30) << 4) | data ); }
NES_POKE(MAPPER83,8315) { ppu->Update(); cRom.SwapBanks<n1k,0x1400>( ((regs[0] & 0x30) << 4) | data ); }
NES_POKE(MAPPER83,8316) { ppu->Update(); cRom.SwapBanks<n1k,0x1800>( ((regs[0] & 0x30) << 4) | data ); }
NES_POKE(MAPPER83,8317) { ppu->Update(); cRom.SwapBanks<n1k,0x1C00>( ((regs[0] & 0x30) << 4) | data ); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER83,8318) 
{
	apu->Update();
	pRom.SwapBanks<n16k,0x0000>( (regs[0] & 0x30) | data );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER83::IrqSync(const UINT delta)
{
	if (IrqCount <= 113)
	{
		cpu->TryIRQ();
		SetIrqEnable( FALSE );
	}
	else
	{
		IrqCount -= delta;
	}
}

NES_NAMESPACE_END
