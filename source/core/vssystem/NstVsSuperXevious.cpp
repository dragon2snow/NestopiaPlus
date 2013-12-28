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

#include "../NstTypes.h"
#include "../NstMap.h"
#include "../NstCpu.h"
#include "../vssystem/NstVsSystem.h"
#include "../vssystem/NstVsSuperXevious.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID VSSUPERXEVIOUS::Reset()
{
	cpu->SetPort( 0x54FF, this, Peek_54FF, Poke );
	cpu->SetPort( 0x5567, this, Peek_5567, Poke );
	cpu->SetPort( 0x5678, this, Peek_5678, Poke );
	cpu->SetPort( 0x578F, this, Peek_578F, Poke );

	protection = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(VSSUPERXEVIOUS,54FF) { return 0x05;                            }
NES_PEEK(VSSUPERXEVIOUS,5567) { return protection ^= 0x1 ? 0x37 : 0x3E; }
NES_PEEK(VSSUPERXEVIOUS,5678) { return protection ? 0x00 : 0x01;        }
NES_PEEK(VSSUPERXEVIOUS,578F) { return protection ? 0xD1 : 0x89;        }

NES_NAMESPACE_END
