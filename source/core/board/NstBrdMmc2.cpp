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
#include "NstBrdMmc2.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif

			void Mmc2::SubReset(const bool hard)
			{
				if (hard)
				{
					selector[1] = selector[0] = 0;
					banks[1][1] = banks[1][0] = banks[0][1] = banks[0][0] = 0;
				}

				chr.SetAccessor( 0, this, &Mmc2::Access_Chr_0000 );
				chr.SetAccessor( 1, this, &Mmc2::Access_Chr_1000 );

				Map( 0xA000U, 0xAFFFU, PRG_SWAP_8K_0 );
				Map( 0xB000U, 0xBFFFU, &Mmc2::Poke_B000 );
				Map( 0xC000U, 0xCFFFU, &Mmc2::Poke_C000 );
				Map( 0xD000U, 0xDFFFU, &Mmc2::Poke_D000 );
				Map( 0xE000U, 0xEFFFU, &Mmc2::Poke_E000 );
				Map( 0xF000U, 0xFFFFU, NMT_SWAP_HV );
			}
	
			void Mmc2::BaseLoad(State::Loader& state,const dword id)
			{
				NST_VERIFY( id == NES_STATE_CHUNK_ID('M','M','2','\0') );

				if (id == NES_STATE_CHUNK_ID('M','M','2','\0'))
				{
					while (const dword id = state.Begin())
					{
						if (id == NES_STATE_CHUNK_ID('R','E','G','\0'))
						{
							const State::Loader::Data<4+1> data( state );

							banks[0][0] = data[0];
							banks[0][1] = data[1];
							banks[1][0] = data[2];
							banks[1][1] = data[3];	
							selector[0] = (data[4] >> 0) & 0x1;
							selector[1] = (data[4] >> 1) & 0x1;
						}

						state.End();
					}
				}
			}
	
			void Mmc2::BaseSave(State::Saver& state) const
			{
				const u8 data[4+1] =
				{
					banks[0][0],
					banks[0][1],
					banks[1][0],
					banks[1][1],
					selector[0] | (selector[1] << 1)
				};

				state.Begin('M','M','2','\0').Begin('R','E','G','\0').Write( data ).End().End();
			}

            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif

			NES_ACCESSOR(Mmc2,Chr_0000)
			{
				const uint data = chr.Peek( address );	

				switch (address & 0xFF8)
				{
					case 0xFD8: selector[0] = 0; break;
					case 0xFE8: selector[0] = 1; break;
					default: return data;
				}
	
				chr.SwapBank<SIZE_4K,0x0000U>( banks[0][selector[0]] );
	
				return data;
			}

			NES_ACCESSOR(Mmc2,Chr_1000)
			{
				const uint data = chr.Peek( address );	

				switch (address & 0xFF8)
				{
					case 0xFD8: selector[1] = 0; break;
					case 0xFE8: selector[1] = 1; break;
					default: return data;
				}
	
				chr.SwapBank<SIZE_4K,0x1000U>( banks[1][selector[1]] );
	
				return data;
			}

			void Mmc2::UpdateChr() const
			{
				chr.SwapBanks<SIZE_4K,0x0000U>( banks[0][selector[0]], banks[1][selector[1]] );
			}

			NES_POKE(Mmc2,B000)
			{
				ppu.Update();
				banks[0][0] = data;
				UpdateChr();
			}
	
			NES_POKE(Mmc2,C000)
			{
				ppu.Update();
				banks[0][1] = data;
				UpdateChr();
			}
	
			NES_POKE(Mmc2,D000)
			{
				ppu.Update();
				banks[1][0] = data;
				UpdateChr();
			}
	
			NES_POKE(Mmc2,E000)
			{
				ppu.Update();
				banks[1][1] = data;
				UpdateChr();
			}
		}
	}
}
