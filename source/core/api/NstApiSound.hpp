////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
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

#ifndef NST_API_SOUND_H
#define NST_API_SOUND_H

#include "NstApi.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#if NST_ICC >= 810
#pragma warning( push )
#pragma warning( disable : 304 444 )
#elif NST_MSVC >= 1200
#pragma warning( push )
#pragma warning( disable : 4512 )
#endif

namespace Nes
{
	namespace Core
	{
		namespace Sound
		{
			class Output
			{
				struct Locker;
				struct Unlocker;

			public:

				enum
				{
					MAX_LENGTH = 0x8000
				};

				void* samples[2];
				uint length[2];

				Output(void* s0=0,uint l0=0,void* s1=0,uint l1=0)
				{
					samples[0] = s0;
					samples[1] = s1;
					length[0] = l0;
					length[1] = l1;
				}

				typedef bool (NST_CALLBACK *LockCallback) (void*,Output&);
				typedef void (NST_CALLBACK *UnlockCallback) (void*,Output&);

				static Locker lockCallback;
				static Unlocker unlockCallback;
			};

			struct Output::Locker : UserCallback<Output::LockCallback>
			{
				bool operator () (Output& output) const
				{
					return (!function || function( userdata, output ));
				}
			};

			struct Output::Unlocker : UserCallback<Output::UnlockCallback>
			{
				void operator () (Output& output) const
				{
					if (function)
						function( userdata, output );
				}
			};
		}
	}

	namespace Api
	{
		class Sound : public Base
		{
		public:

			template<typename T>
			Sound(T& e)
			: Base(e) {}

			enum Channel
			{
				CHANNEL_SQUARE1  = 0x001,
				CHANNEL_SQUARE2  = 0x002,
				CHANNEL_TRIANGLE = 0x004,
				CHANNEL_NOISE    = 0x008,
				CHANNEL_DPCM     = 0x010,
				CHANNEL_FDS      = 0x020,
				CHANNEL_MMC5     = 0x040,
				CHANNEL_VRC6     = 0x080,
				CHANNEL_VRC7     = 0x100,
				CHANNEL_N163     = 0x200,
				CHANNEL_S5B      = 0x400,
				APU_CHANNELS     = CHANNEL_SQUARE1|CHANNEL_SQUARE2|CHANNEL_TRIANGLE|CHANNEL_NOISE|CHANNEL_DPCM,
				EXT_CHANNELS     = CHANNEL_FDS|CHANNEL_MMC5|CHANNEL_VRC6|CHANNEL_VRC7|CHANNEL_N163|CHANNEL_S5B,
				ALL_CHANNELS     = APU_CHANNELS|EXT_CHANNELS
			};

			enum Speaker
			{
				SPEAKER_MONO,
				SPEAKER_STEREO
			};

			enum
			{
				DEFAULT_VOLUME = 85,
				MAX_VOLUME = 100,
				DEFAULT_SPEED = 0,
				MIN_SPEED = 30,
				MAX_SPEED = 240
			};

			Result  SetSampleRate(ulong) throw();
			ulong   GetSampleRate() const throw();
			Result  SetSampleBits(uint) throw();
			uint    GetSampleBits() const throw();
			void    SetSpeaker(Speaker) throw();
			Speaker GetSpeaker() const throw();
			Result  SetVolume(uint,uint) throw();
			uint    GetVolume(uint) const throw();
			Result  SetSpeed(uint) throw();
			uint    GetSpeed() const throw();
			void    SetAutoTranspose(bool) throw();
			bool    IsAutoTransposing() const throw();
			bool    IsAudible() const throw();
			void    EmptyBuffer() throw();

			typedef Core::Sound::Output Output;
		};
	}
}

#if NST_MSVC >= 1200 || NST_ICC >= 810
#pragma warning( pop )
#endif

#endif
