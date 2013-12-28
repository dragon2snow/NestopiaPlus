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

#ifndef NST_SOUND_PCM_H
#define NST_SOUND_PCM_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Sound
		{
			class Pcm : Apu::Channel
			{
			protected:

				Pcm(Cpu&);
				~Pcm();

				void Reset();
				void Play(const i16*,dword,dword);

				static Result CanDo(const void*,dword,uint,dword);

				Cpu& cpu;

			private:

				void UpdateContext(uint,const u8 (&)[MAX_CHANNELS]);
				Sample GetSample();

				struct Wave
				{
					const i16* data;
					dword length;
					dword rate;
				};

				Wave wave;
				qword pos;
				dword rate;

			public:

				void Stop()
				{
					wave.data = NULL;
				}
			};
		}
	}
}

#endif
