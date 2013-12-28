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
#include "NstMapper063.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper63::SubReset(const bool hard)
		{
			Map( 0x8000U, 0xBFFFU, &Mapper63::Peek_Prg, &Mapper63::Poke_Prg );
			Map( 0x8000U, 0xFFFFU, &Mapper63::Poke_Prg );

			openBus = false;

			if (hard)
				NES_DO_POKE(Prg,0x8000,0x00);
		}

		void Mapper63::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == AsciiId<'I','R','Q'>::V)
					openBus = state.Read8() & 0x1;

				state.End();
			}
		}

		void Mapper63::SubSave(State::Saver& state) const
		{
			state.Begin( AsciiId<'R','E','G'>::V ).Write8( openBus ).End();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_PEEK_A(Mapper63,Prg)
		{
			return !openBus ? prg.Peek( address - 0x8000 ) : (address >> 8);
		}

		NES_POKE_A(Mapper63,Prg)
		{
			openBus = ((address & 0x300) == 0x300);

			prg.SwapBanks<SIZE_8K,0x0000>
			(
				(address >> 1 & 0x1FC) | ((address & 0x2) ? 0x0 : (address >> 1 & 0x2) | 0x0),
				(address >> 1 & 0x1FC) | ((address & 0x2) ? 0x1 : (address >> 1 & 0x2) | 0x1),
				(address >> 1 & 0x1FC) | ((address & 0x2) ? 0x2 : (address >> 1 & 0x2) | 0x0),

				(address & 0x800) ? (address >> 0 & 0x07C) | ((address & 0x6) ? 0x3 : 0x1) :
									(address >> 1 & 0x1FC) | ((address & 0x2) ? 0x3 : ((address >> 1 & 0x2) | 0x1))
			);

			ppu.SetMirroring( (address & 0x1) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
		}
	}
}
