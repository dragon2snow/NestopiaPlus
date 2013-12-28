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

#ifndef NST_BOARDS_BANDAI_H
#define NST_BOARDS_BANDAI_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class NST_NO_VTABLE Bandai : public Mapper
			{
			public:
		
				void SaveState(State::Saver&) const;
				void LoadState(State::Loader&);
		
			protected:
		
				enum Type
				{
					TYPE_A,
					TYPE_B,
					TYPE_C
				};
		
				Bandai(Context&,Type);
		
			private:
		
				void SubReset(bool);
				void VSync();
		
				NES_DECL_POKE( 0 )
				NES_DECL_POKE( 8 )
				NES_DECL_POKE( A )
				NES_DECL_POKE( B )
				NES_DECL_POKE( C )
				NES_DECL_POKE( D )
		
				struct Irq
				{
					void Reset(bool);
					ibool Signal();
		
					uint count;
					uint latch;
				};
		
				Clock::M2<Irq> irq;
				const Type type;
			};
		}
	}
}

#endif
