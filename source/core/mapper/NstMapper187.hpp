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

#ifndef NST_MAPPER_187_H
#define NST_MAPPER_187_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Mapper187 : public Boards::Mmc3
		{
		public:

			Mapper187(Context& c)
			: Mmc3(c,WRAM_NONE) {}

		private:

			void SubSave(State::Saver&) const;
			void SubLoad(State::Loader&);
			void SubReset(bool);

			void UpdateChr() const;
			void UpdatePrg();

			NES_DECL_PEEK( 5000 )
			NES_DECL_POKE( 5000 )
			NES_DECL_POKE( 5001 )
			NES_DECL_POKE( 8000 )
			NES_DECL_POKE( 8001	)
			NES_DECL_POKE( 8003	)

			uint exCtrl;
			uint exMode;
			uint exLast;
		};
	}
}

#endif
