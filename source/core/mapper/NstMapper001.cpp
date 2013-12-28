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
#include "NstMapper001.h"

#define NES_CART_512K 0x0080000UL

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER1::Reset()
{
	if (!cRom.Size())
		EnableCartridgeCRam();

	if (pRom.Size() == NES_CART_512K)
	{
		n512k = TRUE;
	}
	else
	{
		switch (pRomCrc)
		{
     		case 0x41413B06UL: // Dragon Warrior 4
			case 0x2E91EB15UL: // -||-
			case 0xFC2B6281UL: // -||-
			case 0x030AB0B2UL: // -||-
			case 0xAB43AA55UL: // -||-
			case 0x506E259DUL: // -||-
			case 0x0794F2A5UL: // Dragon Quest 4
			case 0xB4CAFFFBUL: // -||-
			case 0xAC413EB0UL: // -||-
     		
				n512k = TRUE;
				break;

			default: 
				
				n512k = FALSE;
				break;
		}
	}

	cpu.SetPort( 0x6000, 0x7FFF, this, Peek_wRam, Poke_wRam );
	cpu.SetPort( 0x8000, 0x9FFF, this, Peek_8000, Poke_pRom );
	cpu.SetPort( 0xA000, 0xBFFF, this, Peek_A000, Poke_pRom );
	cpu.SetPort( 0xC000, 0xDFFF, this, Peek_C000, Poke_pRom );
	cpu.SetPort( 0xE000, 0xFFFF, this, Peek_E000, Poke_pRom );

	registers[0] = REG0_RESET;
	registers[1] = 0x00;
	registers[2] = 0x00;
	registers[3] = 0x00;

	banks[0] = 0;
	banks[1] = 1;
	banks[2] = n512k ? (0x20 - 2) : (pRom.NumBanks<n8k>() - 2);
	banks[3] = n512k ? (0x20 - 1) : (pRom.NumBanks<n8k>() - 1);

	latch = 0;
	count = 0;
	last  = 0;
	base  = 0;

	SetBanks();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER1::SetBanks()
{
	apu.Update(); 

	pRom.SwapBanks<n8k,0x0000>( base + banks[0] );
	pRom.SwapBanks<n8k,0x2000>( base + banks[1] );
	pRom.SwapBanks<n8k,0x4000>( base + banks[2] );
	pRom.SwapBanks<n8k,0x6000>( base + banks[3] );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER1,pRom)
{
	if (data & DATA_RESET)
	{
		registers[0] |= REG0_RESET;

		latch = 0;
		count = 0;
	}
	else
	{
		const UINT index = (address & 0x7FFF) >> 13;

		if (last != index)
		{
			last = index;
			count = 0;
			latch = 0;
		}

		latch |= (data & 0x1) << count;

		if (++count == 5)
		{
			registers[index] = latch;

			count = 0;
			latch = 0;

			switch (index)
			{
	 			case 0: 

					ProcessRegister0();
					ProcessRegister1();
					return;

	 			case 1: 
					
					ProcessRegister1(); 
					return;

	   			case 3: 
					
					ProcessRegister3(); 
					return;

	 			case 2: 
					
					ProcessRegister2(); 
					return;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER1,wRam)
{
	return (registers[3] & REG3_NO_WRAM) ? cpu.GetCache() : wRam[address - 0x6000];
}

NES_POKE(MAPPER1,wRam)
{
	if (!(registers[3] & REG3_NO_WRAM))
		wRam[address - 0x6000] = data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER1::ProcessRegister0()
{
	static const UCHAR select[4][4] =
	{
		{0,0,0,0},
		{1,1,1,1},
		{0,1,0,1},
		{0,0,1,1}
	};

	const UCHAR* const index = select[registers[0] & 0x3];

	ppu.SetMirroring
	(
	    index[0],
		index[1],
		index[2],
		index[3]
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER1::ProcessRegister1()
{
	if (n512k && IsCRam)
	{
		base = (registers[1] & 0x10) << 1;
		SetBanks();
	}
	else
	{
		ppu.Update();

		if (registers[0] & CROM_SWAP_4K) cRom.SwapBanks<n4k,0x0000>(registers[1] >> 0);
		else                             cRom.SwapBanks<n8k,0x0000>(registers[1] >> 1);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER1::ProcessRegister2()
{
	if (registers[0] & CROM_SWAP_4K)
	{
		ppu.Update();
		cRom.SwapBanks<n4k,0x1000>(registers[2]);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER1::ProcessRegister3()
{
	const UINT bank = registers[3] << 1;

	if (registers[0] & PROM_SWAP_16K) 
	{
		if (registers[0] & PROM_SWAP_LOW)
		{
			banks[0] = bank + 0;
			banks[1] = bank + 1;
		}
		else
		{
			if (!n512k)
			{
				banks[0] = 0;
				banks[1] = 1;
				banks[2] = bank + 0;
				banks[3] = bank + 1;
			}
		}
	}
	else
	{
		banks[0] = bank + 0;
		banks[1] = bank + 1;

		if (!n512k)
		{
			banks[2] = bank + 2;
			banks[3] = bank + 3;
		}
	}

	SetBanks();
}

NES_NAMESPACE_END
