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

#ifndef NST_MAPPER_211_H
#define NST_MAPPER_211_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Mapper211 : public Mapper
		{
		public:

			Mapper211(Context&);

		private:

			void SubReset(bool);
			void SubSave(State::Saver&) const;
			void SubLoad(State::Loader&);
			void UpdatePrg();
			void UpdateChr() const;
			void VSync();

			NES_DECL_PEEK( 5000 )
			NES_DECL_PEEK( 5800 )
			NES_DECL_POKE( 5800 )
			NES_DECL_PEEK( 5801 )
			NES_DECL_POKE( 5801 )
			NES_DECL_PEEK( 5803 )
			NES_DECL_POKE( 5803 )
			NES_DECL_POKE( 8000 )
			NES_DECL_POKE( 9000 )
			NES_DECL_POKE( B000 )
			NES_DECL_POKE( C002 )
			NES_DECL_POKE( C004 )
			NES_DECL_POKE( C005 )
			NES_DECL_POKE( D000 )

			struct Regs
			{
				enum
				{
					PROM_BANKSWITCH = b00000011,
					PROM_SWAP_16K	= b00000001,
					PROM_SWAP_8K	= b00000010,
					CROM_BANKSWITCH = b00011000,
					CROM_SWAP_8K    = b00000000,
					CROM_SWAP_4K    = b00001000,
					CROM_SWAP_2K    = b00010000,
					CROM_SWAP_1K    = b00011000
				};

				uint toggle;
				uint command;
				uint mul[2];
				uint tmp;
			};

			struct Banks
			{
				uchar prg[4];
				uchar chr[8];
			};

			Regs regs;
			Banks banks;
			Boards::Mmc3::Irq irq;
		};
	}
}

#endif
