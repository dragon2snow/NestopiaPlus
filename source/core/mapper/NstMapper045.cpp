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
#include "NstMapper045.h"
	   
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER45::Reset()
{
	memset( regs, 0x00, sizeof(UINT) * 5 );
	MAPPER4::Reset();
	cpu.SetPort( 0x6000, 0x7FFF, this, Peek_Nop, Poke_6000 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER45,6000)
{
	if (!(regs[3] & 0x40))
	{
		regs[regs[4]] = data;
		regs[4] = (regs[4] + 1) & 0x3;
		
		UpdatePRom();
		UpdateCRom();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER45::UpdatePRom()
{
	apu.Update(); 

	const UINT r = (regs[3] & 0x3F) ^ 0x3F;

	if (command & SWAP_PROM_BANKS)
	{
		pRom.SwapBanks<n8k,0x0000>( (pRomBanks[2] & r) | regs[1] );
		pRom.SwapBanks<n8k,0x2000>( (pRomBanks[1] & r) | regs[1] );
		pRom.SwapBanks<n8k,0x4000>( (pRomBanks[0] & r) | regs[1] );
	}
	else
	{
		pRom.SwapBanks<n8k,0x0000>( (pRomBanks[0] & r) | regs[1] );
		pRom.SwapBanks<n8k,0x2000>( (pRomBanks[1] & r) | regs[1] );
		pRom.SwapBanks<n8k,0x4000>( (pRomBanks[2] & r) | regs[1] );
	}

	pRom.SwapBanks<n8k,0x6000>( (pRomBanks[3] & r) | regs[1] );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER45::UpdateCRom()
{
	ppu.Update();

	const UINT r1 = (regs[2] & 0x8) ? (1 << ((regs[2] & 0x7) + 1)) - 1 : 0;
	const UINT r2 = regs[0] | ((regs[2] & 0xF0) << 4);

	if (command & SWAP_CROM_BANKS)
	{
		cRom.SwapBanks<n1k,0x0000>( (cRomBanks[2] & r1) | r2 ); 
		cRom.SwapBanks<n1k,0x0400>( (cRomBanks[3] & r1) | r2 ); 
		cRom.SwapBanks<n1k,0x0800>( (cRomBanks[4] & r1) | r2 ); 
		cRom.SwapBanks<n1k,0x0C00>( (cRomBanks[5] & r1) | r2 ); 
		cRom.SwapBanks<n1k,0x1000>( (((cRomBanks[0] << 1) + 0) & r1) | r2 ); 
		cRom.SwapBanks<n1k,0x1400>( (((cRomBanks[0] << 1) + 1) & r1) | r2 ); 
		cRom.SwapBanks<n1k,0x1800>( (((cRomBanks[1] << 1) + 0) & r1) | r2 ); 
		cRom.SwapBanks<n1k,0x1C00>( (((cRomBanks[1] << 1) + 1) & r1) | r2 ); 
	}
	else
	{
		cRom.SwapBanks<n1k,0x0000>( (((cRomBanks[0] << 1) + 0) & r1) | r2 ); 
		cRom.SwapBanks<n1k,0x0400>( (((cRomBanks[0] << 1) + 1) & r1) | r2 ); 
		cRom.SwapBanks<n1k,0x0800>( (((cRomBanks[1] << 1) + 0) & r1) | r2 ); 
		cRom.SwapBanks<n1k,0x0C00>( (((cRomBanks[1] << 1) + 1) & r1) | r2 ); 
		cRom.SwapBanks<n1k,0x1000>( (cRomBanks[2] & r1) | r2 ); 
		cRom.SwapBanks<n1k,0x1400>( (cRomBanks[3] & r1) | r2 ); 
		cRom.SwapBanks<n1k,0x1800>( (cRomBanks[4] & r1) | r2 ); 
		cRom.SwapBanks<n1k,0x1C00>( (cRomBanks[5] & r1) | r2 ); 
	}
}

NES_NAMESPACE_END
