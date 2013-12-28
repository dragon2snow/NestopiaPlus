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
#include "NstMapper183.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper183::Mapper183(Context& c)
		:
		Mapper (c,CROM_MAX_256K|WRAM_NONE),
		irq    (c.cpu)
		{}

		void Mapper183::Irq::Reset(const bool hard)
		{
			if (hard)
			{
				enabled = false;
				count[0] = 0;
				count[1] = 0;
			}
		}

		void Mapper183::SubReset(const bool hard)
		{
			irq.Reset( hard, true );

			Map( WRK_PEEK );

			for (uint i=0x0000; i < 0x0800; i += 0x10)
			{
				Map( 0x8800 + i, 0x8803 + i, PRG_SWAP_8K_0 );
				Map( 0x9800 + i, 0x9803 + i, NMT_SWAP_VH01 );
				Map( 0xA000 + i, 0xA003 + i, PRG_SWAP_8K_2 );
				Map( 0xA800 + i, 0xA803 + i, PRG_SWAP_8K_1 );
			}

			for (uint i=0x000; i < 0x1000; i += 0x10)
			{
				Map( 0xB000 + i, 0xE00F + i, &Mapper183::Poke_B000 );
				Map( 0xF000 + i, 0xF003 + i, &Mapper183::Poke_F000 );
				Map( 0xF004 + i, 0xF007 + i, &Mapper183::Poke_F004 );
				Map( 0xF008 + i, 0xF00B + i, &Mapper183::Poke_F008 );
				Map( 0xF00C + i, 0xF00F + i, &Mapper183::Poke_F00C );
			}
		}

		void Mapper183::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == AsciiId<'I','R','Q'>::V)
				{
					State::Loader::Data<2> data( state );

					irq.unit.enabled = data[0] & 0x1;
					irq.unit.count[1] = data[1];
				}

				state.End();
			}
		}

		void Mapper183::SubSave(State::Saver& state) const
		{
			state.Begin( AsciiId<'I','R','Q'>::V ).Write16( (irq.unit.enabled != false) | (irq.unit.count[1] & 0xFF) << 8 ).End();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		ibool Mapper183::Irq::Signal()
		{
			if (++count[0] < 114)
				return false;

			count[0] = 0;
			return enabled && (++count[1] & 0xFF) == 0;
		}

		NES_POKE(Mapper183,B000)
		{
			ppu.Update();
			const uint part = address & 0x4;
			address = ((address - 0x3000) >> 1 | (address << 7)) & 0x1C00;
			chr.SwapBank<SIZE_1K>( address, (chr.GetBank<SIZE_1K>(address) & 0xF0 >> part) | (data & 0x0F) << part );
		}

		NES_POKE(Mapper183,F000)
		{
			irq.Update();
			irq.unit.count[1] = (irq.unit.count[1] & 0xF0) | (data << 0 & 0x0F);
		}

		NES_POKE(Mapper183,F004)
		{
			irq.Update();
			irq.unit.count[1] = (irq.unit.count[1] & 0x0F) | (data << 4 & 0xF0);
		}

		NES_POKE(Mapper183,F008)
		{
			irq.Update();
			irq.unit.enabled = data;

			if (!data)
				irq.ClearIRQ();
		}

		NES_POKE(Mapper183,F00C)
		{
		}

		void Mapper183::VSync()
		{
			irq.VSync();
			irq.unit.count[0] = 0;
		}
	}
}
