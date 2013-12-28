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
#include "NstMapper183.h"
	  
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER183::Reset()
{
	EnableIrqSync(IRQSYNC_COUNT);

    cpu.SetPort( 0x8800, this, Peek_8000, Poke_8800 );
	cpu.SetPort( 0xA800, this, Peek_A000, Poke_A800 );
	cpu.SetPort( 0xA000, this, Peek_A000, Poke_A000 );
	cpu.SetPort( 0xB000, this, Peek_B000, Poke_B000 );
	cpu.SetPort( 0xB004, this, Peek_B000, Poke_B004 );
	cpu.SetPort( 0xB008, this, Peek_B000, Poke_B008 );
	cpu.SetPort( 0xB00C, this, Peek_B000, Poke_B00C );
	cpu.SetPort( 0xC000, this, Peek_C000, Poke_C000 );
	cpu.SetPort( 0xC004, this, Peek_C000, Poke_C004 );
	cpu.SetPort( 0xC008, this, Peek_C000, Poke_C008 );
	cpu.SetPort( 0xC00C, this, Peek_C000, Poke_C00C );
	cpu.SetPort( 0xD000, this, Peek_D000, Poke_D000 );
	cpu.SetPort( 0xD004, this, Peek_D000, Poke_D004 );
	cpu.SetPort( 0xD008, this, Peek_D000, Poke_D008 );
	cpu.SetPort( 0xD00C, this, Peek_D000, Poke_D00C );
	cpu.SetPort( 0xE000, this, Peek_E000, Poke_E000 );
	cpu.SetPort( 0xE004, this, Peek_E000, Poke_E004 );
	cpu.SetPort( 0xE008, this, Peek_E000, Poke_E008 );
	cpu.SetPort( 0xE00C, this, Peek_E000, Poke_E00C );
	cpu.SetPort( 0x9008, this, Peek_9000, Poke_9008 );
	cpu.SetPort( 0x9800, this, Peek_9000, Poke_9800 );
	cpu.SetPort( 0xF000, this, Peek_F000, Poke_F000 );
	cpu.SetPort( 0xF004, this, Peek_F000, Poke_F004 );
	cpu.SetPort( 0xF008, this, Peek_F000, Poke_F008 );

	PDXMemZero( banks, 8 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER183,8800) { apu.Update(); pRom.SwapBanks<n8k,0x0000>( data ); }
NES_POKE(MAPPER183,A800) { apu.Update(); pRom.SwapBanks<n8k,0x2000>( data ); }
NES_POKE(MAPPER183,A000) { apu.Update(); pRom.SwapBanks<n8k,0x4000>( data ); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER183,B000) { ppu.Update(); banks[0].l = data; cRom.SwapBanks<n1k,0x0000>( banks[0].b ); }
NES_POKE(MAPPER183,B004) { ppu.Update(); banks[0].h = data; cRom.SwapBanks<n1k,0x0000>( banks[0].b ); }
NES_POKE(MAPPER183,B008) { ppu.Update(); banks[1].l = data; cRom.SwapBanks<n1k,0x0400>( banks[1].b ); }
NES_POKE(MAPPER183,B00C) { ppu.Update(); banks[1].h = data; cRom.SwapBanks<n1k,0x0400>( banks[1].b ); }
NES_POKE(MAPPER183,C000) { ppu.Update(); banks[2].l = data; cRom.SwapBanks<n1k,0x0800>( banks[2].b ); }
NES_POKE(MAPPER183,C004) { ppu.Update(); banks[2].h = data; cRom.SwapBanks<n1k,0x0800>( banks[2].b ); }
NES_POKE(MAPPER183,C008) { ppu.Update(); banks[3].l = data; cRom.SwapBanks<n1k,0x0C00>( banks[3].b ); }
NES_POKE(MAPPER183,C00C) { ppu.Update(); banks[3].h = data; cRom.SwapBanks<n1k,0x0C00>( banks[3].b ); }
NES_POKE(MAPPER183,D000) { ppu.Update(); banks[4].l = data; cRom.SwapBanks<n1k,0x1000>( banks[4].b ); }
NES_POKE(MAPPER183,D004) { ppu.Update(); banks[4].h = data; cRom.SwapBanks<n1k,0x1000>( banks[4].b ); }
NES_POKE(MAPPER183,D008) { ppu.Update(); banks[5].l = data; cRom.SwapBanks<n1k,0x1400>( banks[5].b ); }
NES_POKE(MAPPER183,D00C) { ppu.Update(); banks[5].h = data; cRom.SwapBanks<n1k,0x1400>( banks[5].b ); }
NES_POKE(MAPPER183,E000) { ppu.Update(); banks[6].l = data; cRom.SwapBanks<n1k,0x1800>( banks[6].b ); }
NES_POKE(MAPPER183,E004) { ppu.Update(); banks[6].h = data; cRom.SwapBanks<n1k,0x1800>( banks[6].b ); }
NES_POKE(MAPPER183,E008) { ppu.Update(); banks[7].l = data; cRom.SwapBanks<n1k,0x1C00>( banks[7].b ); }
NES_POKE(MAPPER183,E00C) { ppu.Update(); banks[7].h = data; cRom.SwapBanks<n1k,0x1C00>( banks[7].b ); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER183,9008) 
{
	if (data == 1)
	{
		PDXMemZero( banks, 8 );

		apu.Update();
		ppu.Update();

		pRom.SwapBanks<n16k,0x0000>(0);
     	pRom.SwapBanks<n16k,0x4000>(pRom.NumBanks<n16k>() - 1);
		cRom.SwapBanks<n8k,0x0000>(0);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER183,9800) 
{
	if (data < 4)
	{
		static const UCHAR select[4][4] =
		{
			{0,1,0,1},
			{0,0,1,1},
			{0,0,0,0},
			{1,1,1,1}
		};

		const UCHAR* const index = select[data];
		ppu.SetMirroring( index[0], index[1], index[2], index[3] );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER183,F000) { IrqCount = (IrqCount & 0xFF00) | (data << 0); }
NES_POKE(MAPPER183,F004) { IrqCount = (IrqCount & 0x00FF) | (data << 8); }
NES_POKE(MAPPER183,F008) { SetIrqEnable(data & 0x2); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER183::IrqSync(const UINT delta)
{
	if ((IrqCount -= delta) <= 0)
	{
		IrqCount = 0;
		cpu.TryIRQ();
	}
}

NES_NAMESPACE_END
