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
#include "NstMapper187.h"
	 
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER187::Reset()
{
	MAPPER4::Reset();

	cpu->SetPort( 0x5000, this, Peek_5000, Poke_5000 );
	cpu->SetPort( 0x5001, 0x7FFF, this, Peek_5000, Poke_5001 );

	for (ULONG i=0x8000; i <= 0xFFFFU; ++i)
	{
		switch (i & 0xE003)
		{
     		case 0x8000: cpu->SetPort( 0x8000, this, Peek_8000, Poke_8000 );
    		case 0x8001: cpu->SetPort( 0x8001, this, Peek_8000, Poke_8001 );
     		case 0x8003: cpu->SetPort( 0x8003, this, Peek_8000, Poke_8003 );
     		case 0xA000: cpu->SetPort( 0xA000, this, Peek_A000, Poke_A000 );
     		case 0xC000: cpu->SetPort( 0xC000, this, Peek_C000, Poke_C000 );
     		case 0xC001: cpu->SetPort( 0xC001, this, Peek_C000, Poke_C001 );
     		case 0xE000: cpu->SetPort( 0xE000, this, Peek_E000, Poke_E000 );
     		case 0xE002: cpu->SetPort( 0xE002, this, Peek_E000, Poke_E000 );
     		case 0xE001: cpu->SetPort( 0xE001, this, Peek_E000, Poke_E001 );
    		case 0xE003: cpu->SetPort( 0xE003, this, Peek_E000, Poke_E001 );
		}
	}

	latch = 0;

	ExBanks[0] = 0;
	ExBanks[1] = 1;

	UseExBank = FALSE;
	ExBankMode = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER187,5000)
{
	apu->Update();

	ExBankMode = data;

	if (data & SWAP_NO_EXBANK)
	{
		if (data & SWAP_32) pRom.SwapBanks<n32k,0x0000>( (data & 0x1F) >> 1 );
		else                pRom.SwapBanks<n16k,0x4000>( (data & 0x1F) >> 0 );
	}
	else
	{
		pRom.SwapBanks<n8k,0x0000>(ExBanks[0]);
		pRom.SwapBanks<n8k,0x2000>(ExBanks[1]);
		pRom.SwapBanks<n16k,0x4000>(pRom.NumBanks<n16k>() - 1);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER187,5001)
{
	latch = data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER187,5000)
{
	switch (latch)
	{
    	case 0:
		case 1:	return 0x83;
		case 2: return 0x42;
	}

	return 0x00;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER187,8000)
{
	UseExBank = FALSE;

	const UINT tmp = command;
	command = data;

	if ((tmp & SWAP_CROM_BANKS) != (data & SWAP_CROM_BANKS))
		UpdateCRom();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER187,8001)
{
	const UINT index = command & COMMAND_INDEX;

	switch (index)
	{
     	case 0x6: ExBanks[0] = data; break;
		case 0x7: ExBanks[1] = data; break;
	}

	if (UseExBank)
	{
		switch (command)
		{
     		case 0x2A: apu->Update(); pRom.SwapBanks<n8k,0x2000>(0x0F); return;
			case 0x28: apu->Update(); pRom.SwapBanks<n8k,0x4000>(0x17); return;
		}
	}
	else
	{
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
		
				cRomBanks[index] = data;
				UpdateCRom(); 
				return;

			case 0x6: 
			case 0x7: 

				if ((ExBankMode & 0xA0) != 0xA0)
				{
					pRomBanks[index - 0x6] = data;
					UpdatePRom();
				}
				return;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER187,8003)
{
	UseExBank = TRUE;

	if (!(data & 0xF0))
	{
		apu->Update();
		pRom.SwapBanks<n8k,0x4000>(pRom.NumBanks<n8k>() - 2);
	}
}

NES_NAMESPACE_END
