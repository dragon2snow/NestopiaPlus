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
#include "NstMapper053.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper53::Mapper53(Context& c)
		:
		Mapper      (c,WRAM_NONE|CROM_NONE),
		eepromFirst (c.pRomCrc == 0x7E449555UL)
		{}

		void Mapper53::SubReset(const bool hard)
		{
			if (hard)
			{
				regs[0] = regs[1] = 0;
				UpdatePrg();
			}

			Map( WRK_PEEK );
			Map( 0x6000U, 0x7FFFU, &Mapper53::Poke_6000 );
			Map( 0x8000U, 0xFFFFU, &Mapper53::Poke_8000  );
		}

		void Mapper53::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
				{
					{
						const State::Loader::Data<2> data( state );
						regs[0] = data[0];
						regs[1] = data[1];
					}

					UpdatePrg();
				}

				state.End();
			}
		}

		void Mapper53::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write16( regs[0] | (regs[1] << 8) ).End();
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Mapper53::UpdatePrg()
		{
			const uint r = regs[0] << 3 & 0x78;

			wrk.SwapBank<SIZE_8K,0x0000U>
			(
				(r << 1 | 0xF) + (eepromFirst ? 0x4 : 0x0)
			);

			prg.SwapBanks<SIZE_16K,0x0000U>
			(
				(regs[0] & 0x10) ? (r | (regs[1] & 0x7)) + (eepromFirst ? 0x2 : 0x0) : eepromFirst ? 0x00 : 0x80,
				(regs[0] & 0x10) ? (r | (0xFF    & 0x7)) + (eepromFirst ? 0x2 : 0x0) : eepromFirst ? 0x01 : 0x81
			);
		}

		NES_POKE(Mapper53,6000)
		{
			regs[0] = data;
			UpdatePrg();
			ppu.SetMirroring( (data & 0x20) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
		}

		NES_POKE(Mapper53,8000)
		{
			regs[1] = data;
			UpdatePrg();
		}
	}
}
