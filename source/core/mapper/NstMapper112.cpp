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
#include "NstMapper112.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper112::SubReset(const bool hard)
		{
			if (hard)
			{
				command = 0;

				for (uint i=0; i < 8; ++i)
					banks.chr[i] = 0;
			}

			for (uint i=0x0000; i < 0x2000; i += 0x2)
			{
				Map( 0x8000U + i, &Mapper112::Poke_8000 );
				Map( 0xA000U + i, &Mapper112::Poke_A000 );
				Map( 0xC000U + i, &Mapper112::Poke_C000 );
				Map( 0xE000U + i, &Mapper112::Poke_E000 );
			}
		}

		void Mapper112::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case AsciiId<'R','E','G'>::V:

						command = state.Read8();
						break;

					case AsciiId<'B','N','K'>::V:

						state.Read( banks.chr );
						break;
				}

				state.End();
			}
		}

		void Mapper112::SubSave(State::Saver& state) const
		{
			state.Begin( AsciiId<'R','E','G'>::V ).Write8( command ).End();
			state.Begin( AsciiId<'B','N','K'>::V ).Write( banks.chr ).End();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Mapper112::UpdateChr() const
		{
			ppu.Update();

			const uint extChr = (banks.chr[1] & 0x2U) ? banks.chr[0] : 0;

			chr.SwapBanks<SIZE_2K,0x0000>
			(
				banks.chr[2] | (extChr << 5 & 0x80),
				banks.chr[3] | (extChr << 4 & 0x80)
			);

			chr.SwapBanks<SIZE_1K,0x1000>
			(
				banks.chr[4] | (extChr << 4 & 0x100),
				banks.chr[5] | (extChr << 3 & 0x100),
				banks.chr[6] | (extChr << 2 & 0x100),
				banks.chr[7] | (extChr << 1 & 0x100)
			);
		}

		NES_POKE_D(Mapper112,8000)
		{
			command = data;
		}

		NES_POKE_D(Mapper112,A000)
		{
			const uint address = command & 0x7;

			if (address < 2)
			{
				prg.SwapBank<SIZE_8K>( address << 13, data );
			}
			else
			{
				banks.chr[address] = data >> uint(address < 4);
				UpdateChr();
			}
		}

		NES_POKE_D(Mapper112,C000)
		{
			banks.chr[0] = data;
			UpdateChr();
		}

		NES_POKE_D(Mapper112,E000)
		{
			banks.chr[1] = data;
			UpdateChr();
			SetMirroringHV( data );
		}
	}
}
