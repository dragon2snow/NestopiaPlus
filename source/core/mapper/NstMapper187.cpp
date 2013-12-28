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
#include "../board/NstBrdMmc3.hpp"
#include "NstMapper187.hpp"
	 
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper187::SubReset(const bool hard)
		{
			if (hard)
			{
				exCtrl = 0;
				exMode = false;
				exLast = 0;
			}
	
			Mmc3::SubReset( hard );

			Map( 0x5000U, &Mapper187::Peek_5000, &Mapper187::Poke_5000 );
			Map( 0x5001U, 0x5FFFU,               &Mapper187::Poke_5001 );

			for (uint i=0x8000U; i < 0xA000U; i += 0x4)
			{
				Map( i + 0x0, &Mapper187::Poke_8000 );
				Map( i + 0x1, &Mapper187::Poke_8001 );			
				Map( i + 0x2, NOP_POKE              );			
				Map( i + 0x3, &Mapper187::Poke_8003 );			
			}
		}
	
		void Mapper187::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
				{
					const State::Loader::Data<3> data( state );

					exCtrl = data[0];
					exMode = data[1] & 0x1;
					exLast = data[2];
				}
	
				state.End();
			}
		}
	
		void Mapper187::SubSave(State::Saver& state) const
		{
			const u8 data[3] = 
			{
				exCtrl, exMode, exLast
			};

			state.Begin('R','E','G','\0').Write( data ).End();
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif

		void Mapper187::UpdatePrg()
		{
			if (exCtrl & 0x80)
			{
				const uint bank = exCtrl & 0x1F;

				if (exCtrl & 0x20)
					prg.SwapBank<SIZE_32K,0x0000U>( bank >> 2 );
				else
					prg.SwapBanks<SIZE_16K,0x0000U>( bank, bank );
			}
			else
			{
				const uint i = (regs.ctrl0 & Regs::CTRL0_XOR_PRG) >> 5;

				prg.SwapBanks<SIZE_8K,0x0000U>( banks.prg[i], banks.prg[1], banks.prg[i^2], banks.prg[3] );
			}
		}

		void Mapper187::UpdateChr() const
		{
			ppu.Update();

			const uint swap = (regs.ctrl0 & Regs::CTRL0_XOR_CHR) << 5;

			chr.SwapBanks<SIZE_2K>( 0x0000U ^ swap, banks.chr[0] | 0x80, banks.chr[1] | 0x80 ); 
			chr.SwapBanks<SIZE_1K>( 0x1000U ^ swap, banks.chr[2], banks.chr[3], banks.chr[4], banks.chr[5] ); 
		}

		NES_PEEK(Mapper187,5000)
		{
			static const uchar protection[4] =
			{
				0x83,0x83,0x42,0x00
			};

			return protection[exLast & 0x3];
		}

		NES_POKE(Mapper187,5000)
		{
			exLast = data;

			if (exCtrl != data)
			{
				exCtrl = data;
				Mapper187::UpdatePrg();
			}
		}

		NES_POKE(Mapper187,5001)
		{
			exLast = data;
		}

		NES_POKE(Mapper187,8000)
		{
			exMode = true;
			NES_CALL_POKE(Mmc3,8000,address,data);
		}

		NES_POKE(Mapper187,8001)
		{
			if (exMode)
				NES_CALL_POKE(Mmc3,8001,address,data);
		}

		NES_POKE(Mapper187,8003)
		{
			exMode = false;

			if (data == 0x28 || data == 0x2A || data == 0x06)
				prg.SwapBanks<SIZE_8K,0x2000U>( data == 0x2A ? 0x0F : 0x1F, data == 0x06 ? 0x1F : 0x17 );
		}
	}
}
