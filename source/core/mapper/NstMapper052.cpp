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
#include "NstMapper052.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER52::Reset()
{
	regs[0] = 0x00;
	regs[1] = 0x00;

	MAPPER4::Reset();
	
	cpu.SetPort( 0x6000, 0x7FFF, this, Peek_6000, Poke_6000 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER52,6000)
{
	return wRam[address - 0x6000];
}

NES_POKE(MAPPER52,6000)
{
	if (regs[1])
	{
		wRam[address - 0x6000] = data;
	}
	else
	{
		regs[1] = 1;
		regs[0] = data;
		
		UpdatePRom();
		UpdateCRom();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER52::UpdatePRom()
{
	apu.Update(); 

	const UINT r1 = 0x1F ^ ((regs[0] & 0x8) << 1);
	const UINT r2 = ((regs[0] & 0x6) | ((regs[0] >> 3) & regs[0] & 0x1)) << 4;

	if (command & SWAP_PROM_BANKS)
	{
		pRom.SwapBanks<n8k,0x0000>( (pRomBanks[2] & r1) | r2 );
		pRom.SwapBanks<n8k,0x2000>( (pRomBanks[1] & r1) | r2 );
		pRom.SwapBanks<n8k,0x4000>( (pRomBanks[0] & r1) | r2 );
	}
	else
	{
		pRom.SwapBanks<n8k,0x0000>( (pRomBanks[0] & r1) | r2 );
		pRom.SwapBanks<n8k,0x2000>( (pRomBanks[1] & r1) | r2 );
		pRom.SwapBanks<n8k,0x4000>( (pRomBanks[2] & r1) | r2 );
	}

	pRom.SwapBanks<n8k,0x6000>( (pRomBanks[3] & r1) | r2 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER52::UpdateCRom()
{
	ppu.Update();

	const UINT r1 = 0xFF ^ ((regs[0] & 0x40) << 1);
	
	const UINT r2 =
	(
     	((regs[0] >> 3) & 0x4) |
	    ((regs[0] >> 1) & 0x2) |
	    ((regs[0] >> 6) & (regs[0] >> 4) & 0x1)
	) << 7;

	if (command & SWAP_CROM_BANKS)
	{
		cRom.SwapBanks<n1k,0x0000>( (cRomBanks[2] & r1)        | r2        ); 
		cRom.SwapBanks<n1k,0x0400>( (cRomBanks[3] & r1)        | r2        ); 
		cRom.SwapBanks<n1k,0x0800>( (cRomBanks[4] & r1)        | r2        ); 
		cRom.SwapBanks<n1k,0x0C00>( (cRomBanks[5] & r1)        | r2        ); 
		cRom.SwapBanks<n2k,0x1000>( (cRomBanks[0] & (r1 >> 1)) | (r2 >> 1) ); 
		cRom.SwapBanks<n2k,0x1800>( (cRomBanks[1] & (r1 >> 1)) | (r2 >> 1) ); 
	}
	else
	{
		cRom.SwapBanks<n2k,0x0000>( (cRomBanks[0] & (r1 >> 1)) | (r2 >> 1) ); 
		cRom.SwapBanks<n2k,0x0800>( (cRomBanks[1] & (r1 >> 1)) | (r2 >> 1) ); 
		cRom.SwapBanks<n1k,0x1000>( (cRomBanks[2] & r1)        | r2        ); 
		cRom.SwapBanks<n1k,0x1400>( (cRomBanks[3] & r1)        | r2        ); 
		cRom.SwapBanks<n1k,0x1800>( (cRomBanks[4] & r1)        | r2        ); 
		cRom.SwapBanks<n1k,0x1C00>( (cRomBanks[5] & r1)        | r2        ); 
	}
}

NES_NAMESPACE_END
