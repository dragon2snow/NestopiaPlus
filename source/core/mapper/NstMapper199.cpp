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
#include "../board/NstBrdMmc3Waixing.hpp"
#include "NstMapper199.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper199::SubReset(const bool hard)
		{
			exChr[1] = exChr[0] = 0;

			Mmc3Waixing::SubReset( hard );

			for (uint i=0x0000; i < 0x2000; i += 0x2)
				Map( i + 0x8001, &Mapper199::Poke_8001 );
		}

		void Mapper199::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == AsciiId<'E','C','H'>::V)
				{
					State::Loader::Data<2> data( state );

					exChr[0] = data[0];
					exChr[1] = data[1];
				}

				state.End();
			}
		}

		void Mapper199::SubSave(State::Saver& state) const
		{
			const byte data[] = { exChr[0], exChr[1] };
			state.Begin( AsciiId<'E','C','H'>::V ).Write( data ).End();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE(Mapper199,8001)
		{
			address = regs.ctrl0 & 0xF;

			switch (address)
			{
				case 0x0:
				case 0x1:
				case 0x2:
				case 0x3:
				case 0x4:
				case 0x5:

					banks.chr[address] = data;
					Mapper199::UpdateChr();
					break;

				case 0x6:
				case 0x7:
				case 0x8:
				case 0x9:

					banks.prg[address - 0x6] = data;
					Mmc3Waixing::UpdatePrg();
					break;

				case 0xA:
				case 0xB:

					exChr[address - 0xA] = data;
					Mapper199::UpdateChr();
					break;
			}
		}

		uint Mapper199::GetChrSource(uint bank) const
		{
			return bank - 8 <= 1;
		}

		void Mapper199::UpdateChr() const
		{
			ppu.Update();

			const uint swap = (regs.ctrl0 & Regs::CTRL0_XOR_CHR) << 5;

			chr.Source( banks.chr[0] <= 7 ).SwapBank<SIZE_1K>( 0x0000 ^ swap, banks.chr[0] );
			chr.Source( exChr[0]     <= 7 ).SwapBank<SIZE_1K>( 0x0400 ^ swap, exChr[0]     );
			chr.Source( banks.chr[1] <= 7 ).SwapBank<SIZE_1K>( 0x0800 ^ swap, banks.chr[1] );
			chr.Source( exChr[1]     <= 7 ).SwapBank<SIZE_1K>( 0x0C00 ^ swap, exChr[1]     );
			chr.Source( banks.chr[2] <= 7 ).SwapBank<SIZE_1K>( 0x1000 ^ swap, banks.chr[2] );
			chr.Source( banks.chr[3] <= 7 ).SwapBank<SIZE_1K>( 0x1400 ^ swap, banks.chr[3] );
			chr.Source( banks.chr[4] <= 7 ).SwapBank<SIZE_1K>( 0x1800 ^ swap, banks.chr[4] );
			chr.Source( banks.chr[5] <= 7 ).SwapBank<SIZE_1K>( 0x1C00 ^ swap, banks.chr[5] );
		}
	}
}
