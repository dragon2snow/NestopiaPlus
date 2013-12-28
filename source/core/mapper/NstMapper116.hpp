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

#ifndef NST_MAPPER_116_H
#define NST_MAPPER_116_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Mapper116 : public Mapper
		{
		public:

			Mapper116(Context&);

		private:

			void SubReset(bool);
			void SubSave(State::Saver&) const;
			void SubLoad(State::Loader&);
			void VSync();

			void UpdatePrg();
			void UpdateChr() const;
			void UpdateNmt() const;

			void Poke_Mmc1_8000(uint,uint);

			void Poke_Vrc2_8000(uint,uint);
			void Poke_Vrc2_9000(uint,uint);
			void Poke_Vrc2_B000(uint,uint);

			void Poke_Mmc3_8000(uint,uint);
			void Poke_Mmc3_A000(uint,uint);
			void Poke_Mmc3_C000(uint,uint);
			void Poke_Mmc3_E000(uint,uint);

			NES_DECL_POKE( 4100 )
			NES_DECL_POKE( 8000 )
			NES_DECL_POKE( 9000 )
			NES_DECL_POKE( A000 )
			NES_DECL_POKE( B000 )
			NES_DECL_POKE( C000 )
			NES_DECL_POKE( D000 )
			NES_DECL_POKE( E000 )
			NES_DECL_POKE( F000 )

			uint mode;

			struct Vrc2
			{
				u8 chr[8];
				u8 prg[2];
				u8 nmt;
				u8 pad;
			};

			struct Mmc3
			{
				u8 banks[10];
				u8 ctrl;
				u8 nmt;
			};

			struct Mmc1
			{
				u8 regs[4];
				u8 buffer;
				u8 shifter;
				u8 pad[2];
			};

			union
			{
				Vrc2 vrc2;
				u8 blockVrc2[8+2+1];
			};

			union
			{
				Mmc3 mmc3;
				u8 blockMmc3[10+1+1];
			};

			union
			{
				Mmc1 mmc1;
				u8 blockMmc1[4+1+1];
			};

			Boards::Mmc3::Irq irq;
		};
	}
}

#endif
