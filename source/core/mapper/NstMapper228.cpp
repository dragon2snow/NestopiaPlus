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
#include "NstMapper228.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER228::Reset()
{
	cpu->SetPort( 0x8000, 0x9FFF, this, Peek_8000, Poke_pRom );
	cpu->SetPort( 0xA000, 0xBFFF, this, Peek_A000, Poke_pRom );
	cpu->SetPort( 0xC000, 0xDFFF, this, Peek_C000, Poke_pRom );
	cpu->SetPort( 0xE000, 0xFFFF, this, Peek_E000, Poke_pRom );

	pRom.SwapBanks<n32k,0x0000>(0);
	cRom.SwapBanks<n8k,0x0000>(0);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER228,pRom) 
{
	apu->Update();

	UINT pBank = ((address & 0x0780) >> 7);

	switch ((address & 0x1800) >> 11)
	{
       	case 1: pBank |= 0x10; break;
		case 3:	pBank |= 0x20; break;
	}

	if (address & 0x20)
	{
		pBank <<= 1;

		if (address & 0x40)
			++pBank;

		pRom.SwapBanks<n16k,0x0000>( pBank << 1 );
		pRom.SwapBanks<n16k,0x4000>( pBank << 1 );
	}
	else
	{
		pRom.SwapBanks<n32k,0x0000>( pBank );
	}

	ppu->SetMirroring( (address & 0x2000) ? MIRROR_HORIZONTAL : MIRROR_VERTICAL );

	cRom.SwapBanks<n8k,0x0000>( ((address & 0xF) << 2) | (data & 0x3) );
}

NES_NAMESPACE_END
