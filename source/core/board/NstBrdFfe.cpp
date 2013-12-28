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
#include "NstBrdFfe.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif
		
			uint Ffe::GetIrqBase(dword pRomCrc)
			{
				switch (pRomCrc)
				{
					case 0x57BAF095UL: // Doki! Doki! Yuuenchi
					case 0xE64138ECUL: // Dragon Ball Z 2 - Gekishin Freeza!!
					case 0xC7A4583EUL: // Dragon Ball Z 3 - Ressen Jinzou Ningen
					case 0xCB7E529DUL: // SD Gundam Gaiden - Knight Gundam Monogatari 2 - Hikari no Kishi
					case 0x8F3F8B1FUL: // Spartan X2
					case 0xA3047263UL: // -||-
					case 0xC529C604UL: // -||-
				
						return 0xFFFFU;
				}
		
				return 0xCFFFU;
			}
		
			Ffe::Ffe(Context& c,const Type t)
			: 
			Mapper (c,t == F4_XXX ? CRAM_32K : 0), 
			irq    (t == F3_XXX ? NULL : new Clock::M2<Irq>(c.cpu,GetIrqBase(c.pRomCrc))),
			type   (t)
			{
			}
		
			Ffe::~Ffe()
			{
				delete irq;
			}
		
			void Ffe::Irq::Reset(const bool hard)
			{
				if (hard)
				{
					count = 0;
					enabled = false;
				}
			}
		
			void Ffe::SubReset(const bool hard)
			{
				if (hard)
					mode = 0;

				Map( 0x42FEU, &Ffe::Poke_42FE );
				Map( 0x42FFU, &Ffe::Poke_42FF );
		
				if (irq)
				{
					irq->Reset( hard, hard || irq->IsLineEnabled() );
		
					Map( 0x4501U, &Ffe::Poke_4501 );
					Map( 0x4502U, &Ffe::Poke_4502 );
					Map( 0x4503U, &Ffe::Poke_4503 );
				}
		
				switch (type)
				{
					case F3_XXX:
		
						Map( 0x8000U, 0xFFFFU, &Ffe::Poke_Prg_F3 );

						if (hard)
							prg.SwapBank<SIZE_16K,0x4000U>(1);

						break;
		
					case F4_XXX:
		
						Map( 0x8000U, 0xFFFFU, &Ffe::Poke_Prg_F4 );

						if (hard)
							prg.SwapBank<SIZE_16K,0x4000U>(7);

						break;
		
					case F8_XXX:
		
						Map( 0x4504U, PRG_SWAP_8K_0 );
						Map( 0x4505U, PRG_SWAP_8K_1 );
						Map( 0x4506U, PRG_SWAP_8K_2 );
						Map( 0x4507U, PRG_SWAP_8K_3 );
						Map( 0x4510U, CHR_SWAP_1K_0 );
						Map( 0x4511U, CHR_SWAP_1K_1 );
						Map( 0x4512U, CHR_SWAP_1K_2 );
						Map( 0x4513U, CHR_SWAP_1K_3 );
						Map( 0x4514U, CHR_SWAP_1K_4 );
						Map( 0x4515U, CHR_SWAP_1K_5 );
						Map( 0x4516U, CHR_SWAP_1K_6 );
						Map( 0x4517U, CHR_SWAP_1K_7 );
		
						break;
				}
			}
		
			void Ffe::BaseLoad(State::Loader& state,const dword id)
			{
				NST_VERIFY( id == NES_STATE_CHUNK_ID('F','F','E','\0') );

				if (id == NES_STATE_CHUNK_ID('F','F','E','\0'))
				{
					while (const dword chunk = state.Begin())
					{
						switch (chunk)
						{
							case NES_STATE_CHUNK_ID('R','E','G','\0'):
						
								NST_VERIFY( type == F4_XXX );
						
								if (type == F4_XXX)
									mode = state.Read8() & 0x1;
						
								break;
						
							case NES_STATE_CHUNK_ID('I','R','Q','\0'):
						
								NST_VERIFY( irq );
						
								if (irq)
								{
									const State::Loader::Data<3> data( state );
						
									irq->unit.enabled = data[0] & 0x1;
									irq->unit.count = data[1] | (data[2] << 8);
								}
								break;
						}

						state.End();
					}
				}
			}
		
			void Ffe::BaseSave(State::Saver& state) const
			{
				state.Begin('F','F','E','\0');

				if (type == F4_XXX)
					state.Begin('R','E','G','\0').Write8( mode ).End();

				if (irq)
				{
					const u8 data[3] =
					{
						irq->unit.enabled != false,
						irq->unit.count & 0xFF,
						irq->unit.count >> 8
					};

					state.Begin('I','R','Q','\0').Write( data ).End();
				}

				state.End();
			}
		
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif
		
			ibool Ffe::Irq::Signal()
			{
				if (enabled && count++ == clock)
				{
					count = 0;
					enabled = false;
					return true;
				}
		
				return false;
			}
		
			NES_POKE(Ffe,42FE)  
			{
				mode = data >> 7 ^ 0x1;
				ppu.SetMirroring( (data & 0x10) ? Ppu::NMT_ONE : Ppu::NMT_ZERO );
			}
		
			NES_POKE(Ffe,42FF)  
			{
				ppu.SetMirroring( (data & 0x10) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
			}
		
			NES_POKE(Ffe,4501)  
			{ 
				irq->Update();
				irq->unit.enabled = data & 0x1;
				irq->ClearIRQ();
			}
		
			NES_POKE(Ffe,4502)  
			{ 
				irq->Update();
				irq->unit.count = (irq->unit.count & 0xFF00U) | data;
			}
		
			NES_POKE(Ffe,4503)  
			{
				irq->Update();
				irq->unit.count = (irq->unit.count & 0x00FFU) | (data << 8);
				irq->unit.enabled = true;
				irq->ClearIRQ();
			}
		
			NES_POKE(Ffe,Prg_F3)
			{
				ppu.Update();
				prg.SwapBank<SIZE_16K,0x0000U>( data >> 3 );
				chr.SwapBank<SIZE_8K,0x0000U>( data & 0x7 );
			}
		
			NES_POKE(Ffe,Prg_F4) 
			{
				ppu.Update();

				if (mode || chr.Source(0).IsWritable())
				{
					prg.SwapBank<SIZE_16K,0x0000U>( data >> 2 );
					data &= 0x3;
				}

				chr.Source( mode ).SwapBank<SIZE_8K,0x0000U>( data );
			}

			void Ffe::VSync()
			{
				if (irq)
					irq->VSync();
			}
		}
	}
}
