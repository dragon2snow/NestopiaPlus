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
#include "NstBrd64in1Nr.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif
		
			void Bmc64In1Nr::SubReset(bool)
			{
				Map( 0x5000U, 0x5003U, &Bmc64In1Nr::Poke_5000 );
				Map( 0x8000U, 0xFFFFU, &Bmc64In1Nr::Poke_8000 );

				regs[0] = 0x80;
				regs[1] = 0x43;
				regs[2] = 0x00;
				regs[3] = 0x00;

				Update();
			}
		
			void Bmc64In1Nr::SubSave(State::Saver& state) const
			{
				state.Begin('R','E','G','\0').Write( regs ).End();
			}

			void Bmc64In1Nr::SubLoad(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
						state.Read( regs );

					state.End();
				}
			}

            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif

			void Bmc64In1Nr::Update()
			{
				uint bank = regs[1] & 0x1F;

				if (regs[0] & regs[1] & 0x80)
				{
					prg.SwapBank<SIZE_32K,0x0000U>( bank );
				}
				else
				{
					bank = (bank << 1) | (regs[1] >> 6 & 0x1);
					prg.SwapBank<SIZE_16K,0x4000U>( bank );

					if (regs[0] & 0x80)
						prg.SwapBank<SIZE_16K,0x0000U>( bank );
				}

				ppu.SetMirroring( (regs[0] & 0x20) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
				chr.SwapBank<SIZE_8K,0x0000U>( (regs[2] << 2) | (regs[0] >> 1 & 0x3) );
			}

			NES_POKE(Bmc64In1Nr,5000) 
			{ 
				regs[address & 0x3] = data;
				Update();
			}

			NES_POKE(Bmc64In1Nr,8000) 
			{ 
				regs[3] = data;
				Update();
			}
		}
	}
}
