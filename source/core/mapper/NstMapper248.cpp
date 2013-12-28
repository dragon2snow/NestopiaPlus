////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
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
#include "NstMapper248.hpp"
		
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		Mapper248::Mapper248(Context& c)
		: 
		Mapper (c,WRAM_NONE), 
		irq    (c.cpu,c.ppu) 
		{}
	
		void Mapper248::SubReset(const bool hard)
		{
			if (hard)
			{
				regs.command = 0;
				regs.ctrl = 0;
				regs.prg[0] = 0;
				regs.prg[1] = 1;
			}

			irq.Reset( hard, hard || irq.IsLineEnabled() );

			Map( 0x6000U, 0x7FFFU, &Mapper248::Poke_6000 );
	
			for (uint i=0x0000U; i < 0x1000U; i += 2)
			{
				Map( 0x8000U + i, &Mapper248::Poke_8000 );
				Map( 0x8001U + i, &Mapper248::Poke_8001 );			
				Map( 0xA000U + i, NMT_SWAP_HV           );
				Map( 0xC000U + i, &Mapper248::Poke_C000 );
				Map( 0xC001U + i, &Mapper248::Poke_C001 );
				Map( 0xE000U + i, &Mapper248::Poke_E000 );
				Map( 0xE001U + i, &Mapper248::Poke_E001 );
			}
		}
	
		void Mapper248::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
     				case NES_STATE_CHUNK_ID('R','E','G','\0'):
					{
						const State::Loader::Data<4> data( state );

						regs.ctrl = data[0];
						regs.command = data[1];
						regs.prg[0] = data[2];
						regs.prg[1] = data[3];

						break;
					}

					case NES_STATE_CHUNK_ID('I','R','Q','\0'):
						
						irq.unit.LoadState( state );
						break;
				}

				state.End();
			}
		}

		void Mapper248::SubSave(State::Saver& state) const
		{
			{
				const u8 data[4] =
				{
					regs.ctrl,
					regs.command,
					regs.prg[0],
					regs.prg[1]
				};

				state.Begin('R','E','G','\0').Write( data ).End();
			}

			irq.unit.SaveState( State::Saver::Subset(state,'I','R','Q','\0').Ref() );
		}

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		void Mapper248::UpdatePrg()
		{
			prg.SwapBanks<SIZE_8K,0x0000U>
			( 
		       	((regs.ctrl & 0x80) ? ((regs.ctrl << 1) | 0x0) : regs.prg[0]) & 0x1F,
				((regs.ctrl & 0x80) ? ((regs.ctrl << 1) | 0x1) : regs.prg[1]) & 0x1F
			);
		}
	
		NES_POKE(Mapper248,6000)
		{
			regs.ctrl = data;
			UpdatePrg();
		}
	
		NES_POKE(Mapper248,8000)
		{
			regs.command = data;
		}
	
		NES_POKE(Mapper248,8001)
		{
			const uint index = regs.command & 0x7;

			if (index < 0x6)
			{
				ppu.Update();

				if (index > 0x1)
				{
					chr.SwapBank<SIZE_1K>( (index << 10) + 0x800U, data );
				}
				else
				{
					chr.SwapBank<SIZE_2K>( index << 11, data >> 1 );
				}
			}
			else 
			{
				regs.prg[index - 0x6] = data; 
				UpdatePrg();
			}
		}
	
		NES_POKE(Mapper248,C000) 
		{
			irq.Update(); 
			irq.unit.SetLatch( data );
		}
	
		NES_POKE(Mapper248,C001) 
		{ 
			irq.Update(); 
			irq.unit.Reload(); 
		}
	
		NES_POKE(Mapper248,E000) 
		{ 
			irq.Update(); 
			irq.unit.Disable( cpu );
		}
	
		NES_POKE(Mapper248,E001) 
		{
			irq.Update(); 
			irq.unit.Enable();
		}

		void Mapper248::VSync()
		{
			irq.VSync();
		}
	}
}
