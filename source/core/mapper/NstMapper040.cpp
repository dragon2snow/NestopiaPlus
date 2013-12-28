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
#include "NstMapper040.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper40::Mapper40(Context& c)
		:
		Mapper (c,PROM_MAX_64K|CROM_MAX_8K|WRAM_NONE),
		irq    (c.cpu)
		{}

		void Mapper40::SubReset(const bool hard)
		{
			if (hard)
				prg.SwapBanks<SIZE_8K,0x0000>( 4, 5, 0, 7 );

			irq.Reset( hard, true );

			Map( 0x6000U, 0x7FFFU, &Mapper40::Peek_6000 );
			Map( 0x8000U, 0x9FFFU, &Mapper40::Poke_8000 );
			Map( 0xA000U, 0xBFFFU, &Mapper40::Poke_A000 );
			Map( 0xE000U, 0xFFFFU, &Mapper40::Poke_E000 );
		}

		void Mapper40::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == AsciiId<'I','R','Q'>::V)
				{
					State::Loader::Data<3> data( state );

					irq.unit.enabled = data[0] & 0x1U;
					irq.unit.count = data[1] | (data[2] << 8 & 0xF00);
				}

				state.End();
			}
		}

		void Mapper40::SubSave(State::Saver& state) const
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

		void Mapper40::Irq::Reset(const bool hard)
		{
			if (hard)
			{
				enabled = false;
				count = 0;
			}
		}

		bool Mapper40::Irq::Clock()
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

		NES_PEEK_A(Mapper40,6000)
		{
			return *prg.Source().Mem( (SIZE_64K-SIZE_16K-0x6000) + address );
		}

		NES_POKE(Mapper40,8000)
		{
			irq.Update();
			irq.unit.enabled = false;
			irq.unit.count = 0;
			irq.ClearIRQ();
		}

		NES_POKE(Mapper40,A000)
		{
			irq.Update();
			irq.unit.enabled = true;
		}

		NES_POKE_D(Mapper40,E000)
		{
			prg.SwapBank<SIZE_8K,0x4000>(data & 0x7);
		}

		void Mapper40::Sync(Event event,Input::Controllers*)
		{
			if (event == EVENT_END_FRAME)
				irq.VSync();
		}
	}
}
