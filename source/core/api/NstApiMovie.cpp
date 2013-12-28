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

#include "../NstMachine.hpp"
#include "NstApiTapeRecorder.hpp"
#include "NstApiMovie.hpp"

namespace Nes
{
	namespace Api
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Movie::StateCaller Movie::stateCallback;

		Result Movie::Play(std::istream& stream,CallbackMode mode) throw()
		{
			Api::TapeRecorder(emulator).Stop();
			return emulator.tracker.MoviePlay( emulator, &stream, mode == ENABLE_CALLBACK );
		}

		Result Movie::Record(std::ostream& stream,How how,CallbackMode mode) throw()
		{
			return emulator.tracker.MovieRecord( emulator, &stream, how == APPEND, mode == ENABLE_CALLBACK );
		}

		void Movie::Stop() throw()
		{
			emulator.tracker.MovieStop();
		}

		void Movie::Eject() throw()
		{
			emulator.tracker.MovieEject();
		}

		void Movie::Cut() throw()
		{
			emulator.tracker.MovieCut();
		}

		bool Movie::IsPlaying() const throw()
		{
			return emulator.tracker.MovieIsPlaying();
		}

		bool Movie::IsRecording() const throw()
		{
			return emulator.tracker.MovieIsRecording();
		}

		bool Movie::IsStopped() const throw()
		{
			return emulator.tracker.MovieIsStopped();
		}

		bool Movie::IsInserted() const throw()
		{
			return emulator.tracker.MovieIsInserted();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
