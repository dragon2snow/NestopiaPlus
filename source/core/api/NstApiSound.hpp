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

#ifndef NST_API_SOUND_H
#define NST_API_SOUND_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "NstApi.hpp"

#ifdef _MSC_VER
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
					MAX_LENGTH = 0x8000U
				};

				void* samples;
				uint length;

				Output(void* s=NULL,uint l=0)
				: samples(s), length(l) {}

				typedef bool (NST_CALLBACK *LockCallback) (void*,Output&);
				typedef void (NST_CALLBACK *UnlockCallback) (void*,Output&);

				static Locker lockCallback;
				static Unlocker unlockCallback;
			};

			struct Output::Locker : UserCallback<Output::LockCallback>
			{
				bool operator () (Output& output) const
				{
					return (!function || function( userdata, output )) && output.samples && output.length;
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

			class Loader
			{
				struct Callbacker;

			public:

				enum Type
				{
					MOERO_PRO_YAKYUU = 1,
					MOERO_PRO_YAKYUU_88,
					MOERO_PRO_TENNIS,
					TERAO_NO_DOSUKOI_OOZUMOU,
					MOE_PRO_90_KANDOU_HEN,
					MOE_PRO_SAIKYOU_HEN,
					SHIN_MOERO_PRO_YAKYUU,
					AEROBICS_STUDIO
				};

				enum
				{
					MOERO_PRO_YAKYUU_SAMPLES = 16,
					MOERO_PRO_YAKYUU_88_SAMPLES = 20,
					MOERO_PRO_TENNIS_SAMPLES = 19,
					TERAO_NO_DOSUKOI_OOZUMOU_SAMPLES = 6,
					MOE_PRO_90_KANDOU_HEN_SAMPLES = 20,
					MOE_PRO_SAIKYOU_HEN_SAMPLES = 20,
					SHIN_MOERO_PRO_YAKYUU_SAMPLES = 20,
					AEROBICS_STUDIO_SAMPLES = 8
				};

				virtual Result Load(uint,const void*,dword,bool,uint,dword) = 0;

				typedef void (NST_CALLBACK *LoadCallback) (void*,Type,Loader&);

				static Callbacker loadCallback;
			};

			struct Loader::Callbacker : UserCallback<Loader::LoadCallback>
			{
				void operator () (Type type,Loader& loader) const
				{
					if (function)
						function( userdata, type, loader );
				}
			};
		}
	}

	namespace Api
	{
		class Sound : public Base
		{
		public:

			Sound(Emulator& e)
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
				CHANNEL_N106     = 0x200,
				CHANNEL_S5B      = 0x400,
				APU_CHANNELS     = CHANNEL_SQUARE1|CHANNEL_SQUARE2|CHANNEL_TRIANGLE|CHANNEL_NOISE|CHANNEL_DPCM,
				EXT_CHANNELS     = CHANNEL_FDS|CHANNEL_MMC5|CHANNEL_VRC6|CHANNEL_VRC7|CHANNEL_N106|CHANNEL_S5B,
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

			Result  SetSampleRate(ulong);
			Result  SetSampleBits(uint);
			Result  SetVolume(uint,uint);
			Result  SetSpeed(uint);
			void    SetAutoTranspose(bool);
			void    SetSpeaker(Speaker);
			bool    IsAutoTransposing() const;
			bool    IsAudible() const;
			ulong   GetSampleRate() const;
			uint    GetSampleBits() const;
			uint    GetVolume(uint) const;
			uint    GetSpeed() const;
			uint    GetLatency() const;
			Speaker GetSpeaker() const;
			void    EmptyBuffer();

			typedef Core::Sound::Output Output;
			typedef Core::Sound::Loader Loader;
		};
	}
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
