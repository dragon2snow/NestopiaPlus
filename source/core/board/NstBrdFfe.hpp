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

#ifndef NST_BOARDS_FFE_H
#define NST_BOARDS_FFE_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class Ffe : public Mapper
			{
			protected:

				enum Type
				{
					F3_XXX,
					F4_XXX,
					F8_XXX_0,
					F8_XXX_1
				};

				Ffe(Context&,Type);
				~Ffe();

			private:

				void SubReset(bool);
				void BaseSave(State::Saver&) const;
				void BaseLoad(State::Loader&,dword);
				void Sync(Event,Input::Controllers*);

				NES_DECL_POKE( 42FC );
				NES_DECL_POKE( 42FD );

				NES_DECL_POKE( 42FE );
				NES_DECL_POKE( 42FF );

				NES_DECL_POKE( 43FC );
				NES_DECL_POKE( 43FD );
				NES_DECL_POKE( 43FE );
				NES_DECL_POKE( 43FF );

				NES_DECL_POKE( 4501 );
				NES_DECL_POKE( 4502 );
				NES_DECL_POKE( 4503 );

				NES_DECL_POKE( Prg_F3 );
				NES_DECL_POKE( Prg_F4 );

				NES_DECL_POKE( Prg_X );

				struct Irq
				{
					void Reset(bool);
					bool Clock();

					uint count;
					ibool enabled;
					const uint clock;

					explicit Irq(uint c)
					: clock(c) {}
				};

				ClockUnits::M2<Irq>* const irq;
				const Type type;
				uint mode;
			};
		}
	}
}

#endif
