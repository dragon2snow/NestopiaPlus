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
#include "NstBrd8237.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			void Unl8237::SubReset(const bool hard)
			{
				if (hard)
				{
					exRegs[0] = 0x00;
					exRegs[1] = 0x00;
				}

				exRegs[2] = false;

				Mmc3::SubReset( hard );

				Map( 0x5000U,          &Unl8237::Poke_5000 );
				Map( 0x5001U,          &Unl8237::Poke_5001 );
				Map( 0x8000U, 0x9FFFU, &Unl8237::Poke_8000 );
				Map( 0xA000U, 0xBFFFU, &Unl8237::Poke_A000 );
				Map( 0xC000U, 0xDFFFU, &Unl8237::Poke_C000 );
				Map( 0xE000U, 0xEFFFU, &Unl8237::Poke_E000 );
				Map( 0xF000U, 0xFFFFU, &Unl8237::Poke_F000 );
			}

			void Unl8237::SubLoad(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					{
						const State::Loader::Data<2> data( state );

						exRegs[0] = data[0];
						exRegs[1] = (data[1] & 0x4) << 6;
						exRegs[2] = (data[1] & 0x1);
					}

					state.End();
				}
			}

			void Unl8237::SubSave(State::Saver& state) const
			{
				const u8 data[2] =
				{
					exRegs[0],
					(exRegs[1] >> 6) | exRegs[2],
				};

				state.Begin('R','E','G','\0').Write( data ).End();
			}

			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void Unl8237::UpdatePrg()
			{
				if (exRegs[0] & 0x80)
				{
					const uint bank = exRegs[0] & 0xF;

					if (exRegs[0] & 0x20)
						prg.SwapBank<SIZE_32K,0x0000U>( bank >> 1 );
					else
						prg.SwapBanks<SIZE_16K,0x0000U>( bank, bank );
				}
				else
				{
					Mmc3::UpdatePrg();
				}
			}

			void Unl8237::UpdateChr() const
			{
				ppu.Update();

				const uint swap = (regs.ctrl0 & Regs::CTRL0_XOR_CHR) << 5;

				chr.SwapBanks<SIZE_2K>
				(
					0x0000U ^ swap,
					exRegs[1] >> 1 | banks.chr[0],
					exRegs[1] >> 1 | banks.chr[1]
				);

				chr.SwapBanks<SIZE_1K>
				(
					0x1000U ^ swap,
					exRegs[1] | banks.chr[2],
					exRegs[1] | banks.chr[3],
					exRegs[1] | banks.chr[4],
					exRegs[1] | banks.chr[5]
				);
			}

			NES_POKE(Unl8237,5000)
			{
				if (exRegs[0] != data)
				{
					exRegs[0] = data;
					UpdatePrg();
				}
			}

			NES_POKE(Unl8237,5001)
			{
				data = data << 6 & 0x100;

				if (exRegs[1] != data)
				{
					exRegs[1] = data;
					UpdateChr();
				}
			}

			NES_POKE(Unl8237,8000)
			{
				SetMirroringHV( data >> 7 | data );
			}

			NES_POKE(Unl8237,A000)
			{
				static const u8 lut[8] = {0,2,6,1,7,3,4,5};

				data = (data & 0xC0) | lut[data & 0x07];
				exRegs[2] = true;

				NES_CALL_POKE(Mmc3,8000,0x8000U,data);
			}

			NES_POKE(Unl8237,C000)
			{
				if (exRegs[2])
				{
					exRegs[2] = false;
					NES_CALL_POKE(Mmc3,8001,0x8001U,data);
				}
			}

			NES_POKE(Unl8237,F000)
			{
				NES_CALL_POKE(Mmc3,E001,0xE001U,data);
				NES_CALL_POKE(Mmc3,C000,0xC000U,data);
				NES_CALL_POKE(Mmc3,C001,0xC001U,data);
			}
		}
	}
}
