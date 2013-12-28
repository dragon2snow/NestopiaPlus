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
#include "NstMapper004.h"
#include "NstMapper049.h"
	   
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER49::Reset()
{
	reg = 0x00;
	MAPPER4::Reset();
	cpu->SetPort( 0x6000, 0x7FFF, this, Peek_Nop, Poke_6000 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER49,6000)
{
	if (wRamEnable)
	{
		reg = data;
		UpdatePRom();
		UpdateCRom();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER49::UpdatePRom()
{
	apu->Update(); 

	if (reg & 0x1)
	{
		const UINT r = (reg & 0xC0) >> 2;

		if (command & SWAP_PROM_BANKS)
		{
			pRom.SwapBanks<n8k,0x0000>( (pRomBanks[2] & 0xF) | r );
			pRom.SwapBanks<n8k,0x2000>( (pRomBanks[1] & 0xF) | r );
			pRom.SwapBanks<n8k,0x4000>( (pRomBanks[0] & 0xF) | r );
		}
		else
		{
			pRom.SwapBanks<n8k,0x0000>( (pRomBanks[0] & 0xF) | r );
			pRom.SwapBanks<n8k,0x2000>( (pRomBanks[1] & 0xF) | r );
			pRom.SwapBanks<n8k,0x4000>( (pRomBanks[2] & 0xF) | r );
		}

		pRom.SwapBanks<n8k,0x6000>( (pRomBanks[3] & 0xF) | r );
	}
	else
	{
		pRom.SwapBanks<n32k,0x0000>( (reg >> 4) & 0x3 );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER49::UpdateCRom()
{
	ppu->Update();

	const UINT r = (reg & 0xC0) << 1;

	if (command & SWAP_CROM_BANKS)
	{
		cRom.SwapBanks<n1k,0x0000>( (cRomBanks[2] & 0x7F) | r        ); 
		cRom.SwapBanks<n1k,0x0400>( (cRomBanks[3] & 0x7F) | r        ); 
		cRom.SwapBanks<n1k,0x0800>( (cRomBanks[4] & 0x7F) | r        ); 
		cRom.SwapBanks<n1k,0x0C00>( (cRomBanks[5] & 0x7F) | r        ); 
		cRom.SwapBanks<n2k,0x1000>( (cRomBanks[0] & 0x3F) | (r >> 1) ); 
		cRom.SwapBanks<n2k,0x1800>( (cRomBanks[1] & 0x3F) | (r >> 1) ); 
	}
	else
	{
		cRom.SwapBanks<n2k,0x0000>( (cRomBanks[0] & 0x3F) | (r >> 1) ); 
		cRom.SwapBanks<n2k,0x0800>( (cRomBanks[1] & 0x3F) | (r >> 1) ); 
		cRom.SwapBanks<n1k,0x1000>( (cRomBanks[2] & 0x7F) | r        ); 
		cRom.SwapBanks<n1k,0x1400>( (cRomBanks[3] & 0x7F) | r        ); 
		cRom.SwapBanks<n1k,0x1800>( (cRomBanks[4] & 0x7F) | r        ); 
		cRom.SwapBanks<n1k,0x1C00>( (cRomBanks[5] & 0x7F) | r        ); 
	}
}

NES_NAMESPACE_END
