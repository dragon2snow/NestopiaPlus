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
#include "NstBrdS8259.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			void S8259::SubReset(const bool hard)
			{
				if (hard)
				{
					ctrl = 0;
					std::memset( regs, 0, sizeof(regs) );
				}

				if (type == TYPE_D && !chr.Source().IsWritable())
					chr.SwapBank<SIZE_4K,0x1000U>( ~0U );

				for (uint i=0x4100U; i < 0x6000U; i += 0x200)
				{
					for (uint j=0; j < 0x100; j += 0x2)
					{
						Map( i + j + 0x0, &S8259::Poke_4100 );
						Map( i + j + 0x1, &S8259::Poke_4101 );
					}
				}
			}

			void S8259::BaseLoad(State::Loader& state,const dword id)
			{
				NST_VERIFY( id == NES_STATE_CHUNK_ID('S','8','2','\0') );

				if (id == NES_STATE_CHUNK_ID('S','8','2','\0'))
				{
					while (const dword chunk = state.Begin())
					{
						if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
						{
							ctrl = state.Read8();
							state.Read( regs );
						}

						state.End();
					}
				}
			}

			void S8259::BaseSave(State::Saver& state) const
			{
				state.Begin('S','8','2','\0').Begin('R','E','G','\0').Write8( ctrl ).Write( regs ).End().End();
			}

			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void S8259::UpdateChr() const
			{
				for (uint i=0; i < 4; ++i)
				{
					uint bank;

					if (type == TYPE_D)
					{
						bank = regs[i] & 0x7;

						switch (i & 3)
						{
							case 01: bank |= (regs[4] & 0x1) << 4; break;
							case 02: bank |= (regs[4] & 0x2) << 3; break;
							case 03: bank |= ((regs[4] & 0x4) << 2) | ((regs[6] & 0x1) << 3); break;
						}

						chr.SwapBank<SIZE_1K>( i << 10, bank );
					}
					else
					{
						bank = (regs[(regs[7] & 0x1) ? 0 : i] & 0x7) | ((regs[4] & 0x7) << 3);

						if (type == TYPE_A)
						{
							bank = (bank << 1) | (i & 0x1);
						}
						else if (type == TYPE_C)
						{
							bank = (bank << 2) | (i & 0x3);
						}

						chr.SwapBank<SIZE_2K>( i << 11, bank );
					}
				}
			}

			NES_POKE(S8259,4100)
			{
				ctrl = data;
			}

			NES_POKE(S8259,4101)
			{
				regs[ctrl & 0x7] = data;

				switch (ctrl & 0x7)
				{
					case 0x5:

						prg.SwapBank<SIZE_32K,0x0000U>( data );
						return;

					case 0x7:
					{
						static const uchar lut[4][4] =
						{
							{0,1,0,1},
							{0,0,1,1},
							{0,1,1,1},
							{0,0,0,0}
						};

						ppu.SetMirroring( lut[(data >> 1) & 0x3] );
					}

					default:

						ppu.Update();
						break;
				}

				if (!chr.Source().IsWritable())
					UpdateChr();
			}
		}
	}
}
