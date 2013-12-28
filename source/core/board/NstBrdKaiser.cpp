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
#include "NstBrdKaiser.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Kaiser::Kaiser(Context& c,Type type)
			:
			Mapper (c,(PROM_MAX_256K|CROM_MAX_256K) | (type == TYPE_7032 ? WRAM_NONE : WRAM_8K)),
			irq    (c.cpu)
			{}

			void Kaiser::Irq::Reset(const bool hard)
			{
				if (hard)
				{
					count = 0;
					latch = 0;
					ctrl = 0;
				}
			}

			void Kaiser::SubReset(const bool hard)
			{
				if (!wrk.Available())
					Map( WRK_PEEK );

				Map( 0x8000U, 0x8FFFU, &Kaiser::Poke_8000 );
				Map( 0x9000U, 0x9FFFU, &Kaiser::Poke_9000 );
				Map( 0xA000U, 0xAFFFU, &Kaiser::Poke_A000 );
				Map( 0xB000U, 0xBFFFU, &Kaiser::Poke_B000 );
				Map( 0xC000U, 0xCFFFU, &Kaiser::Poke_C000 );
				Map( 0xD000U, 0xDFFFU, &Kaiser::Poke_D000 );
				Map( 0xE000U, 0xEFFFU, &Kaiser::Poke_E000 );
				Map( 0xF000U, 0xFFFFU, &Kaiser::Poke_F000 );

				if (hard)
					ctrl = 0;

				irq.Reset( hard, hard ? false : irq.Connected() );
			}

			void Kaiser::SubLoad(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					switch (chunk)
					{
						case AsciiId<'R','E','G'>::V:

							ctrl = state.Read8();
							break;

						case AsciiId<'I','R','Q'>::V:
						{
							State::Loader::Data<5> data( state );

							irq.unit.ctrl = data[0];
							irq.unit.count = data[1] | data[2] << 8;
							irq.unit.latch = data[3] | data[4] << 8;
							irq.Connect( data[0] & 0xF );

							break;
						}
					}

					state.End();
				}
			}

			void Kaiser::SubSave(State::Saver& state) const
			{
				state.Begin( AsciiId<'R','E','G'>::V ).Write8( ctrl ).End();

				const byte data[5] =
				{
					irq.unit.ctrl,
					irq.unit.count & 0xFF,
					irq.unit.count >> 8,
					irq.unit.latch & 0xFF,
					irq.unit.latch >> 8
				};

				state.Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End();
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			NES_POKE_D(Kaiser,8000)
			{
				irq.Update();
				irq.unit.latch = (irq.unit.latch & 0xFFF0) | (data & 0xF) << 0;
			}

			NES_POKE_D(Kaiser,9000)
			{
				irq.Update();
				irq.unit.latch = (irq.unit.latch & 0xFF0F) | (data & 0xF) << 4;
			}

			NES_POKE_D(Kaiser,A000)
			{
				irq.Update();
				irq.unit.latch = (irq.unit.latch & 0xF0FF) | (data & 0xF) << 8;
			}

			NES_POKE_D(Kaiser,B000)
			{
				irq.Update();
				irq.unit.latch = (irq.unit.latch & 0x0FFF) | (data & 0xF) << 12;
			}

			NES_POKE_D(Kaiser,C000)
			{
				irq.Update();

				irq.unit.ctrl = data;

				if (irq.Connect( data & 0xF ))
					irq.unit.count = irq.unit.latch;

				irq.ClearIRQ();
			}

			NES_POKE(Kaiser,D000)
			{
				irq.Update();
				irq.ClearIRQ();
			}

			NES_POKE_D(Kaiser,E000)
			{
				ctrl = data;
			}

			NES_POKE_AD(Kaiser,F000)
			{
				{
					uint offset = (ctrl & 0xF) - 1;

					if (offset < 3)
					{
						offset <<= 13;
						prg.SwapBank<SIZE_8K>( offset, (data & 0x0F) | (prg.GetBank<SIZE_8K>(offset) & 0x10) );
					}
					else if (offset < 4)
					{
						wrk.SwapBank<SIZE_8K,0x0000>( data );
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

			bool Kaiser::Irq::Clock()
			{
				return (count++ == 0xFFFF) ? (count=latch, true) : false;
			}

			void Kaiser::Sync(Event event,Input::Controllers*)
			{
				if (event == EVENT_END_FRAME)
					irq.VSync();
			}
		}
	}
}
