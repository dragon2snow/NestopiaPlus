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

#ifndef NST_MANAGER_FRAMECLOCK_H
#define NST_MANAGER_FRAMECLOCK_H

#pragma once

#include "NstSystemTimer.hpp"

namespace Nestopia
{
	namespace Window
	{
		class FrameClock;
	}

	namespace Managers
	{
		class FrameClock
		{
		public:

			FrameClock(Window::Menu&,Emulator&,const Configuration&);
			~FrameClock();

			void Save(Configuration&) const;

		private:

			void OnEmuEvent(Emulator::Event);
			void OnMenuOptionsTiming(uint);
			void Synchronize(ibool,uint);
			void UpdateRewinderState(ibool=true) const;
			void UpdateSettings();
			void ResetTimer();

			enum
			{
				MAX_SPEED_NO_FRAMESKIP = 60
			};

			struct Timing
			{
				System::Timer::Value counter;
				uint frameSkips;
			};

			struct Settings
			{
				uint refreshRate;
				ibool autoFrameSkip;
				uint maxFrameSkips;
			};

			System::Timer timer;
			Timing time;
			Settings settings;
			Emulator& emulator;
			const Window::Menu& menu;
			Object::Heap<Window::FrameClock> dialog;

		public:

			uint NumFrameSkips()
			{
				uint count = time.frameSkips;
				time.frameSkips = 0;
				return count;
			}

			void GameSynchronize(ibool wait)
			{
				Synchronize( wait, ~0U );
			}

			void SoundSynchronize()
			{
				Synchronize( true, 0U );
			}

			void StartEmulation()
			{
				ResetTimer();
			}

			void StopEmulation()
			{
				ResetTimer();
			}
		};
	}
}

#endif
