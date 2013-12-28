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

#ifndef NST_TRACKER_REWINDER_H
#define NST_TRACKER_REWINDER_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include <sstream>
#include "api/NstApiSound.hpp"

namespace Nes
{
	namespace Core
	{
		class Tracker::Rewinder
		{
			typedef Result (Machine::*EmuExecute)(Video::Output*,Sound::Output*,Input::Controllers*);
			typedef Result (Machine::*EmuSaveState)(StdStream,bool,bool);
			typedef Result (Machine::*EmuLoadState)(StdStream,bool);

		public:

			Rewinder(Machine&,EmuExecute,EmuLoadState,EmuSaveState,Cpu&,Ppu&,bool);
			~Rewinder();

			Result Start();
			Result Stop();
			Result Execute(Video::Output*,Sound::Output*,Input::Controllers*);

		private:

			void Reset(bool);
			void LinkPorts(bool=true);
			void ChangeDirection();

			enum
			{
				NUM_KEYS = 60,
				LAST_KEY = NUM_KEYS-1,
				NUM_FRAMES = 60,
				LAST_FRAME = NUM_FRAMES-1
			};

			class Key
			{
				class Input
				{
					typedef Vector<u8> Buffer;

					enum
					{
						BAD_POS = INT_MAX,
						MIN_COMPRESSION_SIZE = 1024,
						OPEN_BUS = 0x40
					};

					ulong pos;
					Buffer buffer;

				public:

					void Reset();
					inline void BeginForward();
					bool EndForward();
					bool BeginBackward();
					inline void EndBackward();

					inline uint Put(uint);
					inline uint Get();

					inline bool ResumeForward();
					inline bool CanRewind() const;
					inline void Invalidate();
				};

				Input input;
				std::stringstream stream;

			public:

				void Reset();
				bool BeginForward(Machine&,EmuSaveState,EmuLoadState);
				void EndForward();
				bool BeginBackward(Machine&,EmuLoadState);
				inline void EndBackward();

				inline uint Put(uint);
				inline uint Get();

				inline bool CanRewind() const;
				inline bool ResumeForward();
				inline bool TurnForward(Machine&,EmuLoadState);
				inline void Invalidate();
			};

			class ReverseVideo
			{
			public:

				ReverseVideo(Ppu&);
				~ReverseVideo();

				class Mutex;

				ibool Begin();
				void End();
				void Store();
				inline void Flush(const Mutex&);

			private:

				friend class Mutex;

				uint pingpong;
				uint frame;
				Ppu& ppu;
				Video::Screen::Pixels* buffer;
			};

			class ReverseSound
			{
			public:

				typedef Sound::Output Output;

				ReverseSound(const Apu&,bool);
				~ReverseSound();

				class Mutex;

				ibool   Begin();
				void    End();
				void    Enable(bool);
				Output* Store();
				void    Flush(Output*,const Mutex&);

			private:

				template<typename T>
				void ReverseCopy(const Output&);

				void Clear() const;
				ibool Update();

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
					return (input - buffer) * SampleSize();
				}
			};

			inline Key* PrevKey(Key*);
			inline Key* PrevKey();
			inline Key* NextKey(Key*);
			inline Key* NextKey();

			NES_DECL_PEEK( Port_Get )
			NES_DECL_PEEK( Port_Put )
			NES_DECL_POKE( Port     )

			ibool rewinding;
			ibool uturn;
			uint frame;

			const Io::Port* ports[2];

			Key* key;
			Key keys[NUM_KEYS];

			ReverseSound sound;
			ReverseVideo video;

			Machine& emulator;
			const EmuExecute emuExecute;
			const EmuLoadState emuLoadState;
			const EmuSaveState emuSaveState;

			Cpu& cpu;
			Ppu& ppu;

		public:

			void Reset()
			{
				Reset( true );
			}

			void Unload()
			{
				Reset( false );
			}

			void EnableSound(bool enable)
			{
				sound.Enable( enable );
			}

			bool IsRewinding() const
			{
				return rewinding;
			}

			bool IsSoundRewinding() const
			{
				return rewinding && sound.IsRewinding();
			}

			uint GetSoundLatency() const
			{
				return sound.GetLatency();
			}
		};
	}
}

#endif
