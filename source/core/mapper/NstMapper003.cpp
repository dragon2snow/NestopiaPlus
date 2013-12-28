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
#include "NstMapper003.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper3::Mapper3(Context& c)
		:
		Mapper (c,PROM_MAX_32K | (c.attribute == ATR_X79B ? WRAM_8K : WRAM_DEFAULT)),
		sound  (c.attribute == ATR_SAMPLES_AS ? Sound::Player::Create(c.apu,Sound::Loader::AEROBICS_STUDIO,Sound::Loader::AEROBICS_STUDIO_SAMPLES) : NULL)
		{}

		Mapper3::~Mapper3()
		{
			Sound::Player::Destroy( sound );
		}

		void Mapper3::SubReset(bool)
		{
			if (sound)
				Map( 0x6000U, &Mapper3::Poke_6000 );

			Map( 0x8000U, 0xFFFFU, CHR_SWAP_8K );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE_D(Mapper3,6000)
		{
			NST_ASSERT( sound );

			if ((data & 0x40) == 0x00)
				sound->Play( data & 0x0F );
		}
	}
}
