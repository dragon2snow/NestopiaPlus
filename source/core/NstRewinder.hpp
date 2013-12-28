////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2005 Martin Freij
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

#ifndef NST_REWINDER_H
#define NST_REWINDER_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include <sstream>
#include "api/NstApiSound.hpp"

namespace Nes
{
	namespace Core
	{
		class Rewinder
		{
			typedef Result (Api::Emulator::*ExecuteFrameCallback)
			(
				Video::Output*,
				Sound::Output*,
				Input::Controllers*
			);

		public:

			Rewinder(Ppu&,const Apu&,bool,Api::Movie,Api::Emulator&,ExecuteFrameCallback);
			~Rewinder();

			void   Enable(bool);
			Result Start();
			Result Stop();
			Result Execute(Video::Output*,Sound::Output*,Input::Controllers*);

		private:

			enum
			{			
				FRAMES = 60,
				LAST_FRAME = FRAMES-1,
				KEYFRAMES = 60,
				LAST_KEYFRAME = KEYFRAMES-1
			};

			class ReverseVideo
			{
			public:

				ReverseVideo(Ppu&);

				class Mutex;

				void Reset();
				void Store();
				inline void Flush(const Mutex&);

			private:

				friend class Mutex;

				uint pingpong;
				uint frame;
				Ppu& ppu;
				u16 buffer[FRAMES][Ppu::WIDTH * Ppu::HEIGHT];
			};

			class ReverseSound
			{
			public:

				typedef Api::Sound::Output Output;

				ReverseSound(const Apu&,bool);
				~ReverseSound();

				class Mutex;

				void    Reset();
				void    Enable(bool);
				Output* Store();
				void    Flush(Output*,const Mutex&);

			private:

				template<typename T>
				void ReverseCopy(const Output&);

				void Clear() const;

				bool enabled;
				bool good;
				uchar stereo;
				uchar bits;
				dword rate;
				uint index;
				u8* buffer;
				dword size;
				Output output;
				const u8* input;
				const Apu& apu;

				uint SampleSize() const
				{
					return (bits / 8) << stereo;
				}

			public:

				bool IsRewinding() const
				{
					return enabled && good && buffer;
				}

				dword GetLatency() const
				{
					return (dword) (input - buffer) * SampleSize();
				}
			};

			void   Reset(bool);
			void   BeginForward();
			void   BeginBackward();
			Result ExecuteForward(Video::Output*,Sound::Output*,Input::Controllers*);
			Result ExecuteBackward(Video::Output*,Sound::Output*,Input::Controllers*);
			void   ClockForward();
			bool   ClockBackward();

			ibool good;
			ibool rewinding;
			uint uturn;
			uint frame;
			uint keyFrame;
			Api::Movie movie;
			Api::Emulator& emulator;
			ExecuteFrameCallback const ExecuteFrame;
			std::stringstream stream;
			ReverseSound sound;
			ReverseVideo video;
			u32 keyFrames[KEYFRAMES];

		public:

			void Reset()
			{
				Reset( true );
			}

			bool InBadState() const
			{
				return !good;
			}

			void EnableSound(bool state)
			{
				sound.Enable( state );
			}

			bool IsEnabled() const
			{
				return good;
			}

			bool IsRewinding() const
			{
				return (rewinding && !uturn) || (!rewinding && uturn);
			}
			
			bool IsSoundRewinding() const
			{
				return rewinding && sound.IsRewinding();
			}

			dword GetSoundLatency() const
			{
				return sound.GetLatency();
			}
		};
	}
}

#endif
