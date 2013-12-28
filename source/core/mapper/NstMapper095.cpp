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
#include "NstMapper095.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER95::Reset()
{
	command = 0;

	for (ULONG i=0x8000; i <= 0x8FFF; ++i)
	{
		switch (i & 0xF001)
		{
       		case 0x8000: cpu->SetPort( i, this, Peek_8000, Poke_8000 ); continue;
			case 0x8001: cpu->SetPort( i, this, Peek_8000, Poke_8001 ); continue;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER95,8000)
{
	command = data & 0x7;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER95,8001)
{
	apu->Update();
	ppu->Update();

	UINT bank = data;

	if (command < 6)
	{
		ppu->SetMirroring( (data & 0x20) ? MIRROR_ONE : MIRROR_ZERO );
		bank &= 0x1F;
	}

	switch (command)
	{
       	case 0x0: cRom.SwapBanks<n2k,0x0000>(bank >> 1); return;
		case 0x1: cRom.SwapBanks<n2k,0x0800>(bank >> 1); return;
       	case 0x2: cRom.SwapBanks<n1k,0x1000>(bank);      return;
       	case 0x3: cRom.SwapBanks<n1k,0x1400>(bank);      return;
       	case 0x4: cRom.SwapBanks<n1k,0x1800>(bank);      return;
       	case 0x5: cRom.SwapBanks<n1k,0x1C00>(bank);      return;
       	case 0x6: pRom.SwapBanks<n8k,0x0000>(bank);      return;
       	case 0x7: pRom.SwapBanks<n8k,0x2000>(bank);      return;
	}
}

NES_NAMESPACE_END
