////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2005 Martin Freij
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

#ifndef NST_DIALOG_FRAMECLOCK_H
#define NST_DIALOG_FRAMECLOCK_H

#pragma once

#include "NstWindowDialog.hpp"

namespace Nestopia
{
	namespace Window
	{
		class FrameClock
		{
		public:

			explicit FrameClock(const Configuration&);

			void Save(Configuration&) const;

		private:

			struct Handlers;

			enum
			{
				MIN_SPEED = 30,
				MAX_SPEED = 240,
				MIN_FRAME_SKIPS = 1,
				MAX_FRAME_SKIPS = 16,
				DEFAULT_SPEED = 60,
				DEFAULT_ALT_SPEED = 180,
				DEFAULT_FRAME_SKIPS = 8
			};

			struct Settings
			{
				ibool autoFrameSkip;
				ibool vsync;
				ibool useDefaultSpeed;
				ibool pfCounter;
				uint speed;
				uint altSpeed;
				uint maxFrameSkips;
			};

			ibool OnInitDialog      (Param&);
			ibool OnDestroy         (Param&);
			ibool OnHScroll         (Param&);
			ibool OnCmdRefresh      (Param&);
			ibool OnCmdDefaultSpeed (Param&);
			ibool OnCmdDefault      (Param&);
			ibool OnCmdOk           (Param&);

			Settings settings;
			Dialog dialog;

		public:

			void Open()
			{
				dialog.Open();
			}

			uint UsePerformanceCounter() const
			{
				return settings.pfCounter;
			}

			ibool UseAutoFrameSkip() const
			{
				return settings.autoFrameSkip;
			}

			ibool UseVSync() const
			{
				return settings.vsync;
			}

			ibool UseDefaultSpeed() const
			{
				return settings.useDefaultSpeed;
			}

			uint GetSpeed() const
			{
				return settings.speed;
			}

			uint GetAltSpeed() const
			{
				return settings.altSpeed;
			}

			uint GetMaxFrameSkips() const
			{
				return settings.maxFrameSkips;
			}
		};
	}
}

#endif
