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
#include "../NstSoundPlayer.hpp"
#include "NstBrdJaleco.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Sound::Player* Jaleco::DetectSound(Type type,uint attribute,Apu& apu)
			{
				switch (type)
				{
					case TYPE_0:

						if (attribute == ATR_TYPE_0_SAMPLES_MPT)
						{
							return Sound::Player::Create
							(
								apu,
								Sound::Loader::MOERO_PRO_TENNIS,
								Sound::Loader::MOERO_PRO_TENNIS_SAMPLES
							);
						}
						break;

					case TYPE_1:

						if (attribute == ATR_TYPE_1_SAMPLES_MPY88K)
						{
							return Sound::Player::Create
							(
								apu,
								Sound::Loader::MOERO_PRO_YAKYUU_88,
								Sound::Loader::MOERO_PRO_YAKYUU_88_SAMPLES
							);
						}
						break;

					case TYPE_2:

						if (attribute == ATR_TYPE_2_SAMPLES_MPY)
						{
							return Sound::Player::Create
							(
								apu,
								Sound::Loader::MOERO_PRO_YAKYUU,
								Sound::Loader::MOERO_PRO_YAKYUU_SAMPLES
							);
						}
						break;
				}

				return NULL;
			}

			Jaleco::Jaleco(Context& c,Type t)
			:
			Mapper    (c,WRAM_DEFAULT),
			sound     (DetectSound(t,c.attribute,c.apu)),
			prgOffset (t == TYPE_1 ? 0x4000 : 0x0000),
			type      (t)
			{
			}

			Jaleco::~Jaleco()
			{
				Sound::Player::Destroy( sound );
			}

			void Jaleco::SubReset(const bool hard)
			{
				if (type == TYPE_2)
				{
					Map( 0x6000U, &Jaleco::Poke_6000 );

					if (sound)
						Map( 0x7000U, &Jaleco::Poke_7000 );

					if (hard)
						NES_DO_POKE(6000,0x6000,0x00);
				}
				else
				{
					Map( 0x8000U, 0xFFFFU, &Jaleco::Poke_8000 );
				}
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			NES_POKE_D(Jaleco,6000)
			{
				ppu.Update();
				prg.SwapBank<SIZE_32K,0x0000>( data >> 4 & 0x3 );
				chr.SwapBank<SIZE_8K,0x0000>( (data >> 4 & 0x4) | (data & 0x3) );
			}

			NES_POKE_D(Jaleco,7000)
			{
				NST_ASSERT( sound );

				if ((data & 0x30) == 0x20)
					sound->Play( data & 0xF );
			}

			NES_POKE_AD(Jaleco,8000)
			{
				if (data & 0x40)
				{
					ppu.Update();
					chr.SwapBank<SIZE_8K,0x0000>( data & 0xF );
				}

				if (data & 0x80)
					prg.SwapBank<SIZE_16K>( prgOffset, data & 0xF );

				if (sound && (data & 0x30) == 0x20)
					sound->Play( address & 0x1F );
			}
		}
	}
}
