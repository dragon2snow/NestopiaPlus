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
#include "NstBrdMmc2.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			void Mmc2::SubReset(const bool hard)
			{
				if (hard)
				{
					selector[0] = 0;
					selector[1] = 0;

					banks[0][0] = 0;
					banks[0][1] = 0;
					banks[1][0] = 0;
					banks[1][1] = 0;
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
				NST_VERIFY( id == (AsciiId<'M','M','2'>::V) );

				if (id == AsciiId<'M','M','2'>::V)
				{
					while (const dword subId = state.Begin())
					{
						if (subId == AsciiId<'R','E','G'>::V)
						{
							State::Loader::Data<4+1> data( state );

							banks[0][0] = data[0];
							banks[0][1] = data[1];
							banks[1][0] = data[2];
							banks[1][1] = data[3];

							selector[0] = data[4] >> 0 & 0x1;
							selector[1] = data[4] >> 1 & 0x1;
						}

						state.End();
					}
				}
			}

			void Mmc2::BaseSave(State::Saver& state) const
			{
				const byte data[4+1] =
				{
					banks[0][0],
					banks[0][1],
					banks[1][0],
					banks[1][1],
					selector[0] | selector[1] << 1
				};

				state.Begin( AsciiId<'M','M','2'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
			}

			#ifdef NST_MSVC_OPTIMIZE
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

				chr.SwapBank<SIZE_4K,0x0000>( banks[0][selector[0]] );

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

				chr.SwapBank<SIZE_4K,0x1000>( banks[1][selector[1]] );

				return data;
			}

			void Mmc2::UpdateChr() const
			{
				chr.SwapBanks<SIZE_4K,0x0000>( banks[0][selector[0]], banks[1][selector[1]] );
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
