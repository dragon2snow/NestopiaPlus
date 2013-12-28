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

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER4::Reset()
{
	if (!cRom.Size())
		EnableCartridgeCRam();

	EnableIrqSync(IRQSYNC_PPU);

	cpu->SetPort( 0x6000, 0x7FFF, this, Peek_wRam, Poke_wRam );

	for (ULONG i=0x8000; i <= 0xFFFFU; ++i)
	{
		switch (i & 0xE001)
		{
    	   	case 0x8000: cpu->SetPort( i, this, Peek_8000, Poke_8000 ); continue;
     		case 0x8001: cpu->SetPort( i, this, Peek_8000, Poke_8001 ); continue;
     		case 0xA000: cpu->SetPort( i, this, Peek_A000, Poke_A000 ); continue;
			case 0xA001: cpu->SetPort( i, this, Peek_A000, Poke_A001 ); continue;
     		case 0xC000: cpu->SetPort( i, this, Peek_C000, Poke_C000 ); continue;
     		case 0xC001: cpu->SetPort( i, this, Peek_C000, Poke_C001 ); continue;
     		case 0xE000: cpu->SetPort( i, this, Peek_E000, Poke_E000 ); continue;
    		case 0xE001: cpu->SetPort( i, this, Peek_E000, Poke_E001 ); continue;
		}
	}

	command = 0;
	wRamEnable = 0;

	pRomBanks[0] = 0;
	pRomBanks[1] = 1;
	pRomBanks[2] = pRom.NumBanks<n8k>() - 2;
	pRomBanks[3] = pRom.NumBanks<n8k>() - 1;

	cRomBanks[0] = 0;
	cRomBanks[1] = 2;
	cRomBanks[2] = 4;
	cRomBanks[3] = 5;
	cRomBanks[4] = 6;
	cRomBanks[5] = 7;

	latched = FALSE;
	IrqReset = 0;

	ppu->SetMirroring(0,1,2,3);

	UpdatePRom();
	UpdateCRom();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER4,8000)
{
	const UINT tmp = command;
	command = data;

	if ((tmp & SWAP_PROM_BANKS) != (data & SWAP_PROM_BANKS)) UpdatePRom();
	if ((tmp & SWAP_CROM_BANKS) != (data & SWAP_CROM_BANKS)) UpdateCRom();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER4,8001)
{
	const UINT index = command & COMMAND_INDEX;

	switch (index)
	{
     	case 0x0:	
     	case 0x1:	

			cRomBanks[index] = data >> 1;
			UpdateCRom();
     		return;

		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:

			cRomBanks[index] = data >> 0;
			UpdateCRom();
			return;

		case 0x6: 
		case 0x7:

			pRomBanks[index - 0x6] = data;
			UpdatePRom();
			return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER4,A000)
{
	if (mirroring != MIRROR_FOURSCREEN)
		ppu->SetMirroring((data & 0x1) ? MIRROR_HORIZONTAL : MIRROR_VERTICAL);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER4,A001)
{
	wRamEnable = data & WRAM_ENABLE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER4,C000) 
{ 
	IrqLatch = data;
	latched = IrqReset ^ 1;

	if (IrqReset)
		IrqCount = data;
}						 

NES_POKE(MAPPER4,C001) 
{
	IrqCount = IrqLatch;                    
}

NES_POKE(MAPPER4,E000) 
{ 
	SetIrqEnable(FALSE);   
	IrqReset = 1;
}

NES_POKE(MAPPER4,E001) 
{ 
	SetIrqEnable(TRUE); 
	cpu->ClearIRQ(); 

	if (latched)
		IrqCount = IrqLatch;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER4,wRam)
{
	if (wRamEnable)
		wRam[address - 0x6000] = data;
}

NES_PEEK(MAPPER4,wRam)
{
	return wRamEnable ? wRam[address - 0x6000] : cpu->GetCache();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER4::UpdatePRom()
{
	apu->Update(); 

	if (command & SWAP_PROM_BANKS)
	{
		pRom.SwapBanks<n8k,0x0000>( pRomBanks[2] );
		pRom.SwapBanks<n8k,0x2000>( pRomBanks[1] );
		pRom.SwapBanks<n8k,0x4000>( pRomBanks[0] );
	}
	else
	{
		pRom.SwapBanks<n8k,0x0000>( pRomBanks[0] );
		pRom.SwapBanks<n8k,0x2000>( pRomBanks[1] );
		pRom.SwapBanks<n8k,0x4000>( pRomBanks[2] );
	}

	pRom.SwapBanks<n8k,0x6000>( pRomBanks[3] );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER4::UpdateCRom()
{
	ppu->Update();

	if (command & SWAP_CROM_BANKS)
	{
		cRom.SwapBanks<n1k,0x0000>( cRomBanks[2] ); 
		cRom.SwapBanks<n1k,0x0400>( cRomBanks[3] ); 
		cRom.SwapBanks<n1k,0x0800>( cRomBanks[4] ); 
		cRom.SwapBanks<n1k,0x0C00>( cRomBanks[5] ); 
		cRom.SwapBanks<n2k,0x1000>( cRomBanks[0] ); 
		cRom.SwapBanks<n2k,0x1800>( cRomBanks[1] ); 
	}
	else
	{
		cRom.SwapBanks<n2k,0x0000>( cRomBanks[0] ); 
		cRom.SwapBanks<n2k,0x0800>( cRomBanks[1] ); 
		cRom.SwapBanks<n1k,0x1000>( cRomBanks[2] ); 
		cRom.SwapBanks<n1k,0x1400>( cRomBanks[3] ); 
		cRom.SwapBanks<n1k,0x1800>( cRomBanks[4] ); 
		cRom.SwapBanks<n1k,0x1C00>( cRomBanks[5] ); 
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER4::IrqSync()
{
	IrqReset = 0;

	if (IrqCount-- <= 0)
	{
		IrqCount = IrqLatch;
		cpu->DoIRQ();
	}
}

NES_NAMESPACE_END

