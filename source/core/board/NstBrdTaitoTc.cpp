////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2007 Martin Freij
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
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			TaitoTc::TaitoTc(Context& c,const Type type)
			:
			Mapper (c,CROM_MAX_512K|WRAM_DEFAULT|NMT_VERTICAL),
			irq    (type == TYPE_TC190V ? new Mmc3::Irq<IRQ_DELAY>(c.cpu,c.ppu,false) : NULL)
			{
			}

			TaitoTc::~TaitoTc()
			{
				delete irq;
			}

			void TaitoTc::SubReset(const bool hard)
			{
				if (irq)
					irq->Reset( hard, hard || irq->Connected() );

				for (uint i=0x0000; i < 0x1000; i += 0x4)
				{
					if (irq)
					{
						Map( 0x8000 + i, PRG_SWAP_8K_0 );
						Map( 0xC000 + i, &TaitoTc::Poke_C000 );
						Map( 0xC001 + i, &TaitoTc::Poke_C001 );
						Map( 0xC002 + i, &TaitoTc::Poke_C002 );
						Map( 0xC003 + i, &TaitoTc::Poke_C003 );
						Map( 0xE000 + i, &TaitoTc::Poke_E000 );
					}
					else
					{
						Map( 0x8000 + i, &TaitoTc::Poke_8000 );
					}

					Map( 0x8001 + i, PRG_SWAP_8K_1 );
					Map( 0x8002 + i, CHR_SWAP_2K_0 );
					Map( 0x8003 + i, CHR_SWAP_2K_1 );
					Map( 0xA000 + i, CHR_SWAP_1K_4 );
					Map( 0xA001 + i, CHR_SWAP_1K_5 );
					Map( 0xA002 + i, CHR_SWAP_1K_6 );
					Map( 0xA003 + i, CHR_SWAP_1K_7 );
				}
			}

			void TaitoTc::BaseLoad(State::Loader& state,const dword baseChunk)
			{
				NST_VERIFY( baseChunk == (AsciiId<'T','T','C'>::V) );

				if (baseChunk == AsciiId<'T','T','C'>::V)
				{
					while (const dword chunk = state.Begin())
					{
						if (chunk == AsciiId<'I','R','Q'>::V)
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
				state.Begin( AsciiId<'T','T','C'>::V );

				if (irq)
					irq->unit.SaveState( state, AsciiId<'I','R','Q'>::V );

				state.End();
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			NES_POKE_D(TaitoTc,8000)
			{
				prg.SwapBank<SIZE_8K,0x0000>( data );
				ppu.SetMirroring( (data & 0x40) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
			}

			NES_POKE_D(TaitoTc,C000)
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

			NES_POKE_D(TaitoTc,E000)
			{
				ppu.SetMirroring( (data & 0x40) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
			}

			void TaitoTc::Sync(Event event,Input::Controllers*)
			{
				if (event == EVENT_END_FRAME)
				{
					if (irq)
						irq->VSync();
				}
			}
		}
	}
}
