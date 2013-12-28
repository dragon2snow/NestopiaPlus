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
#include "NstMapper041.h"
		 
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER41::Reset()
{
	cpu->SetPort( 0x6000, 0x67FF, this, Peek_Nop,  Poke_6000 );
	cpu->SetPort( 0x8000, 0x9FFF, this, Peek_8000, Poke_pRom );
	cpu->SetPort( 0xA000, 0xBFFF, this, Peek_A000, Poke_pRom );
	cpu->SetPort( 0xC000, 0xDFFF, this, Peek_C000, Poke_pRom );
	cpu->SetPort( 0xE000, 0xFFFF, this, Peek_E000, Poke_pRom );

	pRom.SwapBanks<n32k,0x0000>(0);

	status = 0x4;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER41,6000) 
{
	apu->Update(); 
	pRom.SwapBanks<n32k,0x0000>( address & 0x7 );
	ppu->SetMirroring( (address & 0x20) ? MIRROR_HORIZONTAL : MIRROR_VERTICAL );
	status = address & 0xFF;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER41,pRom) 
{ 
	if (status & 0x4)
	{
		ppu->Update();
		cRom.SwapBanks<n8k,0x0000>(((status & 0x18) >> 1) | (data & 0x3));
	}
}

NES_NAMESPACE_END

