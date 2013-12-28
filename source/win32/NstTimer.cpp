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

#include <Windows.h>
#include <MMSystem.h>
#include <CommCtrl.h>
#include <WindowsX.h>
#include "resource/resource.h"
#include "NstTimer.h"
#include "NstApplication.h"
#include "NstSoundManager.h"

#pragma comment(lib,"winmm")

////////////////////////////////////////////////////////////////////////////////////////
// constructor
////////////////////////////////////////////////////////////////////////////////////////

TIMERMANAGER::TIMERMANAGER(const INT a)
: 
MANAGER       (a),
CustomFps     (NES_FPS_NTSC),
fps           (NES_FPS_NTSC),
IsPAL         (FALSE),
MaxFrameSkips (DEFAULT_FRAME_SKIPS),
HasPFCounter  (QueryPerformanceFrequency(PDX_CAST(LARGE_INTEGER*,&pfRefreshFrequency)) && bool(pfRefreshFrequency))
{
	if (HasPFCounter)
	{
		pfTicksPerMilli = pfRefreshFrequency / 1000;
		pfRefreshClockPAL = pfRefreshFrequency / NES_FPS_PAL;
		pfRefreshClockNTSC = pfRefreshFrequency / NES_FPS_NTSC;
		pfRefreshClock = pfRefreshClockNTSC;
		pfLast = 0;		
	}
	else
	{
		dbRefreshClockPAL = 1000.0 / DOUBLE(NES_FPS_PAL);
		dbRefreshClockNTSC = 1000.0 / DOUBLE(NES_FPS_NTSC);
		dbRefreshClock = dbRefreshClockNTSC;
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

TIMERMANAGER::~TIMERMANAGER()
{
	if (!HasPFCounter && period)
		timeEndPeriod( resolution );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT TIMERMANAGER::Create(CONFIGFILE* const ConfigFile)
{
	if (ConfigFile)
	{
		CONFIGFILE& file = *ConfigFile;
		
		AutoFrameSkip = file[ "timer auto frame skip" ] == "yes" ? TRUE : FALSE;
		MaxFrameSkips = file[ "timer max frame skips" ].ToUlong();
		UseVSync      = file[ "timer vsync"           ] == "no"  ? FALSE : TRUE;
		UseDefaultFps = file[ "timer default fps"     ] == "no"  ? FALSE : TRUE;
		CustomFps     = file[ "timer custom fps"      ].ToUlong();

		if (MaxFrameSkips) MaxFrameSkips = PDX_CLAMP(MaxFrameSkips,MIN_FRAME_SKIPS,MAX_FRAME_SKIPS);
		else               MaxFrameSkips = DEFAULT_FRAME_SKIPS;

		if (CustomFps) CustomFps = PDX_CLAMP(CustomFps,MIN_FPS,MAX_FPS);
		else           CustomFps = DEFAULT_FPS;

		UpdateRefreshRate();
	}
	else
	{
		ResetDialog();
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT TIMERMANAGER::Destroy(CONFIGFILE* const ConfigFile)
{
	if (ConfigFile)
	{
		CONFIGFILE& file = *ConfigFile;

		file[ "timer auto frame skip" ] = (AutoFrameSkip ? "yes" : "no");
		file[ "timer max frame skips" ] = MaxFrameSkips;
		file[ "timer vsync"           ] = (UseVSync      ? "yes" : "no");
		file[ "timer default fps"     ] = (UseDefaultFps ? "yes" : "no");
		file[ "timer custom fps"      ] = CustomFps;
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID TIMERMANAGER::EnableCustomFPS(const BOOL state)
{
	if (bool(UseDefaultFps) == bool(state))
	{
		UseDefaultFps = !state;
		UpdateRefreshRate();
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
	
	UpdateRefreshRate();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL TIMERMANAGER::DialogProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg) 
	{
     	case WM_INITDIALOG:

			SendMessage
			( 
				GetDlgItem(hDlg,IDC_TIMING_FPS), 
				TBM_SETRANGE, 
				WPARAM(FALSE), 
				LPARAM(MAKELONG(MIN_FPS,MAX_FPS)) 
			);

			SendMessage
			( 
				GetDlgItem(hDlg,IDC_TIMING_FRAME_SKIPS), 
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

					AutoFrameSkip = IsDlgButtonChecked( hDlg, IDC_TIMING_AUTO_FRAME_SKIP );
					
					EnableWindow( GetDlgItem( hDlg, IDC_TIMING_FRAME_SKIPS      ), AutoFrameSkip );
					EnableWindow( GetDlgItem( hDlg, IDC_TIMING_FRAME_SKIPS_TEXT ), AutoFrameSkip );
					EnableWindow( GetDlgItem( hDlg, IDC_TIMING_FRAME_SKIPS_NUM  ), AutoFrameSkip );
					return TRUE;

				case IDC_TIMING_VSYNC:

					UseVSync = IsDlgButtonChecked( hDlg, IDC_TIMING_VSYNC );
					return TRUE;

				case IDC_TIMING_DEFAULT_FPS:
				
					UseDefaultFps = IsDlgButtonChecked( hDlg, IDC_TIMING_DEFAULT_FPS );

					EnableWindow( GetDlgItem( hDlg, IDC_TIMING_FPS      ), !UseDefaultFps );
					EnableWindow( GetDlgItem( hDlg, IDC_TIMING_FPS_TEXT ), !UseDefaultFps );
					EnableWindow( GetDlgItem( hDlg, IDC_TIMING_FPS_NUM  ), !UseDefaultFps );
					return TRUE;

				case IDC_TIMING_DEFAULT:

					ResetDialog();
					UpdateDialog( hDlg );
					return TRUE;

				case IDC_TIMING_OK:

					EndDialog( hDlg, 0 );
					return TRUE;
			}
			return FALSE;

		case WM_HSCROLL:
		
     		if (HWND(lParam) == GetDlgItem(hDlg,IDC_TIMING_FPS))
			{
				const UINT fps = (UINT) SendMessage
				(
					GetDlgItem(hDlg,IDC_TIMING_FPS),
					TBM_GETPOS,
					WPARAM(0),
					LPARAM(0)
				);

				if (CustomFps != fps)
				{
					CustomFps = fps;

					SetWindowText
					( 
						GetDlgItem(hDlg,IDC_TIMING_FPS_NUM), 
						PDXSTRING(fps).String() 
					);
				}
			}
			else if (HWND(lParam) == GetDlgItem(hDlg,IDC_TIMING_FRAME_SKIPS))
			{
				const UINT skips = (UINT) SendMessage
				(
					GetDlgItem(hDlg,IDC_TIMING_FRAME_SKIPS),
					TBM_GETPOS,
					WPARAM(0),
					LPARAM(0)
				);

				if (MaxFrameSkips != skips)
				{
					MaxFrameSkips = skips;

					SetWindowText
					( 
						GetDlgItem(hDlg,IDC_TIMING_FRAME_SKIPS_NUM), 
						PDXSTRING(skips).String() 
					);
				}
			}			
			return TRUE;

     	case WM_CLOSE:

     		EndDialog( hDlg, 0 );
     		return TRUE;

		case WM_DESTROY:

			UpdateRefreshRate();
			return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID TIMERMANAGER::UpdateDialog(HWND hDlg)
{
	SendMessage
	( 
     	GetDlgItem(hDlg,IDC_TIMING_FPS), 
		TBM_SETPOS, 
		WPARAM(TRUE), 
		LPARAM(CustomFps) 
	);
	
	SetWindowText
	( 
     	GetDlgItem(hDlg,IDC_TIMING_FPS_NUM), 
		PDXSTRING(CustomFps).String() 
	);

	SendMessage
	( 
		GetDlgItem(hDlg,IDC_TIMING_FRAME_SKIPS), 
		TBM_SETPOS, 
		WPARAM(TRUE), 
		LPARAM(MaxFrameSkips) 
	);

	SetWindowText
	( 
		GetDlgItem(hDlg,IDC_TIMING_FRAME_SKIPS_NUM), 
		PDXSTRING(MaxFrameSkips).String() 
	);

	CheckDlgButton
	( 
     	hDlg, 
		IDC_TIMING_SYNC_REFRESH, 
		AutoFrameSkip ? BST_UNCHECKED : BST_CHECKED 
	);

	CheckDlgButton
	( 
     	hDlg, 
		IDC_TIMING_AUTO_FRAME_SKIP, 
		AutoFrameSkip ? BST_CHECKED : BST_UNCHECKED 
	);

	CheckDlgButton
	( 
     	hDlg, 
		IDC_TIMING_VSYNC, 
		UseVSync ? BST_CHECKED : BST_UNCHECKED 
	);

	CheckDlgButton
	( 
     	hDlg, 
		IDC_TIMING_DEFAULT_FPS, 
		UseDefaultFps ? BST_CHECKED : BST_UNCHECKED 
	);

	EnableWindow( GetDlgItem( hDlg, IDC_TIMING_FRAME_SKIPS      ), AutoFrameSkip  );
	EnableWindow( GetDlgItem( hDlg, IDC_TIMING_FRAME_SKIPS_TEXT ), AutoFrameSkip  );
	EnableWindow( GetDlgItem( hDlg, IDC_TIMING_FRAME_SKIPS_NUM  ), AutoFrameSkip  );
	EnableWindow( GetDlgItem( hDlg, IDC_TIMING_FPS              ), !UseDefaultFps );
	EnableWindow( GetDlgItem( hDlg, IDC_TIMING_FPS_TEXT         ), !UseDefaultFps );
	EnableWindow( GetDlgItem( hDlg, IDC_TIMING_FPS_NUM          ), !UseDefaultFps );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID TIMERMANAGER::EnablePAL(const BOOL pal)
{
	IsPAL = pal;
	UpdateRefreshRate();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID TIMERMANAGER::UpdateRefreshRate()
{
	if (UseDefaultFps)
	{
		if (IsPAL)
		{
			fps = NES_FPS_PAL;

			if (HasPFCounter) pfRefreshClock = pfRefreshClockPAL;
			else              dbRefreshClock = dbRefreshClockPAL;
		}
		else
		{
			fps = NES_FPS_NTSC;

			if (HasPFCounter) pfRefreshClock = pfRefreshClockNTSC;
			else              dbRefreshClock = dbRefreshClockNTSC;
		}
	}
	else
	{
		fps = CustomFps;

		if (HasPFCounter) pfRefreshClock = pfRefreshFrequency / CustomFps;
		else              dbRefreshClock = dbRefreshFrequency / CustomFps;
	}

	application.GetSoundManager().SetRefreshRate( IsPAL, fps );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID TIMERMANAGER::Reset()
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

UINT TIMERMANAGER::SynchRefreshRate(const BOOL InBackground)
{
	if (application.GetGraphicManager().UpdateRefresh( UseVSync, fps ))
	{
		if (!AutoFrameSkip)
			return 0;
	}

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

			if (SkipFrames > MAX_FRAME_SKIPS)
			{
				pfLast = pfCurrent;
			}
			else if (SkipFrames > 1)
			{
				--SkipFrames;

				if (SkipFrames > MaxFrameSkips)
					SkipFrames = MaxFrameSkips;

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

			if (SkipFrames > MAX_FRAME_SKIPS)
			{
				dbLast = dbCurrent;
			}
			else if (SkipFrames > 1)
			{
				--SkipFrames;

				if (SkipFrames > MaxFrameSkips)
					SkipFrames = MaxFrameSkips;

				dbLast += dbRefreshClock * SkipFrames;
				return SkipFrames;
			}
		}
	}

	return 0;
}
