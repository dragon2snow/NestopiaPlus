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
#include "NstMapper236.h"
	 
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER236::Reset()
{
	cpu->SetPort( 0x8000, 0x9FFF, this, Peek_8000, Poke_pRom );
	cpu->SetPort( 0xA000, 0xBFFF, this, Peek_A000, Poke_pRom );
	cpu->SetPort( 0xC000, 0xDFFF, this, Peek_C000, Poke_pRom );
	cpu->SetPort( 0xE000, 0xFFFF, this, Peek_E000, Poke_pRom );

	bank = 0;
	mode = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER236,pRom) 
{ 
	if (address >= 0x8000 && address <= 0xBFFF)
	{
		bank = ((address & 0x3) << 4) | (bank & 0x07);
	}
	else
	{
		bank = ((address & 0x7) << 0) | (bank & 0x30);
		mode = address & 0x30;
	}

	ppu->SetMirroring( (address & 0x20) ? MIRROR_HORIZONTAL : MIRROR_VERTICAL );

	apu->Update();

	switch (mode)
	{
		case 0x00:

			bank |= 0x08;
			pRom.SwapBanks<n16k,0x0000>( (bank | 0x00) );
			pRom.SwapBanks<n16k,0x4000>( (bank | 0x07) );
			return;

		case 0x10:

			bank &= 0x37;
			pRom.SwapBanks<n16k,0x0000>( (bank | 0x00) );
			pRom.SwapBanks<n16k,0x4000>( (bank | 0x07) );
			return;

		case 0x20:

			bank |= 0x08;
			pRom.SwapBanks<n16k,0x0000>( (bank & 0xFE) + 0 );
			pRom.SwapBanks<n16k,0x4000>( (bank & 0xFE) + 1 );
			return;

		case 0x30:

			bank |= 0x08;
			pRom.SwapBanks<n16k,0x0000>( bank );
			pRom.SwapBanks<n16k,0x4000>( bank );
			return;
	}
}

NES_NAMESPACE_END
