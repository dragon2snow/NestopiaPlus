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
#include "NstMapper142.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper142::Mapper142(Context& c)
		:
		Mapper (c,PROM_MAX_128K|CROM_MAX_8K|WRAM_NONE),
		irq    (c.cpu)
		{}

		void Mapper142::Irq::Reset(const bool hard)
		{
			if (hard)
				count = 0;
		}

		void Mapper142::SubReset(const bool hard)
		{
			if (hard)
				ctrl = 0;

			irq.Reset( hard, true );

			Map( WRK_PEEK );
			Map( 0x8000U, 0x8FFFU, &Mapper142::Poke_8000 );
			Map( 0x9000U, 0x9FFFU, &Mapper142::Poke_9000 );
			Map( 0xA000U, 0xAFFFU, &Mapper142::Poke_A000 );
			Map( 0xB000U, 0xBFFFU, &Mapper142::Poke_B000 );
			Map( 0xC000U, 0xCFFFU, &Mapper142::Poke_C000 );
			Map( 0xE000U, 0xEFFFU, &Mapper142::Poke_E000 );
			Map( 0xF000U, 0xFFFFU, &Mapper142::Poke_F000 );
		}

		void Mapper142::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case AsciiId<'R','E','G'>::V:

						ctrl = state.Read8();
						break;

					case AsciiId<'I','R','Q'>::V:
					{
						State::Loader::Data<3> data( state );

						irq.unit.enabled = data[0] & 0x1;
						irq.unit.count = data[1] | data[2] << 8;

						break;
					}
				}

				state.End();
			}
		}

		void Mapper142::SubSave(State::Saver& state) const
		{
			state.Begin( AsciiId<'R','E','G'>::V ).Write8( ctrl ).End();

			const byte data[3] =
			{
				irq.unit.enabled ? 0x1 : 0x0,
				irq.unit.count & 0xFF,
				irq.unit.count >> 8
			};

			state.Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE(Mapper142,8000)
		{
			irq.Update();
			irq.unit.count = (irq.unit.count & 0xFFF0) | (data & 0xF) << 0;
		}

		NES_POKE(Mapper142,9000)
		{
			irq.Update();
			irq.unit.count = (irq.unit.count & 0xFF0F) | (data & 0xF) << 4;
		}

		NES_POKE(Mapper142,A000)
		{
			irq.Update();
			irq.unit.count = (irq.unit.count & 0xF0FF) | (data & 0xF) << 8;
		}

		NES_POKE(Mapper142,B000)
		{
			irq.Update();
			irq.unit.count = (irq.unit.count & 0x0FFF) | (data & 0xF) << 12;
		}

		NES_POKE(Mapper142,C000)
		{
			irq.Update();
			irq.unit.enabled = data & 0xF;
			irq.ClearIRQ();
		}

		NES_POKE(Mapper142,E000)
		{
			ctrl = data;
		}

		NES_POKE(Mapper142,F000)
		{
			address = (ctrl & 0xF) - 1;

			if (address < 3)
			{
				prg.SwapBank<SIZE_8K>( address << 13, data );
			}
			else if (address < 4)
			{
				wrk.SwapBank<SIZE_8K,0x0000>( data );
			}
		}

		ibool Mapper142::Irq::Signal()
		{
			if (enabled && count++ == 0xFFFF)
			{
				enabled = false;
				count = 0;
				return true;
			}

			return false;
		}

		void Mapper142::VSync()
		{
			irq.VSync();
		}
	}
}
