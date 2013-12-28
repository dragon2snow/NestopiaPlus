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
#include "NstMapper250.h"
	   
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER250::Reset()
{
	MAPPER4::Reset();

	cpu->SetPort( 0x8000, 0x9FFF, this, Peek_8000, Poke_8000 );
	cpu->SetPort( 0xA000, 0xBFFF, this, Peek_A000, Poke_8000 );
	cpu->SetPort( 0xC000, 0xDFFF, this, Peek_C000, Poke_C000 );
	cpu->SetPort( 0xE000, 0xFFFF, this, Peek_E000, Poke_C000 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER250,8000)
{
	const UINT offset = 
	( 
		((address & 0xE000) >>  0) | 
		((address & 0x0400) >> 10)
	);

	const UINT value =
	(
	    address & 0xFF
	);

	switch (offset & 0xE001)
	{
       	case 0x8000: MAPPER4::Poke_8000( 0x8000, value ); return;
     	case 0x8001: MAPPER4::Poke_8001( 0x8001, value ); return;
     	case 0xA000: MAPPER4::Poke_A000( 0xA000, value ); return;
       	case 0xA001: MAPPER4::Poke_A001( 0xA001, value ); return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER250,C000)
{
	const UINT offset = 
	( 
		((address & 0xE000) >>  0) | 
		((address & 0x0400) >> 10)
	);

	const UINT value =
	(
	    address & 0xFF
	);

	switch (offset & 0xE001)
	{
       	case 0xC000: MAPPER4::Poke_C000( 0xC000, value ); return;
     	case 0xC001: MAPPER4::Poke_C001( 0xC001, value ); return;
     	case 0xE000: MAPPER4::Poke_E000( 0xE000, value ); return;
       	case 0xE001: MAPPER4::Poke_E001( 0xE001, value ); return;
	}
}

NES_NAMESPACE_END
