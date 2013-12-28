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

#ifndef NST_BOARDS_FFE_H
#define NST_BOARDS_FFE_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class NST_NO_VTABLE Ffe : public Mapper
			{
			public:
		
				void SaveState(State::Saver&) const;
				void LoadState(State::Loader&);
		
			protected:
		
				enum Type
				{
					F3_XXX,
					F4_XXX,
					F8_XXX
				};
		
				Ffe(Context&,Type);
				~Ffe();
				
			private:
		
				void SubReset(bool);
				void VSync();

				static uint GetIrqBase(dword);
				
				NES_DECL_POKE( 42FE )
				NES_DECL_POKE( 42FF )
				NES_DECL_POKE( 4501 )
				NES_DECL_POKE( 4502 )
				NES_DECL_POKE( 4503 )
				NES_DECL_POKE( Prg_F3 )
				NES_DECL_POKE( Prg_F4 )
		
				struct Irq
				{
					void Reset(bool);
					ibool Signal();
		
					uint count;
					ibool enabled;
					const uint clock;
		
					Irq(uint c)
					: clock(c) {}
				};
		
				Clock::M2<Irq>* const irq;
				const Type type;
			};
		}
	}
}

#endif