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
#include "NstMapper243.h"
	 
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER243::Reset()
{
	for (UINT i=0x4100; i < 0x7FFF; ++i)
	{
		switch (i & 0x4101)
		{
     		case 0x4100: cpu->SetPort( i, this, Peek_Nop, Poke_4100 ); continue;
			case 0x4101: cpu->SetPort( i, this, Peek_Nop, Poke_4101 ); continue;
		}
	}

	pRom.SwapBanks<n32k,0x0000>(0);

	if (cRom.NumBanks<n1k>() > 32)
		cRom.SwapBanks<n8k,0x0000>(3);

	ppu->SetMirroring(MIRROR_HORIZONTAL);

	command  = 0;
	pRomBank = 0;
	cRomBank = 3;
	vertical = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER243,4100) 
{ 
	command = data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER243,4101) 
{
	apu->Update();

	switch (command & 0x7)
	{
     	case 0x0: pRomBank = 0; cRomBank = 3;                        break;
		case 0x4: cRomBank = (cRomBank & 0x6) | ((data & 0x1) << 0); break;
		case 0x5: pRomBank = (data & 0x1);                           break;
		case 0x6: cRomBank = (cRomBank & 0x1) | ((data & 0x3) << 1); break;
		case 0x7: vertical = (data & 0x1);							 break;
	}

	ppu->SetMirroring( vertical ? MIRROR_VERTICAL : MIRROR_HORIZONTAL );

	pRom.SwapBanks<n32k,0x0000>(pRomBank);
	cRom.SwapBanks<n8k,0x0000>(cRomBank);
}

NES_NAMESPACE_END
