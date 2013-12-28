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
#include "NstMapper056.hpp"
		 
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		Mapper56::Mapper56(Context& c)
		: Mapper(c,WRAM_8K), irq(c.cpu) {}

		void Mapper56::Irq::Reset(const bool hard)
		{
			if (hard)
			{
				count = 0;
				latch = 0;
				ctrl = 0;
			}
		}

		void Mapper56::SubReset(const bool hard)
		{
			if (hard)
				ctrl = 0;

			irq.Reset( hard, hard ? false : irq.IsLineEnabled() );

			Map( 0x8000U, 0x8FFFU, &Mapper56::Poke_8000 );
			Map( 0x9000U, 0x9FFFU, &Mapper56::Poke_9000 );
			Map( 0xA000U, 0xAFFFU, &Mapper56::Poke_A000 );
			Map( 0xB000U, 0xBFFFU, &Mapper56::Poke_B000 );
			Map( 0xC000U, 0xCFFFU, &Mapper56::Poke_C000 );
			Map( 0xD000U, 0xDFFFU, &Mapper56::Poke_D000 );
			Map( 0xE000U, 0xEFFFU, &Mapper56::Poke_E000 );
			Map( 0xF000U, 0xFFFFU, &Mapper56::Poke_F000 );
		}

		void Mapper56::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case NES_STATE_CHUNK_ID('R','E','G','\0'):
					
						ctrl = state.Read8();
						break;
					
					case NES_STATE_CHUNK_ID('I','R','Q','\0'):
					{
						const State::Loader::Data<5> data( state );

						irq.EnableLine( data[0] & 0x2 );
						irq.unit.ctrl = data[0] & 0x5;
						irq.unit.count = data[1] | (data[2] << 8);
						irq.unit.latch = data[3] | (data[4] << 8);

						break;					
					}
				}

				state.End();
			}
		}
	
		void Mapper56::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write8( ctrl ).End();

			const u8 data[5] =
			{
				(irq.IsLineEnabled() ? 0x2 : 0x0) | irq.unit.ctrl,
				irq.unit.count & 0xFF,
				irq.unit.count >> 8,
				irq.unit.latch & 0xFF,
				irq.unit.latch >> 8
			};

			state.Begin('I','R','Q','\0').Write( data ).End();
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif

		NES_POKE(Mapper56,8000)
		{
			irq.Update();
			irq.unit.latch = (irq.unit.latch & 0xFFF0U) | ((data & 0xF) << 0);
		}

		NES_POKE(Mapper56,9000)
		{
			irq.Update();
			irq.unit.latch = (irq.unit.latch & 0xFF0FU) | ((data & 0xF) << 4);
		}

		NES_POKE(Mapper56,A000)
		{
			irq.Update();
			irq.unit.latch = (irq.unit.latch & 0xF0FFU) | ((data & 0xF) << 8);
		}

		NES_POKE(Mapper56,B000)
		{
			irq.Update();
			irq.unit.latch = (irq.unit.latch & 0x0FFFU) | ((data & 0xF) << 12);
		}

		NES_POKE(Mapper56,C000)
		{
			irq.Update();
			
			irq.unit.ctrl = data & 0x5;

			if (irq.EnableLine( data & 0x2 ))
				irq.unit.count = irq.unit.latch;

			irq.ClearIRQ();
		}

		NES_POKE(Mapper56,D000)
		{
			irq.Update();
			irq.EnableLine( irq.unit.ctrl & 0x1 );
			irq.ClearIRQ();
		}

		NES_POKE(Mapper56,E000)
		{
			ctrl = data;
		}

		NES_POKE(Mapper56,F000)
		{
			{
				uint offset = (ctrl & 0xF) - 1U;

				if (offset < 3)
				{
					offset <<= 13;
					prg.SwapBank<SIZE_8K>( offset, (data & 0x0F) | (prg.GetBank<SIZE_8K>(offset) & 0x10) );
				}
			}

			switch (address & 0xC00)
			{
				case 0x000:

					address &= 0x3;

					if (address < 3)
					{
						address <<= 13;
						prg.SwapBank<SIZE_8K>( address, (prg.GetBank<SIZE_8K>(address) & 0x0F) | (data & 0x10) );
					}
					break;

				case 0x800:

					ppu.SetMirroring( (data & 0x1) ? Ppu::NMT_VERTICAL : Ppu::NMT_HORIZONTAL );
					break;

				case 0xC00: 
					
					ppu.Update();
					chr.SwapBank<SIZE_1K>( (address & 0x7) << 10, data );
					break;
			}
		}

		ibool Mapper56::Irq::Signal()
		{
			if (count++ != 0xFFFFU)
				return false;
			
			count = latch;
			return true;
		}

		void Mapper56::VSync()
		{
			irq.VSync();
		}
	}
}
