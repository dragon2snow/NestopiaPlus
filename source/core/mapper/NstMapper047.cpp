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
#include "NstMapper047.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER47::Reset()
{
	reg = 0x00;
	fix = (pRomCrc == 0x7EEF434CUL); // Super Mario Bros, Tetris, Nintendo World Cup

	MAPPER4::Reset();

	cpu.SetPort( 0x6000, 0x7FFF, this, Peek_Nop, Poke_6000 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER47,6000) 
{
	reg = fix ? (data & 0x6) >> 1 : (data & 0x1) << 1;

	UpdatePRom();
	UpdateCRom();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER47::UpdatePRom()
{
	apu.Update();

	const UINT r = reg << 3;
	const BOOL h = fix && reg != 2; 

	if (command & SWAP_PROM_BANKS)
	{
		pRom.SwapBanks<n8k,0x0000>( (h ? 0x6 : 0xE) | r );
		pRom.SwapBanks<n8k,0x2000>( pRomBanks[1]    | r );
		pRom.SwapBanks<n8k,0x4000>( pRomBanks[0]    | r );
	}
	else
	{
		pRom.SwapBanks<n8k,0x0000>( pRomBanks[0]    | r );
		pRom.SwapBanks<n8k,0x2000>( pRomBanks[1]    | r );
		pRom.SwapBanks<n8k,0x4000>( (h ? 0x6 : 0xE) | r );
	}

	pRom.SwapBanks<n8k,0x6000>( (h ? 0x7 : 0xF) | r );	
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER47::UpdateCRom()
{
	ppu.Update();

	const UINT r = (reg & 0x2) << 6;

	if (command & SWAP_CROM_BANKS)
	{
		cRom.SwapBanks<n1k,0x0000>( cRomBanks[2] + r        ); 
		cRom.SwapBanks<n1k,0x0400>( cRomBanks[3] + r        ); 
		cRom.SwapBanks<n1k,0x0800>( cRomBanks[4] + r        ); 
		cRom.SwapBanks<n1k,0x0C00>( cRomBanks[5] + r        ); 
		cRom.SwapBanks<n2k,0x1000>( cRomBanks[0] + (r >> 1) ); 
		cRom.SwapBanks<n2k,0x1800>( cRomBanks[1] + (r >> 1) ); 
	}
	else
	{
		cRom.SwapBanks<n2k,0x0000>( cRomBanks[0] + (r >> 1) ); 
		cRom.SwapBanks<n2k,0x0800>( cRomBanks[1] + (r >> 1) ); 
		cRom.SwapBanks<n1k,0x1000>( cRomBanks[2] + r        ); 
		cRom.SwapBanks<n1k,0x1400>( cRomBanks[3] + r        ); 
		cRom.SwapBanks<n1k,0x1800>( cRomBanks[4] + r        ); 
		cRom.SwapBanks<n1k,0x1C00>( cRomBanks[5] + r        ); 
	}
}

NES_NAMESPACE_END
