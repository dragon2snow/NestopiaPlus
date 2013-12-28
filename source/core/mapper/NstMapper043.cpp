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

#include "../NstMapper.hpp"
#include "../NstClock.hpp"
#include "NstMapper043.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper43::Mapper43(Context& c)
		: Mapper(c,WRAM_NONE), irq(c.cpu) {}

		void Mapper43::SubReset(const bool hard)
		{
			if (hard)
			{
				title = 0xB000U;
				prg.SwapBanks<SIZE_8K,0x0000U>( 1, 0, 0, 9 );
			}
			else if (title == 0xB000U)
			{
				title = 0xC000U;
			}
			else
			{
				title = 0xB000U;
			}

			irq.Reset( hard, true );

			for (dword i=0x4022U; i < 0x10000UL; i += 0x100)
			{
				switch (i & 0x71FFU)
				{
					case 0x0122U: Map( i, &Mapper43::Poke_4122 ); break;
					case 0x4022U: Map( i, &Mapper43::Poke_4022 ); break;
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
					case NES_STATE_CHUNK_ID('I','R','Q','\0'):
					{
						State::Loader::Data<3> data( state );
						irq.unit.enabled = data[0] & 0x1;
						irq.unit.count = data[1] | ((data[2] & 0xF) << 8);
						break;
					}

					case NES_STATE_CHUNK_ID('T','T','L','\0'):

						title = (state.Read8() & 0x1) ? 0xC000U : 0xB000U;
						break;
				}

				state.End();
			}
		}

		void Mapper43::SubSave(State::Saver& state) const
		{
			state.Begin('T','T','L','\0').Write8( title == 0xC000U ? 0x1 : 0x0 ).End();

			const u8 data[3] =
			{
				irq.unit.enabled != 0,
				irq.unit.count & 0xFF,
				irq.unit.count >> 8
			};

			state.Begin('I','R','Q','\0').Write( data ).End();
		}

		#ifdef NST_PRAGMA_OPTIMIZE
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
				count = (count + 1) & 0xFFFU;

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
			static const u8 banks[8] = {4,3,4,4,4,7,5,6};
			prg.SwapBank<SIZE_8K,0x4000U>( banks[data & 0x7] );
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
			return *prg.Source().Mem( address - (0x6000U-0x4000U) );
		}

		void Mapper43::VSync()
		{
			irq.VSync();
		}
	}
}
