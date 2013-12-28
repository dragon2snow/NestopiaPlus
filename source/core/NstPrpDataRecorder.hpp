////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2007 Martin Freij
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

#ifndef NST_PRP_DATARECORDER_H
#define NST_PRP_DATARECORDER_H

#ifndef NST_VECTOR_H
#include "NstVector.hpp"
#endif

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Peripherals
		{
			class DataRecorder
			{
			public:

				explicit DataRecorder(Cpu&);
				~DataRecorder();

				void   Reset();
				void   PowerOff();
				Result Record();
				Result Play();
				Result Stop();
				bool   Playable();
				void   VSync();
				void   SaveState(State::Saver&,dword) const;
				void   LoadState(State::Loader&);

			private:

				enum
				{
					MAX_LENGTH = SIZE_4096K,
					CLOCK = 32000
				};

				void Start();

				NES_DECL_HOOK( Tape );
				NES_DECL_POKE( 4016 );
				NES_DECL_PEEK( 4016 );

				enum Status
				{
					STOPPED,
					PLAYING,
					RECORDING
				};

				Cpu& cpu;
				Cycle cycles;
				Status status;
				Vector<byte> stream;
				dword pos;
				uint in;
				uint out;
				const Io::Port* p4016;
				ibool loaded;
				File file;

				static const dword clocks[2][2];

			public:

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
}

#endif
