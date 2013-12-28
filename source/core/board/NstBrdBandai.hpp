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

#ifndef NST_BOARDS_BANDAI_H
#define NST_BOARDS_BANDAI_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class NST_NO_VTABLE Bandai : public Mapper
			{
			protected:

				enum Type
				{
					TYPE_A,
					TYPE_B,
					TYPE_C
				};

				Bandai(Context&,Type);
				~Bandai();

			private:

				class DatachJointSystem;

				template<uint N> class E24C0X;
				static bool HasEEPROM(uint,Type,dword);

				void SubReset(bool);
				void BaseSave(State::Saver&) const;
				void BaseLoad(State::Loader&,dword);
				void VSync();
				Device QueryDevice(DeviceType);

				NES_DECL_PEEK( 6000_A1 )
				NES_DECL_PEEK( 6000_A2 )
				NES_DECL_PEEK( 6000_C  )
				NES_DECL_POKE( 8000_B  )
				NES_DECL_POKE( 8008_B  )
				NES_DECL_POKE( 8000_C  )
				NES_DECL_POKE( 800A    )
				NES_DECL_POKE( 800B    )
				NES_DECL_POKE( 800C    )
				NES_DECL_POKE( 800D_A1 )
				NES_DECL_POKE( 800D_A2 )
				NES_DECL_POKE( 800D_C  )

				struct Irq
				{
					void Reset(bool);
					ibool Signal();

					uint count;
					uint latch;
				};

				Clock::M2<Irq> irq;
				DatachJointSystem* const datach;
				E24C0X<128>* const e24C01;
				E24C0X<256>* const e24C02;
				const Type type;
			};
		}
	}
}

#endif
