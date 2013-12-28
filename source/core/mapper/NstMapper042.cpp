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
#include "NstMapper042.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper42::Mapper42(Context& c)
		:
		Mapper (c,WRAM_NONE),
		irq    (c.cpu,c.cpu)
		{}

		void Mapper42::Irq::Reset(const bool hard)
		{
			if (hard)
				count = 0;
		}

		void Mapper42::SubReset(const bool hard)
		{
			irq.Reset( hard, hard ? false : irq.IsLineEnabled() );

			Map( WRK_PEEK );

			for (uint i=0x0000; i < 0x2000; i += 0x4)
			{
				Map( 0x8000 + i, CHR_SWAP_8K          );
				Map( 0xE000 + i, &Mapper42::Poke_E000 );
				Map( 0xE001 + i, &Mapper42::Poke_E001 );
				Map( 0xE002 + i, &Mapper42::Poke_E002 );
			}
		}

		void Mapper42::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == AsciiId<'I','R','Q'>::V)
				{
					State::Loader::Data<3> data( state );

					irq.EnableLine( data[0] & 0x1 );
					irq.unit.count = data[1] | (data[2] << 8 & 0x7F00);
				}

				state.End();
			}
		}

		void Mapper42::SubSave(State::Saver& state) const
		{
			const byte data[3] =
			{
				irq.IsLineEnabled() ? 0x1 : 0x0,
				irq.unit.count >> 0 & 0xFF,
				irq.unit.count >> 8 & 0x7F
			};

			state.Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE(Mapper42,E000)
		{
			wrk.SwapBank<SIZE_8K,0x0000>(data & 0xF);
		}

		NES_POKE(Mapper42,E001)
		{
			ppu.SetMirroring( (data & 0x8) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
		}

		NES_POKE(Mapper42,E002)
		{
			irq.Update();

			if (!irq.EnableLine( data & 0x2 ))
			{
				irq.unit.count = 0;
				irq.ClearIRQ();
			}
		}

		ibool Mapper42::Irq::Signal()
		{
			const uint prev = count++;

			if ((count & 0x6000) != (prev & 0x6000))
			{
				if ((count & 0x6000) == 0x6000)
					return true;
				else
					cpu.ClearIRQ();
			}

			return false;
		}

		void Mapper42::VSync()
		{
			irq.VSync();
		}
	}
}
