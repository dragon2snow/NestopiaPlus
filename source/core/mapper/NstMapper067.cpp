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
#include "../NstClock.hpp"
#include "NstMapper067.hpp"
		  
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		Mapper67::Mapper67(Context& c)
		: Mapper(c), irq (c.cpu) {}

		void Mapper67::Irq::Reset(const bool hard)
		{
			if (hard)
			{
				enabled = false;
				count = 0;
				toggle = 0;
			}
		}

		void Mapper67::SubReset(const bool hard)
		{
			irq.Reset( hard, true );
	
			Map( 0x8800U, 0x8FFFU, CHR_SWAP_2K_0        );
			Map( 0x9800U, 0x9FFFU, CHR_SWAP_2K_1        );
			Map( 0xA800U, 0xAFFFU, CHR_SWAP_2K_2        );
			Map( 0xB800U, 0xBFFFU, CHR_SWAP_2K_3        );
			Map( 0xC000U, 0xCFFFU, &Mapper67::Poke_C000 );
			Map( 0xD800U, 0xDFFFU, &Mapper67::Poke_D800 );
			Map( 0xE800U, 0xEFFFU, NMT_SWAP_VH01        );
			Map( 0xF800U, 0xFFFFU, PRG_SWAP_16K         );
		}
	
		void Mapper67::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('I','R','Q','\0'))
				{
					const State::Loader::Data<3> data( state );

					irq.unit.enabled = data[0] & 0x1;
					irq.unit.toggle = (data[0] & 0x2) >> 1;
					irq.unit.count = data[1] | (data[2] << 8);
				}

				state.End();
			}
		}
	
		void Mapper67::SubSave(State::Saver& state) const
		{
			const u8 data[3] =
			{
				(irq.unit.enabled ? 0x1 : 0x0) | (irq.unit.toggle ? 0x2 : 0x0),
				irq.unit.count & 0xFF,
				irq.unit.count >> 8
			};

			state.Begin('I','R','Q','\0').Write( data ).End();
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper67,C000)
		{
			irq.Update();

			if (irq.unit.toggle ^= 1)
				irq.unit.count = (irq.unit.count & 0x00FFU) | (data << 8);
			else
				irq.unit.count = (irq.unit.count & 0xFF00U) | (data << 0);
		}
	
		NES_POKE(Mapper67,D800) 
		{
			irq.Update();

			irq.unit.toggle = 0;
			irq.unit.enabled = data & 0x10;
			irq.ClearIRQ();
		}

		ibool Mapper67::Irq::Signal()
		{
			if (enabled && count && !--count)
			{
				enabled = false;
				count = 0xFFFFU;
				return true;
			}

			return false;
		}

		void Mapper67::VSync()
		{
			irq.VSync();
		}
	}
}
