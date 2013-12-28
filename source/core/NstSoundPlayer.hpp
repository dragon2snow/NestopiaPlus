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

#ifndef NST_SOUND_PLAYER_H
#define NST_SOUND_PLAYER_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include <vector>
#include "NstSoundPcm.hpp"
#include "api/NstApiSound.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Sound
		{
			class Player : public Pcm
			{
			public:

				static Player* Create(Cpu&,Loader::Type,uint);

				void LoadState(State::Loader&);
				void SaveState(State::Saver&) const;

			private:

				class SampleLoader;

				Player(Cpu&,uint);

				struct Slot
				{
					const i16* data;
					dword length;
					dword rate;

					Slot()
					: data(NULL) {}

					~Slot()
					{
						delete [] data;
					}
				};

				typedef std::vector<Slot> Slots;

				Slots slots;

			public:

				void Play(uint i)
				{
					if (i < slots.size() && slots[i].data)
						Pcm::Play( slots[i].data, slots[i].length, slots[i].rate );
				}
			};
		}
	}
}

#endif

