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
#include "../NstClock.hpp"
#include "NstMapper083.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper83::Irq::Reset(const bool hard)
		{
			if (hard)
			{
				enabled = false;
				count = 0;
				step = 1;
			}
		}

		Mapper83::Mapper83(Context& c)
		:
		Mapper   (c,WRAM_DEFAULT),
		irq      (c.cpu),
		language (c.attribute == ATR_LANGUAGE_SELECT ? 0xFF : 0x00)
		{}

		void Mapper83::SubReset(const bool hard)
		{
			irq.Reset( hard, true );

			if (hard)
			{
				regs.ctrl = 0;

				for (uint i=0; i < 5; ++i)
					regs.prg[i] = 0;

				regs.pr8 = 0;
				dipSwitch = 0xFF;
			}

			dipSwitch ^= language;

			UpdatePrg();

			Map( 0x5000U, &Mapper83::Peek_5000 );
			Map( 0x5100U, 0x51FF, &Mapper83::Peek_5100, &Mapper83::Poke_5100 );

			if (!wrk.Source().Writable())
				Map( 0x6000U, 0x7FFFU, &Mapper83::Peek_6000 );

			for (uint i=0x8000; i < 0x9000; i += 0x400)
			{
				Map( i+0x000, i+0x0FF, &Mapper83::Poke_8000 );
				Map( i+0x100, i+0x1FF, &Mapper83::Poke_8100 );

				for (uint j=i+0x00, n=i+0x100; j < n; j += 0x02)
				{
					Map( j+0x200, &Mapper83::Poke_8200 );
					Map( j+0x201, &Mapper83::Poke_8201 );
				}

				for (uint j=i+0x00, n=i+0x100; j < n; j += 0x20)
				{
					Map( j+0x300, j+0x30F, &Mapper83::Poke_8300 );

					if (chr.Source().Size() == SIZE_512K)
					{
						Map( j+0x310, j+0x311, &Mapper83::Poke_8310_1 );
						Map( j+0x316, j+0x317, &Mapper83::Poke_8310_1 );
					}
					else
					{
						Map( j+0x310, j+0x317, &Mapper83::Poke_8310_0 );
					}
				}
			}

			Map( 0xB000U, &Mapper83::Poke_8000 );
			Map( 0xB0FFU, &Mapper83::Poke_8000 );
			Map( 0xB100U, &Mapper83::Poke_8000 );
		}

		void Mapper83::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case AsciiId<'R','E','G'>::V:

						regs.ctrl = state.Read8();
						state.Read( regs.prg );
						break;

					case AsciiId<'P','R','8'>::V:

						regs.pr8 = state.Read8();
						break;

					case AsciiId<'I','R','Q'>::V:
					{
						State::Loader::Data<3> data( state );

						irq.unit.enabled = data[0] & 0x1;
						irq.unit.step = (data[0] & 0x2) ? ~0U : 1U;
						irq.unit.count = data[1] | data[2] << 8;

						break;
					}

					case AsciiId<'L','A','N'>::V:

						dipSwitch = (state.Read8() & 0x1) ? 0xFF : 0x00;
						break;
				}

				state.End();
			}
		}

		void Mapper83::SubSave(State::Saver& state) const
		{
			{
				const byte data[1+5] =
				{
					regs.ctrl,
					regs.prg[0],
					regs.prg[1],
					regs.prg[2],
					regs.prg[3],
					regs.prg[4]
				};

				state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
			}

			state.Begin( AsciiId<'P','R','8'>::V ).Write8( regs.pr8 ).End();

			{
				const byte data[3] =
				{
					(irq.unit.enabled ? 0x1U : 0x0U) |
					(irq.unit.step == 1  ? 0x0U : 0x2U),
					irq.unit.count & 0xFF,
					irq.unit.count >> 8
				};

				state.Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End();
			}

			state.Begin( AsciiId<'L','A','N'>::V ).Write8( dipSwitch & 0x1 ).End();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Mapper83::UpdatePrg()
		{
			if (regs.ctrl & 0x10U)
			{
				prg.SwapBanks<SIZE_8K,0x0000>( regs.prg[0], regs.prg[1] );
				prg.SwapBank<SIZE_8K,0x4000>( regs.prg[2] );
			}
			else
			{
				prg.SwapBank<SIZE_16K,0x0000>( regs.prg[4] & 0x3FU );
				prg.SwapBank<SIZE_16K,0x4000>( (regs.prg[4] & 0x30U) | 0x0F );
			}
		}

		NES_PEEK(Mapper83,5000)
		{
			return dipSwitch;
		}

		NES_PEEK(Mapper83,5100)
		{
			return regs.pr8;
		}

		NES_POKE(Mapper83,5100)
		{
			regs.pr8 = data;
		}

		NES_PEEK(Mapper83,6000)
		{
			NST_VERIFY( regs.ctrl & 0x20U );

			if (regs.ctrl & 0x20U)
			{
				const uint bank = (regs.ctrl & 0x10U) ? 0x1F : regs.prg[3];
				return *prg.Source().Mem( (bank << 13) | (address & 0x1FFF) );
			}
			else
			{
				return address >> 8;
			}
		}

		NES_POKE(Mapper83,8000)
		{
			if (regs.prg[4] != data)
			{
				regs.prg[4] = data;
				UpdatePrg();
			}
		}

		NES_POKE(Mapper83,8100)
		{
			const uint diff = data ^ regs.ctrl;
			regs.ctrl = data;

			if (diff & 0x10)
				UpdatePrg();

			if (diff & 0xC0)
			{
				irq.Update();
				irq.unit.step = (data & 0x40) ? ~0U : 1U;
			}

			if (diff & 0x03)
				SetMirroringVH01( data );
		}

		NES_POKE(Mapper83,8200)
		{
			irq.Update();
			irq.unit.count = (irq.unit.count & 0xFF00) | data;
			irq.ClearIRQ();
		}

		NES_POKE(Mapper83,8201)
		{
			irq.Update();
			irq.unit.count = (irq.unit.count & 0x00FF) | (data << 8);
			irq.unit.enabled = regs.ctrl & 0x80U;
			irq.ClearIRQ();
		}

		NES_POKE(Mapper83,8310_0)
		{
			ppu.Update();
			chr.SwapBank<SIZE_1K>( (address & 0x7) << 10, (regs.prg[4] << 4 & 0x300U) | data );
		}

		NES_POKE(Mapper83,8310_1)
		{
			ppu.Update();
			chr.SwapBank<SIZE_2K>( (address & 0x3) << 11, data );
		}

		NES_POKE(Mapper83,8300)
		{
			data &= 0x1F;

			if (regs.prg[address & 0x3] != data)
			{
				regs.prg[address & 0x3] = data;
				UpdatePrg();
			}
		}

		ibool Mapper83::Irq::Signal()
		{
			if (enabled && count)
			{
				count = (count + step) & 0xFFFF;

				if (!count)
				{
					enabled = false;
					return true;
				}
			}

			return false;
		}

		void Mapper83::VSync()
		{
			irq.VSync();
		}
	}
}
