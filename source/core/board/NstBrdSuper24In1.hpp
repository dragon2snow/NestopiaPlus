////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2006 Martin Freij
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

#ifndef NST_BOARDS_SUPER24IN1_H
#define NST_BOARDS_SUPER24IN1_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class Super24In1 : public Mmc3
			{
			public:
	
				Super24In1(Context& c)
				: Mmc3(c,WRAM_8K|CRAM_8K) {}
	
			private:
	
				void SubReset(bool);
				void SubLoad(State::Loader&);
				void SubSave(State::Saver&) const;
				void UpdatePrg();
				void UpdateChr() const;
	
				enum
				{
					EXREG_PRG_BANKS = 0x7,
					EXREG_USE_CRAM = 0x20
				};
	
				NES_DECL_POKE( 5FF0 )
				NES_DECL_POKE( 5FF1 )
				NES_DECL_POKE( 5FF2 )
				NES_DECL_POKE( C000 )
				NES_DECL_POKE( C001 )
				NES_DECL_POKE( E000 )
				NES_DECL_POKE( E001 )
	
				uint exRegs[3];
			};
		}
	}
}

#endif
