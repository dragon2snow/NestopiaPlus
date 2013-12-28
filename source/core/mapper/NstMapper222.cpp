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
#include "NstMapper222.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper222::Mapper222(Context& c)
		:
		Mapper (c,CROM_MAX_256K|WRAM_DEFAULT),
		irq    (c.cpu,c.ppu,Irq::SIGNAL_DURATION)
		{}

		void Mapper222::Irq::Reset(bool)
		{
			count = 0;
		}

		void Mapper222::SubReset(bool)
		{
			irq.Reset( true, true );

			for (uint i=0x0000U; i < 0x1000U; i += 0x4)
			{
				Map( 0x8000U + i, PRG_SWAP_8K_0 );
				Map( 0x9000U + i, NMT_SWAP_HV   );
				Map( 0xA000U + i, PRG_SWAP_8K_1 );
				Map( 0xB000U + i, CHR_SWAP_1K_0 );
				Map( 0xB002U + i, CHR_SWAP_1K_1 );
				Map( 0xC000U + i, CHR_SWAP_1K_2 );
				Map( 0xC002U + i, CHR_SWAP_1K_3 );
				Map( 0xD000U + i, CHR_SWAP_1K_4 );
				Map( 0xD002U + i, CHR_SWAP_1K_5 );
				Map( 0xE000U + i, CHR_SWAP_1K_6 );
				Map( 0xE002U + i, CHR_SWAP_1K_7 );
				Map( 0xF000U + i, &Mapper222::Poke_F000 );
			}
		}

		void Mapper222::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('I','R','Q','\0'))
					irq.unit.count = state.Read8();

				state.End();
			}
		}

		void Mapper222::SubSave(State::Saver& state) const
		{
			state.Begin('I','R','Q','\0').Write8( irq.unit.count ).End();
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		ibool Mapper222::Irq::Signal()
		{
			if (!count || ++count < 240)
				return false;

			count = 0;
			return true;
		}

		NES_POKE(Mapper222,F000)
		{
			irq.Update();
			irq.ClearIRQ();
			irq.unit.count = data;
		}

		void Mapper222::VSync()
		{
			irq.VSync();
		}
	}
}
