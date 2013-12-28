////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
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
					MOERO_PRO_YAKYUU = 1
				};

				enum
				{
					MOERO_PRO_YAKYUU_SAMPLES = 16
				};

				virtual Result Load(uint,const u8*,dword,dword) = 0;

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
				NO_CHANNELS      = 0x00,
				CHANNEL_SQUARE1  = 0x01,
				CHANNEL_SQUARE2  = 0x02,
				CHANNEL_TRIANGLE = 0x04,
				CHANNEL_NOISE    = 0x08,
				CHANNEL_DPCM     = 0x10,
				CHANNEL_EXTERNAL = 0x20,
				ALL_CHANNELS     = 0x3F
			};
	
			enum Speaker
			{
				SPEAKER_MONO,
				SPEAKER_STEREO
			};

			enum
			{
				DEFAULT_SPEED,
				MIN_SPEED = 30,
				MAX_SPEED = 240
			};
	
			Result  EnableChannels(uint);
			Result  SetSampleRate(ulong);
			Result  SetSampleBits(uint);
			Result  SetSpeed(uint);
			void    SetAutoTranspose(bool);
			void    SetSpeaker(Speaker);
			bool    IsAutoTransposing() const;
			uint    GetEnabledChannels() const;
			ulong   GetSampleRate() const;
			uint    GetSampleBits() const;
			uint    GetSpeed() const;
			uint    GetLatency() const;
			Speaker	GetSpeaker() const;
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
