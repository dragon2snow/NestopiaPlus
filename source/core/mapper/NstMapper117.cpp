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
#include "../board/NstBrdMmc3.hpp"
#include "NstMapper117.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper117::Mapper117(Context& c)
		:
		Mapper (c,CROM_MAX_256K|WRAM_DEFAULT),
		irq    (c.cpu,c.ppu)
		{}

		void Mapper117::Irq::Reset(bool)
		{
			enabled = false;
			count = 0;
			latch = 0;
		}

		void Mapper117::SubReset(bool)
		{
			irq.Reset( true, true );

			Map( 0x8000U, PRG_SWAP_8K_0         );
			Map( 0x8001U, PRG_SWAP_8K_1         );
			Map( 0x8002U, PRG_SWAP_8K_2         );
			Map( 0x8003U, PRG_SWAP_8K_3         );
			Map( 0xA000U, CHR_SWAP_1K_0         );
			Map( 0xA001U, CHR_SWAP_1K_1         );
			Map( 0xA002U, CHR_SWAP_1K_2         );
			Map( 0xA003U, CHR_SWAP_1K_3         );
			Map( 0xA004U, CHR_SWAP_1K_4         );
			Map( 0xA005U, CHR_SWAP_1K_5         );
			Map( 0xA006U, CHR_SWAP_1K_6         );
			Map( 0xA007U, CHR_SWAP_1K_7         );
			Map( 0xC001U, &Mapper117::Poke_C001 );
			Map( 0xC002U, &Mapper117::Poke_C002 );
			Map( 0xC003U, &Mapper117::Poke_C003 );
			Map( 0xD000U, NMT_SWAP_HV           );
			Map( 0xE000U, &Mapper117::Poke_E000 );
		}

		void Mapper117::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == AsciiId<'I','R','Q'>::V)
				{
					State::Loader::Data<3> data( state );

					irq.unit.enabled = data[0] & 0x1;
					irq.unit.latch = data[1];
					irq.unit.count = data[2];
				}

				state.End();
			}
		}

		void Mapper117::SubSave(State::Saver& state) const
		{
			const byte data[3] =
			{
				irq.unit.enabled ? 0x1 : 0x0,
				irq.unit.latch,
				irq.unit.count
			};

			state.Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		bool Mapper117::Irq::Clock()
		{
			return (enabled && count && !--count);
		}

		NES_POKE_D(Mapper117,C001)
		{
			irq.Update();
			irq.unit.latch = data;
		}

		NES_POKE(Mapper117,C002)
		{
			cpu.ClearIRQ();
		}

		NES_POKE(Mapper117,C003)
		{
			irq.Update();
			irq.unit.count = irq.unit.latch;
		}

		NES_POKE_D(Mapper117,E000)
		{
			irq.Update();
			irq.unit.enabled = data & 0x1;
		}

		void Mapper117::Sync(Event event,Input::Controllers*)
		{
			if (event == EVENT_END_FRAME)
				irq.VSync();
		}
	}
}
