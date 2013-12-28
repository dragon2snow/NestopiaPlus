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
#include "NstMapper245.h"
	  
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER245::Reset()
{
	EnableIrqSync(IRQSYNC_PPU);

	for (ULONG i=0x8000; i <= 0xFFFF; ++i)
	{
		switch (i & 0xF7FF)
		{
    		case 0x8000: cpu.SetPort( i, this, Peek_pRom, Poke_8000 ); continue;
			case 0x8001: cpu.SetPort( i, this, Peek_pRom, Poke_8001 ); continue;
			case 0xA000: cpu.SetPort( i, this, Peek_pRom, Poke_A000 ); continue;
			case 0xC000: cpu.SetPort( i, this, Peek_pRom, Poke_C000 ); continue;
			case 0xC001: cpu.SetPort( i, this, Peek_pRom, Poke_C001 ); continue;
			case 0xE000: cpu.SetPort( i, this, Peek_pRom, Poke_E000 ); continue;
			case 0xE001: cpu.SetPort( i, this, Peek_pRom, Poke_E001 ); continue;
			default:     cpu.SetPort( i, this, Peek_pRom, Poke_Nop  ); continue;
		}
	}

	command = 0;
	latched = FALSE;
	IrqReset = 0;

	regs[0] = 0;
	regs[1] = 0;
	regs[2] = 1;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER245,8000) 
{ 
	command = data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER245,8001) 
{
	apu.Update();

	switch (command)
	{
       	case 0x0:

			regs[0] = (data & 0x2) << 5;
			pRom.SwapBanks<n8k,0x4000>( 0x3E | regs[0] );
			pRom.SwapBanks<n8k,0x6000>( 0x3F | regs[0] ); 
			break;

     	case 0x6: 
			
			regs[1] = data; 
			break;

       	case 0x7: 
			
			regs[2] = data; 
			break;
	}

	pRom.SwapBanks<n8k,0x0000>( regs[0] | regs[1] );
	pRom.SwapBanks<n8k,0x2000>( regs[0] | regs[2] );
}

NES_NAMESPACE_END
