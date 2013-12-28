////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003 Martin Freij
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

#pragma once

#ifndef NST_TIMER_H
#define NST_TIMER_H

#include "NstManager.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class TIMERMANAGER : public MANAGER
{
public:

	TIMERMANAGER();
	~TIMERMANAGER();

	VOID Create  (CONFIGFILE* const);
	VOID Destroy (CONFIGFILE* const);

	UINT SynchRefreshRate(const BOOL,const BOOL,INT);
	VOID EnableCustomFPS(const BOOL);
	VOID EnablePAL(const BOOL);

private:

	VOID UpdateRefreshRate();
	VOID UpdateDialog(HWND);
	VOID ResetDialog();

	BOOL DialogProc(HWND,UINT,WPARAM,LPARAM);

	enum
	{
		MIN_FPS = 30,
		MAX_FPS = 240,
		MIN_FRAME_SKIPS = 1,
		MAX_FRAME_SKIPS = 16,
		FRAME_SKIPS_TIMEOUT = 32,
		DEFAULT_FPS = NES_FPS_NTSC,
		DEFAULT_FRAME_SKIPS = 8,
		SLEEP_MINIMUM = 3,
		SLEEP_LIMIT = 2,
	};

	BOOL IsPAL;
	BOOL UseDefaultFps;
	UINT MaxFrameSkips;
	UINT CustomFps;
	UINT fps;
	BOOL UseVSync;
	BOOL AutoFrameSkip;
	BOOL DoSleep;

	const BOOL HasPFCounter;

	union
	{
		struct  
		{
			I64 pfTicksPerMilli;
			I64 pfRefreshFrequency;
			I64 pfRefreshClockNTSC;
			I64 pfRefreshClockPAL;
			I64 pfRefreshClock;
			I64 pfLast;
		};

		struct  
		{
			DOUBLE dbRefreshClockNTSC;
			DOUBLE dbRefreshClockPAL;
			DOUBLE dbRefreshClock;
			DOUBLE dbLast;
			UINT resolution;
			BOOL period;
		};
	};
};

#endif
