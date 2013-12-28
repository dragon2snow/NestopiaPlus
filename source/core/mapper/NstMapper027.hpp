////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2007 Martin Freij
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

#ifndef NST_MAPPER_27_H
#define NST_MAPPER_27_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Mapper27 : public Mapper
		{
		public:

			explicit Mapper27(Context&);

		private:

			~Mapper27() {}

			void SubReset(bool);
			void SubSave(State::Saver&) const;
			void SubLoad(State::Loader&);
			void Sync(Event,Input::Controllers*);

			template<uint MASK,uint BITS,uint SHIFT>
			void SwapChr(uint,uint) const;

			NES_DECL_POKE( 8  );
			NES_DECL_POKE( 9  );
			NES_DECL_POKE( B0 );
			NES_DECL_POKE( B1 );
			NES_DECL_POKE( B2 );
			NES_DECL_POKE( B3 );
			NES_DECL_POKE( C0 );
			NES_DECL_POKE( C1 );
			NES_DECL_POKE( C2 );
			NES_DECL_POKE( C3 );
			NES_DECL_POKE( D0 );
			NES_DECL_POKE( D1 );
			NES_DECL_POKE( D2 );
			NES_DECL_POKE( D3 );
			NES_DECL_POKE( E0 );
			NES_DECL_POKE( E1 );
			NES_DECL_POKE( E2 );
			NES_DECL_POKE( E3 );
			NES_DECL_POKE( F0 );
			NES_DECL_POKE( F1 );
			NES_DECL_POKE( F2 );
			NES_DECL_POKE( F3 );

			Boards::Vrc4::Irq irq;
			uint prgSwap;
		};
	}
}

#endif
