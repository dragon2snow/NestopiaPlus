////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2005 Martin Freij
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

#include "../NstCore.hpp"
#include "../NstCpu.hpp"
#include "../vssystem/NstVsSystem.hpp"
#include "../vssystem/NstVsSuperXevious.hpp"

namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void VsSuperXevious::Reset()
		{
			cpu.Map( 0x54FFU ).Set( &VsSuperXevious::Peek_54FF );
			cpu.Map( 0x5567U ).Set( &VsSuperXevious::Peek_5567 );
			cpu.Map( 0x5678U ).Set( &VsSuperXevious::Peek_5678 );
			cpu.Map( 0x578FU ).Set( &VsSuperXevious::Peek_578F );
	
			protection = 0;
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_PEEK(VsSuperXevious,54FF) { return 0x05;                              }
		NES_PEEK(VsSuperXevious,5567) { return (protection ^= 0x1) ? 0x37 : 0x3E; }
		NES_PEEK(VsSuperXevious,5678) { return protection ? 0x00 : 0x01;          }
		NES_PEEK(VsSuperXevious,578F) { return protection ? 0xD1 : 0x89;          }
	}
}
