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
#include "NstMapper043.h"
			
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER43::Reset()
{
	EnableIrqSync(IRQSYNC_COUNT);

	cpu.SetPort( 0x8122,         this, Peek_8000, Poke_8122 );
	cpu.SetPort( 0x5000, 0x5FFF, this, Peek_5000, Poke_Nop  );
	cpu.SetPort( 0x6000, 0x7FFF, this, Peek_6000, Poke_Nop  );

	for (UINT i=0x4022; i <= 0x4FFF; i += 0x20)
   		cpu.SetPort( i, this, Peek_Nop, Poke_4022 );

    pRom.SwapBanks<n8k,0x0000>( 1 );
	pRom.SwapBanks<n8k,0x2000>( 0 );
	pRom.SwapBanks<n8k,0x4000>( 4 );
	pRom.SwapBanks<n8k,0x6000>( 9 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER43,4022) 
{
	apu.Update(); 
	static const UCHAR banks[8] = {4,3,4,4,4,7,5,6};
	pRom.SwapBanks<n8k,0x4000>(banks[data & 0x7]);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER43,8122) 
{
	cpu.ClearIRQ();
	SetIrqEnable(data & 0x2);

	if (!(data & 0x2))
		IrqCount = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER43,5000) { return *pRom.Ram( 0x00011000UL + (address - 0x5000) ); }
NES_PEEK(MAPPER43,6000) { return *pRom.Ram( 0x00004000UL + (address - 0x6000) ); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER43::IrqSync(const UINT delta)
{
	if ((IrqCount += delta) >= 0x1000)
		cpu.DoIRQ();
}

NES_NAMESPACE_END

