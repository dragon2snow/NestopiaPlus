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
#include "NstMapper075.h"
		 
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER75::Reset()
{
	cpu->SetPort( 0x8000, 0x8FFF, this, Peek_8000, Poke_8000 );
	cpu->SetPort( 0x9000, 0x9FFF, this, Peek_9000, Poke_9000 );
	cpu->SetPort( 0xA000, 0xAFFF, this, Peek_A000, Poke_A000 );
	cpu->SetPort( 0xC000, 0xCFFF, this, Peek_C000, Poke_C000 );
	cpu->SetPort( 0xE000, 0xEFFF, this, Peek_E000, Poke_E000 );
	cpu->SetPort( 0xF000, 0xFFFF, this, Peek_F000, Poke_F000 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER75,8000) 
{
	apu->Update(); 
	pRom.SwapBanks<n8k,0x0000>(data);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER75,9000) 
{
	ppu->SetMirroring( (data & 0x1) ? MIRROR_HORIZONTAL : MIRROR_VERTICAL );

	cRomBanks[0] = (cRomBanks[0] & 0xF) | ((data & 0x2) << 3);
	cRomBanks[1] = (cRomBanks[1] & 0xF) | ((data & 0x4) << 2);

	cRom.SwapBanks<n4k,0x0000>( cRomBanks[0] );
	cRom.SwapBanks<n4k,0x1000>( cRomBanks[1] );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER75,A000) { apu->Update(); pRom.SwapBanks<n8k,0x2000>(data); }
NES_POKE(MAPPER75,C000) { apu->Update(); pRom.SwapBanks<n8k,0x4000>(data); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER75,E000) { cRomBanks[0] = (cRomBanks[0] & 0x10) | (data & 0xF); ppu->Update(); cRom.SwapBanks<n4k,0x0000>( cRomBanks[0] ); }
NES_POKE(MAPPER75,F000) { cRomBanks[1] = (cRomBanks[1] & 0x10) | (data & 0xF); ppu->Update(); cRom.SwapBanks<n4k,0x1000>( cRomBanks[1] ); }

NES_NAMESPACE_END
