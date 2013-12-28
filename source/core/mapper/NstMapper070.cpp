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
#include "NstMapper070.h"
			   
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER70::Reset()
{
	switch (pRomCrc)
	{
     	case 0xA59CA2EF: // Kamen Rider Kurabu (J)
     	case 0x10BB8F9A: // Family Trainer - Manhattan Police (J)

			cpu.SetPort( 0x8000, 0xFFFF, this, Peek_pRom, Poke_6000_2 );
			break;

		default:

			cpu.SetPort( 0x8000, 0xFFFF, this, Peek_pRom, Poke_6000_1 );
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER70::SetBanks(const UINT data)
{
	apu.Update();
	ppu.Update();

	pRom.SwapBanks<n16k,0x0000>( (data & 0x70) >> 4); 
	cRom.SwapBanks<n8k,0x0000> ( (data & 0x0F) >> 0);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER70,6000_1) 
{
	SetBanks(data);
	ppu.SetMirroring( (data & 0x80) ? MIRROR_ONE : MIRROR_ZERO );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER70,6000_2) 
{
	SetBanks(data);
	ppu.SetMirroring( (data & 0x80) ? MIRROR_HORIZONTAL : MIRROR_VERTICAL );
}

NES_NAMESPACE_END

