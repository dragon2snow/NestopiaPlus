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
#include "../NstClock.hpp"
#include "NstMapper064.hpp"
		
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		Mapper64::Irq::Irq(Cpu& cpu,Ppu& ppu)
		: 
		a12 ( cpu, ppu, A12_SIGNAL, A12::IRQ_DELAY, unit ),
		m2  ( cpu, unit )
		{}
	
		Mapper64::Mapper64(Context& c)
		: Mapper(c), irq(c.cpu,c.ppu) {}
	
		void Mapper64::Irq::Unit::Reset(const bool hard)
		{
			if (hard)
			{
				count = 0;
				reload = false;
				latch = 0;
				enabled = false;
			}
		}

		void Mapper64::Regs::Reset()
		{
			ctrl = 0;

			for (uint i=0; i < 8; ++i)
				chr[i] = i;

			for (uint i=0; i < 3; ++i)
				prg[i] = i;
		}

		void Mapper64::SubReset(const bool hard)
		{
			irq.a12.Reset( hard, hard || irq.a12.IsLineEnabled() );
			irq.m2.Reset( hard, !hard || irq.m2.IsLineEnabled() );

			if (hard)
				regs.Reset();

			for (uint i=0x0000U; i < 0x1000U; i += 0x2)
			{
				Map( 0x8000U + i, &Mapper64::Poke_8000 );
				Map( 0x8001U + i, &Mapper64::Poke_8001 );			
				Map( 0xA000U + i, NMT_SWAP_HV          );			
				Map( 0xC000U + i, &Mapper64::Poke_C000 );
				Map( 0xC001U + i, &Mapper64::Poke_C001 );
				Map( 0xE000U + i, &Mapper64::Poke_E000 );
				Map( 0xE001U + i, &Mapper64::Poke_E001 );
			}
	
			UpdateChr();
			UpdatePrg();
		}
	
		void Mapper64::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case NES_STATE_CHUNK_ID('R','E','G','\0'):
					{
						const State::Loader::Data<1+8+3> data( state );

						regs.ctrl = data[0];

						for (uint i=0; i < 3; ++i)
							regs.prg[i] = data[1+i];

						for (uint i=0; i < 8; ++i)
							regs.chr[i] = data[1+3+i];

						break;
					}

					case NES_STATE_CHUNK_ID('I','R','Q','\0'):
					{
						const State::Loader::Data<3> data( state );

						irq.unit.enabled = data[0] & 0x1;
						irq.a12.EnableLine( data[0] & 0x2 );
						irq.m2.EnableLine( data[0] & 0x2 );
						irq.unit.reload = data[0] & 0x4;
						irq.unit.latch = data[1];
						irq.unit.count = data[2];
						
						break;
					}
				}

				state.End();
			}
		}

		void Mapper64::SubSave(State::Saver& state) const
		{
			{
				const u8 data[1+8+3] =
				{
					regs.ctrl,
					regs.prg[0],
					regs.prg[1],
					regs.prg[2],
					regs.chr[0],
					regs.chr[1],
					regs.chr[2],
					regs.chr[3],
					regs.chr[4],
					regs.chr[5],
					regs.chr[6],
					regs.chr[7]
				};
			
				state.Begin('R','E','G','\0').Write( data ).End();
			}

			{
				const u8 data[3] =
				{
					(irq.unit.enabled       ? 0x1 : 0x0) |
					(irq.m2.IsLineEnabled() ? 0x2 : 0x0) |
					(irq.unit.reload        ? 0x4 : 0x0),
					irq.unit.latch,
					irq.unit.count & 0xFF
				};

				state.Begin('I','R','Q','\0').Write( data ).End();
			}
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif

		ibool Mapper64::Irq::Unit::Signal()
		{
			if (!reload)
			{
				if (count)
				{
					return !--count && enabled;
				}
				else
				{
					count = latch;
					return false;
				}
			}
			else
			{
				reload = false;
				count = latch + 1;
				return false;
			}
		}
	
		void Mapper64::Irq::Update()
		{
			a12.Update();
			m2.Update();
		}

		void Mapper64::UpdatePrg()
		{
			prg.SwapBanks<NES_8K,0x0000U>
			( 
		     	regs.prg[(regs.ctrl & 0x40) ? 2 : 0], 
				regs.prg[(regs.ctrl & 0x40) ? 0 : 1]
			);

			prg.SwapBank<NES_8K,0x4000U>
			( 
				regs.prg[(regs.ctrl & 0x40) ? 1 : 2] 
			);
		}
	
		void Mapper64::UpdateChr() const
		{
			ppu.Update();

			const uint offset = (regs.ctrl & 0x80) << 5;

			if (regs.ctrl & 0x20)
				chr.SwapBanks<NES_1K>( offset, regs.chr[0], regs.chr[6], regs.chr[1], regs.chr[7] );
			else
				chr.SwapBanks<NES_2K>( offset, regs.chr[0] >> 1, regs.chr[1] >> 1 );

			chr.SwapBanks<NES_1K>( offset ^ 0x1000U, regs.chr[2], regs.chr[3], regs.chr[4], regs.chr[5] );
		}

		NES_POKE(Mapper64,8000)
		{
			const uint diff = regs.ctrl ^ data;
			regs.ctrl = data;

			if (diff & 0x40)
				UpdatePrg();
			
			if (diff & (0x20|0x80))
				UpdateChr();
		}
	
		NES_POKE(Mapper64,8001)
		{
			const uint index = regs.ctrl & 0xF;

			if (index < 0x6)
			{
				if (regs.chr[index] != data)
				{
					regs.chr[index] = data;
					UpdateChr();
				}
			}
			else switch (index)
			{
				case 0x6:
				case 0x7:

					if (regs.prg[index - 0x6] != data)
					{
						regs.prg[index - 0x6] = data;
						UpdatePrg();
					}
					break;

				case 0x8:
				case 0x9:

					if (regs.chr[index - 0x2] != data)
					{
						regs.chr[index - 0x2] = data;
						UpdateChr();
					}
					break;

				case 0xF:

					if (regs.prg[2] != data)
					{
						regs.prg[2] = data;
						UpdatePrg();
					}
					break;
			}
		}
	
		NES_POKE(Mapper64,C000) 
		{
			irq.Update();
			irq.unit.latch = data;
		}
	
		NES_POKE(Mapper64,C001) 
		{
			irq.Update();
	
			irq.unit.reload = true;
			data &= Irq::SOURCE;
	
			irq.a12.EnableLine( data == Irq::SOURCE_PPU );
			irq.m2.EnableLine( data == Irq::SOURCE_CPU );
		}
	
		NES_POKE(Mapper64,E000) 
		{
			irq.Update();	
			irq.unit.enabled = false;
			cpu.ClearIRQ();
		}
	
		NES_POKE(Mapper64,E001) 
		{ 
			irq.Update();		
			irq.unit.enabled = true;
		}
	
		void Mapper64::VSync()
		{
			irq.a12.VSync();
			irq.m2.VSync();
		}
	}
}
