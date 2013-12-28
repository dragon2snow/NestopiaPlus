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
#include "NstMapper044.h"
	  
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER44::Reset()
{
	status = 0;

	MAPPER4::Reset();

	cpu->SetPort( 0x6000, 0x7FFF, this, Peek_Nop, Poke_Nop );

	for (UINT i=0xA001; i <= 0xBFFF; i += 2)
		cpu->SetPort( i, this, Peek_A000, Poke_A001 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER44,A001)
{
	status = PDX_MIN(6,data & 0x7);
	UpdateCRom();
	UpdatePRom();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER44::UpdatePRom()
{
	apu->Update(); 

	const UINT bank = status << 4;
	const UINT mask1 = (status == 6) ? 0x1E : 0x0E;
	const UINT mask2 = (status == 6) ? 0x1F : 0x0F;

	if (command & SWAP_PROM_BANKS)
	{
		pRom.SwapBanks<n8k,0x0000>( bank | mask1                  );
		pRom.SwapBanks<n8k,0x2000>( bank | (pRomBanks[1] & mask2) );
		pRom.SwapBanks<n8k,0x4000>( bank | (pRomBanks[0] & mask2) );
		pRom.SwapBanks<n8k,0x6000>( bank | mask2                  );
	}
	else
	{
		pRom.SwapBanks<n8k,0x0000>( bank | (pRomBanks[0] & mask2) );
		pRom.SwapBanks<n8k,0x2000>( bank | (pRomBanks[1] & mask2) );
		pRom.SwapBanks<n8k,0x4000>( bank | mask1                  );
		pRom.SwapBanks<n8k,0x6000>( bank | mask2                  );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER44::UpdateCRom()
{
	ppu->Update();

	const UINT bank = status << 7;
	const UINT mask = (status == 6) ? 0xFF : 0x7F;

	if (command & SWAP_CROM_BANKS)
	{
		cRom.SwapBanks<n1k,0x0000>( bank | ( cRomBanks[2] & mask ) ); 
		cRom.SwapBanks<n1k,0x0400>( bank | ( cRomBanks[3] & mask ) ); 
		cRom.SwapBanks<n1k,0x0800>( bank | ( cRomBanks[4] & mask ) ); 
		cRom.SwapBanks<n1k,0x0C00>( bank | ( cRomBanks[5] & mask ) ); 
		cRom.SwapBanks<n2k,0x1000>( (bank >> 1) | ( cRomBanks[0] & (mask >> 1) ) ); 
		cRom.SwapBanks<n2k,0x1800>( (bank >> 1) | ( cRomBanks[1] & (mask >> 1) ) ); 
	}
	else
	{
		cRom.SwapBanks<n2k,0x0000>( (bank >> 1) | ( cRomBanks[0] & (mask >> 1) ) ); 
		cRom.SwapBanks<n2k,0x0800>( (bank >> 1) | ( cRomBanks[1] & (mask >> 1) ) ); 
		cRom.SwapBanks<n1k,0x1000>( bank | ( cRomBanks[2] & mask ) ); 
		cRom.SwapBanks<n1k,0x1400>( bank | ( cRomBanks[3] & mask ) ); 
		cRom.SwapBanks<n1k,0x1800>( bank | ( cRomBanks[4] & mask ) ); 
		cRom.SwapBanks<n1k,0x1C00>( bank | ( cRomBanks[5] & mask ) ); 
	}
}

NES_NAMESPACE_END

