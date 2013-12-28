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
#include "NstMapper151.h"
			   
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER151::Reset()
{
	cpu->SetPort( 0x8000, 0x8FFF, this, Peek_8000, Poke_8000 );
	cpu->SetPort( 0xA000, 0xAFFF, this, Peek_A000, Poke_A000 );
	cpu->SetPort( 0xC000, 0xCFFF, this, Peek_C000, Poke_C000 );
	cpu->SetPort( 0xE000, 0xEFFF, this, Peek_E000, Poke_E000 );
	cpu->SetPort( 0xF000, 0xFFFF, this, Peek_F000, Poke_F000 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER151,8000) { apu->Update(); pRom.SwapBanks<n8k,0x0000>(data); }
NES_POKE(MAPPER151,A000) { apu->Update(); pRom.SwapBanks<n8k,0x2000>(data); }
NES_POKE(MAPPER151,C000) { apu->Update(); pRom.SwapBanks<n8k,0x4000>(data); }
NES_POKE(MAPPER151,E000) { ppu->Update(); cRom.SwapBanks<n4k,0x0000>(data); }
NES_POKE(MAPPER151,F000) { ppu->Update(); cRom.SwapBanks<n4k,0x1000>(data); }

NES_NAMESPACE_END
