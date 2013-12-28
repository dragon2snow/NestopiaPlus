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
#include "NstMapper096.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER96::Reset()
{
	EnableCartridgeCRam( TRUE, n32k );

	latch[0] = 0;
	latch[1] = 0;

	cpu->SetPort( 0x8000, 0x9FFF, this, Peek_8000, Poke_pRom );
	cpu->SetPort( 0xA000, 0xBFFF, this, Peek_A000, Poke_pRom );
	cpu->SetPort( 0xC000, 0xDFFF, this, Peek_C000, Poke_pRom );
	cpu->SetPort( 0xE000, 0xFFFF, this, Peek_E000, Poke_pRom );

	ppu->SetPort( 0x2000, 0x23BF, this, Peek_NameRam, Poke_NameRam );
	ppu->SetPort( 0x2400, 0x27BF, this, Peek_NameRam, Poke_NameRam );
	ppu->SetPort( 0x2800, 0x2BBF, this, Peek_NameRam, Poke_NameRam );
	ppu->SetPort( 0x2C00, 0x2FBF, this, Peek_NameRam, Poke_NameRam );
	ppu->SetPort( 0x3000, 0x33BF, this, Peek_NameRam, Poke_NameRam );
	ppu->SetPort( 0x3400, 0x37BF, this, Peek_NameRam, Poke_NameRam );
	ppu->SetPort( 0x3800, 0x3BBF, this, Peek_NameRam, Poke_NameRam );
	ppu->SetPort( 0x3C00, 0x3FBF, this, Peek_NameRam, Poke_NameRam );

	pRom.SwapBanks<n32k,0x0000>( 0x0 );
	cRom.SwapBanks<n4k,0x0000>( 0x0 );
	cRom.SwapBanks<n4k,0x1000>( 0x3 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER96,NameRam)
{
	cRom.SwapBanks<n4k,0x0000>
	( 
     	(latch[0] & 0x4) | 
		(latch[1] = ((address >> 8) & 0x3))
	);

	return ppu->Peek_CiRam( address & 0x1FFF );
}

NES_POKE(MAPPER96,NameRam)
{
	ppu->Poke_CiRam( address & 0x1FFF, data );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER96,pRom)
{
	apu->Update();
	ppu->Update();

	latch[0] = data;

	pRom.SwapBanks<n32k,0x0000>( data & 0x3 );
	cRom.SwapBanks<n4k,0x0000>( (data & 0x4) | latch[1] );
	cRom.SwapBanks<n4k,0x1000>( (data & 0x4) | 0x3 );
}

NES_NAMESPACE_END
