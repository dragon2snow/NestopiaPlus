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
#include "NstMapper057.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper57::SubReset(const bool hard)
		{
			if (hard)
				regs[2] = regs[1] = regs[0] = 0;
			else
				regs[0] = (regs[0] + 1) % 4;

			Map( 0x6000U, &Mapper57::Peek_6000 );

			for (dword i=0x8000U; i <= 0xFFFFU; i += 0x1000U)
			{
				Map( i+0x000, i+0x7FF, &Mapper57::Poke_8000 );
				Map( i+0x800, i+0xFFF, &Mapper57::Poke_8800 );
			}
		}

		void Mapper57::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
				{
					const State::Loader::Data<3> data( state );
					regs[0] = data[0] % 4;
					regs[1] = data[1];
					regs[2] = data[2];
				}

				state.End();
			}
		}

		void Mapper57::SubSave(State::Saver& state) const
		{
			const u8 data[3] = {regs[0],regs[1],regs[2]};
			state.Begin('R','E','G','\0').Write( data ).End();
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_PEEK(Mapper57,6000)
		{
			return regs[0];
		}

		void Mapper57::UpdateChr() const
		{
			chr.SwapBank<SIZE_8K,0x0000U>( (regs[1] >> 1 & 0x8) | (regs[1] & 0x7) | (regs[2] & 0x3) );
		}

		NES_POKE(Mapper57,8000)
		{
			regs[2] = data;
			ppu.Update();
			UpdateChr();
		}

		NES_POKE(Mapper57,8800)
		{
			regs[1] = data;
			prg.SwapBanks<SIZE_16K,0x0000U>( (data >> 5) & ~(data >> 7), (data >> 5) | (data >> 7) );
			ppu.SetMirroring( (data & 0x8) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
			UpdateChr();
		}
	}
}
