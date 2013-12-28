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

#include <new>
#include <string>
#include "NstMachine.hpp"
#include "NstTrackerRewinder.hpp"
#include "api/NstApiRewinder.hpp"

#ifndef NST_NO_ZLIB
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define ZLIB_WINAPI
#endif
#include "../zlib/zlib.h"
#endif

namespace Nes
{
	namespace Core
	{
		class Tracker::Rewinder::ReverseVideo::Mutex
		{
			friend class ReverseVideo;

			Ppu& ppu;
			Video::Screen::Pixel* const pixels;

		public:

			Mutex(const ReverseVideo& r)
			: ppu(r.ppu), pixels(r.ppu.GetOutputPixels()) {}

			~Mutex()
			{
				ppu.SetOutputPixels( pixels );
			}
		};

		class Tracker::Rewinder::ReverseSound::Mutex
		{
			friend class ReverseSound;

			Output::LockCallback funcLock;
			void* userLock;
			Output::UnlockCallback funcUnlock;
			void* userUnlock;

		public:

			Mutex()
			{
				Output::lockCallback.Get( funcLock, userLock );
				Output::unlockCallback.Get( funcUnlock, userUnlock );
				Output::lockCallback.Set( NULL, NULL );
				Output::unlockCallback.Set( NULL, NULL );
			}

			~Mutex()
			{
				Output::lockCallback.Set( funcLock, userLock );
				Output::unlockCallback.Set( funcUnlock, userUnlock );
			}
		};

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Tracker::Rewinder::ReverseVideo::ReverseVideo(Ppu& p)
		:
		ppu      (p),
		pingpong (1),
		frame    (0),
		buffer   (NULL)
		{}

		Tracker::Rewinder::ReverseSound::ReverseSound(const Apu& a,bool e)
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

		Tracker::Rewinder::Rewinder(Machine& e,EmuExecute x,EmuLoadState l,EmuSaveState s,Cpu& c,Ppu& p,bool b)
		:
		rewinding    (false),
		sound        (c.GetApu(),b),
		video        (p),
		emulator     (e),
		emuExecute   (x),
		emuLoadState (l),
		emuSaveState (s),
		cpu          (c),
		ppu          (p)
		{
			Reset( false );
		}

		Tracker::Rewinder::ReverseVideo::~ReverseVideo()
		{
			End();
		}

		Tracker::Rewinder::ReverseSound::~ReverseSound()
		{
			End();
		}

		Tracker::Rewinder::~Rewinder()
		{
			LinkPorts( false );
		}

		void Tracker::Rewinder::Key::Input::Reset()
		{
			pos = BAD_POS;
			buffer.Destroy();
		}

		void Tracker::Rewinder::Key::Reset()
		{
			stream.str( std::string() );
			input.Reset();
		}

		void Tracker::Rewinder::Reset(bool on)
		{
			video.End();
			sound.End();

			if (rewinding)
			{
				rewinding = false;
				Api::Rewinder::stateCallback( Api::Rewinder::STOPPED );
			}

			uturn = false;
			frame = LAST_FRAME;
			key = keys + LAST_KEY;

			for (uint i=0; i < NUM_FRAMES; ++i)
				keys[i].Reset();

			LinkPorts( on );
		}

		ibool Tracker::Rewinder::ReverseVideo::Begin()
		{
			pingpong = 1;
			frame = 0;

			if (buffer == NULL)
				buffer = new (std::nothrow) Video::Screen::Pixels [NUM_FRAMES];

			return buffer != NULL;
		}

		void Tracker::Rewinder::ReverseVideo::End()
		{
			delete [] buffer;
			buffer = NULL;
		}

		ibool Tracker::Rewinder::ReverseSound::Begin()
		{
			good = true;
			index = 0;
			return true;
		}

		void Tracker::Rewinder::ReverseSound::End()
		{
			std::free( buffer );
			buffer = NULL;
		}

		void Tracker::Rewinder::ReverseSound::Enable(bool state)
		{
			enabled = state;

			if (!state)
				End();
		}

