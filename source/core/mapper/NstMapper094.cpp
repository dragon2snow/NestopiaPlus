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
#include "NstMapper094.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER94::Reset()
{
	cpu.SetPort( 0x8000, 0x8FFF, this, Peek_8000, Poke_pRom );
	cpu.SetPort( 0xA000, 0xBFFF, this, Peek_A000, Poke_pRom );
	cpu.SetPort( 0xC000, 0xDFFF, this, Peek_C000, Poke_pRom );
	cpu.SetPort( 0xE000, 0xFFFF, this, Peek_E000, Poke_pRom );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER94,pRom)
{
	if ((address & 0xFFF0) == 0xFF00)
	{
		apu.Update();
		pRom.SwapBanks<n16k,0x0000>( (data & 0x1C) >> 2 );
	}
}

NES_NAMESPACE_END
