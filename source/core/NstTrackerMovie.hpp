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

#ifndef NST_TRACKER_MOVIE_H
#define NST_TRACKER_MOVIE_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Tracker::Movie
		{
			typedef Result (Api::Emulator::*EmuLoadState)(StdStream,bool);
			typedef Result (Api::Emulator::*EmuSaveState)(StdStream,bool);
			typedef Result (Api::Emulator::*EmuReset)(bool);

		public:

			Movie(Api::Emulator&,EmuReset,EmuLoadState,EmuSaveState,Cpu&,dword);
			~Movie();

			Result Play(StdStream,bool);
			Result Record(StdStream,bool,bool);
			void   Stop();
			void   Cut();

			bool Reset(bool);
			bool BeginFrame(dword);
			bool EndFrame();

		private:

			void SaveCpuPorts();
			bool Stop(Result);
			void Close();

			enum
			{
				VERSION         = 0x01,
				MAX_FRAME_READS = 64,
				LOCK_BIT        = 0x80,
				LOCK_SIZE_BIT   = 0x40,
				OPEN_BUS        = 0x40
			};

			enum Status
			{
				STOPPED,
				PLAYING,
				RECORDING
			};

			class Player;
			class Recorder;

			NES_DECL_PEEK( 4016_Record )
			NES_DECL_PEEK( 4017_Record )
			NES_DECL_PEEK( 4016_Play   )
			NES_DECL_PEEK( 4017_Play   )
			NES_DECL_POKE( 4016        )
			NES_DECL_POKE( 4017        )

			const Io::Port* ports[2];
			Cpu& cpu;
			Status status;
			Player* player;
			Recorder* recorder;
			Api::Emulator& emulator;
			const EmuSaveState emuSaveState;
			const EmuLoadState emuLoadState;
			const EmuReset emuReset;
			ibool callbackEnable;
			const dword prgCrc;

		public:

			bool IsPlaying() const
			{
				return status == PLAYING;
			}

			bool IsRecording() const
			{
				return status == RECORDING;
			}

			bool IsStopped() const
			{
				return status == STOPPED;
			}
		};
	}
}

#endif