		ibool Tracker::Rewinder::ReverseSound::Update()
		{
			bits = apu.GetSampleBits();
			rate = apu.GetSampleRate();
			stereo = apu.InStereo();

			const dword minSize = SampleSize() * rate * 2;
			NST_ASSERT( minSize );

			if (!buffer || size != minSize)
			{
				if (u8* const next = static_cast<u8*>(std::realloc( buffer, minSize )))
				{
					buffer = next;
					size = minSize;
				}
				else
				{
					End();

					good = false;
					return false;
				}
			}

			Clear();

			good = true;
			index = 0;

			return true;
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		inline void Tracker::Rewinder::Key::Input::Invalidate()
		{
			pos = BAD_POS;
		}

		inline void Tracker::Rewinder::Key::Input::BeginForward()
		{
			const ulong hint = pos;
			pos = 0;
			buffer.Clear();

			if (hint != BAD_POS)
				buffer.Reserve( hint );
		}

		bool Tracker::Rewinder::Key::Input::EndForward()
		{
			if (pos == 0)
			{
				pos = buffer.Size();

			#ifndef NST_NO_ZLIB
				if (pos >= MIN_COMPRESSION_SIZE)
				{
					{
						ulong size = pos - 1;
						Buffer tmp( size );

						if (compress( tmp.Begin(), &size, buffer.Begin(), buffer.Size() ) == Z_OK)
						{
							NST_ASSERT( size < pos );
							tmp.SetTo( size );
							Buffer::Swap( tmp, buffer );
						}
						else
						{
							NST_DEBUG_MSG("compress() in Tracker::Rewinder::Key::Input failed!");
						}
					}

					buffer.Defrag();
				}
			#endif

				return true;
			}

			return false;
		}

		bool Tracker::Rewinder::Key::Input::BeginBackward()
		{
			ulong size = pos;
			pos = 0;

		#ifndef NST_NO_ZLIB
			if (size > buffer.Size())
			{
				Buffer tmp( size );

				if (uncompress( tmp.Begin(), &size, buffer.Begin(), buffer.Size() ) == Z_OK)
				{
					NST_VERIFY( size == tmp.Size() );
					Buffer::Swap( tmp, buffer );
				}
				else
				{
					NST_DEBUG_MSG("uncompress() in Tracker::Rewinder::Key::Input failed!");
					return false;
				}
			}
		#endif

			return true;
		}

		inline void Tracker::Rewinder::Key::Input::EndBackward()
		{
			pos = 0;
		}

		inline bool Tracker::Rewinder::Key::Input::ResumeForward()
		{
			if (pos != BAD_POS)
			{
				ulong size = pos;
				pos = 0;
				buffer.Resize( size );
				return true;
			}

			return false;
		}

		inline bool Tracker::Rewinder::Key::Input::CanRewind() const
		{
			return pos != BAD_POS;
		}

		inline uint Tracker::Rewinder::Key::Input::Put(const uint data)
		{
			if (pos != BAD_POS)
			{
				try
				{
					buffer << data;
				}
				catch (...)
				{
					NST_DEBUG_MSG("buffer << data failed!");
					pos = BAD_POS;
				}
			}

			return data;
		}

		inline uint Tracker::Rewinder::Key::Input::Get()
		{
			if (pos < buffer.Size())
			{
				return buffer[pos++];
			}
			else
			{
				NST_DEBUG_MSG("buffer >> data failed!");
				pos = BAD_POS;
				return OPEN_BUS;
			}
		}

		inline void Tracker::Rewinder::Key::Invalidate()
		{
			input.Invalidate();
		}

		inline bool Tracker::Rewinder::Key::CanRewind() const
		{
			return input.CanRewind();
		}

		inline bool Tracker::Rewinder::Key::ResumeForward()
		{
			return input.ResumeForward();
		}

		bool Tracker::Rewinder::Key::BeginForward(Machine& emulator,EmuSaveState saveState,EmuLoadState loadState)
		{
			NST_ASSERT( !(bool(saveState) && bool(loadState)) );

			input.BeginForward();

			if (saveState)
			{
				stream.seekp( 0, std::stringstream::beg );
				stream.clear();

				return NES_SUCCEEDED((emulator.*saveState)( &static_cast<std::ostream&>(stream), false, true ));
			}
			else if (loadState)
			{
				stream.seekg( 0, std::stringstream::beg );
				stream.clear();

				return NES_SUCCEEDED((emulator.*loadState)( &static_cast<std::istream&>(stream), false ));
			}

			return true;
		}

		void Tracker::Rewinder::Key::EndForward()
		{
			if (!input.EndForward())
				Reset();
		}

		bool Tracker::Rewinder::Key::TurnForward(Machine& emulator,EmuLoadState loadState)
		{
			stream.seekg( 0, std::stringstream::beg );
			stream.clear();

			return NES_SUCCEEDED((emulator.*loadState)( &static_cast<std::istream&>(stream), false ));
		}

		bool Tracker::Rewinder::Key::BeginBackward(Machine& emulator,EmuLoadState loadState)
		{
			NST_VERIFY( CanRewind() );
			return TurnForward( emulator, loadState ) && input.BeginBackward();
		}

		inline void Tracker::Rewinder::Key::EndBackward()
		{
			input.EndBackward();
		}

		inline uint Tracker::Rewinder::Key::Put(uint data)
		{
			return input.Put( data );
		}

		inline uint Tracker::Rewinder::Key::Get()
		{
			return input.Get();
		}

		inline Tracker::Rewinder::Key* Tracker::Rewinder::PrevKey(Key* key)
		{
			return (key != keys ? key-1 : keys+LAST_KEY);
		}

		inline Tracker::Rewinder::Key* Tracker::Rewinder::PrevKey()
		{
			return PrevKey( key );
		}

		inline Tracker::Rewinder::Key* Tracker::Rewinder::NextKey(Key* key)
		{
			return (key != keys+LAST_KEY ? key+1 : keys);
		}

		inline Tracker::Rewinder::Key* Tracker::Rewinder::NextKey()
		{
			return NextKey( key );
		}

		void Tracker::Rewinder::LinkPorts(bool on)
		{
			for (uint i=0; i < 2; ++i)
			{
				cpu.Unlink( 0x4016+i, this, &Rewinder::Peek_Port_Get, &Rewinder::Poke_Port );
				cpu.Unlink( 0x4016+i, this, &Rewinder::Peek_Port_Put, &Rewinder::Poke_Port );
			}

			if (on)
			{
				for (uint i=0; i < 2; ++i)
					ports[i] = cpu.Link( 0x4016+i, Cpu::LEVEL_HIGHEST, this, rewinding ? &Rewinder::Peek_Port_Get : &Rewinder::Peek_Port_Put, &Rewinder::Poke_Port );
			}
		}

		void Tracker::Rewinder::ReverseSound::Clear() const
		{
			std::memset( buffer, bits == 16 ? 0x00 : 0x80, size );
		}

		inline void Tracker::Rewinder::ReverseVideo::Flush(const Mutex& mutex)
		{
			std::memcpy( mutex.pixels, buffer[frame], sizeof(buffer[frame]) );
		}

		void Tracker::Rewinder::ReverseVideo::Store()
		{
			NST_ASSERT( frame < NUM_FRAMES && (pingpong == 1U-0U || pingpong == 0U-1U) );

			ppu.SetOutputPixels( buffer[frame] );
			frame += pingpong;

			if (frame == NUM_FRAMES)
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

		Sound::Output* Tracker::Rewinder::ReverseSound::Store()
		{
			NST_COMPILE_ASSERT( NUM_FRAMES % 2 == 0 );

			if (!enabled || !good)
				return NULL;

			if (!buffer || bits != apu.GetSampleBits() || rate != apu.GetSampleRate() || bool(stereo) != bool(apu.InStereo()))
			{
				if (!Update())
					return NULL;
			}

			switch (index++)
			{
				case 0:

					output.samples[0] = buffer;
					output.length[0] = rate / NUM_FRAMES;
					input = buffer + (size / 1);
					break;

				case LAST_FRAME:

					output.samples[0] = static_cast<u8*>(output.samples[0]) + (output.length[0] * SampleSize());
					output.length[0] = (buffer + (size / 2) - static_cast<u8*>(output.samples[0])) / SampleSize();
					break;

				case NUM_FRAMES:

					output.samples[0] = buffer + (size / 2);
					output.length[0] = rate / NUM_FRAMES;
					input = buffer + (size / 2);
					break;

				case NUM_FRAMES+LAST_FRAME:

					index = 0;
					output.samples[0] = static_cast<u8*>(output.samples[0]) + (output.length[0] * SampleSize());
					output.length[0] = (buffer + (size / 1) - static_cast<u8*>(output.samples[0])) / SampleSize();
					break;

				default:

					output.samples[0] = static_cast<u8*>(output.samples[0]) + (output.length[0] * SampleSize());
					break;
			}

			return &output;
		}

		void Tracker::Rewinder::ReverseSound::Flush(Output* const target,const Mutex& mutex)
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
		void Tracker::Rewinder::ReverseSound::ReverseCopy(const Output& target)
		{
			for (uint i=0; i < 2; ++i)
			{
				if (const uint length = target.length[i])
				{
					T* dst = static_cast<T*>(target.samples[i]);

					if (enabled && good)
					{
						const T* const dstEnd = dst + (length << stereo);

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
						std::memset( dst, sizeof(T) == sizeof(u8) ? 0x80 : 0x00, (length << stereo) * sizeof(T) );
					}
				}
			}
		}

		Result Tracker::Rewinder::Execute(Video::Output* videoOut,Sound::Output* soundOut,Input::Controllers* inputOut)
		{
			if (uturn)
				ChangeDirection();

			NST_ASSERT( frame < NUM_FRAMES );

			if (!rewinding)
			{
				if (++frame == NUM_FRAMES)
				{
					frame = 0;
					key->EndForward();
					key = NextKey();
					key->BeginForward( emulator, emuSaveState, NULL );
				}
			}
			else
			{
				if (++frame == NUM_FRAMES)
				{
					frame = 0;
					key->EndBackward();

					Key* const prev = PrevKey();

					if (!prev->CanRewind())
					{
						rewinding = false;

						key->Invalidate();
						key = NextKey();
						key->BeginForward( emulator, NULL, emuLoadState );

						LinkPorts();

						return (emulator.*emuExecute)( videoOut, soundOut, inputOut );
					}

					if (!prev->BeginBackward( emulator, emuLoadState ))
					{
						Reset();
						return (emulator.*emuExecute)( videoOut, soundOut, inputOut );
					}

					key = prev;
				}

				const ReverseVideo::Mutex videoMutex( video );
				video.Flush( videoMutex );
				video.Store();

				const ReverseSound::Mutex soundMutex;
				sound.Flush( soundOut, soundMutex );
				soundOut = sound.Store();

				return (emulator.*emuExecute)( videoOut, soundOut, inputOut );
			}

			return (emulator.*emuExecute)( videoOut, soundOut, inputOut );
		}

		void Tracker::Rewinder::ChangeDirection()
		{
			Api::Rewinder::stateCallback( Api::Rewinder::PREPARING );

			uturn = false;

			if (rewinding)
			{
				for (uint i=frame; i < LAST_FRAME; ++i)
				{
					if (NES_FAILED((emulator.*emuExecute)( NULL, NULL, NULL )))
					{
						Reset();
						return;
					}
				}

				NextKey()->Invalidate();

				if (!video.Begin() || !sound.Begin() || !key->BeginBackward( emulator, emuLoadState ))
				{
					Reset();
					return;
				}

				LinkPorts();

				{
					const ReverseVideo::Mutex videoMutex( video );
					const ReverseSound::Mutex soundMutex;

					for (uint i=0; i < NUM_FRAMES; ++i)
					{
						video.Store();

						if (NES_FAILED((emulator.*emuExecute)( NULL, sound.Store(), NULL )))
						{
							Reset();
							return;
						}
					}
				}

				uint align = LAST_FRAME - frame;
				frame = LAST_FRAME;

				while (align--)
				{
					if (NES_FAILED(Execute( NULL, NULL, NULL )) || !rewinding)
					{
						Reset();
						return;
					}
				}

				Api::Rewinder::stateCallback( Api::Rewinder::REWINDING );
			}
			else
			{
				for (uint i=NUM_FRAMES+LAST_FRAME-frame*2; i; --i)
				{
					if (++frame == NUM_FRAMES)
					{
						frame = 0;
						key = NextKey();

						if (!key->TurnForward( emulator, emuLoadState ))
						{
							Reset();
							return;
						}
					}

					if (NES_FAILED((emulator.*emuExecute)( NULL, NULL, NULL )))
					{
						Reset();
						return;
					}
				}

				if (!key->ResumeForward())
				{
					Reset();
					return;
				}

				LinkPorts();

				video.End();
				sound.End();

				Api::Rewinder::stateCallback( Api::Rewinder::STOPPED );
			}
		}

		Result Tracker::Rewinder::Start()
		{
			if (rewinding)
				return RESULT_NOP;

			if (uturn || !PrevKey()->CanRewind())
				return RESULT_ERR_NOT_READY;

			uturn = true;
			rewinding = true;

			return RESULT_OK;
		}

		Result Tracker::Rewinder::Stop()
		{
			if (!rewinding)
				return RESULT_NOP;

			if (uturn)
				return RESULT_ERR_NOT_READY;

			uturn = true;
			rewinding = false;

			return RESULT_OK;
		}

		NES_PEEK(Tracker::Rewinder,Port_Put)
		{
			return key->Put( ports[address-0x4016]->Peek( address ) );
		}

		NES_PEEK(Tracker::Rewinder,Port_Get)
		{
			return key->Get();
		}

		NES_POKE(Tracker::Rewinder,Port)
		{
			ports[address-0x4016]->Poke( address, data );
		}
	}
}
