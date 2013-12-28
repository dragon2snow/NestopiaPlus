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
#include "NstMapper004.h"
#include "NstMapper100.h"
	
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER100::Reset()
{
	MAPPER4::Reset();

	for (UINT i=0x8000; i < 0x9000; i += 2)
	{
		cpu->SetPort( i+0x0, this, Peek_8000, Poke_8000 );
		cpu->SetPort( i+0x1, this, Peek_8000, Poke_8001 );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER100,8000)
{
	command = data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER100,8001)
{
	const UINT index = command & COMMAND_INDEX;

    switch (command & 0xC7)
	{
          case 0x00:

			  cRomBanks[0] = (data & 0xFE) + 0;
			  cRomBanks[1] = (data & 0xFE) + 1;
			  UpdateCRom();
			  return;

          case 0x01: 

			  cRomBanks[2] = (data & 0xFE) + 0;
			  cRomBanks[3] = (data & 0xFE) + 1;
			  UpdateCRom();
			  return;

		  case 0x02:
		  case 0x03:
		  case 0x04:
		  case 0x05:

			  cRomBanks[index] = data;
			  UpdateCRom();
			  return;

		  case 0x06:
		  case 0x07:

			  pRomBanks[index - 6] = data;
			  UpdatePRom();
			  return;

          case 0x46: 
			  
			  pRomBanks[2] = data; 
			  UpdatePRom(); 
			  return;

          case 0x47: 
			  
			  pRomBanks[3] = data; 
			  UpdatePRom(); 
			  return;

          case 0x80:

			  cRomBanks[4] = (data & 0xFE) + 0; 
			  cRomBanks[5] = (data & 0xFE) + 1; 
			  UpdateCRom();
			  return;

		  case 0x81:

			  cRomBanks[6] = (data & 0xFE) + 0; 
			  cRomBanks[7] = (data & 0xFE) + 1; 
			  UpdateCRom();
			  return;

		  case 0x82:
		  case 0x83:
		  case 0x84:
		  case 0x85:

			  cRomBanks[index - 2] = data; 
			  UpdateCRom();
			  return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER100::UpdatePRom()
{
	apu->Update(); 

	pRom.SwapBanks<n8k,0x0000>( pRomBanks[0] );
	pRom.SwapBanks<n8k,0x2000>( pRomBanks[1] );
	pRom.SwapBanks<n8k,0x4000>( pRomBanks[2] );
	pRom.SwapBanks<n8k,0x6000>( pRomBanks[3] );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER100::UpdateCRom()
{
	ppu->Update();

	cRom.SwapBanks<n1k,0x0000>( cRomBanks[0] ); 
	cRom.SwapBanks<n1k,0x0400>( cRomBanks[1] ); 
	cRom.SwapBanks<n1k,0x0800>( cRomBanks[2] ); 
	cRom.SwapBanks<n1k,0x0C00>( cRomBanks[3] ); 
	cRom.SwapBanks<n1k,0x1000>( cRomBanks[4] ); 
	cRom.SwapBanks<n1k,0x1400>( cRomBanks[5] ); 
	cRom.SwapBanks<n1k,0x1800>( cRomBanks[6] ); 
	cRom.SwapBanks<n1k,0x1C00>( cRomBanks[7] ); 
}

NES_NAMESPACE_END
