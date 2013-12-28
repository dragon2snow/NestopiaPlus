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

	DOUBLE CurrentTime() const;
	VOID Synchronize(const BOOL,BOOL);

	PDX_NO_INLINE VOID Update();
	PDX_NO_INLINE VOID EnableCustomFPS(const BOOL);
	PDX_NO_INLINE VOID EnablePAL(const BOOL);
	PDX_NO_INLINE BOOL CalculateFPS(DOUBLE&);

	inline UINT NumFrameSkips() const
	{ return FrameSkips; }

	inline DOUBLE GetFPS() const
	{ return dbFps; }
	
private:

	enum
	{
		MIN_FPS = 30,
		MAX_FPS = 240,
		MIN_FRAME_SKIPS = 1,
		MAX_FRAME_SKIPS = 16,
		DEFAULT_FPS = NES_FPS_NTSC,
		DEFAULT_FRAME_SKIPS = 8,
	};

	BOOL DialogProc(HWND,UINT,WPARAM,LPARAM);

	PDX_NO_INLINE VOID SwitchToPFCounter();
	PDX_NO_INLINE VOID SwitchToMMTimer();
	PDX_NO_INLINE VOID ResetDialog();
	PDX_NO_INLINE VOID UpdateDialog(HWND);
	PDX_NO_INLINE VOID UpdateSettings(HWND);

	const BOOL HasPFCounter;

	I64 pfFrequency;

	BOOL IsPAL;
	BOOL UseDefaultFps;
	UINT MaxFrameSkips;
	UINT CustomFps;
	UINT fps;
	BOOL period;
	UINT resolution;
	BOOL AutoFrameSkip;
	BOOL UseVSync;
	BOOL UsePFCounter;

	class SLEEPER
	{
	public:

		SLEEPER()
		{
			Reset( 1000 / NES_FPS_NTSC );
		}

		VOID Reset(const UINT);
		VOID GoToBed(TIMERMANAGER&,DOUBLE&);

		inline BOOL IsFired() const
		{ return fired; }

	private:

		enum
		{
			MIN_SLEEP_TIME = 4,
			ZOMBIE_TIME = 2,
			OVERSLEPT = 1
		};

		BOOL fired;
		UINT ZombieTime;
		UINT AvailableCoffee;
		UINT Overflows;
	};

	SLEEPER sleeper;

	UINT FrameSkips;
	UINT SleepThreshold;
	BOOL vSyncOnly;

	union
	{
		I64 pfStart;
		DWORD dwStart;
	};

	DOUBLE dbScale;
	DOUBLE dbRefresh;
	DOUBLE dbTarget;
	DOUBLE dbFps;

	union
	{
		I64 pfLastFps;
		DWORD dwLastFps;
	};

	ULONG FrameCounter;

public:

	inline BOOL NoSleeping() const
	{ return vSyncOnly || sleeper.IsFired(); }
};

#endif
