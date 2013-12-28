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
#include "NstMapper150.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper150::SubReset(const bool hard)
		{
			if (hard)
				command = 0x00;

			for (uint i=0x4100U; i < 0x6000U; ++i)
			{
				switch (i & 0x4101U)
				{
					case 0x4100U: Map( i, &Mapper150::Poke_4100 ); break;
					case 0x4101U: Map( i, &Mapper150::Poke_4101 ); break;
				}
			}
		}

		void Mapper150::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					command = state.Read8();

				state.End();
			}
		}

		void Mapper150::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write8( command ).End();
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE(Mapper150,4100)
		{
			command = data;
		}

		NES_POKE(Mapper150,4101)
		{
			uint banks[2] = {~0U,~0U};

			switch (command & 0x7)
			{
				case 0x2:

					banks[0] = data & 0x1;
					banks[1] = (chr.GetBank<SIZE_8K,0x0000U>() & ~0x8U) | (data << 3 & 0x8);
					break;

				case 0x4:

					banks[1] = (chr.GetBank<SIZE_8K,0x0000U>() & ~0x4U) | (data << 2 & 0x4);
					break;

				case 0x5:

					banks[0] = data & 0x7;
					break;

				case 0x6:

					banks[1] = (chr.GetBank<SIZE_8K,0x0000U>() & ~0x3U) | (data << 0 & 0x3);
					break;

				case 0x7:
				{
					static const u8 mirroring[4][4] =
					{
						{0,1,0,1},
						{0,0,1,1},
						{0,1,1,1},
						{0,0,0,0}
					};

					ppu.SetMirroring( mirroring[data >> 1 & 0x3] );
				}

				default: return;
			}

			if (banks[0] != ~0U)
				prg.SwapBank<SIZE_32K,0x0000U>( banks[0] );

			if (banks[1] != ~0U)
			{
				ppu.Update();
				chr.SwapBank<SIZE_8K,0x0000U>( banks[1] );
			}
		}
	}
}
