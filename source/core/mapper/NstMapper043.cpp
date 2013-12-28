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
		irq    (c.cpu)
		{}

		void Mapper43::SubReset(const bool hard)
		{
			if (hard)
			{
				title = 0xB000;
				prg.SwapBanks<SIZE_8K,0x0000>( 1, 0, 0, 9 );
			}
			else if (title == 0xB000)
			{
				title = 0xC000;
			}
			else
			{
				title = 0xB000;
			}

			irq.Reset( hard, true );

			for (dword i=0x4022; i <= 0xFFFF; i += 0x100)
			{
				switch (i & 0x71FF)
				{
					case 0x0122: Map( i, &Mapper43::Poke_4122 ); break;
					case 0x4022: Map( i, &Mapper43::Poke_4022 ); break;
				}
			}

			Map( 0x5000U, 0x5FFFU, &Mapper43::Peek_5000 );
			Map( 0x6000U, 0x7FFFU, &Mapper43::Peek_6000 );
		}

		void Mapper43::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case AsciiId<'I','R','Q'>::V:
					{
						State::Loader::Data<3> data( state );

						irq.unit.enabled = data[0] & 0x1;
						irq.unit.count = data[1] | (data[2] << 8 & 0xF00);

						break;
					}

					case AsciiId<'T','T','L'>::V:

						title = (state.Read8() & 0x1) ? 0xC000 : 0xB000;
						break;
				}

				state.End();
			}
		}

		void Mapper43::SubSave(State::Saver& state) const
		{
			state.Begin( AsciiId<'T','T','L'>::V ).Write8( title == 0xC000 ? 0x1 : 0x0 ).End();

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

		ibool Mapper43::Irq::Signal()
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

		NES_POKE(Mapper43,4022)
		{
			static const byte banks[8] = {4,3,4,4,4,7,5,6};
			prg.SwapBank<SIZE_8K,0x4000>( banks[data & 0x7] );
		}

		NES_POKE(Mapper43,4122)
		{
			irq.Update();
			irq.ClearIRQ();
			irq.unit.enabled = data & 0x3;
			irq.unit.count = 0;
		}

		NES_PEEK(Mapper43,5000)
		{
			return *prg.Source().Mem( address + title );
		}

		NES_PEEK(Mapper43,6000)
		{
			return *prg.Source().Mem( address - (0x6000-0x4000) );
		}

		void Mapper43::VSync()
		{
			irq.VSync();
		}
	}
}
