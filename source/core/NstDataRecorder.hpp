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

#ifndef NST_DATARECORDER_H
#define NST_DATARECORDER_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "NstVector.hpp"

namespace Nes
{
	namespace Core
	{
		class Cpu;

		namespace State
		{
			class Saver;
			class Loader;
		}

		class NST_NO_VTABLE DataRecorder
		{
		public:

			DataRecorder(Cpu&);
			~DataRecorder();

			Result Record();
			Result Play();
			void Stop();
			void VSync();

			void SaveState(State::Saver&) const;
			void LoadState(State::Loader&);

		private:

			void Load();
			void Prepare();

			NES_DECL_HOOK( Tape )
			NES_DECL_POKE( 4016 )
			NES_DECL_PEEK( 4016 )
			NES_DECL_POKE( 4017 )
			NES_DECL_PEEK( 4017 )

			enum
			{
				CLOCK = 32000
			};

			enum Status
			{
				STOPPED,
				PLAYING,
				RECORDING
			};

			Cpu& cpu;
			Cycle cycles;
			Status status;
			Vector<u8> stream;		
			dword pos;
			uint in;
			uint out;
			const Io::Port* p4016;
			ibool loaded;
			dword crc;

			static const Cycle clocks[2][2];

		public:

			void Reset()
			{	
				cycles = NES_CYCLE_MAX;
				status = STOPPED;
			}

			bool CanPlay()
			{
				if (!loaded)
					Load();

				return stream.Size();
			}

			bool CanSaveState() const
			{
				return stream.Size();
			}

			bool IsStopped() const
			{
				return status == STOPPED;
			}

			bool IsRecording() const
			{
				return status == RECORDING;
			}

			bool IsPlaying() const
			{
				return status == PLAYING;
			}
		};
	}
}

#endif
