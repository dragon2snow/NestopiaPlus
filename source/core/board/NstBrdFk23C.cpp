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
#include "NstBrdFk23C.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			void Fk23C::SubReset(const bool hard)
			{
				if (hard)
					dipSwitch = 0x0;
				else
					dipSwitch = (dipSwitch + 1) & 0x3;

				exRegs[3] = exRegs[2] = exRegs[1] = exRegs[0] = 0x00;
				exRegs[7] = exRegs[6] = exRegs[5] = exRegs[4] = 0xFF;
				unromChr = 0x0;

				Mmc3::SubReset( hard );

				Map( 0x5001U, 0x5FFFU, &Fk23C::Poke_5001 );
				Map( 0x8000U, 0xFFFFU, &Fk23C::Poke_Prg  );
			}

			void Fk23C::SubLoad(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					{
						const State::Loader::Data<9> data( state );

						for (uint i=0; i < 8; ++i)
							exRegs[i] = data[i];

						unromChr = data[8] & 0x3;
						dipSwitch = data[8] >> 2 & 0x3;
					}

					state.End();
				}
			}

			void Fk23C::SubSave(State::Saver& state) const
			{
				const u8 data[] =
				{
					exRegs[0],
					exRegs[1],
					exRegs[2],
					exRegs[3],
					exRegs[4],
					exRegs[5],
					exRegs[6],
					exRegs[7],
					unromChr | dipSwitch << 2
				};

				state.Begin('R','E','G','\0').Write( data ).End();
			}

			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void Fk23C::UpdatePrg()
			{
				if (exRegs[0] & 0x4)
				{
					prg.SwapBank<SIZE_32K,0x0000U>( exRegs[1] >> 1 );
				}
				else
				{
					const uint exBanks[2] =
					{
						(exRegs[0] & 0x2) ? 0x3F >> (exRegs[0] & 0x3) : 0xFF,
						(exRegs[0] & 0x2) ? exRegs[1] << 1 : 0x00
					};

					const uint i = (regs.ctrl0 & Regs::CTRL0_XOR_PRG) >> 5;

					prg.SwapBanks<SIZE_8K,0x0000U>
					(
						(banks.prg[i] & exBanks[0]) | exBanks[1],
						(banks.prg[1] & exBanks[0]) | exBanks[1],
						(exRegs[3] & 0x2) ? exRegs[4] : (banks.prg[i^2] & exBanks[0]) | exBanks[1],
						(exRegs[3] & 0x2) ? exRegs[5] : (banks.prg[3]   & exBanks[0]) | exBanks[1]
					);
				}
			}

			void Fk23C::UpdateChr() const
			{
				ppu.Update();

				if (exRegs[0] & 0x40)
				{
					chr.SwapBank<SIZE_8K,0x0000U>( exRegs[2] | unromChr );
				}
				else
				{
					uint base = (exRegs[2] & 0x7F) << 2;
					const uint swap = (regs.ctrl0 & Regs::CTRL0_XOR_CHR) << 5;

					chr.SwapBanks<SIZE_2K>( 0x0000U ^ swap, base | banks.chr[0], base | banks.chr[1] );
					base <<= 1;
					chr.SwapBanks<SIZE_1K>( 0x1000U ^ swap, base | banks.chr[2], base | banks.chr[3], base | banks.chr[4], base | banks.chr[5] );

					if (exRegs[3] & 0x2)
					{
						chr.SwapBank<SIZE_1K,0x0400U>( base | exRegs[6] );
						chr.SwapBank<SIZE_1K,0x0C00U>( base | exRegs[7] );
					}
				}
			}

			NES_POKE(Fk23C,5001)
			{
				if ((address & (1U << (dipSwitch+4))))
				{
					exRegs[address & 0x3] = data;
					Fk23C::UpdatePrg();
					Fk23C::UpdateChr();
				}
			}

			NES_POKE(Fk23C,Prg)
			{
				if (exRegs[0] & 0x40)
				{
					unromChr = (exRegs[0] & 0x20) ? 0x0 : data & 0x3;
					Fk23C::UpdateChr();
				}
				else switch (address & 0xE001)
				{
					case 0x8000: NES_CALL_POKE(Mmc3,8000,address,data); break;
					case 0x8001:

						if (exRegs[3] << 2 & regs.ctrl0 & 0x8)
						{
							exRegs[4 + (regs.ctrl0 & 0x3)] = data;
							Fk23C::UpdatePrg();
							Fk23C::UpdateChr();
						}
						else
						{
							NES_CALL_POKE(Mmc3,8001,address,data);
						}
						break;

					case 0xA000:
					case 0xA001: NES_CALL_POKE(Mmc3,A001,address,data); break;
					case 0xC000: NES_CALL_POKE(Mmc3,C000,address,data); break;
					case 0xC001: NES_CALL_POKE(Mmc3,C001,address,data); break;
					case 0xE000: NES_CALL_POKE(Mmc3,E000,address,data); break;
					case 0xE001: NES_CALL_POKE(Mmc3,E001,address,data); break;

					NST_UNREACHABLE
				}
			}
		}
	}
}
