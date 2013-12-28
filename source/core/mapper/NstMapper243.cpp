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
#include "NstMapper243.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper243::SubReset(const bool hard)
		{
			if (hard)
				command = 0;

			for (uint i=0x4100U; i < 0x5000U; i += 0x2)
			{
				Map( i + 0x0, &Mapper243::Poke_4100 );
				Map( i + 0x1, &Mapper243::Poke_4101 );
			}
		}

		void Mapper243::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					command = state.Read8();

				state.End();
			}
		}

		void Mapper243::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write8( command ).End();
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE(Mapper243,4100)
		{
			command = data;
		}

		NES_POKE(Mapper243,4101)
		{
			ppu.Update();

			switch (command & 0x7)
			{
				case 0x0:

					prg.SwapBank<SIZE_32K,0x0000U>( 0x0 );
					chr.SwapBank<SIZE_8K,0x0000U>( 0x3 );
					break;

				case 0x2:

					chr.SwapBank<SIZE_8K,0x0000U>( (chr.GetBank<SIZE_8K,0x0000U>() & 0x7) | (data << 3 & 0x8) );
					break;

				case 0x4:

					chr.SwapBank<SIZE_8K,0x0000U>( (chr.GetBank<SIZE_8K,0x0000U>() & 0xE) | (data & 0x1) );
					break;

				case 0x5:

					prg.SwapBank<SIZE_32K,0x0000U>( data & 0x1 );
					break;

				case 0x6:

					chr.SwapBank<SIZE_8K,0x0000U>( (chr.GetBank<SIZE_8K,0x0000U>() & 0x9) | (data << 1 & 0x6) );
					break;

				case 0x7:

					nmt.SwapBank<SIZE_1K,0x0000U>( data & 0x1 );
					break;
			}
		}
	}
}
