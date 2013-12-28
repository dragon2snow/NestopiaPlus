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
#include "NstMapper014.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper14::SubReset(const bool hard)
		{
			exMode = 0;

			if (hard)
			{
				for (uint i=0; i < 2; ++i)
					exPrg[i] = 0;

				for (uint i=0; i < 8; ++i)
					exChr[i] = 0;

				exNmt = 0;
			}

			Mmc3::SubReset( hard );

			Map( 0x8000U, 0xFFFFU, &Mapper14::Poke_Prg );
		}

		void Mapper14::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == AsciiId<'R','E','G'>::V)
				{
					State::Loader::Data<12> data( state );

					exMode = data[0];

					for (uint i=0; i < 2; ++i)
						exPrg[i] = data[1+i];

					for (uint i=0; i < 8; ++i)
						exChr[i] = data[1+2+i];

					exNmt = data[11];
				}

				state.End();
			}
		}

		void Mapper14::SubSave(State::Saver& state) const
		{
			const byte data[12] =
			{
				exMode,
				exPrg[0],
				exPrg[1],
				exChr[0],
				exChr[1],
				exChr[2],
				exChr[3],
				exChr[4],
				exChr[5],
				exChr[6],
				exChr[7],
				exNmt
			};

			state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Mapper14::UpdatePrg()
		{
			if (exMode & 0x2)
				Mmc3::UpdatePrg();
			else
				prg.SwapBanks<SIZE_8K,0x0000>( exPrg[0], exPrg[1], ~1U, ~0U );
		}

		void Mapper14::UpdateChr() const
		{
			ppu.Update();

			if (exMode & 0x2)
			{
				const uint swap = (regs.ctrl0 & Regs::CTRL0_XOR_CHR) << 5;

				chr.SwapBanks<SIZE_2K>
				(
					0x0000 ^ swap,
					(exMode << 4 & 0x80U) | banks.chr[0],
					(exMode << 4 & 0x80U) | banks.chr[1]
				);

				chr.SwapBanks<SIZE_1K>
				(
					0x1000 ^ swap,
					(exMode << 3 & 0x100U) | banks.chr[2],
					(exMode << 3 & 0x100U) | banks.chr[3],
					(exMode << 1 & 0x100U) | banks.chr[4],
					(exMode << 1 & 0x100U) | banks.chr[5]
				);
			}
			else
			{
				chr.SwapBanks<SIZE_1K,0x0000>
				(
					exChr[0],
					exChr[1],
					exChr[2],
					exChr[3],
					exChr[4],
					exChr[5],
					exChr[6],
					exChr[7]
				);
			}
		}

		NES_POKE(Mapper14,Prg)
		{
			if ((address & 0xA131) == 0xA131 && exMode != data)
			{
				exMode = data;

				Mapper14::UpdatePrg();
				Mapper14::UpdateChr();

				if (!(exMode & 0x2))
					SetMirroringHV( exNmt );
			}

			if (exMode & 0x2)
			{
				switch (address & 0xE001)
				{
					case 0x8000: Mmc3::NES_DO_POKE( 8000, address, data ); break;
					case 0x8001: Mmc3::NES_DO_POKE( 8001, address, data ); break;
					case 0xA000: SetMirroringVH( exNmt );                  break;
					case 0xA001: Mmc3::NES_DO_POKE( A001, address, data ); break;
					case 0xC000: Mmc3::NES_DO_POKE( C000, address, data ); break;
					case 0xC001: Mmc3::NES_DO_POKE( C001, address, data ); break;
					case 0xE000: Mmc3::NES_DO_POKE( E000, address, data ); break;
					case 0xE001: Mmc3::NES_DO_POKE( E001, address, data ); break;
				}
			}
			else if (address >= 0xB000 && address <= 0xE003)
			{
				const uint offset = address << 2 & 0x4;
				address = ((((address & 0x2) | address >> 10) >> 1) + 2) & 0x7;
				exChr[address] = (exChr[address] & 0xF0U >> offset) | ((data & 0x0F) << offset);

				Mapper14::UpdateChr();
			}
			else switch (address & 0xF003)
			{
				case 0x8000:

					if (exPrg[0] != data)
					{
						exPrg[0] = data;
						Mapper14::UpdatePrg();
					}
					break;

				case 0x9000:

					if (exNmt != data)
					{
						exNmt = data;
						SetMirroringHV( exNmt );
					}
					break;

				case 0xA000:

					if (exPrg[1] != data)
					{
						exPrg[1] = data;
						Mapper14::UpdatePrg();
					}
					break;
			}
		}
	}
}
