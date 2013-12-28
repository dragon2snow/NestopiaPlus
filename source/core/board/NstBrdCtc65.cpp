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
#include "NstBrdCtc65.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			void Ctc65::SubReset(bool)
			{
				for (dword i=0x8000; i <= 0xFFFF; i += 0x2)
				{
					Map( i + 0x0, &Ctc65::Peek_Prg, &Ctc65::Poke_8000 );
					Map( i + 0x1, &Ctc65::Peek_Prg, &Ctc65::Poke_8001 );
				}

				regs[0] = 0;
				regs[1] = 0;

				UpdatePrg();
			}

			void Ctc65::SubSave(State::Saver& state) const
			{
				const byte data[2] = {regs[0],regs[1]};
				state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
			}

			void Ctc65::SubLoad(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					if (chunk == AsciiId<'R','E','G'>::V)
					{
						State::Loader::Data<2> data( state );

						regs[0] = data[0];
						regs[1] = data[1];

						UpdatePrg();
					}

					state.End();
				}
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void Ctc65::UpdatePrg()
			{
				const uint chip = (regs[1] << 5 & 0x20) << (regs[0] >> 7);
				openBus = chip < (regs[0] >> 7);

				prg.SwapBanks<SIZE_16K,0x0000>
				(
					chip | (regs[0] & 0x1E) | (regs[0] >> 5 & regs[0]),
					chip | (regs[0] & 0x1F) | (~regs[0] >> 5 & 0x01)
				);
			}

			NES_POKE_D(Ctc65,8000)
			{
				if (regs[0] != data)
				{
					regs[0] = data;
					UpdatePrg();
					ppu.SetMirroring( (data & 0x40) ? Ppu::NMT_VERTICAL : Ppu::NMT_HORIZONTAL );
				}
			}

			NES_POKE_D(Ctc65,8001)
			{
				if (regs[1] != data)
				{
					regs[1] = data;
					UpdatePrg();
				}
			}

			NES_PEEK_A(Ctc65,Prg)
			{
				return !openBus ? prg.Peek( address - 0x8000 ) : (address >> 8);
			}
		}
	}
}
