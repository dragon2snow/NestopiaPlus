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
#include "NstMapper041.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper41::SubReset(const bool hard)
		{
			Map( 0x6000U, 0x67FFU, &Mapper41::Poke_6000 );
			Map( 0x8000U, 0xFFFFU, &Mapper41::Poke_Prg );

			if (hard)
				NES_DO_POKE(6000,0x6000,0x00);
		}

		void Mapper41::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == AsciiId<'R','E','G'>::V)
					reg = state.Read8();

				state.End();
			}
		}

		void Mapper41::SubSave(State::Saver& state) const
		{
			state.Begin( AsciiId<'R','E','G'>::V ).Write8( reg ).End();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE_A(Mapper41,6000)
		{
			reg = address & 0xFF;
			prg.SwapBank<SIZE_32K,0x0000>( address & 0x7 );
			ppu.SetMirroring( (address & 0x10) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
		}

		NES_POKE_D(Mapper41,Prg)
		{
			NST_VERIFY( reg & 0x4 );

			if (reg & 0x4)
			{
				ppu.Update();
				chr.SwapBank<SIZE_8K,0x0000>( (reg >> 1 & 0xC) | (data & 0x3) );
			}
		}
	}
}
