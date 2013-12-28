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

#include <cstdlib>
#include "NstCore.hpp"
#include "api/NstApiEmulator.hpp"
#include "api/NstApiMovie.hpp"
#include "api/NstApiRewinder.hpp"
#include "NstRewinder.hpp"

namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif

		Rewinder::Rewinder(Ppu& ppu,const Apu& apu,bool useSound,Api::Movie m,Api::Emulator& e,ExecuteFrameCallback c)
		: movie(m), emulator(e), ExecuteFrame(c), sound(apu,useSound), video(ppu), rewinding(false) 
		{
			Reset();
		}

		Rewinder::~Rewinder()
		{
			movie.Eject( NULL );
		}

		Rewinder::ReverseVideo::ReverseVideo(Ppu& p)
		: ppu(p) 
		{
			Reset();
		}

		void Rewinder::ReverseVideo::Reset()
		{
			pingpong = 1U-0U;
			frame = 0;
		}

		Rewinder::ReverseSound::ReverseSound(const Apu& a,bool e)
		: 
		enabled (e),
		good    (false),
		stereo  (false),
		bits    (0),
		rate    (0),
		index   (0),
		buffer  (NULL), 
		size    (0), 
		input   (NULL),
		apu     (a)
		{}

		Rewinder::ReverseSound::~ReverseSound()
		{
			std::free( buffer );
		}

		void Rewinder::ReverseSound::Reset()
		{
			good = true;
			index = 0;

			if (buffer)
				Clear();
		}

		void Rewinder::ReverseSound::Enable(bool state)
		{
			enabled = state;

			if (!state)
			{
				std::free( buffer );
				buffer = NULL;
			}
		}

		void Rewinder::ReverseSound::Clear() const
		{
			std::memset( buffer, bits == 16 ? 0x00 : 0x80, size );
		}

		void Rewinder::Reset(const bool success)
		{
			NST_VERIFY( success );

			if (rewinding)
			{
				rewinding = false;
				Api::Rewinder::stateCallback( Api::Rewinder::STOPPED );
			}

			movie.Eject( NULL );

			good = success;
			uturn = 0;
			frame = LAST_FRAME;
			keyFrame = LAST_KEYFRAME;

			stream.str( std::string() );

			for (uint i=0; i < KEYFRAMES; ++i)
				keyFrames[i] = ~u32(0);
		}

		Result Rewinder::Start()
		{
			if (!good)
			{
				return RESULT_ERR_UNSUPPORTED;
			}
			else if (!rewinding)
			{
				if (keyFrames[keyFrame] != ~u32(0))
				{
					if (uturn == 0)
						uturn = FRAMES - frame;
					else
						uturn = 0;

					return RESULT_OK;
				}
				else
				{
					return RESULT_ERR_NOT_READY;
				}
			}
			else if (uturn)
			{
				uturn = 0;
				return RESULT_OK;
			}
			else
			{
				return RESULT_NOP;
			}
		}

		Result Rewinder::Stop()
		{
			if (rewinding)
			{
				if (uturn == 0)
					uturn = FRAMES - frame;
				else
					uturn = 0;

				return RESULT_OK;
			}
			else if (uturn)
			{
				uturn = 0;
				return RESULT_OK;
			}
			else
			{
				return RESULT_NOP;
			}
		}

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif

		class Rewinder::ReverseVideo::Mutex
		{
			friend class ReverseVideo;

			Ppu& ppu;
			u16* const screen;

		public:

			Mutex(const ReverseVideo& r)
			: ppu(r.ppu), screen(r.ppu.GetScreen()) {}

			~Mutex()
			{
				ppu.SetScreen( screen );
			}
		};

		void Rewinder::ReverseVideo::Store()
		{
			NST_ASSERT( frame < FRAMES && (pingpong == 1U-0U || pingpong == 0U-1U) );

			ppu.SetScreen( buffer[frame] );
			frame += pingpong;

			if (frame == FRAMES)
			{
				frame = LAST_FRAME;
				pingpong = 0U-1U;
			}
			else if (frame == 0U-1U)
			{
				frame = 0;
				pingpong = 1U-0U;
			}
		}

		inline void Rewinder::ReverseVideo::Flush(const Mutex& mutex)
		{
			std::memcpy( mutex.screen, buffer[frame], sizeof(u16) * Ppu::WIDTH * Ppu::HEIGHT );
		}

		class Rewinder::ReverseSound::Mutex
		{
			friend class ReverseSound;

			Api::Sound::Output::LockCallback funcLock;
			Api::Sound::UserData userLock;
			Api::Sound::Output::UnlockCallback funcUnlock;
			Api::Sound::UserData userUnlock;

		public:

			Mutex()
			{
				Api::Sound::Output::lockCallback.Get( funcLock, userLock );
				Api::Sound::Output::unlockCallback.Get( funcUnlock, userUnlock );
				Api::Sound::Output::lockCallback.Set( NULL, NULL );
				Api::Sound::Output::unlockCallback.Set( NULL, NULL );
			}

			~Mutex()
			{
				Api::Sound::Output::lockCallback.Set( funcLock, userLock );
				Api::Sound::Output::unlockCallback.Set( funcUnlock, userUnlock );
			}
		};

		Sound::Output* Rewinder::ReverseSound::Store()
		{
			NST_COMPILE_ASSERT( FRAMES % 2 == 0 );

			if (!enabled || !good)
			{
				return NULL;
			}
			else if (bits != apu.GetSampleBits() || rate != apu.GetSampleRate() || stereo != (uchar) (bool) apu.InStereo() || !buffer)
			{
				bits = apu.GetSampleBits();
				rate = apu.GetSampleRate();
				stereo = (uchar) (bool) apu.InStereo();

				const dword minSize = SampleSize() * rate * 2;

				if (!buffer || size != minSize)
				{
					buffer = static_cast<u8*>(buffer ? std::realloc( buffer, minSize ) : std::malloc( minSize ));

					if (!buffer)
					{
						good = false;
						return NULL;
					}

					size = minSize;
				}

				good = true;
				Clear();
				index = 0;
			}

			switch (index++)
			{
				case 0:
					
					output.samples = buffer;
					output.length = rate / FRAMES; 
					input = buffer + (size / 1); 
					break;

				case FRAMES-1:

					output.samples = static_cast<u8*>(output.samples) + (output.length * SampleSize());
					output.length = (buffer + (size / 2) - static_cast<u8*>(output.samples)) / SampleSize();
					break;

				case FRAMES: 
					
					output.samples = buffer + (size / 2);
					output.length = rate / FRAMES; 
					input = buffer + (size / 2);
					break;

				case FRAMES*2-1: 
					
					index = 0; 
					output.samples = static_cast<u8*>(output.samples) + (output.length * SampleSize());
					output.length = (buffer + (size / 1) - static_cast<u8*>(output.samples)) / SampleSize();
					break;

				default:

					output.samples = static_cast<u8*>(output.samples) + (output.length * SampleSize());
					break;
			}
			
			return &output;
		}

		void Rewinder::ReverseSound::Flush(Output* const target,const Mutex& mutex)
		{
			if (target && (!mutex.funcLock || mutex.funcLock( mutex.userLock, *target )))
			{
				if (bits == 16)
					ReverseCopy<i16>( *target );
				else
					ReverseCopy<u8>( *target );

				if (mutex.funcUnlock)
					mutex.funcUnlock( mutex.userUnlock, *target );
			}
		}

		template<typename T>
		void Rewinder::ReverseSound::ReverseCopy(const Output& target)
		{
			T* dst = static_cast<T*>(target.samples);

			if (enabled && good)
			{
				const T* const dstEnd = dst + (target.length << stereo);

				const T* src = reinterpret_cast<const T*>(input);
				const T* const srcEnd = reinterpret_cast<const T*>(buffer);

				while (dst != dstEnd && src-- != srcEnd)
					*dst++ = *src;

				input = reinterpret_cast<const u8*>(src + 1);
				const T last = src[1];

				while (dst != dstEnd)
					*dst++ = last;
			}
			else
			{
				std::memset( dst, uint(sizeof(T) == sizeof(u8)) << 7, (target.length << stereo) * sizeof(T) );
			}
		}

		Result Rewinder::Execute
		(
			Video::Output* const videoOut,
			Sound::Output* const soundOut,
			Input::Controllers* const inputOut
		)
		{
			if (good)
			{
				try
				{
					if (uturn && !--uturn)
					{
						if (rewinding)
							BeginForward();
						else
							BeginBackward();
					}

					if (rewinding)
						return ExecuteBackward( videoOut, soundOut, inputOut );
					else
						return ExecuteForward( videoOut, soundOut, inputOut );
				}
				catch (...)
				{
					Reset( false );
				}
			}

			return (emulator.*ExecuteFrame)( videoOut, soundOut, inputOut );
		}

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif

		void Rewinder::BeginForward()
		{
			NST_ASSERT( rewinding && !uturn && frame <= FRAMES && keyFrame < KEYFRAMES && keyFrames[keyFrame < LAST_KEYFRAME ? keyFrame+1 : 0] != ~u32(0) );

			frame = LAST_FRAME;
			stream.seekp( keyFrames[keyFrame < LAST_KEYFRAME ? keyFrame+1 : 0], std::stringstream::beg );

			rewinding = false;
			Api::Rewinder::stateCallback( Api::Rewinder::STOPPED );
		}

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif

		Result Rewinder::ExecuteForward
		(
			Video::Output* const videoOut,
			Sound::Output* const soundOut,
			Input::Controllers* const inputOut
		)
		{
			NST_ASSERT( !rewinding && frame < FRAMES );

			if (++frame == FRAMES)
				ClockForward();

			if (!movie.IsRecording( NULL ))
			{
				NST_DEBUG_MSG("Rewinder::ExecuteForward() recording failed!");
				keyFrames[keyFrame] = ~u32(0);
			}

			return (emulator.*ExecuteFrame)( videoOut, soundOut, inputOut );
		}

		void Rewinder::ClockForward()
		{
			NST_ASSERT( !rewinding && frame <= FRAMES && keyFrame < KEYFRAMES );

			movie.Stop( NULL );			
			frame = 0;

			if (keyFrame < LAST_KEYFRAME)
			{
				keyFrames[++keyFrame] = stream.tellp();
			}
			else
			{
				stream.seekp( 0, std::stringstream::beg );
				stream.clear();

				keyFrames[0] = keyFrame = 0;
			}

			movie.Record( stream, Api::Movie::CLEAN, Api::Movie::DISABLE_CALLBACK, NULL );
		}

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif

		void Rewinder::BeginBackward()
		{
			NST_ASSERT( !rewinding && !uturn && frame == LAST_FRAME && keyFrame < KEYFRAMES );

			if (keyFrames[keyFrame] != ~u32(0))
			{
				rewinding = true;
				Api::Rewinder::stateCallback( Api::Rewinder::PREPARING );

				movie.Stop( NULL );

				// invalidate all tail key frames that got overwritten by the current one

				for (uint i=keyFrame+2,pos=stream.tellp(); i < KEYFRAMES; ++i)
				{
					if (pos <= keyFrames[i])
						pos = keyFrames[i];
					else
						keyFrames[i] = ~u32(0);
				}

				keyFrames[keyFrame < LAST_KEYFRAME ? keyFrame+1 : 0] = ~u32(0);
				stream.seekg( keyFrames[keyFrame], std::stringstream::beg );

				if (NES_SUCCEEDED(movie.Play( stream, Api::Movie::DISABLE_CALLBACK, false, NULL )))
				{
					video.Reset();
					const ReverseVideo::Mutex videoMutex( video );

					sound.Reset();
					const ReverseSound::Mutex soundMutex;

					for (uint i=0; i < FRAMES; ++i)
					{
						video.Store();

						if (NES_FAILED((emulator.*ExecuteFrame)( NULL, sound.Store(), NULL )))
						{
							Reset( false );
							break;
						}
					}

					Api::Rewinder::stateCallback( Api::Rewinder::REWINDING );
				}
				else
				{
					Reset( true );
				}
			}
		}

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif

		Result Rewinder::ExecuteBackward
		(
	    	Video::Output* const videoOut,
 	     	Sound::Output* soundOut,
	       	Input::Controllers* const inputOut
		)
		{
			NST_ASSERT( rewinding && frame < FRAMES );

			if (++frame != FRAMES || ClockBackward())
			{
				if (movie.IsPlaying( NULL ))
				{
					const ReverseVideo::Mutex videoMutex( video );
					video.Flush( videoMutex );						
					video.Store();

					const ReverseSound::Mutex soundMutex;
					sound.Flush( soundOut, soundMutex );
					soundOut = sound.Store();
					
					return (emulator.*ExecuteFrame)( videoOut, soundOut, inputOut );
				}
				else
				{
					Reset( true );
				}
			}

			return (emulator.*ExecuteFrame)( videoOut, soundOut, inputOut );
		}

		bool Rewinder::ClockBackward()
		{
			NST_ASSERT( rewinding && frame == FRAMES && keyFrame < KEYFRAMES && keyFrames[keyFrame] != ~u32(0) );

			movie.Stop( NULL );

			frame = 0;
			keyFrame = (keyFrame ? keyFrame-1 : LAST_KEYFRAME);

			if (keyFrames[keyFrame] != ~u32(0))
			{
				stream.seekg( keyFrames[keyFrame], std::stringstream::beg );
				movie.Play( stream, Api::Movie::DISABLE_CALLBACK, false, NULL );
				
				return true;
			}
			else
			{
				BeginForward();
				ClockForward();
				
				return false;
			}
		}
	}
}
