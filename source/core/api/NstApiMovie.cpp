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
#include "../NstStream.hpp"
#include "NstApiEmulator.hpp"
#include "NstApiMachine.hpp"
#include "NstApiMovie.hpp"
#include "../NstImage.hpp"

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("s", on)
#endif

namespace Nes
{
	namespace Api
	{
		Movie::StateCaller Movie::stateCallback;

		Result Movie::Play(std::istream& stream,CallbackMode mode)
		{
			return emulator.tracker.MoviePlay( emulator, &stream, mode == ENABLE_CALLBACK );
		}
	
		Result Movie::Record(std::ostream& stream,How how,CallbackMode mode)
		{
			return emulator.tracker.MovieRecord( emulator, &stream, how == APPEND, mode == ENABLE_CALLBACK );
		}
	
		void Movie::Stop()
		{
			emulator.tracker.MovieStop();
		}

		void Movie::Eject()
		{
			emulator.tracker.MovieEject();
		}

		void Movie::Cut()
		{
			emulator.tracker.MovieCut();
		}

		bool Movie::IsPlaying() const
		{
			return emulator.tracker.MovieIsPlaying();
		}

		bool Movie::IsRecording() const
		{
			return emulator.tracker.MovieIsRecording();
		}

		bool Movie::IsStopped() const
		{
			return emulator.tracker.MovieIsStopped();
		}

		bool Movie::IsInserted() const
		{
			return emulator.tracker.MovieIsInserted();
		}	
	}
}

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif
