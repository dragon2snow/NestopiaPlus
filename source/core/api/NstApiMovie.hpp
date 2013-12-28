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

#ifndef NST_API_MOVIE_H
#define NST_API_MOVIE_H

#include <iosfwd>
#include "NstApi.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#if NST_ICC >= 810
#pragma warning( push )
#pragma warning( disable : 304 444 )
#elif NST_MSVC >= 1200
#pragma warning( push )
#pragma warning( disable : 4512 )
#endif

namespace Nes
{
	namespace Api
	{
		class Movie : public Base
		{
			struct EventCaller;

		public:

			template<typename T>
			Movie(T& e)
			: Base(e) {}

			enum How
			{
				CLEAN,
				APPEND
			};

			Result Play(std::istream&) throw();
			Result Record(std::iostream&,How=CLEAN) throw();
			void   Stop() throw();
			void   Eject() {}

			bool IsPlaying() const throw();
			bool IsRecording() const throw();
			bool IsStopped() const throw();

			enum Event
			{
				EVENT_PLAYING,
				EVENT_PLAYING_STOPPED,
				EVENT_RECORDING,
				EVENT_RECORDING_STOPPED
			};

			enum
			{
				NUM_EVENT_CALLBACKS = 4
			};

			typedef void (NST_CALLBACK *EventCallback) (UserData,Event,Result);

			static EventCaller eventCallback;
		};

		struct Movie::EventCaller : Core::UserCallback<Movie::EventCallback>
		{
			void operator () (Event event,Result result=RESULT_OK) const
			{
				if (function)
					function( userdata, event, result );
			}
		};
	}
}

#if NST_MSVC >= 1200 || NST_ICC >= 810
#pragma warning( pop )
#endif

#endif
