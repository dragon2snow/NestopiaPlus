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
#include "NstMapper057.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER57::Reset()
{
	cpu.SetPort( 0x8000, 0x8003, this, Peek_8000, Poke_8000 );
	cpu.SetPort( 0x8800,         this, Peek_8000, Poke_8800 );

	reg = 0x00;

	pRom.SwapBanks<n16k,0x0000>(0);
	pRom.SwapBanks<n16k,0x4000>(0);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER57,8000) 
{
	if (data & 0x40)
	{
		ppu.Update();

		cRom.SwapBanks<n8k,0x0000>
		( 
	       	((data & 0x03) >> 0) + 
			((reg  & 0x10) >> 1) +
			((reg  & 0x07) >> 0)
		);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER57,8800) 
{
	apu.Update();

	reg = data;

	ppu.SetMirroring( (data & 0x8) ? MIRROR_HORIZONTAL : MIRROR_VERTICAL );

	if (data & 0x80)
	{
		pRom.SwapBanks<n32k,0x0000>( ((data & 0x40) >> 6) + 2 );
	}
	else
	{
		pRom.SwapBanks<n16k,0x0000>( (data & 0x60) >> 5 );
		pRom.SwapBanks<n16k,0x4000>( (data & 0x60) >> 5 );
	}

	cRom.SwapBanks<n8k,0x0000>
	( 
		((data & 0x07) >> 0) + 
		((data & 0x10) >> 1)
	);
}

NES_NAMESPACE_END
