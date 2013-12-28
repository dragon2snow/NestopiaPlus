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

#ifndef NST_MAPPER_252_H
#define NST_MAPPER_252_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Mapper252 : public Mapper
		{
		public:

			Mapper252(Context&);

		private:

			void SubReset(bool);
			void SubSave(State::Saver&) const;
			void SubLoad(State::Loader&);
			void VSync();

			template<uint MASK,uint SHIFT>
			void SwapChr(uint,uint) const;

			NES_DECL_POKE( B000 )
			NES_DECL_POKE( B004 )
			NES_DECL_POKE( B008 )
			NES_DECL_POKE( B00C )
			NES_DECL_POKE( C000 )
			NES_DECL_POKE( C004 )
			NES_DECL_POKE( C008 )
			NES_DECL_POKE( C00C )
			NES_DECL_POKE( D000 )
			NES_DECL_POKE( D004 )
			NES_DECL_POKE( D008 )
			NES_DECL_POKE( D00C )
			NES_DECL_POKE( E000 )
			NES_DECL_POKE( E004 )
			NES_DECL_POKE( E008 )
			NES_DECL_POKE( E00C )
			NES_DECL_POKE( F000 )
			NES_DECL_POKE( F004 )
			NES_DECL_POKE( F008 )
			NES_DECL_POKE( F00C )

			Boards::Vrc4::Irq irq;
		};
	}
}

#endif
