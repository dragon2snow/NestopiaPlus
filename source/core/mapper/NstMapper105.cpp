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
#include "NstMapper105.h"
			  
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER105::Reset()
{
	EnableIrqSync(IRQSYNC_COUNT);

	cpu->SetPort( 0x8000, 0x9FFF, this, Peek_8000, Poke_pRom );
	cpu->SetPort( 0xA000, 0xBFFF, this, Peek_A000, Poke_pRom );
	cpu->SetPort( 0xC000, 0xDFFF, this, Peek_C000, Poke_pRom );
	cpu->SetPort( 0xE000, 0xFFFF, this, Peek_E000, Poke_pRom );

	registers[0] = REG0_RESET;
	registers[1] = 0x00;
	registers[2] = 0x00;
	registers[3] = 0x10;

	pRom.SwapBanks<n32k,0x0000>(0);

	latch = 0;
	count = 0;
	ready = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER105,pRom)
{
	const UINT index = (address & 0x7FFF) >> 13;

	if (data & DATA_RESET)
	{
		latch = 0;
		count = 0;

		if (!index)
			registers[0] |= REG0_RESET;
	}
	else
	{
		latch |= (data & 0x1) << count++;

		if (count == 5)
		{
			registers[index] = latch & 0x1F;
			latch = 0;
			count = 0;
		}
	}

	UpdateMirroring();

	if (ready < 2)
	{
		++ready;
	}
	else
	{
		UpdateBanks();
		UpdateIRQ();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER105::UpdateMirroring()
{
	static const UCHAR select[4][4] =
	{
		{0,0,0,0},
		{1,1,1,1},
		{0,1,0,1},
		{0,0,1,1}
	};

	const UCHAR* const index = select[registers[0] & 0x3];

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

VOID MAPPER105::UpdateIRQ()
{
	const BOOL state = (registers[1] & IRQ_DISABLE) ? FALSE : TRUE;

	SetIrqEnable(state);

	if (!state)
		IrqCount = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER105::UpdateBanks()
{
	apu->Update();

	if (registers[1] & 0x8)
	{
		const UINT mode = registers[0] & 0xC;

		switch (mode)
		{
     		case 0x0:
     		case 0x4:

     			pRom.SwapBanks<n32k,0x0000>( (0x8 + (registers[3] & 0x6)) >> 1 );
     			return;

			case 0x8:

				pRom.SwapBanks<n16k,0x0000>( 0x8 );
				pRom.SwapBanks<n16k,0x4000>( 0x8 + (registers[3] & 0x7) );
				return;

			case 0xC:

				pRom.SwapBanks<n16k,0x0000>( 0x8 + (registers[3] & 0x7) );
				pRom.SwapBanks<n16k,0x4000>( 0xF );
				return;
		}
	}
	else
	{	
		pRom.SwapBanks<n32k,0x0000>( (registers[1] & 0x6) >> 1 );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER105::IrqSync(const UINT delta)
{
	if (((IrqCount += delta) | 0x21FFFFFFUL) >= 0x3E000000UL)
	{
		IrqCount = 0;
		cpu->DoIRQ();
	}
}

NES_NAMESPACE_END
