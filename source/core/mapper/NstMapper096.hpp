////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
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

#ifndef NST_MAPPER_96_H
#define NST_MAPPER_96_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Mapper96 : public Mapper
		{
		public:

			Mapper96(Context& c)
			: Mapper(c,PROM_MAX_128K|CROM_NONE|CRAM_32K|WRAM_DEFAULT) {}

		private:

			void SubReset(bool);
			void UpdateLatch(uint) const;

			NES_DECL_POKE( Prg )
			NES_DECL_PEEK( 2006 )
			NES_DECL_POKE( 2006 )
			NES_DECL_ACCESSOR( Name_2000 )
			NES_DECL_ACCESSOR( Name_2400 )
			NES_DECL_ACCESSOR( Name_2800 )
			NES_DECL_ACCESSOR( Name_2C00 )

			Io::Port p2006;
		};
	}
}

#endif
