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
#include "../NstSoundPlayer.hpp"
#include "NstBrdJaleco.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Sound::Player* Jaleco::DetectSound(dword crc,Cpu& cpu)
			{
				switch (crc)
				{
					case 0xDB53A88DUL:
					case 0x93B9B15CUL:
					case 0xE30B210EUL:
					case 0xE374C3E7UL:

						return Sound::Player::Create
						(
							cpu,
							Sound::Loader::MOERO_PRO_YAKYUU,
							Sound::Loader::MOERO_PRO_YAKYUU_SAMPLES
						);

					case 0x9F50A100UL:

						return Sound::Player::Create
						(
							cpu,
							Sound::Loader::MOERO_PRO_YAKYUU_88,
							Sound::Loader::MOERO_PRO_YAKYUU_88_SAMPLES
						);

					case 0x598A7398UL:

						return Sound::Player::Create
						(
							cpu,
							Sound::Loader::MOERO_PRO_TENNIS,
							Sound::Loader::MOERO_PRO_TENNIS_SAMPLES
						);
				}

				return NULL;
			}

			Jaleco::Jaleco(Context& c,Type t)
			:
			Mapper    (c),
			prgOffset (t == TYPE_1 ? 0x4000U : 0x0000U),
			sound     (DetectSound(c.prgCrc,c.cpu)),
			type      (t)
			{
			}

			Jaleco::~Jaleco()
			{
				delete sound;
			}

			void Jaleco::SubReset(bool)
			{
				if (type == TYPE_2)
				{
					regs[1] = regs[0] = 0;

					Map( 0x6000U, &Jaleco::Poke_6000 );

					if (sound)
						Map( 0x7000U, &Jaleco::Poke_7000 );
				}
				else
				{
					regs[0] = chr.GetBank<SIZE_8K,0x0000U>();
					regs[1] = prg.GetBank<SIZE_16K>( prgOffset );

					Map( 0x8000U, 0xFFFFU, &Jaleco::Poke_8000 );
				}
			}

			void Jaleco::BaseLoad(State::Loader& state,const dword id)
			{
				NST_VERIFY( id == NES_STATE_CHUNK_ID('J','A','L','\0') );

				if (sound)
					sound->Stop();

				if (id == NES_STATE_CHUNK_ID('J','A','L','\0'))
				{
					while (const dword chunk = state.Begin())
					{
						if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
						{
							const uint data = state.Read8();

							if (type == TYPE_2)
							{
								regs[0] = data;
							}
							else
							{
								regs[1] = data >> 4;
								regs[0] = data & 0xF;
							}
						}

						state.End();
					}
				}
			}

			void Jaleco::BaseSave(State::Saver& state) const
			{
				state.Begin('J','A','L','\0').Begin('R','E','G','\0').Write8( regs[0] | (regs[1] << 4) ).End().End();
			}

			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("", on)
			#endif

			NES_POKE(Jaleco,6000)
			{
				ppu.Update();
				prg.SwapBank<SIZE_32K,0x0000U>( data >> 4 & 0x3 );
				chr.SwapBank<SIZE_8K,0x0000U>( (data >> 4 & 0x4) | (data & 0x3) );
			}

			NES_POKE(Jaleco,7000)
			{
				NST_ASSERT( sound );

				uint tmp = regs[0];
				regs[0] = data;

				if ((tmp & 0x10) < (data & 0x10))
					sound->Play( data & 0xF );
			}

			NES_POKE(Jaleco,8000)
			{
				if ((data & 0x10) == 0x00 && sound)
					sound->Play( address & 0x1F );

				if ((data & 0xC0) == 0x00)
				{
					ppu.Update();
					prg.SwapBank<SIZE_16K>( prgOffset, regs[1] );
					chr.SwapBank<SIZE_8K,0x0000U>( regs[0] );
				}
				else
				{
					if (data & 0x40)
						regs[0] = data & 0x0F;

					if (data & 0x80)
						regs[1] = data & 0x0F;
				}
			}
		}
	}
}
