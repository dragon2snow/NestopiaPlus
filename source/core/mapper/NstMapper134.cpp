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
#include "NstMapper134.h"
		 
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER134::Reset()
{
	cpu.SetPort( 0x4020, 0x5FFF, this, Peek_Nop, Poke_4100 );
	pRom.SwapBanks<n32k,0x0000>(0);

	command = 0x0;
	pBank = 0x0;
	cBank = 0x0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER134,4100) 
{ 
	ppu.Update();
	apu.Update();

	switch (address & 0x4101)
	{
       	case 0x4100: 
			
			command = data & 0x7;
			break;

       	case 0x4101:

			switch (command)
			{
     			case 0x0: pBank = 0; cBank = 3; break;
     			case 0x4: cBank = (cBank & 0x03) | ((data & 0x7) << 2); break;
     			case 0x5: pBank = data & 0x7; break;
      			case 0x6: cBank = (cBank & 0x1C) | (data & 0x3); break;
      			case 0x7: ppu.SetMirroring( (data & 0x1) ? MIRROR_HORIZONTAL : MIRROR_VERTICAL ); break;
			}
			break;
	}

	pRom.SwapBanks<n32k,0x0000>(pBank);
	cRom.SwapBanks<n8k,0x0000>(cBank);
}

NES_NAMESPACE_END
