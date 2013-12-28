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
#include "NstBrdMmc3.hpp"
#include "NstBrdTaitoTc.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			TaitoTc::TaitoTc(Context& c,const Type type)
			:
			Mapper (c),
			irq    (type == TYPE_TC190V ? new Mmc3::Irq(c.cpu,c.ppu,Mmc3::Irq::IRQ_DELAY) : NULL)
			{
			}

			TaitoTc::~TaitoTc()
			{
				delete irq;
			}

			void TaitoTc::SubReset(const bool hard)
			{
				if (irq)
					irq->Reset( hard, hard || irq->IsLineEnabled() );

				for (uint i=0x0000U; i < 0x1000U; i += 0x4)
				{
					if (irq)
					{
						Map( 0x8000U + i, PRG_SWAP_8K_0 );
						Map( 0xC000U + i, &TaitoTc::Poke_C000 );
						Map( 0xC001U + i, &TaitoTc::Poke_C001 );
						Map( 0xC002U + i, &TaitoTc::Poke_C002 );
						Map( 0xC003U + i, &TaitoTc::Poke_C003 );
						Map( 0xE000U + i, &TaitoTc::Poke_E000 );
					}
					else
					{
						Map( 0x8000U + i, &TaitoTc::Poke_8000 );
					}

					Map( 0x8001U + i, PRG_SWAP_8K_1 );
					Map( 0x8002U + i, CHR_SWAP_2K_0 );
					Map( 0x8003U + i, CHR_SWAP_2K_1 );
					Map( 0xA000U + i, CHR_SWAP_1K_4 );
					Map( 0xA001U + i, CHR_SWAP_1K_5 );
					Map( 0xA002U + i, CHR_SWAP_1K_6 );
					Map( 0xA003U + i, CHR_SWAP_1K_7 );
				}
			}

			void TaitoTc::BaseLoad(State::Loader& state,const dword id)
			{
				NST_VERIFY( id == NES_STATE_CHUNK_ID('T','T','C','\0') );

				if (id == NES_STATE_CHUNK_ID('T','T','C','\0'))
				{
					while (const dword chunk = state.Begin())
					{
						if (chunk == NES_STATE_CHUNK_ID('I','R','Q','\0'))
						{
							NST_VERIFY( irq );

							if (irq)
								irq->unit.LoadState( state );
						}

						state.End();
					}
				}
			}

			void TaitoTc::BaseSave(State::Saver& state) const
			{
				state.Begin('T','T','C','\0');

				if (irq)
					irq->unit.SaveState( State::Saver::Subset(state,'I','R','Q','\0').Ref() );

				state.End();
			}

			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("", on)
			#endif

			NES_POKE(TaitoTc,8000)
			{
				prg.SwapBank<SIZE_8K,0x0000U>( data );
				ppu.SetMirroring( (data & 0x40) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
			}

			NES_POKE(TaitoTc,C000)
			{
				irq->Update();
				irq->unit.SetLatch( (0x100 - data) & 0xFF );
			}

			NES_POKE(TaitoTc,C001)
			{
				irq->Update();
				irq->unit.Reload();
			}

			NES_POKE(TaitoTc,C002)
			{
				irq->Update();
				irq->unit.Enable();
			}

			NES_POKE(TaitoTc,C003)
			{
				irq->Update();
				irq->unit.Disable( cpu );
			}

			NES_POKE(TaitoTc,E000)
			{
				ppu.SetMirroring( (data & 0x40) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
			}

			void TaitoTc::VSync()
			{
				if (irq)
					irq->VSync();
			}
		}
	}
}
