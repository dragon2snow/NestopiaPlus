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
#include "NstMapper015.h"
		   
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER15::Reset()
{
	cpu->SetPort( 0x8000, this, Peek_8000, Poke_8000 );
	cpu->SetPort( 0x8001, this, Peek_8000, Poke_8001 );
	cpu->SetPort( 0x8002, this, Peek_8000, Poke_8002 );
	cpu->SetPort( 0x8003, this, Peek_8000, Poke_8003 );

	pRom.SwapBanks<n32k,0x0000>(0); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER15,8000)
{
	apu->Update();

	if (data & 0x80)
	{
		pRom.SwapBanks<n8k,0x0000>( (data << 1) + 1 );
		pRom.SwapBanks<n8k,0x2000>( (data << 1) + 0 );
		pRom.SwapBanks<n8k,0x4000>( (data << 1) + 2 );
		pRom.SwapBanks<n8k,0x6000>( (data << 1) + 1 );
	}
	else
	{
		pRom.SwapBanks<n16k,0x0000>( data + 0 );
		pRom.SwapBanks<n16k,0x4000>( data + 1 );
	}

	ppu->SetMirroring( (data & 0x40) ? MIRROR_HORIZONTAL : MIRROR_VERTICAL ); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER15,8001)
{
	apu->Update();

	pRom.SwapBanks<n16k,0x0000>( data );
	pRom.SwapBanks<n16k,0x4000>( ~0U  );

	ppu->SetMirroring( MIRROR_VERTICAL );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER15,8002)
{
	apu->Update();

	const UINT bank = (data << 1) + ((data & 0x80) ? 1 : 0);

	pRom.SwapBanks<n8k,0x0000>( bank );
	pRom.SwapBanks<n8k,0x2000>( bank );
	pRom.SwapBanks<n8k,0x4000>( bank );
	pRom.SwapBanks<n8k,0x6000>( bank );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER15,8003)
{
	apu->Update();

	if (data & 0x80)
	{
		pRom.SwapBanks<n8k,0x4000>( (data << 1) + 1 );
		pRom.SwapBanks<n8k,0x6000>( (data << 1) + 0 );
	}
	else
	{
		pRom.SwapBanks<n16k,0x4000>( data );
	}

	ppu->SetMirroring( (data & 0x40) ? MIRROR_HORIZONTAL : MIRROR_VERTICAL ); 
}

NES_NAMESPACE_END

