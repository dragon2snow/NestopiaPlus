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
#include "../NstClock.hpp"
#include "NstMapper073.hpp"
	  
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		Mapper73::Mapper73(Context& c)
		: Mapper(c,CROM_NONE), irq(c.cpu) {}
	
		void Mapper73::Irq::Reset(const bool hard)
		{
			if (hard)
			{
				enabled = false;
				count = 0;
			}
		}
	
		void Mapper73::SubReset(const bool hard)
		{
			irq.Reset( hard, true );
	
			Map( 0x8000U, 0x8FFFU, &Mapper73::Poke_8000 );
			Map( 0x9000U, 0x9FFFU, &Mapper73::Poke_9000 );
			Map( 0xA000U, 0xAFFFU, &Mapper73::Poke_A000 );
			Map( 0xB000U, 0xBFFFU, &Mapper73::Poke_B000 );
			Map( 0xC000U, 0xCFFFU, &Mapper73::Poke_C000 );
			Map( 0xD000U, 0xDFFFU, &Mapper73::Poke_D000 );	
			Map( 0xF000U, 0xFFFFU, PRG_SWAP_16K );
		}
	
		void Mapper73::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('I','R','Q','\0'))
				{
					const State::Loader::Data<3> data( state );
					irq.unit.enabled = data[0] & 0x1;
					irq.unit.count = data[1] | (data[2] << 8);
				}

				state.End();
			}
		}
	
		void Mapper73::SubSave(State::Saver& state) const
		{
			const u8 data[3] =
			{
				irq.unit.enabled != 0,
				irq.unit.count & 0xFF,
				irq.unit.count >> 8
			};

			state.Begin('I','R','Q','\0').Write( data ).End();
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		ibool Mapper73::Irq::Signal()
		{
			if (enabled)
			{
				count = (count + 1) & 0xFFFFU;

				if (!count)
				{
					enabled = false;
					return true;
				}
			}

			return false;
		}

		NES_POKE(Mapper73,8000) 
		{
			irq.Update();
			irq.unit.count = (irq.unit.count & 0xFFF0U) | ((data & 0xF) << 0); 
		}

		NES_POKE(Mapper73,9000) 
		{
			irq.Update();
			irq.unit.count = (irq.unit.count & 0xFF0FU) | ((data & 0xF) << 4); 
		}

		NES_POKE(Mapper73,A000) 
		{
			irq.Update();
			irq.unit.count = (irq.unit.count & 0xF0FFU) | ((data & 0xF) << 8); 		
		}

		NES_POKE(Mapper73,B000) 
		{
			irq.Update();
			irq.unit.count = (irq.unit.count & 0x0FFFU) | ((data & 0xF) << 12); 
		}
		
		NES_POKE(Mapper73,C000) 
		{ 
			irq.Update();
			irq.unit.enabled = data & 0x2;
			irq.ClearIRQ();
		}

		NES_POKE(Mapper73,D000) 
		{
			irq.Update();
			irq.unit.enabled = false;
			irq.ClearIRQ();
		}
	
		void Mapper73::VSync()
		{
			irq.VSync();
		}
	}
}
