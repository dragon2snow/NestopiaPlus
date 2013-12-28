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

#ifndef NST_BOARDS_22211_H
#define NST_BOARDS_22211_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class Unl22211 : public Mapper
			{
			protected:

				enum Type
				{
					TYPE_STD,
					TYPE_SECURITY_1,
					TYPE_SECURITY_2
				};

				Unl22211(Context&,Type);
				~Unl22211() {}

			private:

				void SubReset(bool);
				void SubLoad(State::Loader&);
				void SubSave(State::Saver&) const;

				NES_DECL_PEEK( 4100 );
				NES_DECL_POKE( 4100 );
				NES_DECL_POKE( 8000 );

				byte regs[4];
				const Type type;
			};
		}
	}
}

#endif
