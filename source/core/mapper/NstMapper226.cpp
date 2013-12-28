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
#include "NstMapper226.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper226::SubReset(const bool hard)
		{
			Map( 0x8000U, 0xFFFFU, &Mapper226::Poke_Prg );

			if (hard)
			{
				regs[0] = 0;
				regs[1] = 0;

				NES_DO_POKE(Prg,0x8000,0x00);
			}
		}

		void Mapper226::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == AsciiId<'R','E','G'>::V)
				{
					State::Loader::Data<2> data( state );

					regs[0] = data[0];
					regs[1] = data[1];
				}

				state.End();
			}
		}

		void Mapper226::SubSave(State::Saver& state) const
		{
			state.Begin( AsciiId<'R','E','G'>::V ).Write16( regs[0] | uint(regs[1]) << 8 ).End();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE_AD(Mapper226,Prg)
		{
			regs[address & 0x1] = data;

			uint bank =
			(
				(regs[0] >> 1 & 0x0F) |
				(regs[0] >> 3 & 0x10) |
				(regs[1] << 5 & 0x20)
			);

			if (regs[0] & 0x20)
			{
				bank = (bank << 1) | (regs[0] & 0x1);
				prg.SwapBanks<SIZE_16K,0x0000>( bank, bank );
			}
			else
			{
				prg.SwapBank<SIZE_32K,0x0000>( bank );
			}

			ppu.SetMirroring( (regs[0] & 0x40) ? Ppu::NMT_VERTICAL : Ppu::NMT_HORIZONTAL );
		}
	}
}
