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
#include "NstMapper249.h"
	   
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER249::Reset()
{
	reg = 0;
	MAPPER4::Reset();
	cpu->SetPort( 0x5000, this, Peek_Nop, Poke_5000 );
}


////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#define NES_PROM_BANK(x)						  \
(												  \
     reg ?										  \
	 (											  \
	     (x) < 0x20 ?				     		  \
		 (										  \
		     ( ( (x) >> 0 ) & 0x01 ) |			  \
			 ( ( (x) >> 3 ) & 0x02 ) |			  \
			 ( ( (x) >> 1 ) & 0x04 ) |			  \
			 ( ( (x) << 2 ) & 0x08 ) |			  \
			 ( ( (x) << 2 ) & 0x10 )			  \
		 )										  \
		 :										  \
         (									  	  \
			 ( ( ( (x) - 0x20 ) >> 0 ) & 0x03 ) | \
			 ( ( ( (x) - 0x20 ) >> 1 ) & 0x04 ) | \
			 ( ( ( (x) - 0x20 ) >> 4 ) & 0x08 ) | \
			 ( ( ( (x) - 0x20 ) >> 2 ) & 0x10 ) | \
			 ( ( ( (x) - 0x20 ) << 3 ) & 0x20 ) | \
			 ( ( ( (x) - 0x20 ) << 2 ) & 0xC0 )	  \
		 )										  \
	 )											  \
     :											  \
	 (											  \
		 (x)									  \
	 )											  \
)

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER249::UpdatePRom()
{
	apu->Update(); 

	if (command & SWAP_PROM_BANKS)
	{
		pRom.SwapBanks<n8k,0x0000>( NES_PROM_BANK( pRomBanks[2] ) );
		pRom.SwapBanks<n8k,0x2000>( NES_PROM_BANK( pRomBanks[1] ) );
		pRom.SwapBanks<n8k,0x4000>( NES_PROM_BANK( pRomBanks[0] ) );
	}
	else
	{
		pRom.SwapBanks<n8k,0x0000>( NES_PROM_BANK( pRomBanks[0] ) );
		pRom.SwapBanks<n8k,0x2000>( NES_PROM_BANK( pRomBanks[1] ) );
		pRom.SwapBanks<n8k,0x4000>( NES_PROM_BANK( pRomBanks[2] ) );
	}

	pRom.SwapBanks<n8k,0x6000>( NES_PROM_BANK( pRomBanks[3] ) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#define NES_CROM_BANK(x)	      \
(							      \
    reg ?					      \
	(						      \
     	( ( (x) >> 0 ) & 0x03 ) | \
		( ( (x) >> 1 ) & 0x04 ) | \
		( ( (x) >> 4 ) & 0x08 ) | \
		( ( (x) >> 2 ) & 0x10 ) | \
		( ( (x) << 3 ) & 0x20 ) | \
		( ( (x) << 2 ) & 0xC0 )	  \
	)							  \
    :							  \
	(							  \
		(x)						  \
	)							  \
)

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER249::UpdateCRom()
{
	ppu->Update();

	if (command & SWAP_CROM_BANKS)
	{
		cRom.SwapBanks<n1k,0x0000>( NES_CROM_BANK( cRomBanks[2] ) ); 
		cRom.SwapBanks<n1k,0x0400>( NES_CROM_BANK( cRomBanks[3] ) ); 
		cRom.SwapBanks<n1k,0x0800>( NES_CROM_BANK( cRomBanks[4] ) ); 
		cRom.SwapBanks<n1k,0x0C00>( NES_CROM_BANK( cRomBanks[5] ) ); 
		cRom.SwapBanks<n2k,0x1000>( NES_CROM_BANK( cRomBanks[0] ) ); 
		cRom.SwapBanks<n2k,0x1800>( NES_CROM_BANK( cRomBanks[1] ) ); 
	}
	else
	{
		cRom.SwapBanks<n2k,0x0000>( NES_CROM_BANK( cRomBanks[0] ) ); 
		cRom.SwapBanks<n2k,0x0800>( NES_CROM_BANK( cRomBanks[1] ) ); 
		cRom.SwapBanks<n1k,0x1000>( NES_CROM_BANK( cRomBanks[2] ) ); 
		cRom.SwapBanks<n1k,0x1400>( NES_CROM_BANK( cRomBanks[3] ) ); 
		cRom.SwapBanks<n1k,0x1800>( NES_CROM_BANK( cRomBanks[4] ) ); 
		cRom.SwapBanks<n1k,0x1C00>( NES_CROM_BANK( cRomBanks[5] ) ); 
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER249,5000)
{
	reg = data & 0x2;
	UpdatePRom();
	UpdateCRom();
}

NES_NAMESPACE_END
