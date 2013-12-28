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
#include "NstBrdMmc3.hpp"
#include "NstBrdSl1632.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			void Sl1632::SubReset(const bool hard)
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

				Map( 0x8000U, 0xFFFFU, &Sl1632::Poke_Prg );
			}

			void Sl1632::SubLoad(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					{
						const State::Loader::Data<12> data( state );

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

			void Sl1632::SubSave(State::Saver& state) const
			{
				const u8 data[12] =
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

				state.Begin('R','E','G','\0').Write( data ).End();
			}

			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void Sl1632::UpdatePrg()
			{
				if (exMode & 0x2)
					Mmc3::UpdatePrg();
				else
					prg.SwapBanks<SIZE_8K,0x0000U>( exPrg[0], exPrg[1], ~1U, ~0U );
			}

			void Sl1632::UpdateChr() const
			{
				ppu.Update();

				if (exMode & 0x2)
				{
					const uint swap = (regs.ctrl0 & Regs::CTRL0_XOR_CHR) << 5;

					chr.SwapBanks<SIZE_2K>
					(
						0x0000U ^ swap,
						(exMode << 4 & 0x80) | banks.chr[0],
						(exMode << 4 & 0x80) | banks.chr[1]
					);

					chr.SwapBanks<SIZE_1K>
					(
						0x1000U ^ swap,
						(exMode << 3 & 0x100) | banks.chr[2],
						(exMode << 3 & 0x100) | banks.chr[3],
						(exMode << 1 & 0x100) | banks.chr[4],
						(exMode << 1 & 0x100) | banks.chr[5]
					);
				}
				else
				{
					chr.SwapBanks<SIZE_1K,0x0000U>
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

			NES_POKE(Sl1632,Prg)
			{
				if ((address & 0xA131U) == 0xA131U && exMode != data)
				{
					exMode = data;

					Sl1632::UpdatePrg();
					Sl1632::UpdateChr();

					if (!(exMode & 0x2))
						NES_CALL_POKE(Mmc3,Nmt_Hv,0xA000U,exNmt);
				}

				if (exMode & 0x2)
				{
					switch (address & 0xE001U)
					{
						case 0x8000U: NES_CALL_POKE(Mmc3,8000,address,data);   break;
						case 0x8001U: NES_CALL_POKE(Mmc3,8001,address,data);   break;
						case 0xA000U: NES_CALL_POKE(Mmc3,Nmt_Hv,address,data); break;
						case 0xA001U: NES_CALL_POKE(Mmc3,A001,address,data);   break;
						case 0xC000U: NES_CALL_POKE(Mmc3,C000,address,data);   break;
						case 0xC001U: NES_CALL_POKE(Mmc3,C001,address,data);   break;
						case 0xE000U: NES_CALL_POKE(Mmc3,E000,address,data);   break;
						case 0xE001U: NES_CALL_POKE(Mmc3,E001,address,data);   break;
					}
				}
				else if (address >= 0xB000U && address <= 0xE003U)
				{
					const uint offset = address << 2 & 0x4;
					address = ((((address & 0x2) | address >> 10) >> 1) + 2) & 0x7;
					exChr[address] = (exChr[address] & 0xF0 >> offset) | ((data & 0x0F) << offset);

					Sl1632::UpdateChr();
				}
				else switch (address & 0xF003U)
				{
					case 0x8000U:

						if (exPrg[0] != data)
						{
							exPrg[0] = data;
							Sl1632::UpdatePrg();
						}
						break;

					case 0x9000U:

						if (exNmt != data)
						{
							exNmt = data;
							NES_CALL_POKE(Sl1632,Nmt_Hv,address,data);
						}
						break;

					case 0xA000U:

						if (exPrg[1] != data)
						{
							exPrg[1] = data;
							Sl1632::UpdatePrg();
						}
						break;
				}
			}
		}
	}
}
