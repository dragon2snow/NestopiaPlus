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
#include "NstMapper211.hpp"

namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		Mapper211::Mapper211(Context& c)
		: 
		Mapper (c), 
		irq    (c.cpu,c.ppu)
		{}
	
		void Mapper211::SubReset(const bool hard)
		{
			irq.Reset( hard, hard || irq.IsLineEnabled() );

			if (hard)
			{
				regs.toggle = 0xFF;
				regs.tmp = 0;
				regs.command = 0;

				for (uint i=0; i < 2; ++i)
					regs.mul[i] = 0;

				for (uint i=0; i < 4; ++i) 
					banks.prg[i] = 0;

				for (uint i=0; i < 8; ++i) 
					banks.chr[i] = i;

				UpdatePrg();
				UpdateChr();
			}
			else
			{
				regs.toggle ^= 0xFF;
			}

			Map( 0x5000U, &Mapper211::Peek_5000 );
			Map( 0x5800U, &Mapper211::Peek_5800, &Mapper211::Poke_5800 );
			Map( 0x5801U, &Mapper211::Peek_5801, &Mapper211::Poke_5801 );
			Map( 0x5803U, &Mapper211::Peek_5803, &Mapper211::Poke_5803 );

			Map( 0x8000U, 0x8FFFU, &Mapper211::Poke_8000 );
			Map( 0x9000U, 0x9FFFU, &Mapper211::Poke_9000 );
			Map( 0xB000U, 0xBFFFU, &Mapper211::Poke_B000 );

			for (uint i=0x0000U; i < 0x1000U; i += 0x8)
			{
				Map( 0xC002U + i, &Mapper211::Poke_C002 );
				Map( 0xC003U + i, &Mapper211::Poke_C004 );
				Map( 0xC004U + i, &Mapper211::Poke_C004 );
				Map( 0xC005U + i, &Mapper211::Poke_C005 );
				Map( 0xD000U + i, &Mapper211::Poke_D000 );
			}
		}
	
		void Mapper211::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
     				case NES_STATE_CHUNK_ID('R','E','G','\0'):
					{
						const State::Loader::Data<17> data( state );

						regs.command = data[0];
						regs.mul[0]  = data[1];
						regs.mul[1]  = data[2];
						regs.tmp     = data[3];
						regs.toggle  = data[4];

						for (uint i=0; i < 4; ++i)
							banks.prg[i] = data[5+i];

						for (uint i=0; i < 8; ++i)
							banks.chr[i] = data[5+4+i];

						break;
					}

					case NES_STATE_CHUNK_ID('I','R','Q','\0'):
						
						irq.unit.LoadState( state );
						break;
				}

				state.End();
			}
		}

		void Mapper211::SubSave(State::Saver& state) const
		{
			{
				const u8 data[17] =
				{
					regs.command,
					regs.mul[0],
					regs.mul[1],
					regs.tmp,
					regs.toggle,
					banks.prg[0],
					banks.prg[1],
					banks.prg[2],
					banks.prg[3],
					banks.chr[0],
					banks.chr[1],
					banks.chr[2],
					banks.chr[3],
					banks.chr[4],
					banks.chr[5],
					banks.chr[6],
					banks.chr[7]
				};

				state.Begin('R','E','G','\0').Write( data ).End();
			}

			irq.unit.SaveState( State::Saver::Subset(state,'I','R','Q','\0').Ref() );
		}

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		void Mapper211::UpdatePrg()
		{
			switch (regs.command & Regs::PROM_BANKSWITCH)
			{
				case Regs::PROM_SWAP_16K: 
		
					prg.SwapBanks<NES_16K,0x0000U>( banks.prg[0], banks.prg[2] );
					break;
		
				case Regs::PROM_SWAP_8K:
		
					prg.SwapBanks<NES_8K,0x0000U>( banks.prg[0], banks.prg[1], banks.prg[2], ~0U );
					break;
			}
		}
	
		void Mapper211::UpdateChr() const
		{
			ppu.Update();
	
			switch (regs.command & Regs::CROM_BANKSWITCH)
			{
				case Regs::CROM_SWAP_8K: 
		
					chr.SwapBank<NES_8K,0x0000U>( banks.chr[0] ); 
					break;
		
				case Regs::CROM_SWAP_4K: 
		
					chr.SwapBanks<NES_4K,0x0000U>( banks.chr[0], banks.chr[4] ); 
					break;
		
				case Regs::CROM_SWAP_2K: 
		
					chr.SwapBanks<NES_2K,0x0000U>( banks.chr[0], banks.chr[2], banks.chr[4], banks.chr[6] ); 
					break;
		
				case Regs::CROM_SWAP_1K: 
		
					chr.SwapBanks<NES_1K,0x0000U>( banks.chr[0], banks.chr[1], banks.chr[2], banks.chr[3] ); 
					chr.SwapBanks<NES_1K,0x1000U>( banks.chr[4], banks.chr[5], banks.chr[6], banks.chr[7] ); 
					break;
			}
		}

		NES_PEEK(Mapper211,5000)
		{ 
			return regs.toggle; 
		}
	
		NES_POKE(Mapper211,5800) 
		{ 
			regs.mul[0] = data; 
		}

		NES_POKE(Mapper211,5801) 
		{
			regs.mul[1] = data; 
		}

		NES_PEEK(Mapper211,5800) 
		{
			return (regs.mul[0] * regs.mul[1]) & 0xFF; 
		}

		NES_PEEK(Mapper211,5801)
		{
			return (regs.mul[0] * regs.mul[1]) >> 8;   
		}
	
		NES_POKE(Mapper211,5803) 
		{ 
			regs.tmp = data; 
		}

		NES_PEEK(Mapper211,5803) 
		{ 
			return regs.tmp; 
		}
	
		NES_POKE(Mapper211,8000) 
		{ 
			if (banks.prg[address & 0x3] != data)
			{
				banks.prg[address & 0x3] = data;
				UpdatePrg(); 
			}
		}

		NES_POKE(Mapper211,9000) 
		{ 
			if (banks.chr[address & 0x7] != data)
			{
				banks.chr[address & 0x7] = data; 
				UpdateChr(); 
			}
		}

		NES_POKE(Mapper211,B000) 
		{ 
			ppu.Update();
			nmt.SwapBank<NES_1K>( (address & 0x3) << 10, data & 0x3 );
		}

		NES_POKE(Mapper211,C002) 
		{
			irq.Update();
			irq.unit.Disable( cpu );
		}
	
		NES_POKE(Mapper211,C004) 
		{
			irq.Update();
			irq.unit.Enable();
			irq.unit.Reload();
		}
	
		NES_POKE(Mapper211,C005) 
		{
			irq.Update();
			irq.unit.SetLatch( data );
		}
	
		NES_POKE(Mapper211,D000) 
		{
			const uint diff = regs.command ^ data;
			regs.command = data;

			if (diff & Regs::PROM_BANKSWITCH)
				UpdatePrg(); 

			if (diff & Regs::CROM_BANKSWITCH)
				UpdateChr(); 
		}

		void Mapper211::VSync()
		{
			irq.VSync();
		}
	}
}
