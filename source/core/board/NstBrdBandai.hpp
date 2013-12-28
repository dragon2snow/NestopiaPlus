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

#ifndef NST_BOARDS_BANDAI_H
#define NST_BOARDS_BANDAI_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class Bandai : public Mapper
			{
			protected:

				enum Type
				{
					TYPE_FCG1,
					TYPE_FCG2,
					TYPE_LZ93D50_E2401,
					TYPE_LZ93D50_E2402,
					TYPE_WRAM,
					TYPE_DATACH
				};

				Bandai(Context&,Type);
				~Bandai();

			private:

				class DatachJointSystem;
				class Eeprom;

				void SubReset(bool);
				void BaseSave(State::Saver&) const;
				void BaseLoad(State::Loader&,dword);
				void Sync(Event,Input::Controllers*);
				Device QueryDevice(DeviceType);

				NES_DECL_PEEK( 6000_A1 );
				NES_DECL_PEEK( 6000_A2 );
				NES_DECL_PEEK( 6000_C  );
				NES_DECL_POKE( 8000_B  );
				NES_DECL_POKE( 8008_B  );
				NES_DECL_POKE( 8000_C  );
				NES_DECL_POKE( 800A    );
				NES_DECL_POKE( 800B    );
				NES_DECL_POKE( 800C    );
				NES_DECL_POKE( 800D_A1 );
				NES_DECL_POKE( 800D_A2 );
				NES_DECL_POKE( 800D_C  );

				struct Irq
				{
					void Reset(bool);
					bool Clock();

					uint count;
					uint latch;
				};

				ClockUnits::M2<Irq> irq;
				Pointer<DatachJointSystem> datach;
				Pointer<Eeprom> eeprom;
				const Type type;
			};
		}
	}
}

#endif
