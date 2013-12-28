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
#include "NstBrdMmc3.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif
		
			Mmc3::Irq::Irq(Cpu& cpu,Ppu& ppu,IrqDelay delay,bool persistant)
			: Clock::A12<BaseIrq>(cpu,ppu,SIGNAL_DURATION,delay,persistant) {}

			Mmc3::Mmc3(Context& c,const uint settings,const bool persistantIrq)
			: Mapper(c,settings), irq(c.cpu,c.ppu,Irq::NO_IRQ_DELAY,persistantIrq) 
			{
			}
		
			void Mmc3::Regs::Reset()
			{
				ctrl0 = 0;
				ctrl1 = 0;
			}
		
			void Mmc3::Banks::Reset()
			{
				chr[0] = 0x0;
				chr[1] = 0x1;
				chr[2] = 0x4;
				chr[3] = 0x5;
				chr[4] = 0x6;
				chr[5] = 0x7;
		
				prg[0] = 0x00;
				prg[1] = 0x01;
				prg[2] = 0xFE;
				prg[3] = 0xFF;
			}
		
			void Mmc3::BaseIrq::Reset(const bool hard)
			{
				if (hard)
				{
					count = 0;
					latch = 0;
					reload = false;
					enabled = false;
				}
			}
		
			void Mmc3::SubReset(const bool hard)
			{
				if (hard)
				{
					regs.Reset();
					banks.Reset();
					wrk.Source().SetSecurity( false, false );
				}

				irq.Reset( hard, hard || irq.IsLineEnabled() );

				for (uint i=0x0000U; i < 0x2000U; i += 0x2)
				{
					Map( 0x8000U + i, &Mmc3::Poke_8000 );
					Map( 0x8001U + i, &Mmc3::Poke_8001 );				
					Map( 0xA000U + i, NMT_SWAP_HV      );
					Map( 0xA001U + i, &Mmc3::Poke_A001 );
					Map( 0xC000U + i, &Mmc3::Poke_C000 );
					Map( 0xC001U + i, &Mmc3::Poke_C001 );
					Map( 0xE000U + i, &Mmc3::Poke_E000 );
					Map( 0xE001U + i, &Mmc3::Poke_E001 );
				}
		
				if (wrk.HasRam())
					Map( WRK_PEEK_POKE_BUS );
						
				UpdatePrg();
				UpdateChr();
			}
		
			void Mmc3::BaseIrq::LoadState(State::Loader& state)
			{
				const State::Loader::Data<3> data( state );
		
				enabled = data[0] & 0x1;
				reload = data[0] & 0x2;
				count = data[1];
				latch = data[2];
			}
		
			void Mmc3::BaseIrq::SaveState(State::Saver& state) const
			{
				const u8 data[3] =
				{
					(enabled ? 0x1 : 0x0) | (reload ? 0x2 : 0x0),
					count,
					latch
				};

				state.Write( data );
			}
		
			void Mmc3::LoadState(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					switch (chunk)
					{
						case NES_STATE_CHUNK_ID('R','E','G','\0'):
						{
							u8 data[12];
							state.Read( data );
		
							regs.ctrl0 = data[0];
							regs.ctrl1 = data[1];
							banks.prg[0] = data[2];
							banks.prg[1] = data[3];
							banks.prg[2] = data[4];
							banks.prg[3] = data[5];
							banks.chr[0] = data[6];
							banks.chr[1] = data[7];
							banks.chr[2] = data[8];
							banks.chr[3] = data[9];
							banks.chr[4] = data[10];
							banks.chr[5] = data[11];
		
							break;
						}
		
						case NES_STATE_CHUNK_ID('I','R','Q','\0'):
		
							irq.unit.LoadState( state );
							break;
					}
		
					state.End();
				}
			}
		
			void Mmc3::SaveState(State::Saver& state) const
			{
				{
					const u8 data[12] =
					{
						regs.ctrl0,
						regs.ctrl1,
						banks.prg[0],
						banks.prg[1],
						banks.prg[2],
						banks.prg[3],
						banks.chr[0],
						banks.chr[1],
						banks.chr[2],
						banks.chr[3],
						banks.chr[4],
						banks.chr[5]
					};

					state.Begin('R','E','G','\0').Write( data ).End();
				}
		
				irq.unit.SaveState( State::Saver::Subset(state,'I','R','Q','\0').Ref() );
			}
		
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #pragma optimize("w", on)
            #endif
		
			NES_POKE(Mmc3,8000)
			{
				const uint diff = regs.ctrl0 ^ data;
				regs.ctrl0 = data;
		
				if (diff & Regs::CTRL0_XOR_PRG) 
					UpdatePrg();
		
				if (diff & (Regs::CTRL0_XOR_CHR|Regs::CTRL0_MODE))
					UpdateChr();
			}
		
			NES_POKE(Mmc3,8001)
			{
				address = (regs.ctrl0 & Regs::CTRL0_MODE);
		
				if (address < 2)
					data >>= 1;
		
				if (bankBlock[address] != data)
				{
					bankBlock[address] = data;
		
					if (address < 6)
						UpdateChr();
					else
						UpdatePrg();
				}
			}
		
			NES_POKE(Mmc3,A001)
			{
				regs.ctrl1 = data;
				
				wrk.Source().SetSecurity
				( 
			     	(data & Regs::CTRL1_WRAM_ENABLED), 
					(data & Regs::CTRL1_WRAM) == Regs::CTRL1_WRAM_ENABLED 
				);
			}
		
			NES_POKE(Mmc3,C000) 
			{ 
				irq.Update();
				irq.unit.SetLatch( data );
			}						 
		
			NES_POKE(Mmc3,C001) 
			{
				irq.Update();
				irq.unit.Reload();
			}
		
			NES_POKE(Mmc3,E000) 
			{ 
				irq.Update();
				irq.unit.Disable( cpu );
			}
		
			NES_POKE(Mmc3,E001) 
			{ 
				irq.Update();
				irq.unit.Enable();
			}

			void Mmc3::UpdatePrg()
			{
				const uint i = (regs.ctrl0 & Regs::CTRL0_XOR_PRG) >> 5;
		
				prg.SwapBanks<NES_8K,0x0000U>( banks.prg[i], banks.prg[1], banks.prg[i^2], banks.prg[3] );
			}
		
            void Mmc3::UpdateChr() const
			{
				ppu.Update();
		
				const uint swap = (regs.ctrl0 & Regs::CTRL0_XOR_CHR) << 5;
		
				chr.SwapBanks<NES_2K>( 0x0000U ^ swap, banks.chr[0], banks.chr[1] ); 
				chr.SwapBanks<NES_1K>( 0x1000U ^ swap, banks.chr[2], banks.chr[3], banks.chr[4], banks.chr[5] ); 
			}
		
			void Mmc3::VSync()
			{
				irq.VSync();
			}
		
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif
		}
	}
}
