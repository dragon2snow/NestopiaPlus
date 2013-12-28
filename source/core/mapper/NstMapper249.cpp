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
	MAPPER4::Reset();

	for (ULONG i=0x8000; i <= 0xFFFF; ++i)
	{
		switch (i & 0xFF01) 
		{
     		case 0x8000: case 0x8800: cpu.SetPort( i, this, Peek_8000, Poke_8000 ); continue;
			case 0x8001: case 0x8801: cpu.SetPort( i, this, Peek_8000, Poke_8001 ); continue;
			case 0xA000: case 0xA800: cpu.SetPort( i, this, Peek_A000, Poke_A000 ); continue;
			case 0xA001: case 0xA801: cpu.SetPort( i, this, Peek_A000, Poke_A001 ); continue;
			case 0xC000: case 0xC800: cpu.SetPort( i, this, Peek_C000, Poke_C000 ); continue;
			case 0xC001: case 0xC801: cpu.SetPort( i, this, Peek_C000, Poke_C001 ); continue;
			case 0xE000: case 0xE800: cpu.SetPort( i, this, Peek_E000, Poke_E000 ); continue;
			case 0xE001: case 0xE801: cpu.SetPort( i, this, Peek_E000, Poke_E001 ); continue;
		}
	}

	cpu.SetPort( 0x5000, this, Peek_Nop, Poke_5000 );

	reg = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER249,5000)
{
	reg = data & 0x2;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER249,8000)
{
	command = data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

UINT MAPPER249::Unscramble1(const UINT data)
{
	return
	(
		( ( ( data & 0x01 ) >> 0 ) << 0 ) |
		( ( ( data & 0x02 ) >> 1 ) << 1 ) |
		( ( ( data & 0x04 ) >> 2 ) << 5 ) |
		( ( ( data & 0x08 ) >> 3 ) << 2 ) |
		( ( ( data & 0x10 ) >> 4 ) << 6 ) |
		( ( ( data & 0x20 ) >> 5 ) << 7 ) |
		( ( ( data & 0x40 ) >> 6 ) << 4 ) |
		( ( ( data & 0x80 ) >> 7 ) << 3 )
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

UINT MAPPER249::Unscramble2(const UINT data)
{
	return
	(
		( ( ( data & 0x01 ) >> 0 ) << 0 ) |
		( ( ( data & 0x02 ) >> 1 ) << 3 ) |
		( ( ( data & 0x04 ) >> 2 ) << 4 ) |
		( ( ( data & 0x08 ) >> 3 ) << 2 ) |
		( ( ( data & 0x10 ) >> 4 ) << 1 )
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER249,8001)
{
	UINT bank = data;
	UINT index = command & COMMAND_INDEX;

	switch (index) 
	{
     	case 0x0:
		case 0x1:

			ppu.Update();

			if (reg) 
				bank = Unscramble1( bank );

			index <<= 11;
			cRom.SwapBanks<n1k>( 0x0000 + index, bank & 0xFE );
			cRom.SwapBanks<n1k>( 0x0400 + index, bank | 0x01 );
			break;

   		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:

			ppu.Update();

			if (reg) 
				bank = Unscramble1( bank );

			cRom.SwapBanks<n1k>( 0x1000 + ((index - 0x2) << 10), bank );
			break;	 

		case 0x6:
		case 0x7:

			apu.Update();

			if (reg)
			{
				if (bank < 0x20)
					bank = Unscramble2( bank );
				else
					bank = Unscramble1( bank - 0x20 );
			}

			pRom.SwapBanks<n8k>( (index - 0x6) << 13, bank );
			break;
	}
}

NES_NAMESPACE_END
