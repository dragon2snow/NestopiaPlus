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
#include <CommCtrl.h>
#include "NstGraphicManager.h"
#include "NstSoundManager.h"
#include "NstTimerManager.h"
#include "NstApplication.h"

#pragma comment(lib,"winmm")

////////////////////////////////////////////////////////////////////////////////////////
// constructor
////////////////////////////////////////////////////////////////////////////////////////

TIMERMANAGER::TIMERMANAGER()
: 
MANAGER        (IDD_TIMING),
CustomFps      (NES_FPS_NTSC),
fps            (NES_FPS_NTSC),
IsPAL          (FALSE),
MaxFrameSkips  (DEFAULT_FRAME_SKIPS),
period         (FALSE),
HasPFCounter   (::QueryPerformanceFrequency(PDX_CAST(LARGE_INTEGER*,&pfFrequency)) && bool(pfFrequency))
{
	UsePFCounter = HasPFCounter;

	TIMECAPS caps;

	if (::timeGetDevCaps( &caps, sizeof(caps) ) == TIMERR_NOERROR)
	{
		period = TRUE;
		resolution = PDX_MIN(PDX_MAX(caps.wPeriodMin,1),caps.wPeriodMax);
		::timeBeginPeriod( resolution );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// destructor
////////////////////////////////////////////////////////////////////////////////////////

TIMERMANAGER::~TIMERMANAGER()
{
	if (period)
		::timeEndPeriod( resolution );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID TIMERMANAGER::Create(CONFIGFILE* const ConfigFile)
{
	LOGFILE::Output
	(
	    HasPFCounter ?
		"TIMER: performance counter present" :
        "TIMER: performance counter not present, forcing to use default multimedia timer"
	);

	if (ConfigFile)
	{
		CONFIGFILE& file = *ConfigFile;
		
		AutoFrameSkip = ( file[ "timer auto frame skip" ] == "yes" );
		UseVSync      = ( file[ "timer vsync"           ] != "no"  );
		UseDefaultFps = ( file[ "timer default fps"     ] != "no"  );

		CustomFps     = file[ "timer custom fps"      ].ToUlong();
		MaxFrameSkips = file[ "timer max frame skips" ].ToUlong();

		UsePFCounter  = ( HasPFCounter && file[ "timer performance counter" ] != "no" );

		if (MaxFrameSkips) MaxFrameSkips = PDX_CLAMP(MaxFrameSkips,MIN_FRAME_SKIPS,MAX_FRAME_SKIPS);
		else               MaxFrameSkips = DEFAULT_FRAME_SKIPS;

		if (CustomFps) CustomFps = PDX_CLAMP(CustomFps,MIN_FPS,MAX_FPS);
		else           CustomFps = DEFAULT_FPS;

		Update();
	}
	else
	{
		ResetDialog();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID TIMERMANAGER::SwitchToPFCounter()
{
	PDX_ASSERT( HasPFCounter );
	::QueryPerformanceCounter(PDX_CAST_PTR(LARGE_INTEGER,pfStart));
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID TIMERMANAGER::SwitchToMMTimer()
{
	dwStart = ::timeGetTime();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID TIMERMANAGER::Destroy(CONFIGFILE* const ConfigFile)
{
	if (ConfigFile)
	{
		CONFIGFILE& file = *ConfigFile;

		file[ "timer auto frame skip"     ] = (AutoFrameSkip ? "yes" : "no");
		file[ "timer max frame skips"     ] = MaxFrameSkips;
		file[ "timer vsync"               ] = (UseVSync      ? "yes" : "no");
		file[ "timer default fps"         ] = (UseDefaultFps ? "yes" : "no");
		file[ "timer custom fps"          ] = CustomFps;
		file[ "timer performance counter" ] = (UsePFCounter ? "yes" : "no");
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID TIMERMANAGER::EnableCustomFPS(const BOOL state)
{
	if (bool(UseDefaultFps) == bool(state))
	{
		UseDefaultFps = !state;
		Update();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID TIMERMANAGER::ResetDialog()
{
	AutoFrameSkip = FALSE;
	UseVSync = TRUE;
	CustomFps = DEFAULT_FPS;
	UseDefaultFps = TRUE;
	MaxFrameSkips = DEFAULT_FRAME_SKIPS;
	
	Update();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL TIMERMANAGER::DialogProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg) 
	{
     	case WM_INITDIALOG:

			::SendMessage
			( 
				::GetDlgItem(hDlg,IDC_TIMING_FPS), 
				TBM_SETRANGE, 
				WPARAM(FALSE), 
				LPARAM(MAKELONG(MIN_FPS,MAX_FPS)) 
			);

			::SendMessage
			( 
				::GetDlgItem(hDlg,IDC_TIMING_FRAME_SKIPS), 
				TBM_SETRANGE, 
				WPARAM(FALSE), 
				LPARAM(MAKELONG(MIN_FRAME_SKIPS,MAX_FRAME_SKIPS)) 
			);

			UpdateDialog( hDlg );
			return TRUE;
			
		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDC_TIMING_SYNC_REFRESH:
				case IDC_TIMING_AUTO_FRAME_SKIP:

					AutoFrameSkip = (::IsDlgButtonChecked( hDlg, IDC_TIMING_AUTO_FRAME_SKIP ) == BST_CHECKED);
					
					::EnableWindow( ::GetDlgItem( hDlg, IDC_TIMING_FRAME_SKIPS      ), AutoFrameSkip );
					::EnableWindow( ::GetDlgItem( hDlg, IDC_TIMING_FRAME_SKIPS_TEXT ), AutoFrameSkip );
					::EnableWindow( ::GetDlgItem( hDlg, IDC_TIMING_FRAME_SKIPS_NUM  ), AutoFrameSkip );
					return TRUE;

				case IDC_TIMING_VSYNC:

					UseVSync = ( ::IsDlgButtonChecked( hDlg, IDC_TIMING_VSYNC ) == BST_CHECKED );
					return TRUE;

				case IDC_TIMING_DEFAULT_FPS:
				
					UseDefaultFps = ( ::IsDlgButtonChecked( hDlg, IDC_TIMING_DEFAULT_FPS ) == BST_CHECKED );

					::EnableWindow( ::GetDlgItem( hDlg, IDC_TIMING_FPS      ), !UseDefaultFps );
					::EnableWindow( ::GetDlgItem( hDlg, IDC_TIMING_FPS_TEXT ), !UseDefaultFps );
					::EnableWindow( ::GetDlgItem( hDlg, IDC_TIMING_FPS_NUM  ), !UseDefaultFps );
					return TRUE;

				case IDC_TIMING_DEFAULT:

					ResetDialog();
					UpdateDialog( hDlg );
					return TRUE;

				case IDC_TIMING_OK:

					UpdateSettings( hDlg );
					::EndDialog( hDlg, 0 );
					return TRUE;
			}
			return FALSE;

		case WM_HSCROLL:
		
     		if (HWND(lParam) == ::GetDlgItem( hDlg, IDC_TIMING_FPS ))
			{
				const UINT fps = (UINT) ::SendMessage
				(
					::GetDlgItem( hDlg, IDC_TIMING_FPS ),
					TBM_GETPOS,
					WPARAM(0),
					LPARAM(0)
				);

				if (CustomFps != fps)
				{
					CustomFps = fps;

					::SetWindowText
					( 
						::GetDlgItem( hDlg, IDC_TIMING_FPS_NUM ), 
						PDXSTRING(fps).String() 
					);
				}
			}
			else if (PDX_CAST(HWND,lParam) == ::GetDlgItem( hDlg, IDC_TIMING_FRAME_SKIPS ))
			{
				const UINT skips = (UINT) ::SendMessage
				(
					::GetDlgItem( hDlg, IDC_TIMING_FRAME_SKIPS ),
					TBM_GETPOS,
					WPARAM(0),
					LPARAM(0)
				);

				if (MaxFrameSkips != skips)
				{
					MaxFrameSkips = skips;

					::SetWindowText
					( 
						::GetDlgItem( hDlg, IDC_TIMING_FRAME_SKIPS_NUM ), 
						PDXSTRING(skips).String() 
					);
				}
			}			
			return TRUE;

     	case WM_CLOSE:

			UpdateSettings( hDlg );
     		::EndDialog( hDlg, 0 );
     		return TRUE;

		case WM_DESTROY:

			Update();
			return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID TIMERMANAGER::UpdateSettings(HWND hDlg)
{
	UsePFCounter = (HasPFCounter && (::IsDlgButtonChecked( hDlg, IDC_TIMING_PFC ) == BST_CHECKED));
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID TIMERMANAGER::UpdateDialog(HWND hDlg)
{
	::SendMessage
	( 
     	::GetDlgItem( hDlg, IDC_TIMING_FPS ), 
		TBM_SETPOS, 
		WPARAM(TRUE), 
		LPARAM(CustomFps) 
	);
	
	::SetWindowText
	( 
     	::GetDlgItem( hDlg, IDC_TIMING_FPS_NUM ), 
		PDXSTRING(CustomFps).String() 
	);

	::SendMessage
	( 
		::GetDlgItem( hDlg, IDC_TIMING_FRAME_SKIPS ), 
		TBM_SETPOS, 
		WPARAM(TRUE), 
		LPARAM(MaxFrameSkips) 
	);

	::SetWindowText
	( 
		::GetDlgItem( hDlg, IDC_TIMING_FRAME_SKIPS_NUM ), 
		PDXSTRING(MaxFrameSkips).String() 
	);

	::CheckDlgButton
	( 
     	hDlg, 
		IDC_TIMING_SYNC_REFRESH, 
		AutoFrameSkip ? BST_UNCHECKED : BST_CHECKED 
	);

	::CheckDlgButton
	( 
     	hDlg, 
		IDC_TIMING_AUTO_FRAME_SKIP, 
		AutoFrameSkip ? BST_CHECKED : BST_UNCHECKED 
	);

	::CheckDlgButton
	( 
     	hDlg, 
		IDC_TIMING_VSYNC, 
		UseVSync ? BST_CHECKED : BST_UNCHECKED 
	);

	::CheckDlgButton
	( 
     	hDlg, 
		IDC_TIMING_DEFAULT_FPS, 
		UseDefaultFps ? BST_CHECKED : BST_UNCHECKED 
	);

	::CheckDlgButton
	( 
		hDlg, 
		IDC_TIMING_PFC, 
		UsePFCounter ? BST_CHECKED : BST_UNCHECKED 
	);

	::EnableWindow( ::GetDlgItem( hDlg, IDC_TIMING_PFC              ), HasPFCounter   );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_TIMING_FRAME_SKIPS      ), AutoFrameSkip  );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_TIMING_FRAME_SKIPS_TEXT ), AutoFrameSkip  );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_TIMING_FRAME_SKIPS_NUM  ), AutoFrameSkip  );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_TIMING_FPS              ), !UseDefaultFps );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_TIMING_FPS_TEXT         ), !UseDefaultFps );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_TIMING_FPS_NUM          ), !UseDefaultFps );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID TIMERMANAGER::EnablePAL(const BOOL pal)
{
	IsPAL = pal;
	Update();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID TIMERMANAGER::Update()
{
	fps = (UseDefaultFps ? (IsPAL ? NES_FPS_PAL : NES_FPS_NTSC) : CustomFps);

	if (UsePFCounter) 
	{
		SwitchToPFCounter();
		dbScale = 1000.0 / DOUBLE(pfFrequency);
		dwLastFps = 0;
	}
	else
	{
		SwitchToMMTimer();
		dbScale = 1.0;
		pfLastFps = 0;
	}

	dbRefresh = (1000.0 / DOUBLE(fps));
	sleeper.Reset( UINT(floor(dbRefresh)) );

	dbTarget = 0;	
	dbFps = 0.0;
	FrameSkips = 0;
	SleepThreshold = 0;
	FrameCounter = 0;

	vSyncOnly = (UseVSync && fps == application.GetGraphicManager().GetRefreshRate());

	application.GetSoundManager().SetRefreshRate( IsPAL, fps );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

DOUBLE TIMERMANAGER::CurrentTime() const
{
	DOUBLE dbCurrent;

	if (UsePFCounter)
	{
		U64 pfCurrent;
		::QueryPerformanceCounter(PDX_CAST_PTR(LARGE_INTEGER,pfCurrent));
		dbCurrent = DOUBLE(pfCurrent - pfStart) * dbScale;
	}
	else
	{
		dbCurrent = (::timeGetTime() - dwStart);
	}

	return dbCurrent;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID TIMERMANAGER::SLEEPER::Reset(const UINT PurchasedCoffee)
{
	fired = FALSE;
	ZombieTime = ZOMBIE_TIME;
	Overflows = 0;
	AvailableCoffee = PurchasedCoffee;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID TIMERMANAGER::SLEEPER::GoToBed(TIMERMANAGER& timer,DOUBLE& BedTime)
{
	if (fired)
		return;

	const DOUBLE WorkTime = timer.dbTarget;
	const DOUBLE FreeTime = WorkTime - BedTime;

	if (FreeTime < MIN_SLEEP_TIME)
		return;

	const DWORD SleepTime = DWORD(FreeTime);

	if (SleepTime < ZombieTime)
		return;

	::Sleep( (SleepTime - ZombieTime) );
	BedTime = timer.CurrentTime();

	if (BedTime < WorkTime + OVERSLEPT)
	{
		if (ZombieTime > (ZOMBIE_TIME+2) && BedTime < (WorkTime - 4))
		{
			if (Overflows && !--Overflows)
				--ZombieTime;
		}
	}
	else
	{
		const UINT extra = (BedTime > WorkTime ? BedTime - WorkTime : 1);
		Overflows += PDX_MAX(1,extra);

		if (ZombieTime++ >= AvailableCoffee)
			fired = TRUE;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID TIMERMANAGER::Synchronize(const BOOL GfxOut,BOOL UseAutoFrameSkip)
{
	FrameSkips = 0;

	if (UseAutoFrameSkip)
		UseAutoFrameSkip = AutoFrameSkip;

	if (!UseAutoFrameSkip)
		SleepThreshold = 0;

	const BOOL vSynched = 
	(
       	GfxOut && 
		application.GetGraphicManager().UpdateRefresh( UseVSync, fps ) &&
		!UseAutoFrameSkip
	);

	if (vSynched)
		return;

	DOUBLE dbCurrent = CurrentTime();
	const DOUBLE dbNext = dbTarget + dbRefresh;

	if (dbCurrent > dbNext)
	{
		if (UseAutoFrameSkip)
		{
			ULONG count = ULONG(floor((dbCurrent - dbTarget) / dbRefresh));

			if (count > MaxFrameSkips)
				count = MaxFrameSkips;

			dbTarget += dbRefresh * (count + 1);

			if (dbTarget >= dbCurrent)
			{
				FrameSkips = count;

				if (SleepThreshold < 100)
					++SleepThreshold;

				return;
			}
		}

		dbTarget = dbCurrent - fmod( dbCurrent, dbRefresh );
	}
	else
	{
		if (UseAutoFrameSkip && SleepThreshold > 0)
			--SleepThreshold;

		dbTarget = dbNext;

		if (SleepThreshold == 0)
			sleeper.GoToBed( *this, dbCurrent );

		while (dbCurrent < dbTarget)
			dbCurrent = CurrentTime();

		if (dbTarget + dbRefresh < dbCurrent)
			dbTarget = dbCurrent - fmod( dbCurrent, dbRefresh );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL TIMERMANAGER::CalculateFPS(DOUBLE& value)
{
	++FrameCounter;

	if (UsePFCounter)
	{
		U64 pfCurrent;
		::QueryPerformanceCounter(PDX_CAST_PTR(LARGE_INTEGER,pfCurrent));		
		const U64 pfElapsed = pfCurrent - pfLastFps;

		if (pfElapsed >= (pfFrequency * 2))
		{
			dbFps = (DOUBLE(pfFrequency) / DOUBLE(pfElapsed)) * FrameCounter;
			pfLastFps = pfCurrent;
			FrameCounter = 0;
		}
	}
	else
	{
		const DWORD dwCurrent = ::timeGetTime();		
		DWORD dwElapsed = dwCurrent;

		if (dwElapsed < dwLastFps)
			dwElapsed += (0xFFFFFFFFUL - dwLastFps);
		else
			dwElapsed -= dwLastFps;

		if (dwElapsed >= 2000)
		{
			dbFps = (1000.0 / DOUBLE(dwElapsed)) * FrameCounter;
			dwLastFps = dwCurrent;
			FrameCounter = 0;
		}
	}

	value = dbFps;

	return FrameCounter == 0;
}
