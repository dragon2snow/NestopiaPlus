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
#include "NstMapper114.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper114::SubReset(const bool hard)
		{
			exRegs[0] = 0x00;
			exRegs[1] = false;

			Mmc3::SubReset( hard );

			Map( 0x5000U, 0x7FFFU, &Mapper114::Poke_5000 );
			Map( 0x8000U, 0x9FFFU, NMT_SWAP_HV           );
			Map( 0xA000U, 0xBFFFU, &Mapper114::Poke_A000 );
			Map( 0xC000U, 0xDFFFU, &Mapper114::Poke_C000 );
			Map( 0xE000U, 0xFFFFU, NOP_POKE              );
			Map( 0xE002U,          &Mapper114::Poke_E000 );
			Map( 0xE003U,          &Mapper114::Poke_E003 );
		}

		void Mapper114::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == AsciiId<'R','E','G'>::V)
				{
					State::Loader::Data<2> data( state );

					exRegs[0] = data[0];
					exRegs[1] = data[1] & 0x1;
				}

				state.End();
			}
		}

		void Mapper114::SubSave(State::Saver& state) const
		{
			const byte data[2] =
			{
				exRegs[0],
				exRegs[1]
			};

			state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE_D(Mapper114,5000)
		{
			exRegs[0] = data;

			if (data & 0x80)
			{
				data &= 0x1F;
				prg.SwapBanks<SIZE_16K,0x0000>( data, data );
			}
			else
			{
				Mmc3::UpdatePrg();
			}
		}

		NES_POKE_D(Mapper114,A000)
		{
			static const byte security[8] = {0,3,1,5,6,7,2,4};

			data = (data & 0xC0) | security[data & 0x07];
			exRegs[1] = true;

			Mmc3::NES_DO_POKE(8000,0x8000,data);
		}

		NES_POKE_D(Mapper114,C000)
		{
			if (exRegs[1] && ((exRegs[0] & 0x80) == 0 || (regs.ctrl0 & Regs::CTRL0_MODE) < 6))
			{
				exRegs[1] = false;
				Mmc3::NES_DO_POKE(8001,0x8001,data);
			}
		}

		NES_POKE_D(Mapper114,E003)
		{
			Mmc3::NES_DO_POKE(E001,0xE001,data);
			Mmc3::NES_DO_POKE(C000,0xC000,data);
			Mmc3::NES_DO_POKE(C001,0xC001,data);
		}
	}
}
