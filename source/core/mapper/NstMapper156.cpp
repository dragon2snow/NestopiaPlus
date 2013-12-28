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
#include "NstMapper156.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER156::Reset()
{
	cpu->SetPort( 0xC000, 0xC003, this, Peek_C000, Poke_C000 );
	cpu->SetPort( 0xC008, 0xC00B, this, Peek_C000, Poke_C008 );
	cpu->SetPort( 0xC010,         this, Peek_C000, Poke_C010 );

	ppu->SetMirroring( MIRROR_ZERO );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER156,C000) { ppu->Update(); cRom.SwapBanks<n1k>( 0x0000 + (address & 0x3) * n1k, data ); }
NES_POKE(MAPPER156,C008) { ppu->Update(); cRom.SwapBanks<n1k>( 0x1000 + (address & 0x3) * n1k, data ); }
NES_POKE(MAPPER156,C010) { apu->Update(); pRom.SwapBanks<n16k,0x0000>(data); }

NES_NAMESPACE_END