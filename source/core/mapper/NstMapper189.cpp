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
#include "NstMapper189.h"
	   
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER189::Reset()
{
	MAPPER4::Reset();

	cpu.SetPort( 0x6000, 0x7FFF, this, Peek_Nop, Poke_Nop );

	for (ULONG i=0x4100; i <= 0x8FFF; ++i)
	{
		switch (i & 0xF100)
		{
   			case 0x4100: cpu.SetPort( i, this, Peek_Nop, Poke_4100 ); continue;
			case 0x6100: cpu.SetPort( i, this, Peek_Nop, Poke_6100 ); continue;
		}

		switch (i & 0xE001)
		{
   			case 0x8000: cpu.SetPort( i, this, Peek_8000, Poke_8000 ); continue;
   			case 0x8001: cpu.SetPort( i, this, Peek_8000, Poke_8001 ); continue;
		}
	}

	pRom.SwapBanks<n32k,0x0000>(0);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER189,4100)
{
	apu.Update();
	pRom.SwapBanks<n32k,0x0000>( data >> 4 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER189,6100)
{
	apu.Update();
	pRom.SwapBanks<n32k,0x0000>( data & 0x3 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER189,8000)
{
	command = data & COMMAND_INDEX;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER189,8001)
{
	ppu.Update();

	switch (command)
	{
     	case 0x0: cRom.SwapBanks<n2k,0x0000>(data >> 1); return;
		case 0x1: cRom.SwapBanks<n2k,0x0800>(data >> 1); return;
		case 0x2: cRom.SwapBanks<n1k,0x1000>(data);      return;
		case 0x3: cRom.SwapBanks<n1k,0x1400>(data);      return;
		case 0x4: cRom.SwapBanks<n1k,0x1800>(data);      return;
		case 0x5: cRom.SwapBanks<n1k,0x1C00>(data);      return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER189::IrqSync()
{
	if (IrqCount-- <= 0)
	{
		IrqCount = 0;
		cpu.DoIRQ();
	}
}

NES_NAMESPACE_END
