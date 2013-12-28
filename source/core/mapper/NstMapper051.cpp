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
#include "NstMapper051.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper51::SubReset(const bool hard)
		{
			Map( WRK_PEEK );
			Map( 0x6000U, 0x7FFFU, &Mapper51::Poke_6000 );
			Map( 0x8000U, 0xBFFFU, &Mapper51::Poke_8000 );
			Map( 0xC000U, 0xDFFFU, &Mapper51::Poke_C000 );
			Map( 0xE000U, 0xFFFFU, &Mapper51::Poke_8000 );

			if (hard)
			{
				bank = 0x0;
				mode = 0x1;

				UpdateBanks();
			}
		}

		void Mapper51::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == AsciiId<'R','E','G'>::V)
				{
					{
						const uint data = state.Read8();
						mode = data & 0x3;
						bank = data >> 4;
					}

					UpdateBanks();
				}

				state.End();
			}
		}

		void Mapper51::SubSave(State::Saver& state) const
		{
			state.Begin( AsciiId<'R','E','G'>::V ).Write8( mode | (bank << 4) ).End();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Mapper51::UpdateBanks()
		{
			uint offset;

			if (mode & 0x1)
			{
				prg.SwapBank<SIZE_32K,0x0000>( bank );
				offset = 0x23;
			}
			else
			{
				prg.SwapBanks<SIZE_16K,0x0000>( (bank << 1) | (mode >> 1), bank << 1 | 0x7 );
				offset = 0x2F;
			}

			wrk.SwapBank<SIZE_8K,0x0000>( offset | (bank << 2) );
			ppu.SetMirroring( (mode == 0x3) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
		}

		NES_POKE_D(Mapper51,6000)
		{
			mode = (data >> 3 & 0x2) | (data >> 1 & 0x1);
			UpdateBanks();
		}

		NES_POKE_D(Mapper51,8000)
		{
			bank = data & 0xF;
			UpdateBanks();
		}

		NES_POKE_D(Mapper51,C000)
		{
			bank = data & 0xF;
			mode = (data >> 3 & 0x2) | (mode & 0x1);
			UpdateBanks();
		}
	}
}
