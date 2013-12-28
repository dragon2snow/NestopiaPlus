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
#include "NstBrdVrc4.hpp"
			  
namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif

			Vrc4::Vrc4(Context& c,const Type t)
			: 
			Mapper (c), 
			irq    (t != TYPE_A ? new Irq(c.cpu) : NULL), 
			type   (t)
			{
			}

			Vrc4::~Vrc4()
			{
				delete irq;
			}

			void Vrc4::BaseIrq::Reset(bool)
			{
				ctrl = 0;
				count[0] = 0;
				count[1] = 0;
				latch = 0;
			}

			void Vrc4::SubReset(const bool hard)
			{
				if (type == TYPE_A)
				{
					Map( 0x8000U, 0x8FFFU, PRG_SWAP_8K_0 );
					Map( 0x9000U, 0x9FFFU, NMT_SWAP_VH01 );
					Map( 0xA000U, 0xAFFFU, PRG_SWAP_8K_1 );

					for (uint i=0x0000U; i < 0x1000U; i += 0x4)
					{
						Map( 0xB000 + i, &Vrc4::Poke_B0_A );
						Map( 0xB001 + i, &Vrc4::Poke_B1_A );
						Map( 0xC000 + i, &Vrc4::Poke_C0_A );
						Map( 0xC001 + i, &Vrc4::Poke_C1_A );
						Map( 0xD000 + i, &Vrc4::Poke_D0_A );
						Map( 0xD001 + i, &Vrc4::Poke_D1_A );
						Map( 0xE000 + i, &Vrc4::Poke_E0_A );
						Map( 0xE001 + i, &Vrc4::Poke_E1_A );
					}
				}
				else
				{
					if (hard)
						prgSwap = 0;

					irq->Reset( hard, hard ? false : irq->IsLineEnabled() );

					Map( 0x8000U, 0x8FFFU, &Vrc4::Poke_8 );
					Map( 0xA000U, 0xAFFFU, PRG_SWAP_8K_1 );
				
					if (type == TYPE_2A)
					{
						for (dword i=0x9000U; i <= 0xFFFFU; ++i)
						{
							switch ((i | ((i >> 5) & 0xF)) & 0xF006U)
							{
     							case 0x9000: 
								case 0x9002: Map( i, NMT_SWAP_VH01    ); break;
       							case 0x9004: 
								case 0x9006: Map( i, &Vrc4::Poke_9    ); break;
								case 0xB000: Map( i, &Vrc4::Poke_B0_B ); break;
								case 0xB002: Map( i, &Vrc4::Poke_B1_B ); break;
								case 0xB004: Map( i, &Vrc4::Poke_B2_B ); break;
								case 0xB006: Map( i, &Vrc4::Poke_B3_B ); break;
								case 0xC000: Map( i, &Vrc4::Poke_C0_B ); break;
								case 0xC002: Map( i, &Vrc4::Poke_C1_B ); break;
								case 0xC004: Map( i, &Vrc4::Poke_C2_B ); break;
								case 0xC006: Map( i, &Vrc4::Poke_C3_B ); break;
								case 0xD000: Map( i, &Vrc4::Poke_D0_B ); break;
								case 0xD002: Map( i, &Vrc4::Poke_D1_B ); break;
								case 0xD004: Map( i, &Vrc4::Poke_D2_B ); break;
								case 0xD006: Map( i, &Vrc4::Poke_D3_B ); break;
								case 0xE000: Map( i, &Vrc4::Poke_E0_B ); break;
								case 0xE002: Map( i, &Vrc4::Poke_E1_B ); break;
								case 0xE004: Map( i, &Vrc4::Poke_E2_B ); break;
								case 0xE006: Map( i, &Vrc4::Poke_E3_B ); break;
								case 0xF000: Map( i, &Vrc4::Poke_F0   ); break;
								case 0xF002: Map( i, &Vrc4::Poke_F1   ); break;
								case 0xF004: Map( i, &Vrc4::Poke_F2   ); break;
								case 0xF006: Map( i, &Vrc4::Poke_F3   ); break;
							}
						}
					}
					else if (type == TYPE_B)
					{
						for (dword i=0x9000U; i <= 0xFFFFU; ++i)
						{
							switch (((i | (i >> 2) | (i >> 4) | (i >> 6)) & 0x3) | (i & 0xF000U))
							{
								case 0x9000: 
								case 0x9001: Map( i, NMT_SWAP_VH01    ); break;
								case 0x9002: 
								case 0x9003: Map( i, &Vrc4::Poke_9    ); break;
								case 0xB000: Map( i, &Vrc4::Poke_B0_B ); break;
								case 0xB001: Map( i, &Vrc4::Poke_B1_B ); break;
								case 0xB002: Map( i, &Vrc4::Poke_B2_B ); break;
								case 0xB003: Map( i, &Vrc4::Poke_B3_B ); break;
								case 0xC000: Map( i, &Vrc4::Poke_C0_B ); break;
								case 0xC001: Map( i, &Vrc4::Poke_C1_B ); break;
								case 0xC002: Map( i, &Vrc4::Poke_C2_B ); break;
								case 0xC003: Map( i, &Vrc4::Poke_C3_B ); break;
								case 0xD000: Map( i, &Vrc4::Poke_D0_B ); break;
								case 0xD001: Map( i, &Vrc4::Poke_D1_B ); break;
								case 0xD002: Map( i, &Vrc4::Poke_D2_B ); break;
								case 0xD003: Map( i, &Vrc4::Poke_D3_B ); break;
								case 0xE000: Map( i, &Vrc4::Poke_E0_B ); break;
								case 0xE001: Map( i, &Vrc4::Poke_E1_B ); break;
								case 0xE002: Map( i, &Vrc4::Poke_E2_B ); break;
								case 0xE003: Map( i, &Vrc4::Poke_E3_B ); break;
								case 0xF000: Map( i, &Vrc4::Poke_F0   ); break;
								case 0xF001: Map( i, &Vrc4::Poke_F1   ); break;
								case 0xF002: Map( i, &Vrc4::Poke_F2   ); break;
								case 0xF003: Map( i, &Vrc4::Poke_F3   ); break;
							}
						}
					}
					else
					{
						for (dword i=0x9000U; i <= 0xFFFFU; ++i)
						{
							switch ((i | ((i >> 2) & 0x3)) & 0xF003U)
							{
								case 0x9000: 
								case 0x9002: Map( i, NMT_SWAP_VH01    ); break;
								case 0x9001: 
								case 0x9003: Map( i, &Vrc4::Poke_9    ); break;
								case 0xB000: Map( i, &Vrc4::Poke_B0_B ); break;
								case 0xB001: Map( i, &Vrc4::Poke_B2_B ); break;
								case 0xB002: Map( i, &Vrc4::Poke_B1_B ); break;
								case 0xB003: Map( i, &Vrc4::Poke_B3_B ); break;
								case 0xC000: Map( i, &Vrc4::Poke_C0_B ); break;
								case 0xC001: Map( i, &Vrc4::Poke_C2_B ); break;
								case 0xC002: Map( i, &Vrc4::Poke_C1_B ); break;
								case 0xC003: Map( i, &Vrc4::Poke_C3_B ); break;
								case 0xD000: Map( i, &Vrc4::Poke_D0_B ); break;
								case 0xD001: Map( i, &Vrc4::Poke_D2_B ); break;
								case 0xD002: Map( i, &Vrc4::Poke_D1_B ); break;
								case 0xD003: Map( i, &Vrc4::Poke_D3_B ); break;
								case 0xE000: Map( i, &Vrc4::Poke_E0_B ); break;
								case 0xE001: Map( i, &Vrc4::Poke_E2_B ); break;
								case 0xE002: Map( i, &Vrc4::Poke_E1_B ); break;
								case 0xE003: Map( i, &Vrc4::Poke_E3_B ); break;
								case 0xF000: Map( i, &Vrc4::Poke_F0   ); break;
								case 0xF001: Map( i, &Vrc4::Poke_F2   ); break;
								case 0xF002: Map( i, &Vrc4::Poke_F1   ); break;
								case 0xF003: Map( i, &Vrc4::Poke_F3   ); break;
							}
						}
					}
				}
			}
		
			void Vrc4::BaseLoad(State::Loader& state,const dword id)
			{
				NST_ASSERT( id == NES_STATE_CHUNK_ID('V','R','4','\0') );

				if (id == NES_STATE_CHUNK_ID('V','R','4','\0'))
				{
					while (const dword chunk = state.Begin())
					{
						switch (chunk)
						{
							case NES_STATE_CHUNK_ID('R','E','G','\0'):
						
								NST_VERIFY( type != TYPE_A );
						
								if (type != TYPE_A)
									prgSwap = state.Read8() & 0x2;
						
								break;
						
							case NES_STATE_CHUNK_ID('I','R','Q','\0'):
						
								NST_VERIFY( type != TYPE_A );
						
								if (type != TYPE_A)
									irq->LoadState( State::Loader::Subset(state).Ref() );
						
								break;
						}

						state.End();
					}
				}
			}
		
			void Vrc4::BaseSave(State::Saver& state) const
			{
				state.Begin('V','R','4','\0');

				if (type != TYPE_A)
				{
					state.Begin('R','E','G','\0').Write8( prgSwap ).End();
					irq->SaveState( State::Saver::Subset(state,'I','R','Q','\0').Ref() );
				}

				state.End();
			}
		
			void Vrc4::Irq::LoadState(State::Loader& state)
			{
				const State::Loader::Data<5> data( state );

				unit.ctrl = data[0] & (BaseIrq::ENABLE_1|BaseIrq::NO_PPU_SYNC);
				EnableLine( data[0] & BaseIrq::ENABLE_0 );
				unit.latch = data[1];
				unit.count[0] = NST_MIN(340,data[2] | (data[3] << 8));
				unit.count[1] = data[4];
			}

			void Vrc4::Irq::SaveState(State::Saver& state) const
			{
				const u8 data[5] =
				{
					unit.ctrl | (IsLineEnabled() ? BaseIrq::ENABLE_0 : 0),
					unit.latch,
					unit.count[0] & 0xFF,
					unit.count[0] >> 8,
					unit.count[1]
				};

				state.Write( data );
			}

            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif
		
			NES_POKE(Vrc4,8)
			{
				prg.SwapBank<SIZE_8K>( (prgSwap << 13), data );
			}
	
			NES_POKE(Vrc4,9) 
			{ 
				data &= 0x2;

				if (prgSwap != data)
				{
					prgSwap = data;
					
					prg.SwapBanks<SIZE_8K,0x0000U>
					( 
				     	prg.GetBank<SIZE_8K,0x4000U>(), 
						prg.GetBank<SIZE_8K,0x0000U>() 
					);
				}
			}

			void Vrc4::SwapChrA(const uint address,const uint data) const
			{
				ppu.Update(); 
				chr.SwapBank<SIZE_1K>( address, data >> 1 );
			}

			NES_POKE(Vrc4,B0_A) { SwapChrA( 0x0000U, data ); }
			NES_POKE(Vrc4,B1_A) { SwapChrA( 0x0400U, data ); }																   
			NES_POKE(Vrc4,C0_A) { SwapChrA( 0x0800U, data ); }																   
			NES_POKE(Vrc4,C1_A) { SwapChrA( 0x0C00U, data ); }																   
			NES_POKE(Vrc4,D0_A) { SwapChrA( 0x1000U, data ); }																   
			NES_POKE(Vrc4,D1_A) { SwapChrA( 0x1400U, data ); }																   
			NES_POKE(Vrc4,E0_A) { SwapChrA( 0x1800U, data ); }																   
			NES_POKE(Vrc4,E1_A) { SwapChrA( 0x1C00U, data ); }																   

			template<uchar MASK,uchar SHIFT>
			void Vrc4::SwapChrB(const uint address,const uint data) const
			{
				ppu.Update(); 
				chr.SwapBank<SIZE_1K>( address, (chr.GetBank<SIZE_1K>(address) & MASK) | ((data & 0xF) << SHIFT) );
			}

			NES_POKE(Vrc4,B0_B) { SwapChrB<0xF0,0>( 0x0000U, data ); }
			NES_POKE(Vrc4,B1_B) { SwapChrB<0x0F,4>( 0x0000U, data ); }
			NES_POKE(Vrc4,B2_B) { SwapChrB<0xF0,0>( 0x0400U, data ); }
			NES_POKE(Vrc4,B3_B) { SwapChrB<0x0F,4>( 0x0400U, data ); }
			NES_POKE(Vrc4,C0_B) { SwapChrB<0xF0,0>( 0x0800U, data ); }
			NES_POKE(Vrc4,C1_B) { SwapChrB<0x0F,4>( 0x0800U, data ); }
			NES_POKE(Vrc4,C2_B) { SwapChrB<0xF0,0>( 0x0C00U, data ); }
			NES_POKE(Vrc4,C3_B) { SwapChrB<0x0F,4>( 0x0C00U, data ); }
			NES_POKE(Vrc4,D0_B) { SwapChrB<0xF0,0>( 0x1000U, data ); }
			NES_POKE(Vrc4,D1_B) { SwapChrB<0x0F,4>( 0x1000U, data ); }
			NES_POKE(Vrc4,D2_B) { SwapChrB<0xF0,0>( 0x1400U, data ); }
			NES_POKE(Vrc4,D3_B) { SwapChrB<0x0F,4>( 0x1400U, data ); }
			NES_POKE(Vrc4,E0_B) { SwapChrB<0xF0,0>( 0x1800U, data ); }
			NES_POKE(Vrc4,E1_B) { SwapChrB<0x0F,4>( 0x1800U, data ); }
			NES_POKE(Vrc4,E2_B) { SwapChrB<0xF0,0>( 0x1C00U, data ); }
			NES_POKE(Vrc4,E3_B) { SwapChrB<0x0F,4>( 0x1C00U, data ); }

			void Vrc4::Irq::WriteLatch0(const uint data) 
			{ 
				Update();
				unit.latch = (unit.latch & 0xF0) | (data & 0xF);
			}

			void Vrc4::Irq::WriteLatch1(const uint data) 
			{
				Update();
				unit.latch = (unit.latch & 0x0F) | ((data & 0xF) << 4);
			}

			void Vrc4::Irq::Toggle(const uint data) 
			{
				Update();
				unit.ctrl = data & (BaseIrq::ENABLE_1|BaseIrq::NO_PPU_SYNC);

				if (EnableLine( data & BaseIrq::ENABLE_0 ))
				{
					unit.count[0] = 0;
					unit.count[1] = unit.latch;
				}

				ClearIRQ();
			}

			void Vrc4::Irq::Toggle() 
			{ 
				Update();
				EnableLine( unit.ctrl & BaseIrq::ENABLE_1 );
				ClearIRQ();
			}

			NES_POKE(Vrc4,F0) 
			{ 
				irq->WriteLatch0( data );
			}

			NES_POKE(Vrc4,F1) 
			{
				irq->WriteLatch1( data );
			}

			NES_POKE(Vrc4,F2) 
			{
				irq->Toggle( data );
			}

			NES_POKE(Vrc4,F3)
			{ 
				irq->Toggle();
			}
		
			ibool Vrc4::BaseIrq::Signal()
			{
				if (!(ctrl & NO_PPU_SYNC))
				{
					count[0] += 3;

					if (count[0] < 341)
						return false;

					count[0] -= 341;
				}

				if (count[1]++ == 0xFF)
				{
					count[1] = latch;
					return true;
				}

				return false;
			}

			void Vrc4::VSync()
			{
				if (irq)
					irq->VSync();
			}
		}
	}
}
