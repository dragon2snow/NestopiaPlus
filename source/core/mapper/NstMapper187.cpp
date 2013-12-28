////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2005 Martin Freij
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
				latch = 0;
				hack = true;

				exBanks[0] = 0;
				exBanks[1] = 1;

				useExBank = false;
				exBankMode = 0;
			}
	
			Mmc3::SubReset( hard );
	
			Map( 0x5000U,          &Mapper187::Peek_5000, &Mapper187::Poke_5000 );
			Map( 0x5001U, 0x7FFFU, &Mapper187::Peek_5000, &Mapper187::Poke_5001 );
	
			for (uint i=0x8000U; i < 0xA000U; )
			{
				Map( i++, &Mapper187::Poke_8000 );
				Map( i++, &Mapper187::Poke_8001 );			
				Map( i++, NOP_POKE              );			
				Map( i++, &Mapper187::Poke_8003 );
			}
	
			Map( 0x8000U,          &Mapper187::Poke_8000 );
			Map( 0x8002U,          NOP_POKE              );
			Map( 0x8001U,          &Mapper187::Poke_8001 );
			Map( 0x8003U,          &Mapper187::Poke_8003 );
			Map( 0x8004U, 0x9FFFU, NOP_POKE              );				
			Map( 0xA000U,          NMT_SWAP_HV           );
			Map( 0xA001U,          &Mapper187::Poke_A001 );
			Map( 0xA003U, 0xBFFFU, NOP_POKE              );
			Map( 0xC000U,          &Mapper187::Poke_C000 );
			Map( 0xC001U,          &Mapper187::Poke_C001 );
			Map( 0xC002U, 0xDFFFU, NOP_POKE              );
			Map( 0xE000U,          &Mapper187::Poke_E000 );
			Map( 0xE002U,          &Mapper187::Poke_E000 );
			Map( 0xE001U,          &Mapper187::Poke_E001 );
			Map( 0xE003U,          &Mapper187::Poke_E001 );
			Map( 0xE004U, 0xFFFFU, NOP_POKE              );
		}
	
		void Mapper187::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
				{
					const State::Loader::Data<5> data( state );
	
					exBankMode = data[0];
					exBanks[0] = data[1];
					exBanks[1] = data[2];
					useExBank = data[3] & 0x1;
					latch = (data[3] >> 1) & 0x3;
				}
	
				state.End();
			}
		}
	
		void Mapper187::SubSave(State::Saver& state) const
		{
			const u8 data[4] =
			{
				exBankMode,
				exBanks[0],
				exBanks[1],
				useExBank | (latch << 1)
			};

			state.Begin('R','E','G','\0').Write( data ).End();
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper187,5000)
		{
			latch = data & 0x3;
			exBankMode = data;
	
			if (data & SWAP_NO_EXBANK)
			{
				if (data & SWAP_32)
				{
					const uint bank = (data & 0x1E) << 1;
	
					banks.prg[0] = bank + 0;
					banks.prg[1] = bank + 1;
					banks.prg[2] = bank + 2;
					banks.prg[3] = bank + 3;
				}
				else
				{
					const uint bank = (data & 0x1F) << 1;
	
					banks.prg[2] = bank + 0;
					banks.prg[3] = bank + 1;
				}
			}
			else
			{
				banks.prg[0] = exBanks[0];
				banks.prg[1] = exBanks[1];
				banks.prg[2] = 0xFE;
				banks.prg[3] = 0xFF;
			}
	
			Mapper187::UpdatePrg();
		}
	
		NES_PEEK(Mapper187,5000)
		{
			switch (latch)
			{
				case 0:
				case 1:	return 0x83;
				case 2: return 0x42;
			}
	
			return 0x00;
		}
	
		NES_POKE(Mapper187,5001)
		{
			if (hack)
			{
				hack = false;

				// Sonic 3D Blast 6 will not work unless APU 
				// frame IRQ's are disabled on power-on

				cpu.Poke( 0x4017, 0x40 ); 
			}

			latch = data & 0x3;
		}
	
		NES_POKE(Mapper187,8000)
		{
			useExBank = false;
			regs.ctrl0 = data;
		}
	
		NES_POKE(Mapper187,8001)
		{
			const uint index = regs.ctrl0 & Regs::CTRL0_MODE;
	
			if (index == 6 || index == 7)
				exBanks[index - 6] = data;
	
			if (useExBank)
			{
				switch (regs.ctrl0)
				{
					case 0x2A: banks.prg[1] = 0x0F; break;
					case 0x28: banks.prg[2] = 0x17; break;
				}
	
				Mapper187::UpdatePrg();
			}
			else switch (index)
			{
				case 0:		
				case 1:					
			
					banks.chr[index] = (data | 0x100) >> 1;
					UpdateChr(); 
					break;
			
				case 2:
				case 3:
				case 4:
				case 5:
			
					banks.chr[index] = data >> 0;
					UpdateChr(); 
					break;
			
				case 6: 
				case 7: 
			
					if ((exBankMode & 0xA0) != 0xA0)
					{
						banks.prg[index - 0x6] = data;
						Mapper187::UpdatePrg();
					}
					break;
			}
		}
	
		NES_POKE(Mapper187,8003)
		{
			useExBank = true;
			regs.ctrl0 = data;
	
			if (!(data & 0xF0))
			{
				banks.prg[2] = 0xFE;
				Mapper187::UpdatePrg();
			}
		}
	
		void Mapper187::UpdatePrg()
		{
			prg.SwapBanks<NES_8K,0x0000U>( banks.prg[0], banks.prg[1], banks.prg[2], banks.prg[3] );
		}
	}
}
