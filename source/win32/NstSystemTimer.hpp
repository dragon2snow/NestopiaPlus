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

#ifndef NST_SYSTEM_TIMER_H
#define NST_SYSTEM_TIMER_H

#pragma once

#include "NstMain.hpp"

namespace Nestopia
{
	namespace System
	{
		class Timer
		{
		public:

			typedef u64 Value;

			enum Type
			{
				MULTIMEDIA,
				PERFORMANCE
			};

			explicit Timer(Type=PERFORMANCE);

			ibool Reset(Type);
			Value Elapsed() const;
			void  Wait(Value,Value);

		private:

			enum 
			{
				THRESHOLD = 1
			};

			struct Settings
			{
				Settings();
				~Settings();

				Value pfFrequency;
				uint period;
			};

			Value start;
			Type type;
			uint threshold;
			uint giveup;

			static const Settings settings;

		public:

			static ibool HasPerformanceCounter()
			{
				return settings.pfFrequency != 0;
			}

			Type GetType() const
			{
				return type;
			}

			Value GetFrequency() const
			{
				return type == PERFORMANCE ? settings.pfFrequency : Value(1000);
			}
		};
	}
}

#endif
