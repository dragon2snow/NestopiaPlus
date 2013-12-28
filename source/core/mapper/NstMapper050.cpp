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
#include "NstMapper050.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

MAPPER50::~MAPPER50()
{
	if (cpu)
		cpu->RemoveEvent( this );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER50::Reset()
{
	cpu->RemoveEvent( this );
	cpu->SetEvent( this, Synch );

	for (UINT i=0x4020; i < 0x6000; ++i)
	{
		if ((i & 0xE060) == 0x4020)
		{
			if (i & 0x0100) cpu->SetPort( i, this, Peek_Nop, Poke_4120 );
			else            cpu->SetPort( i, this, Peek_Nop, Poke_4020 );
		}
	}

	cpu->SetPort( 0x6000, 0x7FFF, this, Peek_wRom, Poke_Nop );
	cpu->SetPort( 0x8000, 0x9FFF, this, Peek_8000, Poke_Nop );
	cpu->SetPort( 0xA000, 0xBFFF, this, Peek_A000, Poke_Nop );
	cpu->SetPort( 0xC000, 0xDFFF, this, Peek_C000, Poke_Nop );
	cpu->SetPort( 0xE000, 0xFFFF, this, Peek_E000, Poke_Nop );

	pRom.SwapBanks<n8k,0x0000>( 0x8 );
	pRom.SwapBanks<n8k,0x2000>( 0x9 );
	pRom.SwapBanks<n8k,0x4000>( 0x0 );
	pRom.SwapBanks<n8k,0x6000>( 0xB );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER50,wRom)
{
	return *pRom.Ram(0x1E000UL + address - 0x6000);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER50,4020) 
{
	apu->Update();

	pRom.SwapBanks<n8k,0x4000>
	( 
     	((data & 0x8) << 0) |
		((data & 0x1) << 2)	|
		((data & 0x6) >> 1)
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER50,4120) 
{
	SetIrqEnable( data & 0x1 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER50::Synch()
{
	ppu->Update();

	if (ppu->GetScanLine() == 20)
		cpu->TryIRQ();
}

NES_NAMESPACE_END
