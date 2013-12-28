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

#ifndef NST_API_TAPERECORDER_H
#define NST_API_TAPERECORDER_H

#include "NstApi.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#if NST_ICC >= 810
#pragma warning( push )
#pragma warning( disable : 444 )
#elif NST_MSVC >= 1200
#pragma warning( push )
#pragma warning( disable : 4512 )
#endif

namespace Nes
{
	namespace Core
	{
		namespace Peripherals
		{
			class DataRecorder;
		}
	}

	namespace Api
	{
		class TapeRecorder : public Base
		{
			Core::Peripherals::DataRecorder* Query() const;
			Core::Peripherals::DataRecorder* CommandQuery() const;

		public:

			template<typename T>
			TapeRecorder(T& e)
			: Base(e) {}

			bool IsPlaying() const throw();
			bool IsRecording() const throw();
			bool IsStopped() const throw();

			bool CanPlay() const throw();

			Result Play() throw();
			Result Record() throw();
			Result Stop() throw();

			bool IsConnected() const throw()
			{
				return Query();
			}

			bool CanRecord() const throw()
			{
				return CommandQuery();
			}

			bool CanStop() const throw()
			{
				return CommandQuery();
			}
		};
	}
}

#if NST_MSVC >= 1200 || NST_ICC >= 810
#pragma warning( pop )
#endif

#endif
