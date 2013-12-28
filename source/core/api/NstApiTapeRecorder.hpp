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

#ifndef NST_API_TAPERECORDER_H
#define NST_API_TAPERECORDER_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "NstApi.hpp"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4512 )
#endif

namespace Nes
{
	namespace Core
	{
		class DataRecorder;
	}

	namespace Api
	{
		class TapeRecorder : public Base
		{
			Core::DataRecorder* Query() const;

		public:

			TapeRecorder(Emulator& e)
			: Base(e) {}

			bool IsStopped() const;
			bool IsRecording() const;
			bool IsPlaying() const;
			bool CanPlay() const;
			
			Result Play();
			Result Record();
			void Stop();

			bool IsConnected() const
			{
				return Query() != NULL;
			}
		};
	}
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
