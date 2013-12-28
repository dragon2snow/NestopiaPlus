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
#include "NstMapper255.h"
		 
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER255::Reset()
{
	cpu->SetPort( 0x5800, 0x5FFF, this, Peek_5800, Poke_5800 );
	cpu->SetPort( 0x8000, 0x9FFF, this, Peek_8000, Poke_pRom );
	cpu->SetPort( 0xA000, 0xBFFF, this, Peek_A000, Poke_pRom );
	cpu->SetPort( 0xC000, 0xDFFF, this, Peek_C000, Poke_pRom );
	cpu->SetPort( 0xE000, 0xFFFF, this, Peek_E000, Poke_pRom );

	pRom.SwapBanks<n32k,0x0000>(0);
	ppu->SetMirroring(MIRROR_VERTICAL);

	regs[0] = 
	regs[1] = 
	regs[2] = 
	regs[3] = 0xF;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER255,pRom) 
{ 
	apu->Update();
	ppu->Update();

	const UINT rBank = (address >> 14) & 0x01;
	const UINT pBank = ((address >> 7) & 0x1F) | (rBank << 5);
	const UINT cBank = ((address >> 0) & 0x3F) | (rBank << 6);

	if (address & 0x1000)
	{
		pRom.SwapBanks<n16k,0x0000>( (pBank << 1) | ((address & 0x40) >> 6) );
		pRom.SwapBanks<n16k,0x4000>( (pBank << 1) | ((address & 0x40) >> 6) );	  
	}
	else
	{
		pRom.SwapBanks<n32k,0x0000>(pBank);
	}

	ppu->SetMirroring( (address & 0x2000) ? MIRROR_HORIZONTAL : MIRROR_VERTICAL );
	cRom.SwapBanks<n8k,0x0000>(cBank); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER255,5800)
{
	regs[address & 0x3] = data & 0xF;
}

NES_PEEK(MAPPER255,5800)
{
	return regs[address & 0x3];
}

NES_NAMESPACE_END
