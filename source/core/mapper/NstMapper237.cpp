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
#include "NstMapper237.h"
	   
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER237::Reset()
{
	cpu.SetPort( 0x8000, 0xFFFF, this, Peek_pRom, Poke_pRom );
	memset( ExRam, 0x00, sizeof(U8) * 0x8000 );
	UseExRam = FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER237,pRom)
{
	return UseExRam ? ExRam[address - 0x8000] : pRom[address - 0x8000];
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER237,pRom) 
{ 
	if (address & 0x4000)
	{
		apu.Update(); 

		switch (address & 0x0030)
		{
     		case 0x00:

				pRom.SwapBanks<n16k,0x0000>( address & 0x7 );
				pRom.SwapBanks<n16k,0x4000>( pRom.NumBanks<n16k>() - 1 );
				break;

			case 0x10:
			{
				const UINT offset[2][2] =
				{
					{
						((address & 0x7) * 2 + 0) * 0x2000 + 0xD,
						((address & 0x7) * 2 + 1) * 0x2000 + 0xD
					},
					{
						(pRom.NumBanks<n8k>() - 2) * 0x2000 + 0xD,
						(pRom.NumBanks<n8k>() - 1) * 0x2000 + 0xD
					}
				};

				for (UINT i=0x0000; i < 0x2000; ++i)
				{
					const UINT index = i & 0x1FF0;

					ExRam[i+0x0000] = *pRom.Ram( offset[0][0] + index );
					ExRam[i+0x2000] = *pRom.Ram( offset[0][1] + index );
					ExRam[i+0x4000] = *pRom.Ram( offset[1][0] + index );
					ExRam[i+0x6000] = *pRom.Ram( offset[1][1] + index );
				}

				UseExRam = TRUE;
				return;
			}

			case 0x20:

				pRom.SwapBanks<n32k,0x0000>( (address & 0x6) >> 1 );
				break;

			case 0x30:

				pRom.SwapBanks<n16k,0x0000>( address & 0x7 );
				pRom.SwapBanks<n16k,0x4000>( address & 0x7 );
				break;
		}

		UseExRam = FALSE;
	}
	else
	{
		ppu.SetMirroring( (address & 0x0020) ? MIRROR_VERTICAL : MIRROR_HORIZONTAL );
		cRom.SwapBanks<n8k,0x0000>( address & 0x7 );
	}
}

NES_NAMESPACE_END
