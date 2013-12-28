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
#include "NstMapper232.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER232::Reset()
{
	cpu->SetPort( 0x9000,         this, Peek_9000, Poke_9000 );
	cpu->SetPort( 0xA000, 0xBFFF, this, Peek_A000, Poke_A000 );
	cpu->SetPort( 0xC000, 0xDFFF, this, Peek_C000, Poke_A000 );
	cpu->SetPort( 0xE000, 0xFFFF, this, Peek_E000, Poke_A000 );

	regs[0] = 0x0C;
	regs[1] = 0x00;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER232,9000) { regs[0] = (data & 0x18) >> 1; BankSwitch(); }
NES_POKE(MAPPER232,A000) { regs[1] = (data & 0x03) >> 0; BankSwitch(); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER232::BankSwitch()
{
	apu->Update();
	pRom.SwapBanks<n16k,0x0000>( regs[0] | regs[1] );
	pRom.SwapBanks<n16k,0x4000>( regs[0] | 0x3     );
}

NES_NAMESPACE_END
