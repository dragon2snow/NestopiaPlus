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

#ifndef NST_BOARDS_VRC4_H
#define NST_BOARDS_VRC4_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class Vrc4 : public Mapper
			{
			protected:

				enum Type
				{
					TYPE_VRC4_352889,
					TYPE_VRC4_352396,
					TYPE_VRC4_351406
				};

				Vrc4(Context&,Type,dword=CROM_MAX_256K|NMT_VERTICAL);
				~Vrc4() {}

				void SubReset(bool);

			private:

				struct BaseIrq
				{
					void Reset(bool);
					bool Clock();

					enum
					{
						ENABLE_0    = 0x2,
						ENABLE_1    = 0x1,
						NO_PPU_SYNC = 0x4,
						CTRL        = 0x1U|0x2U|0x4U
					};

					uint ctrl;
					uint count[2];
					uint latch;
				};

			public:

				struct Irq : ClockUnits::M2<BaseIrq>
				{
					void WriteLatch0(uint);
					void WriteLatch1(uint);
					void Toggle(uint);
					void Toggle();
					void LoadState(State::Loader&);
					void SaveState(State::Saver&,dword) const;

					explicit Irq(Cpu& c)
					: ClockUnits::M2<BaseIrq>(c) {}
				};

			private:

				void BaseSave(State::Saver&) const;
				void BaseLoad(State::Loader&,dword);
				void Sync(Event,Input::Controllers*);

				template<uint OFFSET>
				void SwapChr(uint,uint) const;

				NES_DECL_POKE( 8  );
				NES_DECL_POKE( 9  );
				NES_DECL_POKE( B0 );
				NES_DECL_POKE( B1 );
				NES_DECL_POKE( B2 );
				NES_DECL_POKE( B3 );
				NES_DECL_POKE( C0 );
				NES_DECL_POKE( C1 );
				NES_DECL_POKE( C2 );
				NES_DECL_POKE( C3 );
				NES_DECL_POKE( D0 );
				NES_DECL_POKE( D1 );
				NES_DECL_POKE( D2 );
				NES_DECL_POKE( D3 );
				NES_DECL_POKE( E0 );
				NES_DECL_POKE( E1 );
				NES_DECL_POKE( E2 );
				NES_DECL_POKE( E3 );
				NES_DECL_POKE( F0 );
				NES_DECL_POKE( F1 );
				NES_DECL_POKE( F2 );
				NES_DECL_POKE( F3 );

				Irq irq;

			protected:

				uint prgSwap;
				const Type type;
			};
		}
	}
}

#endif
