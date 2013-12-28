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
#include "NstMapper112.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper112::SubReset(const bool hard)
		{
			if (hard)
				command = 0;

			for (uint i=0x0000U; i < 0x2000U; i += 0x2)
			{
				Map( 0x8000U + i, &Mapper112::Poke_8000 );
				Map( 0xA000U + i, &Mapper112::Poke_A000 );
				Map( 0xC000U + i, &Mapper112::Poke_C000 );
				Map( 0xE000U + i, NMT_SWAP_HV );
			}
		}

		void Mapper112::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					command = state.Read8();

				state.End();
			}
		}

		void Mapper112::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write8( command ).End();
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE(Mapper112,8000)
		{
			command = data;
		}

		NES_POKE(Mapper112,A000)
		{
			address = command & 0x7;

			if (address < 2)
			{
				prg.SwapBank<SIZE_8K>( address << 13, data );
			}
			else
			{
				ppu.Update();

				if (address < 4)
					chr.SwapBank<SIZE_2K>( (address-2) << 11, data >> 1 );
				else
					chr.SwapBank<SIZE_1K>( address << 10, data );
			}
		}

		NES_POKE(Mapper112,C000)
		{
			// CHR scrambling ?
		}
	}
}
