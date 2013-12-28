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

#ifndef NST_BOARDS_VRC4_H
#define NST_BOARDS_VRC4_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class NST_NO_VTABLE Vrc4 : public Mapper
			{
				struct BaseIrq
				{
					void Reset(bool);
					ibool Signal();

					enum
					{
						ENABLE_0    = 0x2,
						ENABLE_1    = 0x1,
						NO_PPU_SYNC = 0x4,
						CTRL        = 0x1|0x2|0x4
					};

					uint ctrl;
					uint count[2];
					uint latch;
				};

			public:

				struct Irq : Clock::M2<BaseIrq>
				{
					void WriteLatch0(uint);
					void WriteLatch1(uint);
					void Toggle(uint);
					void Toggle();
					void LoadState(State::Loader&);
					void SaveState(State::Saver&) const;

					Irq(Cpu& cpu)
					: Clock::M2<BaseIrq>(cpu) {}
				};

			protected:

				enum Type
				{
					TYPE_2A,
					TYPE_A,
					TYPE_B,
					TYPE_Y
				};

				Vrc4(Context&,Type);
				~Vrc4();

			private:

				void SubReset(bool);
				void BaseSave(State::Saver&) const;
				void BaseLoad(State::Loader&,dword);
				void VSync();
				void SwapChrA(uint,uint) const;
				
				template<uchar MASK,uchar SHIFT>
				void SwapChrB(uint,uint) const;

				NES_DECL_POKE( 8    )
				NES_DECL_POKE( 9    )
				NES_DECL_POKE( B0_A )
				NES_DECL_POKE( B0_B )
				NES_DECL_POKE( B1_A )
				NES_DECL_POKE( B1_B )
				NES_DECL_POKE( B2_B )
				NES_DECL_POKE( B3_B )
				NES_DECL_POKE( C0_A )
				NES_DECL_POKE( C0_B )
				NES_DECL_POKE( C1_A )
				NES_DECL_POKE( C1_B )
				NES_DECL_POKE( C2_B )
				NES_DECL_POKE( C3_B )
				NES_DECL_POKE( D0_A )
				NES_DECL_POKE( D0_B )
				NES_DECL_POKE( D1_A )
				NES_DECL_POKE( D1_B )
				NES_DECL_POKE( D2_B )
				NES_DECL_POKE( D3_B )
				NES_DECL_POKE( E0_A )
				NES_DECL_POKE( E0_B )
				NES_DECL_POKE( E1_A )
				NES_DECL_POKE( E1_B )
				NES_DECL_POKE( E2_B )
				NES_DECL_POKE( E3_B )
				NES_DECL_POKE( F0   )
				NES_DECL_POKE( F1   )
				NES_DECL_POKE( F2   )
				NES_DECL_POKE( F3   )

				Irq* const irq;
				uint prgSwap;
				const Type type;
			};
		}
	}
}

#endif
