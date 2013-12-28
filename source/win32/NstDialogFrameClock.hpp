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
				DEFAULT_REWIND_SPEED = DEFAULT_SPEED,
				DEFAULT_FRAME_SKIPS = 8
			};

			struct Settings
			{
				bool autoFrameSkip;
				bool vsync;
				bool rewinder;
				bool useDefaultSpeed;
				bool useDefaultRewindSpeed;
				bool noRewindSound;
				bool pfCounter;
				uchar speed;
				uchar altSpeed;
				uchar rewindSpeed;
				uchar maxFrameSkips;
			};

			void UpdateRewinderEnable() const;

			ibool OnInitDialog      (Param&);
			ibool OnDestroy         (Param&);
			ibool OnHScroll         (Param&);
			ibool OnCmdRefresh      (Param&);
			ibool OnCmdDefaultSpeed (Param&);
			ibool OnCmdRewinder		(Param&);
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

			bool UseAutoFrameSkip() const
			{
				return settings.autoFrameSkip;
			}

			bool UseVSync() const
			{
				return settings.vsync;
			}

			bool UseRewinder() const
			{
				return settings.rewinder;
			}

			bool UseDefaultSpeed() const
			{
				return settings.useDefaultSpeed;
			}

			bool UseDefaultRewindSpeed() const
			{
				return settings.useDefaultRewindSpeed;
			}

			bool NoRewindSound() const
			{
				return settings.noRewindSound;
			}

			uint GetSpeed() const
			{
				return settings.speed;
			}

			uint GetAltSpeed() const
			{
				return settings.altSpeed;
			}

			uint GetRewindSpeed() const
			{
				return settings.rewindSpeed;
			}

			uint GetMaxFrameSkips() const
			{
				return settings.maxFrameSkips;
			}
		};
	}
}

#endif
