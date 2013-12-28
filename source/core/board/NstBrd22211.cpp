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
#include "NstBrd22211.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Unl22211::Unl22211(Context& c,Type t)
			:
			Mapper (c,PROM_MAX_64K|CROM_MAX_32K|WRAM_DEFAULT),
			type   (t)
			{}

			void Unl22211::SubReset(const bool hard)
			{
				if (hard)
				{
					for (uint i=0; i < 4; ++i)
						regs[i] = 0;
				}

				Map( 0x4100U,          &Unl22211::Peek_4100 );
				Map( 0x4100U, 0x4103U, &Unl22211::Poke_4100 );
				Map( 0x8000U, 0xFFFFU, &Unl22211::Poke_8000 );
			}

			void Unl22211::SubLoad(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					if (chunk == AsciiId<'R','E','G'>::V)
						state.Read( regs );

					state.End();
				}
			}

			void Unl22211::SubSave(State::Saver& state) const
			{
				state.Begin( AsciiId<'R','E','G'>::V ).Write( regs ).End();
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			NES_PEEK(Unl22211,4100)
			{
				return (uint(regs[1]) ^ regs[2]) | (type == TYPE_SECURITY_2 ? 0x41 : 0x40);
			}

			NES_POKE(Unl22211,4100)
			{
				regs[address & 0x3] = data;
			}

			NES_POKE(Unl22211,8000)
			{
				ppu.Update();
				prg.SwapBank<SIZE_32K,0x0000>( regs[2] >> 2 );
				chr.SwapBank<SIZE_8K,0x0000>( (type == TYPE_SECURITY_1) ? ((data^regs[2]) >> 3 & 0x2) | ((data^regs[2]) >> 5 & 0x1) : regs[2] );
			}
		}
	}
}
