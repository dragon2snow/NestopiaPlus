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

#include "../NstMapper.hpp"
#include "../NstClock.hpp"
#include "NstMapper043.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper43::Mapper43(Context& c)
		:
		Mapper (c,CROM_MAX_8K|WRAM_NONE),
		prg2   (prg.Source().Size() - (SIZE_8K + 0x6000U)),
		irq    (c.cpu)
		{}

		void Mapper43::SubReset(const bool hard)
		{
			irq.Reset( hard, true );

			prg.SwapBank<SIZE_32K,0x0000>( prg.Source().Size() >= SIZE_64K );

			if (prg.Source().Size() >= SIZE_64K)
				Map( 0x4022U, &Mapper43::Poke_4022 );

			Map( 0x4122U,          &Mapper43::Poke_4122 );
			Map( 0x5000U, 0x7FFFU, &Mapper43::Peek_5000 );
		}

		void Mapper43::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == AsciiId<'I','R','Q'>::V)
				{
					State::Loader::Data<3> data( state );

					irq.unit.enabled = data[0] & 0x1;
					irq.unit.count = data[1] | (data[2] << 8 & 0xF00);
				}

				state.End();
			}
		}

		void Mapper43::SubSave(State::Saver& state) const
		{
			const byte data[3] =
			{
				irq.unit.enabled != 0,
				irq.unit.count & 0xFF,
				irq.unit.count >> 8
			};

			state.Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Mapper43::Irq::Reset(const bool hard)
		{
			if (hard)
			{
				enabled = false;
				count = 0;
			}
		}

		bool Mapper43::Irq::Clock()
		{
			if (enabled)
			{
				count = (count + 1) & 0xFFF;

				if (!count)
				{
					enabled = false;
					return true;
				}
			}

			return false;

		}

		NES_POKE_D(Mapper43,4022)
		{
			prg.SwapBank<SIZE_32K,0x0000>( data & 0x1 );
		}

		NES_POKE_D(Mapper43,4122)
		{
			irq.Update();
			irq.ClearIRQ();
			irq.unit.enabled = data & 0x3;
			irq.unit.count = 0;
		}

		NES_PEEK_A(Mapper43,5000)
		{
			return *prg.Source().Mem( address + prg2 );
		}

		void Mapper43::Sync(Event event,Input::Controllers*)
		{
			if (event == EVENT_END_FRAME)
				irq.VSync();
		}
	}
}
