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

#ifndef NST_MAPPER_64_H
#define NST_MAPPER_64_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Mapper64 : public Mapper
		{
		public:

			Mapper64(Context&);

		private:

			void SubReset(bool);
			void SubSave(State::Saver&) const;
			void SubLoad(State::Loader&);
			void UpdatePrg();
			void UpdateChr() const;
			void VSync();

			NES_DECL_POKE( 8000 )
			NES_DECL_POKE( 8001 )
			NES_DECL_POKE( C000 )
			NES_DECL_POKE( C001 )
			NES_DECL_POKE( E000 )
			NES_DECL_POKE( E001 )

			struct Regs
			{
				void Reset();

				uint ctrl;
				uchar chr[8];
				uchar prg[3];
				uchar pad;
			};

			struct Irq
			{
				Irq(Cpu&,Ppu&);

				void Update();

				enum
				{
					M2_CLOCK   = 4,
					A12_SIGNAL = 16,
					SOURCE_PPU = b00,
					SOURCE_CPU = b01,
					SOURCE     = b01
				};

				struct Unit
				{
					void Reset(bool=true);
					ibool Signal();

					uint count;
					uint latch;
					ibool reload;
					ibool enabled;
				};

				typedef Clock::A12<Unit&> A12;
				typedef Clock::M2<Unit&,M2_CLOCK> M2;

				Unit unit;
				A12 a12;
				M2 m2;
			};

			Regs regs;
			Irq irq;
		};
	}
}

#endif
