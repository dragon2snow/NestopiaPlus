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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif 

#include <Windows.h>
#include <MMSystem.h>
#include "../paradox/PdxLibrary.h"
#include "../NstNes.h"
#include "NstTimer.h"

#pragma comment(lib,"winmm")

////////////////////////////////////////////////////////////////////////////////////////
// constructor
////////////////////////////////////////////////////////////////////////////////////////

TIMER::TIMER()
: 
HasPFCounter(QueryPerformanceFrequency(PDX_CAST(LARGE_INTEGER*,&pfRefreshClockNTSC)))
{
	if (HasPFCounter)
	{
		PDX_ASSERT(pfRefreshClockNTSC);

		pfTicksPerMilli = pfRefreshClockNTSC / 1000;
		pfRefreshClockPAL = pfRefreshClockNTSC / NES_FPS_REAL_PAL;
		pfRefreshClock = pfRefreshClockNTSC /= NES_FPS_NTSC;
		pfLast = 0;		
	}
	else
	{
		dbRefreshClockPAL = 1000.0 / NES_FPS_PAL;
		dbRefreshClock = dbRefreshClockNTSC = 1000.0 / NES_FPS_NTSC;
		dbLast = 0.0;
		period = FALSE;

		TIMECAPS caps;

		if (timeGetDevCaps(&caps,sizeof(caps)) == TIMERR_NOERROR)
		{
			period = TRUE;
			resolution = PDX_MIN(PDX_MAX(caps.wPeriodMin,1),caps.wPeriodMax);
			timeBeginPeriod( resolution );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// destructor
////////////////////////////////////////////////////////////////////////////////////////

TIMER::~TIMER()
{
	if (!HasPFCounter && period)
		timeEndPeriod( resolution );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID TIMER::EnablePAL(const BOOL pal)
{
	if (pal)
	{
		if (HasPFCounter) pfRefreshClock = pfRefreshClockPAL;
		else              dbRefreshClock = dbRefreshClockPAL;
	}
	else
	{
		if (HasPFCounter) pfRefreshClock = pfRefreshClockNTSC;
		else              dbRefreshClock = dbRefreshClockNTSC;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID TIMER::Reset()
{
	if (HasPFCounter) 
	{
		QueryPerformanceCounter(PDX_CAST_PTR(LARGE_INTEGER,pfLast));
	}
	else 
	{
		dbLast = timeGetTime();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

UINT TIMER::SynchRefreshRate(const BOOL AutoFrameSkip,const BOOL InBackground)
{
	if (HasPFCounter)
	{
		I64 pfCurrent;

		if (InBackground)
		{
			QueryPerformanceCounter(PDX_CAST_PTR(LARGE_INTEGER,pfCurrent));

			if ((pfCurrent -= pfLast) < pfRefreshClock)
			{
				const LONG remaining = ((pfRefreshClock - pfCurrent) / pfTicksPerMilli) - 2;

				if (remaining > 0)
					Sleep( remaining );
			}
		}

		do 
		{	
			QueryPerformanceCounter(PDX_CAST_PTR(LARGE_INTEGER,pfCurrent));
		} 
		while (pfCurrent - pfLast < pfRefreshClock);

		pfLast += pfRefreshClock;

		if (AutoFrameSkip)
		{
			UINT SkipFrames = (pfCurrent - pfLast) / pfRefreshClock;

			if (SkipFrames > RESET_TIMER_FRAMES)
			{
				pfLast = pfCurrent;
			}
			else if (SkipFrames > 1)
			{
				--SkipFrames;

				if (SkipFrames > MAX_SKIP_FRAMES)
					SkipFrames = MAX_SKIP_FRAMES;

				pfLast += pfRefreshClock * SkipFrames;
				return SkipFrames;
			}
		}
	}
	else
	{
		DOUBLE dbCurrent;

		if (InBackground)
		{
			if ((dbCurrent = timeGetTime() - dbLast) < dbRefreshClock)
			{
				dbCurrent = dbRefreshClock - dbCurrent;

				if (dbCurrent > 2.0)
					Sleep( DWORD(dbCurrent) - 1 );
			}
		}

		while ((dbCurrent = timeGetTime()) - dbLast < dbRefreshClock);
		
		dbLast += dbRefreshClock;

		if (AutoFrameSkip)
		{
			UINT SkipFrames = (dbCurrent - dbLast) / dbRefreshClock;

			if (SkipFrames > RESET_TIMER_FRAMES)
			{
				dbLast = dbCurrent;
			}
			else if (SkipFrames > 1)
			{
				--SkipFrames;

				if (SkipFrames > MAX_SKIP_FRAMES)
					SkipFrames = MAX_SKIP_FRAMES;

				dbLast += dbRefreshClock * SkipFrames;
				return SkipFrames;
			}
		}
	}

	return 0;
}
