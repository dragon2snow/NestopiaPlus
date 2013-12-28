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
#include "NstMapper135.h"
		 
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER135::Reset()
{
	cpu.SetPort( 0x4020, 0x5FFF, this, Peek_Nop, Poke_4100 );
	pRom.SwapBanks<n32k,0x0000>(0);

	command = 0x0;
	
	cBanks[0] = 
	cBanks[1] =
	cBanks[2] =
	cBanks[3] =
	cBanks[4] = 0x0;

	UpdateCRom();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER135,4100) 
{ 
	switch (address & 0x4101)
	{
       	case 0x4100: 
			
			command = data & 0x7;
			break;

       	case 0x4101:

			switch (command)
			{
     			case 0x0:
				case 0x1:
				case 0x2:
				case 0x3:
				case 0x4:

					cBanks[command] = (data & 0x7) << 1;
					ppu.Update();
					UpdateCRom();
					break;

				case 0x5:

					apu.Update();
					pRom.SwapBanks<n32k,0x0000>(data & 0x7);
					break;

				case 0x7:

					switch ((data >> 1) & 0x3)
					{
       					case 0x0: ppu.SetMirroring( MIRROR_ZERO       ); break;
						case 0x1: ppu.SetMirroring( MIRROR_HORIZONTAL ); break;
						case 0x2: ppu.SetMirroring( MIRROR_VERTICAL   ); break;
						case 0x3: ppu.SetMirroring( MIRROR_ONE        ); break;
					}
					break;
			}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER135::UpdateCRom()
{
	const UINT in = cBanks[4] << 3;

	cRom.SwapBanks<n2k,0x0000>( cBanks[0] | in | 0x0 );
	cRom.SwapBanks<n2k,0x0800>( cBanks[1] | in | 0x1 );
	cRom.SwapBanks<n2k,0x1000>( cBanks[2] | in | 0x0 );
	cRom.SwapBanks<n2k,0x1800>( cBanks[3] | in | 0x1 );
}

NES_NAMESPACE_END
