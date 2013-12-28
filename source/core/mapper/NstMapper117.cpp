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
#include "NstMapper117.hpp"
	  
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		Mapper117::Mapper117(Context& c)
		: 
		Mapper (c),
		irq    (c.cpu,c.ppu)
		{}
	
		void Mapper117::SubReset(const bool hard)
		{
			if (hard)
				irqState = 0;

			irq.Reset( hard, hard || irq.IsLineEnabled() );

			Map( 0x8000U, PRG_SWAP_8K_0 );
			Map( 0x8001U, PRG_SWAP_8K_1 );
			Map( 0x8002U, PRG_SWAP_8K_2 );
			Map( 0x8003U, PRG_SWAP_8K_3 );		
			Map( 0xA000U, CHR_SWAP_1K_0 );
			Map( 0xA001U, CHR_SWAP_1K_1 );
			Map( 0xA002U, CHR_SWAP_1K_2 );
			Map( 0xA003U, CHR_SWAP_1K_3 );
			Map( 0xA004U, CHR_SWAP_1K_4 );
			Map( 0xA005U, CHR_SWAP_1K_5 );
			Map( 0xA006U, CHR_SWAP_1K_6 );
			Map( 0xA007U, CHR_SWAP_1K_7 );
			Map( 0xC001U, &Mapper117::Poke_C001 );
			Map( 0xC002U, &Mapper117::Poke_C002 );
			Map( 0xC003U, &Mapper117::Poke_C003 );
			Map( 0xE000U, &Mapper117::Poke_E000 );
		}
	
		void Mapper117::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('I','R','Q','\0'))
				{
					irq.unit.LoadState( state );
					irqState = state.Read8() & (IRQ_LATCH_0|IRQ_LATCH_1);
				}

				state.End();
			}
		}

		void Mapper117::SubSave(State::Saver& state) const
		{
			state.Begin('I','R','Q','\0');
			irq.unit.SaveState( state );
			state.Write8( irqState );
			state.End();
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper117,C001) 
		{ 
			irq.Update();
			irq.unit.SetLatch( data );
		}
	
		NES_POKE(Mapper117,C002) 
		{ 
			cpu.ClearIRQ();
		}
	
		NES_POKE(Mapper117,C003) 
		{ 
			irq.Update();
	
			irqState |= IRQ_LATCH_0;
	
			if (irqState == IRQ_ENABLE)
				irq.unit.Enable();
			else
				irq.unit.Disable( cpu );
	
			irq.unit.Reload();
		}
	
		NES_POKE(Mapper117,E000) 
		{ 
			irq.Update();
	
			irqState = (irqState & ~uint(IRQ_LATCH_1)) | (data & IRQ_LATCH_1);
	
			if (irqState == IRQ_ENABLE)
				irq.unit.Enable();
			else
				irq.unit.Disable( cpu );
		}

		void Mapper117::VSync()
		{
			irq.VSync();
		}
	}
}
