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
#include "NstMapper092.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER92::Reset()
{
	cpu.SetPort( 0x8000, 0x8FFF, this, Peek_8000, Poke_8000 );
	cpu.SetPort( 0xA000, 0xBFFF, this, Peek_A000, Poke_9000 );
	cpu.SetPort( 0xC000, 0xDFFF, this, Peek_C000, Poke_9000 );
	cpu.SetPort( 0xE000, 0xFFFF, this, Peek_E000, Poke_9000 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER92,8000)
{
	switch (address & 0xF0)
	{
       	case 0xB0: 
			
			apu.Update();
			pRom.SwapBanks<n16k,0x0000>( 0 ); 
			pRom.SwapBanks<n16k,0x4000>( address & 0xF );
			return;

		case 0x70: 
			
			ppu.Update();
			cRom.SwapBanks<n8k,0x0000>( address & 0xF );
			return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER92,9000)
{
	switch (address & 0xF0)
	{
       	case 0xD0: 
			
			apu.Update();
			pRom.SwapBanks<n16k,0x0000>( 0 ); 
			pRom.SwapBanks<n16k,0x4000>( address & 0xF );
			return;

		case 0xE0: 
			
			ppu.Update();
			cRom.SwapBanks<n8k,0x0000>( address & 0xF );
			return;
	}
}

NES_NAMESPACE_END
