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

#ifndef NST_MAPPER_215_H
#define NST_MAPPER_215_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Mapper215 : public Boards::Mmc3
		{
		public:

			Mapper215(Context&);

		private:

			enum
			{
				ATR_6000_CTRL = 1
			};

			void SubReset(bool);
			void SubSave(State::Saver&) const;
			void SubLoad(State::Loader&);

			uint GetPrgBank(uint) const;
			uint GetChrBank(uint) const;

			void UpdatePrg();
			void UpdateChr() const;

			NES_DECL_POKE( 5000 )
			NES_DECL_POKE( 5001 )
			NES_DECL_POKE( 5007 )
			NES_DECL_POKE( 8000 )
			NES_DECL_POKE( 8001 )
			NES_DECL_POKE( A000 )
			NES_DECL_POKE( C000 )
			NES_DECL_POKE( C001 )
			NES_DECL_POKE( E001 )

			uint exRegs[4];
			const ibool ctrl6000;
		};
	}
}

#endif
