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

			typedef System::Timer::Value Value;

			void Save(Configuration&) const;

		private:

			void OnEmuEvent(Emulator::Event);
			void OnMenuOptionsTiming(uint);
			void Synchronize();
			uint Synchronize(ibool,uint);
			void UpdateRewinderState(ibool=true) const;
			void UpdateSettings();
			void ResetTimer();

			enum
			{
				MAX_SPEED_NO_FRAMESKIP = 60,
				SAFE_WAIT_TIME = 1
			};

			struct Settings
			{
				uint refreshRate;
				ibool autoFrameSkip;
				uint maxFrameSkips;
			};

			struct Time
			{
				System::Timer::Value counter;
				System::Timer::Value base;
				System::Timer::Value session;
				System::Timer::Value safeFrame;
			};

			System::Timer timer;
			Time time;
			Settings settings;
			Emulator& emulator;
			const Window::Menu& menu;
			Object::Heap<Window::FrameClock> dialog;

		public:

			void GameHalt()
			{
				time.session = timer.Elapsed() - time.session;
			}

			void GameResume()
			{
				time.base = timer.Elapsed();
			}

			uint GameSynchronize(ibool exclusive)
			{
				return (exclusive | settings.autoFrameSkip) ? Synchronize( exclusive, ~0U ) : (Synchronize(), 0);
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

			uint GetRefreshRate() const
			{
				return settings.refreshRate;
			}

			ibool UsesAutoFrameSkip() const
			{
				return settings.autoFrameSkip;
			}
		};
	}
}

#endif
