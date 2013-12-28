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

#ifndef NST_TRACKER_H
#define NST_TRACKER_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Api
	{
		class Emulator;
	}

	namespace Core
	{
		namespace Video
		{
			class Output;
		}

		namespace Sound
		{
			class Output;
		}

		namespace Input
		{
			class Controllers;
		}

		class Tracker
		{
			class Movie;
			class Rewinder;

			Rewinder* rewinder;
			Movie* movie;
			ibool rewinderSound;

		public:

			Tracker();
			~Tracker();

			void   Reset(bool);
			Result Execute(Api::Emulator&,Video::Output*,Sound::Output*,Input::Controllers*);
			void   Flush();
			void   Unload();
			uint   GetSoundLatency(const Apu&) const;
			bool   IsLocked() const;

			Result RewinderEnable(Api::Emulator*);
			void   RewinderEnableSound(bool);
			void   RewinderReset() const;
			Result RewinderStart() const;
			Result RewinderStop() const;
			bool   IsRewinding() const;

			Result MoviePlay(Api::Emulator&,StdStream,bool);
			Result MovieRecord(Api::Emulator&,StdStream,bool,bool);
			void   MovieStop();
			void   MovieCut();
			void   MovieEject();
			bool   MovieIsPlaying() const;
			bool   MovieIsRecording() const;
			bool   MovieIsStopped() const;

			bool RewinderIsEnabled() const
			{
				return rewinder != NULL;
			}

			bool RewinderIsSoundEnabled() const
			{
				return rewinderSound;
			}

			bool MovieIsInserted() const
			{
				return movie != NULL;
			}
		};
	}
}

#endif
