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
#include "NstMapper226.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER226::Reset()
{
	cpu.SetPort( 0x8000, 0x9FFF, this, Peek_8000, Poke_pRom );
	cpu.SetPort( 0xA000, 0xBFFF, this, Peek_A000, Poke_pRom );
	cpu.SetPort( 0xC000, 0xDFFF, this, Peek_C000, Poke_pRom );
	cpu.SetPort( 0xE000, 0xFFFF, this, Peek_E000, Poke_pRom );

	pRom.SwapBanks<n32k,0x0000>(0);

	regs[0] = 0;
	regs[1] = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER226,pRom) 
{
	apu.Update();

	regs[address & 0x1] = data;

	UINT bank = 
	(
      	((regs[0] & 0x1E) >> 1) |
		((regs[0] & 0x80) >> 3) |
		((regs[1] & 0x01) << 5)
	);

	if (regs[0] & 0x20)
	{
		bank = (bank << 1) + (regs[0] & 0x1);
		pRom.SwapBanks<n16k,0x0000>( bank );
		pRom.SwapBanks<n16k,0x4000>( bank );
	}
	else
	{
		pRom.SwapBanks<n32k,0x0000>( bank );
	}

	ppu.SetMirroring( (address & 0x40) ? MIRROR_VERTICAL : MIRROR_HORIZONTAL );
}

NES_NAMESPACE_END
