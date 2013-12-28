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
#include "NstMapper089.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER89::Reset()
{
	for (ULONG i=0x8000; i <= 0xFFFFU; ++i)
	{
		if ((i & 0xFF00) == 0xC000)
			cpu.SetPort( i, this, Peek_C000, Poke_pRom );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER89,pRom)
{
	apu.Update();

	ppu.SetMirroring( (data & 0x8) ? MIRROR_ONE : MIRROR_ZERO );

	pRom.SwapBanks<n16k,0x0000>( (data & 0x70) >> 4 );
	cRom.SwapBanks<n8k,0x0000>( ((data & 0x80) >> 4) | (data & 0x7) );
}

NES_NAMESPACE_END
