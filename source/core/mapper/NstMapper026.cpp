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
#include "../sound/NstSndVrc6.h"
#include "NstMapper024.h"
#include "NstMapper026.h"
	
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER26::Reset()
{
	MAPPER24::Reset();

	MAPPER24* const m24 = PDX_STATIC_CAST(MAPPER24*,this);

	for (ULONG i=0x8000; i <= 0xFFFFU; ++i)
	{
		switch (((i & 0xFFFC) | ((i >> 1) & 0x1) | ((i << 1) & 0x2)) & 0xF003)
		{
       		case 0x8000: cpu->SetPort( i, m24, MAPPER24::Peek_pRom, MAPPER24::Poke_8000 ); continue;
			case 0x9000: cpu->SetPort( i, m24, MAPPER24::Peek_pRom, MAPPER24::Poke_9000 ); continue;
			case 0x9001: cpu->SetPort( i, m24, MAPPER24::Peek_pRom, MAPPER24::Poke_9001 ); continue;
			case 0x9002: cpu->SetPort( i, m24, MAPPER24::Peek_pRom, MAPPER24::Poke_9002 ); continue;
			case 0xA000: cpu->SetPort( i, m24, MAPPER24::Peek_pRom, MAPPER24::Poke_A000 ); continue;
			case 0xA001: cpu->SetPort( i, m24, MAPPER24::Peek_pRom, MAPPER24::Poke_A001 ); continue;
			case 0xA002: cpu->SetPort( i, m24, MAPPER24::Peek_pRom, MAPPER24::Poke_A002 ); continue;
			case 0xB000: cpu->SetPort( i, m24, MAPPER24::Peek_pRom, MAPPER24::Poke_B000 ); continue;
			case 0xB001: cpu->SetPort( i, m24, MAPPER24::Peek_pRom, MAPPER24::Poke_B001 ); continue;
			case 0xB002: cpu->SetPort( i, m24, MAPPER24::Peek_pRom, MAPPER24::Poke_B002 ); continue;
			case 0xB003: cpu->SetPort( i, m24, MAPPER24::Peek_pRom, MAPPER24::Poke_B003 ); continue;
			case 0xC000: cpu->SetPort( i, m24, MAPPER24::Peek_pRom, MAPPER24::Poke_C000 ); continue;
			case 0xD000: cpu->SetPort( i, m24, MAPPER24::Peek_pRom, MAPPER24::Poke_D000 ); continue;
			case 0xD001: cpu->SetPort( i, m24, MAPPER24::Peek_pRom, MAPPER24::Poke_D001 ); continue;
			case 0xD002: cpu->SetPort( i, m24, MAPPER24::Peek_pRom, MAPPER24::Poke_D002 ); continue;
			case 0xD003: cpu->SetPort( i, m24, MAPPER24::Peek_pRom, MAPPER24::Poke_D003 ); continue;
			case 0xE000: cpu->SetPort( i, m24, MAPPER24::Peek_pRom, MAPPER24::Poke_E000 ); continue;
			case 0xE001: cpu->SetPort( i, m24, MAPPER24::Peek_pRom, MAPPER24::Poke_E001 ); continue;
			case 0xE002: cpu->SetPort( i, m24, MAPPER24::Peek_pRom, MAPPER24::Poke_E002 ); continue;
			case 0xE003: cpu->SetPort( i, m24, MAPPER24::Peek_pRom, MAPPER24::Poke_E003 ); continue;
			case 0xF000: cpu->SetPort( i, m24, MAPPER24::Peek_pRom, MAPPER24::Poke_F000 ); continue;
			case 0xF001: cpu->SetPort( i, m24, MAPPER24::Peek_pRom, MAPPER24::Poke_F001 ); continue;
			case 0xF002: cpu->SetPort( i, m24, MAPPER24::Peek_pRom, MAPPER24::Poke_F002 ); continue;
		}																		
	}
}

NES_NAMESPACE_END
