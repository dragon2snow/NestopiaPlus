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
#include "NstMapper022.h"
			 
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER22::Reset()
{
	cpu.SetPort( 0x8000, this, Peek_8000, Poke_8000 );
	cpu.SetPort( 0x9000, this, Peek_9000, Poke_9000 );
	cpu.SetPort( 0xA000, this, Peek_A000, Poke_A000 );
	cpu.SetPort( 0xB000, this, Peek_B000, Poke_B000 );
	cpu.SetPort( 0xB001, this, Peek_B000, Poke_B001 );
	cpu.SetPort( 0xC000, this, Peek_C000, Poke_C000 );
	cpu.SetPort( 0xC001, this, Peek_C000, Poke_C001 );
	cpu.SetPort( 0xD000, this, Peek_D000, Poke_D000 );
	cpu.SetPort( 0xD001, this, Peek_D000, Poke_D001 );
	cpu.SetPort( 0xE000, this, Peek_E000, Poke_E000 );
	cpu.SetPort( 0xE001, this, Peek_E000, Poke_E001 );

	cRom.SwapBanks<n1k,0x0000>(0);
	cRom.SwapBanks<n1k,0x0400>(0);
	cRom.SwapBanks<n1k,0x0800>(0);
	cRom.SwapBanks<n1k,0x0C00>(0);
	cRom.SwapBanks<n1k,0x1000>(0);
	cRom.SwapBanks<n1k,0x1400>(0);
	cRom.SwapBanks<n1k,0x1800>(0);
	cRom.SwapBanks<n1k,0x1C00>(0);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER22,9000) 
{
	static const UCHAR select[4][4] =
	{
		{0,1,0,1},
		{0,0,1,1},
		{1,1,1,1},
		{0,0,0,0}
	};

	const UCHAR* const index = select[data & 0x3];

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

NES_POKE(MAPPER22,8000) { apu.Update(); pRom.SwapBanks<n8k,0x0000>( data ); }
NES_POKE(MAPPER22,A000) { apu.Update(); pRom.SwapBanks<n8k,0x2000>( data ); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER22,B000) { ppu.Update(); cRom.SwapBanks<n1k,0x0000>( data >> 1 ); }
NES_POKE(MAPPER22,B001) { ppu.Update(); cRom.SwapBanks<n1k,0x0400>( data >> 1 ); }
NES_POKE(MAPPER22,C000) { ppu.Update(); cRom.SwapBanks<n1k,0x0800>( data >> 1 ); }
NES_POKE(MAPPER22,C001) { ppu.Update(); cRom.SwapBanks<n1k,0x0C00>( data >> 1 ); }
NES_POKE(MAPPER22,D000) { ppu.Update(); cRom.SwapBanks<n1k,0x1000>( data >> 1 ); }
NES_POKE(MAPPER22,D001) { ppu.Update(); cRom.SwapBanks<n1k,0x1400>( data >> 1 ); }
NES_POKE(MAPPER22,E000) { ppu.Update(); cRom.SwapBanks<n1k,0x1800>( data >> 1 ); }
NES_POKE(MAPPER22,E001) { ppu.Update(); cRom.SwapBanks<n1k,0x1C00>( data >> 1 ); }

NES_NAMESPACE_END
