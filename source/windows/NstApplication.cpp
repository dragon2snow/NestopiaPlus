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

#include "NstApplication.h"
#include <new>
#include <WindowsX.h>

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDX_COMPILE_ASSERT
(
	( IDM_MACHINE_PORT1_PAD1		            - IDM_MACHINE_PORT1_UNCONNECTED ) == 1  &&  
	( IDM_MACHINE_PORT1_PAD2		            - IDM_MACHINE_PORT1_UNCONNECTED ) == 2  && 
	( IDM_MACHINE_PORT1_PAD3		            - IDM_MACHINE_PORT1_UNCONNECTED ) == 3  && 
	( IDM_MACHINE_PORT1_PAD4	                - IDM_MACHINE_PORT1_UNCONNECTED ) == 4  && 
	( IDM_MACHINE_PORT1_ZAPPER	                - IDM_MACHINE_PORT1_UNCONNECTED ) == 5  && 
	( IDM_MACHINE_PORT1_PADDLE	                - IDM_MACHINE_PORT1_UNCONNECTED ) == 6  && 
	( IDM_MACHINE_PORT1_POWERPAD                - IDM_MACHINE_PORT1_UNCONNECTED ) == 7  && 
	( IDM_MACHINE_PORT2_UNCONNECTED             - IDM_MACHINE_PORT1_UNCONNECTED ) == 8  && 
	( IDM_MACHINE_PORT2_PAD1		            - IDM_MACHINE_PORT1_UNCONNECTED ) == 9	&&
	( IDM_MACHINE_PORT2_PAD2		            - IDM_MACHINE_PORT1_UNCONNECTED ) == 10 && 
	( IDM_MACHINE_PORT2_PAD3	  	            - IDM_MACHINE_PORT1_UNCONNECTED ) == 11 &&  
	( IDM_MACHINE_PORT2_PAD4	                - IDM_MACHINE_PORT1_UNCONNECTED ) == 12 && 
	( IDM_MACHINE_PORT2_ZAPPER	                - IDM_MACHINE_PORT1_UNCONNECTED ) == 13 && 
	( IDM_MACHINE_PORT2_PADDLE	                - IDM_MACHINE_PORT1_UNCONNECTED ) == 14 && 
	( IDM_MACHINE_PORT2_POWERPAD                - IDM_MACHINE_PORT1_UNCONNECTED ) == 15 && 
	( IDM_MACHINE_PORT3_UNCONNECTED             - IDM_MACHINE_PORT1_UNCONNECTED ) == 16 && 
	( IDM_MACHINE_PORT3_PAD1		            - IDM_MACHINE_PORT1_UNCONNECTED ) == 17 && 
	( IDM_MACHINE_PORT3_PAD2		            - IDM_MACHINE_PORT1_UNCONNECTED ) == 18 && 
	( IDM_MACHINE_PORT3_PAD3		            - IDM_MACHINE_PORT1_UNCONNECTED ) == 19 &&
	( IDM_MACHINE_PORT3_PAD4	                - IDM_MACHINE_PORT1_UNCONNECTED ) == 20 && 
	( IDM_MACHINE_PORT4_UNCONNECTED             - IDM_MACHINE_PORT1_UNCONNECTED ) == 21 && 
	( IDM_MACHINE_PORT4_PAD1		            - IDM_MACHINE_PORT1_UNCONNECTED ) == 22 && 
	( IDM_MACHINE_PORT4_PAD2		            - IDM_MACHINE_PORT1_UNCONNECTED ) == 23 && 
	( IDM_MACHINE_PORT4_PAD3		            - IDM_MACHINE_PORT1_UNCONNECTED ) == 24 && 
	( IDM_MACHINE_PORT4_PAD4	                - IDM_MACHINE_PORT1_UNCONNECTED ) == 25 && 
	( IDM_MACHINE_EXPANSION_UNCONNECTED         - IDM_MACHINE_PORT1_UNCONNECTED ) == 26 &&
	( IDM_MACHINE_EXPANSION_FAMILYBASICKEYBOARD - IDM_MACHINE_PORT1_UNCONNECTED ) == 27
);

PDX_COMPILE_ASSERT
(
	( IDM_FDS_INSERT_DISK_2  - IDM_FDS_INSERT_DISK_1 ) == 1  && 
	( IDM_FDS_INSERT_DISK_3  - IDM_FDS_INSERT_DISK_1 ) == 2  &&  
	( IDM_FDS_INSERT_DISK_4  - IDM_FDS_INSERT_DISK_1 ) == 3  && 
	( IDM_FDS_INSERT_DISK_5  - IDM_FDS_INSERT_DISK_1 ) == 4  && 
	( IDM_FDS_INSERT_DISK_6  - IDM_FDS_INSERT_DISK_1 ) == 5  && 
	( IDM_FDS_INSERT_DISK_7  - IDM_FDS_INSERT_DISK_1 ) == 6  && 
	( IDM_FDS_INSERT_DISK_8  - IDM_FDS_INSERT_DISK_1 ) == 7  && 
	( IDM_FDS_INSERT_DISK_9  - IDM_FDS_INSERT_DISK_1 ) == 8  && 
	( IDM_FDS_INSERT_DISK_10 - IDM_FDS_INSERT_DISK_1 ) == 9	 &&
	( IDM_FDS_INSERT_DISK_11 - IDM_FDS_INSERT_DISK_1 ) == 10 &&
	( IDM_FDS_INSERT_DISK_12 - IDM_FDS_INSERT_DISK_1 ) == 11 &&
	( IDM_FDS_INSERT_DISK_13 - IDM_FDS_INSERT_DISK_1 ) == 12 &&
	( IDM_FDS_INSERT_DISK_14 - IDM_FDS_INSERT_DISK_1 ) == 13 &&
	( IDM_FDS_INSERT_DISK_15 - IDM_FDS_INSERT_DISK_1 ) == 14 &&
	( IDM_FDS_INSERT_DISK_16 - IDM_FDS_INSERT_DISK_1 ) == 15
);

PDX_COMPILE_ASSERT
(
	( IDM_FILE_QUICK_LOAD_STATE_SLOT_1 - IDM_FILE_QUICK_LOAD_STATE_SLOT_LAST ) == 1 && 
	( IDM_FILE_QUICK_LOAD_STATE_SLOT_2 - IDM_FILE_QUICK_LOAD_STATE_SLOT_LAST ) == 2 &&  
	( IDM_FILE_QUICK_LOAD_STATE_SLOT_3 - IDM_FILE_QUICK_LOAD_STATE_SLOT_LAST ) == 3 && 
	( IDM_FILE_QUICK_LOAD_STATE_SLOT_4 - IDM_FILE_QUICK_LOAD_STATE_SLOT_LAST ) == 4 && 
	( IDM_FILE_QUICK_LOAD_STATE_SLOT_5 - IDM_FILE_QUICK_LOAD_STATE_SLOT_LAST ) == 5 && 
	( IDM_FILE_QUICK_LOAD_STATE_SLOT_6 - IDM_FILE_QUICK_LOAD_STATE_SLOT_LAST ) == 6 && 
	( IDM_FILE_QUICK_LOAD_STATE_SLOT_7 - IDM_FILE_QUICK_LOAD_STATE_SLOT_LAST ) == 7 && 
	( IDM_FILE_QUICK_LOAD_STATE_SLOT_8 - IDM_FILE_QUICK_LOAD_STATE_SLOT_LAST ) == 8 && 
	( IDM_FILE_QUICK_LOAD_STATE_SLOT_9 - IDM_FILE_QUICK_LOAD_STATE_SLOT_LAST ) == 9
);

PDX_COMPILE_ASSERT
(
	( IDM_FILE_QUICK_SAVE_STATE_SLOT_1 - IDM_FILE_QUICK_SAVE_STATE_SLOT_NEXT ) == 1 && 
	( IDM_FILE_QUICK_SAVE_STATE_SLOT_2 - IDM_FILE_QUICK_SAVE_STATE_SLOT_NEXT ) == 2 &&  
	( IDM_FILE_QUICK_SAVE_STATE_SLOT_3 - IDM_FILE_QUICK_SAVE_STATE_SLOT_NEXT ) == 3 && 
	( IDM_FILE_QUICK_SAVE_STATE_SLOT_4 - IDM_FILE_QUICK_SAVE_STATE_SLOT_NEXT ) == 4 && 
	( IDM_FILE_QUICK_SAVE_STATE_SLOT_5 - IDM_FILE_QUICK_SAVE_STATE_SLOT_NEXT ) == 5 && 
	( IDM_FILE_QUICK_SAVE_STATE_SLOT_6 - IDM_FILE_QUICK_SAVE_STATE_SLOT_NEXT ) == 6 && 
	( IDM_FILE_QUICK_SAVE_STATE_SLOT_7 - IDM_FILE_QUICK_SAVE_STATE_SLOT_NEXT ) == 7 && 
	( IDM_FILE_QUICK_SAVE_STATE_SLOT_8 - IDM_FILE_QUICK_SAVE_STATE_SLOT_NEXT ) == 8 && 
	( IDM_FILE_QUICK_SAVE_STATE_SLOT_9 - IDM_FILE_QUICK_SAVE_STATE_SLOT_NEXT ) == 9
);

PDX_COMPILE_ASSERT
(
	( IDM_FILE_RECENT_1 - IDM_FILE_RECENT_0 ) == 1 && 
	( IDM_FILE_RECENT_2 - IDM_FILE_RECENT_0 ) == 2 &&  
	( IDM_FILE_RECENT_3 - IDM_FILE_RECENT_0 ) == 3 && 
	( IDM_FILE_RECENT_4 - IDM_FILE_RECENT_0 ) == 4 && 
	( IDM_FILE_RECENT_5 - IDM_FILE_RECENT_0 ) == 5 && 
	( IDM_FILE_RECENT_6 - IDM_FILE_RECENT_0 ) == 6 && 
	( IDM_FILE_RECENT_7 - IDM_FILE_RECENT_0 ) == 7 && 
	( IDM_FILE_RECENT_8 - IDM_FILE_RECENT_0 ) == 8 && 
	( IDM_FILE_RECENT_9 - IDM_FILE_RECENT_0 ) == 9
);

PDX_COMPILE_ASSERT
(
    ( IDM_VIEW_WINDOWSIZE_1X - IDM_VIEW_WINDOWSIZE_MAX ) == 1 &&
	( IDM_VIEW_WINDOWSIZE_2X - IDM_VIEW_WINDOWSIZE_MAX ) == 2 && 
	( IDM_VIEW_WINDOWSIZE_4X - IDM_VIEW_WINDOWSIZE_MAX ) == 3 &&
	( IDM_VIEW_WINDOWSIZE_8X - IDM_VIEW_WINDOWSIZE_MAX ) == 4
);

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#define NST_MENUSTATE(x) ((x) ? MF_ENABLED : MF_GRAYED)
#define NST_WINDOWSTYLE	 (WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_THICKFRAME)

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXSTRING APPLICATION::ScreenMsg;

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_NAMESPACE_BEGIN

	PDXRESULT MsgWarning(const CHAR* const text) 
	{ return application.OnWarning( text ); }
	
	BOOL MsgQuestion(const CHAR* const text,const CHAR* const head) 
	{ return application.OnQuestion( text, head ); }

	VOID MsgOutput(const CHAR* const text)
	{ application.StartScreenMsg( 1500, text ); }
			 
	VOID LogOutput(const CHAR* const text)
	{ LOGFILEMANAGER::Output( text ); }

	BOOL MsgInput(const CHAR* const title,const CHAR* const msg,PDXSTRING& input)
	{ return application.OnUserInput( title, msg, input ); }

NES_NAMESPACE_END

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

HWND APPLICATION::Init(HINSTANCE hInstance)
{
	{
		WNDCLASSEX wndcls;
		PDXMemZero( wndcls );

		wndcls.cbSize        = sizeof(wndcls);
		wndcls.style         = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
		wndcls.lpfnWndProc	 = WndProc;
		wndcls.hInstance     = hInstance;
		wndcls.hIcon         = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_GEEK_1));
		wndcls.hCursor       = LoadCursor(NULL,IDC_ARROW);
		wndcls.hbrBackground = HBRUSH(GetStockObject(NULL_BRUSH));
		wndcls.lpszClassName = TEXT("Nestopia Window");
		wndcls.hIconSm       = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_GEEK_2));

		if (!RegisterClassEx(&wndcls))
			return NULL;
	}

	HWND const hWnd = CreateWindowEx
	(
		0,
		TEXT("Nestopia Window"),
		TEXT("Nestopia"),
		NST_WINDOWSTYLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		256,
		224,
		0,
		LoadMenu(hInstance,MAKEINTRESOURCE(IDR_MENU)),
		hInstance,
		NULL
	);

	return hWnd;
}

////////////////////////////////////////////////////////////////////////////////////////
// constructor
////////////////////////////////////////////////////////////////////////////////////////

APPLICATION::APPLICATION(HINSTANCE hInstance,const CHAR* const RomImage,const INT iCmdShow)
: 
hWnd                 (Init(hInstance)),
hMenu                (NULL),
hCursor              (LoadCursor(NULL,IDC_ARROW)),
hAccel               (LoadAccelerators(hInstance,MAKEINTRESOURCE(IDR_ACCELERATOR))),
active               (FALSE),
ready                (FALSE),
UseZapper            (FALSE),
windowed             (TRUE),
InBackground         (FALSE),
ExitSuccess          (FALSE),
AutoSelectController (FALSE),
AcceleratorEnabled   (bool(hAccel)),
FrameSkips           (0),
NesMode              (NES::MODE_AUTO)
{
	if (!hWnd)
		throw ("Failed to create window!");

	SelectPort[0] = IDM_MACHINE_PORT1_PAD1;
	SelectPort[1] = IDM_MACHINE_PORT2_PAD2;
	SelectPort[2] = IDM_MACHINE_PORT3_UNCONNECTED;
	SelectPort[3] = IDM_MACHINE_PORT4_UNCONNECTED;
	SelectPort[4] = IDM_MACHINE_EXPANSION_UNCONNECTED;
	
	SetRect( &rcScreen, 0, 0, 256, 224 );	
	rcWindow = rcClient = rcScreen = rcDefWindow = rcDefClient = rcRestoreWindow = rcScreen;


	{
		CONFIGFILE file;
		CONFIGFILE* ConfigFile;
	
		if (InitConfigFile( file, CFG_LOAD ))
		{
			log.Output("APPLICATION: loading settings from: ",file.FileName());
			ConfigFile = &file;
		}
		else
		{
			log.Output("APPLICATION: configuration file not present, default settings will be used.");
			ConfigFile = NULL;
		}

		{
			UINT factor = 1;
	
			if (ConfigFile)
			{
				const PDXSTRING& string = file["window size"];
	
     				 if (string == "1x") factor = 0;
				else if (string == "3x") factor = 2;
				else if (string == "4x") factor = 3;
			}
	
			RECT rect;
			SetRect( &rect, 0, 0, 0, 0 );
	
			GetWindowRect( GetDesktopWindow(), &rect );
			GetWindowRect( hWnd, &rcWindow );
	
			OnWindowSize
			( 
				factor, 
				PDX_MAX(rect.right - rect.left,256), 
				PDX_MAX(rect.bottom - rect.top,224) 
			);
		}
	
		TimerManager.Create       ( ConfigFile );
		FileManager.Create        ( ConfigFile );
		GraphicManager.Create     ( ConfigFile );
		SoundManager.Create       ( ConfigFile );
		InputManager.Create       ( ConfigFile );
		FdsManager.Create         ( ConfigFile );
		preferences.Create        ( ConfigFile );
		GameGenieManager.Create   ( ConfigFile );
		SaveStateManager.Create   ( ConfigFile );
		MovieManager.Create       ( ConfigFile );
		VsDipSwitchManager.Create ( ConfigFile );
		log.Create                ( ConfigFile );
		RomInfo.Create            ( ConfigFile );
		UserInputManager.Create   ( ConfigFile );
	}
	
	UpdateRecentFiles();
	
	if (RomImage && strlen( RomImage ))
	{
		FileManager.AddRecentFile( RomImage );
		FileManager.Load( 0, preferences.EmulateImmediately() );
	}
	
	OnAutoSelectController();
	UpdateWindowItems();
	GraphicManager.SwitchToWindowed( rcScreen );
	
	if (preferences.StartUpFullScreen())
	{
		SwitchScreen();
	}
	else
	{
		ShowWindow( hWnd, iCmdShow );
	}
	
	active = ready = TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
// destructor
////////////////////////////////////////////////////////////////////////////////////////

APPLICATION::~APPLICATION()
{
	active = ready = FALSE;

	SaveStateManager.Flush();
	nes.Power( FALSE );
	
	{
		CONFIGFILE file;
		CONFIGFILE* ConfigFile = NULL;

		if (ExitSuccess)
		{
			ConfigFile = &file;

			const UINT factor = GetAspectRatio();

			if (factor != UINT_MAX)
			{
				switch (factor)
				{				
	     			case 0: file["window size"] = "1x"; break;
	     			case 1: file["window size"] = "2x"; break;
	     			case 2: file["window size"] = "3x"; break;
	       			case 3: file["window size"] = "4x"; break;
				}
			}
		}

		TimerManager.Destroy     ( ConfigFile );
		FileManager.Destroy      ( ConfigFile );
		GraphicManager.Destroy   ( ConfigFile );
		SoundManager.Destroy     ( ConfigFile );
		InputManager.Destroy     ( ConfigFile );
		FdsManager.Destroy       ( ConfigFile );
		GameGenieManager.Destroy ( ConfigFile );
		preferences.Destroy      ( ConfigFile );

		if (ConfigFile)
			InitConfigFile( file, CFG_SAVE );
	}

	if (hAccel)
		DestroyAcceleratorTable( hAccel );

	if (hMenu)
	{
		DestroyMenu( hMenu );
		hMenu = NULL;
	}

	if (hWnd)
		DestroyWindow( hWnd );

	log.Close( !ExitSuccess || preferences.LogFileEnabled() );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL APPLICATION::InitConfigFile(CONFIGFILE& file,const CFG_OP op)
{
	PDXSTRING filename;

	if (PDX_SUCCEEDED(FILEMANAGER::GetExeFileName(filename)))
	{
		filename.ReplaceFileExtension( "cfg" );

		switch (op)
		{
     		case CFG_LOAD: if (PDX_SUCCEEDED(file.Load(filename))) return TRUE; break;
			case CFG_SAVE: if (PDX_SUCCEEDED(file.Save(filename))) return TRUE; break;
		}
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
// main loop
////////////////////////////////////////////////////////////////////////////////////////

INT APPLICATION::Run()
{
	PDX_ASSERT(hWnd);

	MSG msg;	
	msg.message = WM_NULL;
	PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE );

	while (msg.message != WM_QUIT)
	{
		if ((active ? PeekMessage(&msg,NULL,0,0,PM_REMOVE) : GetMessage(&msg,NULL,0,0)))
		{
			if (!AcceleratorEnabled || !TranslateAccelerator( hWnd, hAccel, &msg ))
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		}
		else if (active && ready)
		{
			ExecuteFrame();
		}
	}

	ExitSuccess = TRUE;

	return INT(msg.wParam);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::ExecuteFrame()
{
	const BOOL WindowVisible = !IsIconic( hWnd );
	const BOOL IsRunning = nes.IsOn() && !nes.IsPaused();

	if (IsRunning && nes.IsImage())
	{
		if (!FrameSkips)
		{
			const BOOL ScreenNotCleared = WindowVisible && !GraphicManager.TryClearScreen();

			InputManager.Poll();

			if (ScreenNotCleared)
				GraphicManager.ClearScreen();

			if (WindowVisible)
			{
				nes.Execute
				(
					GraphicManager.GetFormat(),
					SoundManager.GetFormat(),
					InputManager.GetFormat()
				);

				if (ScreenMsg.Length())
					OutputScreenMsg();

				FrameSkips = TimerManager.SynchRefreshRate( TRUE, InBackground, TRUE );
				GraphicManager.Present();
			}
			else
			{
				nes.Execute
				(
					NULL,
					SoundManager.GetFormat(),
					InputManager.GetFormat()
				);

				FrameSkips = TimerManager.SynchRefreshRate( FALSE, TRUE, TRUE );
			}
		}
		else
		{
			--FrameSkips;

			InputManager.Poll();

			nes.Execute
			(
				NULL,
				SoundManager.GetFormat(),
				InputManager.GetFormat()
			);
		}
	}
	else if (IsRunning && nes.IsNsf())
	{
		if (WindowVisible)
			GraphicManager.ClearScreen();

		nes.Execute
		(
			NULL,
			SoundManager.GetFormat(),
			NULL
		);

		if (WindowVisible)
		{
			if (nes.IsNsf())
				OutputNsfInfo();

			if (ScreenMsg.Length())
				OutputScreenMsg();

			TimerManager.SynchRefreshRate( TRUE, InBackground, FALSE );
			GraphicManager.Present();
			GraphicManager.Wait();
		}
		else
		{
			TimerManager.SynchRefreshRate( FALSE, TRUE, FALSE );
		}
	}
	else
	{
		if (WindowVisible)
		{
			GraphicManager.ClearScreen();

			if (ScreenMsg.Length())
				OutputScreenMsg();

			GraphicManager.Repaint();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL APPLICATION::OnUserInput(const CHAR* const title,const CHAR* const text,PDXSTRING& input)
{
	return UserInputManager.Start( title, text, input );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT APPLICATION::OnWarning(const CHAR* const text)
{
	SoundManager.Clear();
	GraphicManager.EnableGDI( TRUE );
	MessageBox( hWnd, text, "Nestopia Warning!", MB_OK|MB_ICONWARNING );
	GraphicManager.EnableGDI( FALSE );

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL APPLICATION::OnQuestion(const CHAR* const head,const CHAR* const text)
{
	SoundManager.Clear();
	GraphicManager.EnableGDI( TRUE );
	const BOOL yep = (MessageBox( hWnd, text, head, MB_YESNO|MB_ICONQUESTION ) == IDYES);		
	GraphicManager.EnableGDI( FALSE );

	return yep;
}

////////////////////////////////////////////////////////////////////////////////////////
// static window callback procedure
////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK APPLICATION::WndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return application.MsgProc( hWnd, uMsg, wParam, lParam );
}

////////////////////////////////////////////////////////////////////////////////////////
// window callback procedure
////////////////////////////////////////////////////////////////////////////////////////

LRESULT APPLICATION::MsgProc(const HWND hWnd,const UINT uMsg,const WPARAM wParam,const LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_SETCURSOR:

			if (LOWORD(lParam) == HTCLIENT)
			{
				SetCursor( hCursor );
				return TRUE;
			}
			break;

		case WM_NCHITTEST:
			break;

		case WM_MOUSEMOVE:

			OnMouseMove( lParam );
			return 0;

		case WM_PAINT:

			OnPaint();
			return 0;

		case WM_ERASEBKGND:
			return TRUE;

		case WM_LBUTTONDOWN:

			OnLeftMouseButtonDown( lParam );
			return 0;

		case WM_LBUTTONUP:

			OnLeftMouseButtonUp();
			return 0;

		case WM_RBUTTONDOWN:

			OnRightMouseButtonDown();
			return 0;

		case WM_COMMAND:

			if (OnCommand( wParam ))
				return 0;

			break;

		case WM_MOVE:

			OnMove( lParam );
			return 0;

		case WM_SIZE:

			OnSize( lParam );
			return 0;

		case WM_SYSCOMMAND:

			if (OnSysCommand(wParam))
				return 0;

			break;

		case WM_EXITSIZEMOVE:

			OnExitSizeMove();

		case WM_EXITMENULOOP:

			OnActive();
			return 0;

		case WM_ENTERSIZEMOVE:

			OnInactive(TRUE);
			return 0;

		case WM_ENTERMENULOOP:

			OnInactive(TRUE);
			UpdateDynamicMenuItems();
			return 0;

		case WM_NCLBUTTONDOWN:
		case WM_NCRBUTTONDOWN:
			
			SoundManager.Clear();
			break;
  
		case WM_ACTIVATEAPP:

			if (wParam) OnActive();
			else        OnInactive();			
			return 0;

		case WM_ACTIVATE:

			OnActivate( wParam );
			return 0;

		case WM_POWERBROADCAST:

			switch (wParam)
			{
     			case PBT_APMQUERYSUSPEND:  OnInactive(TRUE); return 0;
				case PBT_APMRESUMESUSPEND: OnActive(); return 0;
			}
			break;
		
		case WM_CLOSE:

			OnCloseWindow();
			return 0;
	}

	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL APPLICATION::OnCommand(const WPARAM wParam)
{
	const UINT idm = LOWORD(wParam);

	switch (idm)
	{
		case IDM_FILE_OPEN:                       OnOpen( FILE_ALL );                 return TRUE;
		case IDM_FILE_CLOSE:                      OnClose();                          return TRUE;
		case IDM_FILE_LOAD_NSP:                   OnLoadNsp();                        return TRUE;
		case IDM_FILE_LOAD_NST:                   OnLoadState();                      return TRUE;
		case IDM_FILE_SAVE_NSP:                   OnSaveNsp();                        return TRUE;
		case IDM_FILE_SAVE_NST:                   OnSaveState();                      return TRUE;
		case IDM_FILE_QUIT:			              OnCloseWindow();                    return TRUE;
		case IDM_FILE_SAVE_SCREENSHOT:            OnSaveScreenShot();                 return TRUE;
		case IDM_FILE_SOUND_CAPTURE_FILE:		  
		case IDM_FILE_SOUND_CAPTURE_RECORD:		  
		case IDM_FILE_SOUND_CAPTURE_STOP:		  
		case IDM_FILE_SOUND_CAPTURE_RESET:        OnSoundRecorder( idm );             return TRUE;
		case IDM_FILE_MOVIE_FILE:                 MovieManager.StartDialog();         return TRUE;
		case IDM_FILE_MOVIE_PLAY:                 MovieManager.Play();                return TRUE;
		case IDM_FILE_MOVIE_RECORD:               MovieManager.Record();              return TRUE;
		case IDM_FILE_MOVIE_STOP:                 MovieManager.Stop();                return TRUE;
		case IDM_FILE_MOVIE_REWIND:               MovieManager.Rewind();              return TRUE;
		case IDM_FILE_MOVIE_FORWARD:              MovieManager.Forward();             return TRUE;
		case IDM_MACHINE_POWER_ON:		          OnPower( TRUE  );                   return TRUE;
		case IDM_MACHINE_POWER_OFF:               OnPower( FALSE );                   return TRUE;
		case IDM_MACHINE_RESET_SOFT:              OnReset( FALSE );                   return TRUE;
		case IDM_MACHINE_RESET_HARD:              OnReset( TRUE  );                   return TRUE;
		case IDM_MACHINE_AUTOSELECTCONTROLLER:    OnAutoSelectController();           return TRUE;                                
		case IDM_MACHINE_MODE_AUTO:                                    
		case IDM_MACHINE_MODE_NTSC:		  
		case IDM_MACHINE_MODE_PAL:	              OnMode( idm );                      return TRUE;
		case IDM_MACHINE_PAUSE:	                  OnPause();                          return TRUE;
		case IDM_FDS_EJECT_DISK:                  OnFdsEjectDisk();                   return TRUE;
		case IDM_FDS_SIDE_A:                      
		case IDM_FDS_SIDE_B:                      OnFdsSide( idm );                   return TRUE;
		case IDM_FDS_OPTIONS:                     FdsManager.StartDialog();           return TRUE;
		case IDM_NSF_PLAY:                        OnNsfCommand( NES::IO::NSF::PLAY ); return TRUE;
		case IDM_NSF_STOP:                        OnNsfCommand( NES::IO::NSF::STOP ); return TRUE;
		case IDM_NSF_NEXT:                        OnNsfCommand( NES::IO::NSF::NEXT ); return TRUE;
		case IDM_NSF_PREV:                        OnNsfCommand( NES::IO::NSF::PREV ); return TRUE;
		case IDM_VIEW_ROM_INFO:                   RomInfo.StartDialog();              return TRUE;
		case IDM_VIEW_LOGFILE:                    log.StartDialog();                  return TRUE;
		case IDM_VIEW_SWITCH_SCREEN:              SwitchScreen();                     return TRUE;
		case IDM_VIEW_WINDOWSIZE_MAX:             OnWindowSize( UINT_MAX );           return TRUE;
		case IDM_VIEW_WINDOWSIZE_1X:              OnWindowSize( 0 );                  return TRUE;
		case IDM_VIEW_WINDOWSIZE_2X:              OnWindowSize( 1 );                  return TRUE;
		case IDM_VIEW_WINDOWSIZE_4X:              OnWindowSize( 2 );                  return TRUE;
		case IDM_VIEW_WINDOWSIZE_8X:              OnWindowSize( 3 );                  return TRUE;
		case IDM_VIEW_HIDE_MENU:                  OnHideMenu();                       return TRUE;
		case IDM_OPTIONS_PREFERENCES:             preferences.StartDialog();          return TRUE;
		case IDM_OPTIONS_VIDEO:                   GraphicManager.StartDialog();       return TRUE;
		case IDM_OPTIONS_SOUND:                   SoundManager.StartDialog();         return TRUE;
		case IDM_OPTIONS_INPUT:                   InputManager.StartDialog();         return TRUE;
		case IDM_OPTIONS_TIMING:                  TimerManager.StartDialog();         return TRUE;
		case IDM_OPTIONS_PATHS:                   FileManager.StartDialog();          return TRUE;
		case IDM_OPTIONS_GAME_GENIE:              GameGenieManager.StartDialog();     return TRUE;
		case IDM_OPTIONS_AUTO_SAVE:               SaveStateManager.StartDialog();     return TRUE;
		case IDM_OPTIONS_DIP_SWITCHES:            VsDipSwitchManager.StartDialog();   return TRUE;
		case IDM_HELP_ABOUT:                      
		case IDM_HELP_LICENCE:                    OnHelp( idm );                      return TRUE;
		case IDM_TOGGLE_MENU:              		  OnRightMouseButtonDown();           return TRUE;
	}

	if (idm >= IDM_MACHINE_PORT1_UNCONNECTED && idm <= IDM_MACHINE_EXPANSION_FAMILYBASICKEYBOARD)
	{
		OnPort( idm );
		return TRUE;
	}

	if (idm >= IDM_FILE_QUICK_LOAD_STATE_SLOT_LAST && idm <= IDM_FILE_QUICK_LOAD_STATE_SLOT_9)
	{
		OnLoadStateSlot( idm );
		return TRUE;
	}

	if (idm >= IDM_FILE_QUICK_SAVE_STATE_SLOT_NEXT && idm <= IDM_FILE_QUICK_SAVE_STATE_SLOT_9)
	{
		OnSaveStateSlot( idm );
		return TRUE;
	}

	if (idm >= IDM_FILE_RECENT_0 && idm <= IDM_FILE_RECENT_9)
	{
		OnRecent( idm );
		return TRUE;
	}

	if (idm >= IDM_FDS_INSERT_DISK_1 && idm <= IDM_FDS_INSERT_DISK_16)
	{
		OnFdsInsertDisk( idm );
		return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#define NST_CASE_(i,j,k) case i: port = SelectPort + j; nes.ConnectController( j, k ); break

VOID APPLICATION::OnPort(const UINT wParam)
{
	PDX_ASSERT( wParam >= IDM_MACHINE_PORT1_UNCONNECTED && wParam <= IDM_MACHINE_EXPANSION_FAMILYBASICKEYBOARD );

	UINT* port = NULL;

	switch (wParam)
	{
	    NST_CASE_( IDM_MACHINE_PORT1_UNCONNECTED,             0, NES::CONTROLLER_UNCONNECTED );
       	NST_CASE_( IDM_MACHINE_PORT1_PAD1,		              0, NES::CONTROLLER_PAD1        );
     	NST_CASE_( IDM_MACHINE_PORT1_PAD2,		              0, NES::CONTROLLER_PAD2        );
     	NST_CASE_( IDM_MACHINE_PORT1_PAD3,		              0, NES::CONTROLLER_PAD3        );
       	NST_CASE_( IDM_MACHINE_PORT1_PAD4,	                  0, NES::CONTROLLER_PAD4        );
       	NST_CASE_( IDM_MACHINE_PORT1_ZAPPER,	              0, NES::CONTROLLER_ZAPPER      );
    	NST_CASE_( IDM_MACHINE_PORT1_PADDLE,	              0, NES::CONTROLLER_PADDLE      );
		NST_CASE_( IDM_MACHINE_PORT1_POWERPAD,	              0, NES::CONTROLLER_POWERPAD    );
       	NST_CASE_( IDM_MACHINE_PORT2_UNCONNECTED,             1, NES::CONTROLLER_UNCONNECTED );
     	NST_CASE_( IDM_MACHINE_PORT2_PAD1,		              1, NES::CONTROLLER_PAD1        );
     	NST_CASE_( IDM_MACHINE_PORT2_PAD2,		              1, NES::CONTROLLER_PAD2        );
     	NST_CASE_( IDM_MACHINE_PORT2_PAD3,	  	              1, NES::CONTROLLER_PAD3        );
     	NST_CASE_( IDM_MACHINE_PORT2_PAD4,	                  1, NES::CONTROLLER_PAD4        );
     	NST_CASE_( IDM_MACHINE_PORT2_ZAPPER,	              1, NES::CONTROLLER_ZAPPER      );
     	NST_CASE_( IDM_MACHINE_PORT2_PADDLE,	              1, NES::CONTROLLER_PADDLE      );
		NST_CASE_( IDM_MACHINE_PORT2_POWERPAD,	              1, NES::CONTROLLER_POWERPAD    );
     	NST_CASE_( IDM_MACHINE_PORT3_UNCONNECTED,             2, NES::CONTROLLER_UNCONNECTED );
     	NST_CASE_( IDM_MACHINE_PORT3_PAD1,		              2, NES::CONTROLLER_PAD1        );
     	NST_CASE_( IDM_MACHINE_PORT3_PAD2,		              2, NES::CONTROLLER_PAD2        );
     	NST_CASE_( IDM_MACHINE_PORT3_PAD3,		              2, NES::CONTROLLER_PAD3        );
    	NST_CASE_( IDM_MACHINE_PORT3_PAD4,	                  2, NES::CONTROLLER_PAD4        );
     	NST_CASE_( IDM_MACHINE_PORT4_UNCONNECTED,             3, NES::CONTROLLER_UNCONNECTED );
     	NST_CASE_( IDM_MACHINE_PORT4_PAD1,		              3, NES::CONTROLLER_PAD1        );
     	NST_CASE_( IDM_MACHINE_PORT4_PAD2,		              3, NES::CONTROLLER_PAD2        );
     	NST_CASE_( IDM_MACHINE_PORT4_PAD3,		              3, NES::CONTROLLER_PAD3        );
    	NST_CASE_( IDM_MACHINE_PORT4_PAD4,	                  3, NES::CONTROLLER_PAD4        );
		NST_CASE_( IDM_MACHINE_EXPANSION_UNCONNECTED,	      4, NES::CONTROLLER_UNCONNECTED );
     	NST_CASE_( IDM_MACHINE_EXPANSION_FAMILYBASICKEYBOARD, 4, NES::CONTROLLER_KEYBOARD    );
	}

	if (port)
	{
		CheckMenuItem( GetMenu(), *port, MF_UNCHECKED );
		*port = wParam;
		CheckMenuItem( GetMenu(), *port, MF_CHECKED );

		if (nes.IsAnyControllerConnected(NES::CONTROLLER_KEYBOARD))
		{
			AcceleratorEnabled = FALSE;
			UseZapper = FALSE;
		}
		else
		{
			AcceleratorEnabled = bool(hAccel);
			UseZapper = nes.IsAnyControllerConnected(NES::CONTROLLER_ZAPPER);
		}

		RefreshCursor();
	}
}														 

#undef NST_CASE_

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnAutoSelectController()
{
	if (AutoSelectController)
	{
		AutoSelectController = FALSE;
		CheckMenuItem( GetMenu(), IDM_MACHINE_AUTOSELECTCONTROLLER, MF_UNCHECKED );
	}
	else
	{
		AutoSelectController = TRUE;
		CheckMenuItem( GetMenu(), IDM_MACHINE_AUTOSELECTCONTROLLER, MF_CHECKED );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID CALLBACK APPLICATION::OnScreenMsgEnd(HWND hWnd,UINT,UINT_PTR,DWORD)
{
	KillTimer( hWnd, TIMER_ID_SCREEN_MSG );
	ScreenMsg.Clear();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL APPLICATION::OnSysCommand(const WPARAM wParam)
{
	SoundManager.Clear();

	if (!windowed)
	{
		switch (wParam)
		{
     		case SC_MOVE:
       		case SC_SIZE:
			case SC_MONITORPOWER:
				return TRUE;
		}
	}
  
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnPause()
{
	if (nes.IsPaused())
	{
		nes.Pause( FALSE );
		CheckMenuItem( GetMenu(), IDM_MACHINE_PAUSE, MF_UNCHECKED );
		StartScreenMsg( 1000, "Resumed.." );
		OutputScreenMsg();
	}
	else
	{
		nes.Pause( TRUE );
		SoundManager.Clear();
		CheckMenuItem( GetMenu(), IDM_MACHINE_PAUSE, MF_CHECKED );
		StartScreenMsg( 1000, "Paused.." );
		OutputScreenMsg();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::RefreshCursor(const BOOL force)
{
	if (!force && UseZapper && nes.IsOn() && nes.IsCartridge())
	{
		hCursor = LoadCursor( NULL, IDC_CROSS );
		SetCursor( hCursor );
		while (ShowCursor( TRUE ) <= -1);
	}
	else if (force || windowed || !hMenu)
	{
		hCursor = LoadCursor( NULL, IDC_ARROW );
		SetCursor( hCursor );
		while (ShowCursor( TRUE ) <= -1);
	}
	else
	{
		while (ShowCursor( FALSE ) >= 0);
		SetCursor( NULL );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnNsfCommand(const NES::IO::NSF::OP op)
{
	NES::IO::NSF::CONTEXT context;
	context.op = op;

	if (PDX_SUCCEEDED(nes.SetNsfContext( context )))
	{
		if (op == NES::IO::NSF::NEXT || op == NES::IO::NSF::PREV)
		{
			if (PDX_SUCCEEDED(nes.GetNsfContext(context)))
			{
				NsfInfo.song  = "Song: ";
				NsfInfo.song += (context.song+1);
				NsfInfo.song += "/";
				NsfInfo.song += context.NumSongs;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnExitSizeMove()
{
	GraphicManager.UpdateScreenRect( rcScreen, TRUE );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnHelp(const UINT wParam)
{
	SoundManager.Clear();

	switch (wParam)
	{
     	case IDM_HELP_ABOUT:   HelpManager.StartAboutDialog();
     	case IDM_HELP_LICENCE: HelpManager.StartLicenceDialog();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnActive()
{
	if (active = ready)
		active = GraphicManager.OnFocus(TRUE);
	
	if (!hMenu)
		DrawMenuBar(hWnd);

	InBackground = FALSE;
	SoundManager.Start();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnInactive(const BOOL force)
{
	InBackground = TRUE;

	if (force || (!preferences.RunInBackground() && !(nes.IsNsf() && nes.IsOn() && preferences.RunNsfInBackground())))
	{
		active = FALSE;
		SoundManager.Stop();
		GraphicManager.OnFocus(FALSE);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnOpen(const FILETYPE FileType,const INT recent)
{
	SoundManager.Clear();
	SaveStateManager.Flush();

	const BOOL WasPAL = nes.IsPAL();
	PDXRESULT result = PDX_FAILURE;

	switch (FileType)
	{
   		case FILE_ALL: result = FileManager.Load    ( recent, preferences.EmulateImmediately() ); break;
   		case FILE_NSP: result = FileManager.LoadNSP ( recent, preferences.EmulateImmediately() ); break;
   		default: return;
	}

	if (PDX_SUCCEEDED(result))
	{
		const BOOL IsPAL = nes.IsPAL();

		if (NesMode == NES::MODE_AUTO)
		{
			TimerManager.EnablePAL( IsPAL );
			GraphicManager.UpdateDirectDraw();

			if (bool(IsPAL) != bool(WasPAL) && windowed)
			{
				const UINT factor = GetAspectRatio();

				if (factor != UINT_MAX)
					OnWindowSize( factor );
			}
		}

		if (AutoSelectController)
			UpdateControllerPorts();

		StartScreenMsg
		(
		    1500,
		    "Loaded ",
			FileManager.GetRecentFile().GetFileName(),
			", ",
			(IsPAL ? "PAL" : "NTSC")
		);

		const PDXSTRING* const name = nes.GetMovieFileName();

		if (name)
			MovieManager.SetFile( *name );
	}

	UpdateWindowItems();

	if (FileManager.UpdatedRecentFiles())
		UpdateRecentFiles();

	DrawMenuBar( hWnd );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnSaveScreenShot()
{
	SoundManager.Clear();
	GraphicManager.SaveScreenShot();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::UpdateControllerPorts()
{
	nes.AutoSelectController();
	
	switch (nes.ConnectedController(0))
	{
     	case NES::CONTROLLER_UNCONNECTED: OnPort( IDM_MACHINE_PORT1_UNCONNECTED ); break;
		case NES::CONTROLLER_PAD1:        OnPort( IDM_MACHINE_PORT1_PAD1        ); break;
		case NES::CONTROLLER_PAD2:        OnPort( IDM_MACHINE_PORT1_PAD2        ); break;
		case NES::CONTROLLER_PAD3:        OnPort( IDM_MACHINE_PORT1_PAD3        ); break;
		case NES::CONTROLLER_PAD4:        OnPort( IDM_MACHINE_PORT1_PAD4        ); break;
		case NES::CONTROLLER_ZAPPER:      OnPort( IDM_MACHINE_PORT1_ZAPPER      ); break;
		case NES::CONTROLLER_PADDLE:      OnPort( IDM_MACHINE_PORT1_PADDLE      ); break;
		case NES::CONTROLLER_POWERPAD:    OnPort( IDM_MACHINE_PORT1_POWERPAD    ); break;
	}

	switch (nes.ConnectedController(1))
	{
     	case NES::CONTROLLER_UNCONNECTED: OnPort( IDM_MACHINE_PORT2_UNCONNECTED ); break;
		case NES::CONTROLLER_PAD1:        OnPort( IDM_MACHINE_PORT2_PAD1        ); break;
		case NES::CONTROLLER_PAD2:        OnPort( IDM_MACHINE_PORT2_PAD2        ); break;
		case NES::CONTROLLER_PAD3:        OnPort( IDM_MACHINE_PORT2_PAD3        ); break;
		case NES::CONTROLLER_PAD4:        OnPort( IDM_MACHINE_PORT2_PAD4        ); break;
		case NES::CONTROLLER_ZAPPER:      OnPort( IDM_MACHINE_PORT2_ZAPPER      ); break;
		case NES::CONTROLLER_PADDLE:      OnPort( IDM_MACHINE_PORT2_PADDLE      ); break;
		case NES::CONTROLLER_POWERPAD:    OnPort( IDM_MACHINE_PORT2_POWERPAD    ); break;
	}

	switch (nes.ConnectedController(2))
	{
     	case NES::CONTROLLER_UNCONNECTED: OnPort( IDM_MACHINE_PORT3_UNCONNECTED ); break;
		case NES::CONTROLLER_PAD1:        OnPort( IDM_MACHINE_PORT3_PAD1        ); break;
		case NES::CONTROLLER_PAD2:        OnPort( IDM_MACHINE_PORT3_PAD2        ); break;
		case NES::CONTROLLER_PAD3:        OnPort( IDM_MACHINE_PORT3_PAD3        ); break;
		case NES::CONTROLLER_PAD4:        OnPort( IDM_MACHINE_PORT3_PAD4        ); break;
	}

	switch (nes.ConnectedController(3))
	{
     	case NES::CONTROLLER_UNCONNECTED: OnPort( IDM_MACHINE_PORT4_UNCONNECTED ); break;
		case NES::CONTROLLER_PAD1:        OnPort( IDM_MACHINE_PORT4_PAD1        ); break;
		case NES::CONTROLLER_PAD2:        OnPort( IDM_MACHINE_PORT4_PAD2        ); break;
		case NES::CONTROLLER_PAD3:        OnPort( IDM_MACHINE_PORT4_PAD3        ); break;
		case NES::CONTROLLER_PAD4:        OnPort( IDM_MACHINE_PORT4_PAD4        ); break;
	}

	switch (nes.ConnectedController(4))
	{
     	case NES::CONTROLLER_UNCONNECTED: OnPort( IDM_MACHINE_EXPANSION_UNCONNECTED         ); break;
		case NES::CONTROLLER_KEYBOARD:    OnPort( IDM_MACHINE_EXPANSION_FAMILYBASICKEYBOARD ); break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnLoadNsp()
{
	OnOpen(FILE_NSP);
	UpdateFdsMenu();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnSaveNsp()
{
	SoundManager.Clear();
	FileManager.SaveNSP();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnLoadState()
{
	SoundManager.Clear();
	FileManager.LoadNST();
	UpdateFdsMenu();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnSaveState()
{
	SoundManager.Clear();
	FileManager.SaveNST();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnSoundRecorder(const UINT wParam)
{
	switch (wParam)
	{
     	case IDM_FILE_SOUND_CAPTURE_FILE:   SoundManager.StartSoundRecordDialog(); break;
     	case IDM_FILE_SOUND_CAPTURE_RECORD: SoundManager.StartSoundRecording();    break;
     	case IDM_FILE_SOUND_CAPTURE_STOP:   SoundManager.StopSoundRecording();     break;
     	case IDM_FILE_SOUND_CAPTURE_RESET:  SoundManager.ResetSoundRecording();    break;
	}
	
	UpdateSoundRecorderMenu();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::UpdateDynamicMenuItems()
{
	UpdateMovieMenu();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnFdsInsertDisk(const UINT wParam)
{
	PDX_ASSERT( wParam >= IDM_FDS_INSERT_DISK_1 && wParam <= IDM_FDS_INSERT_DISK_16 );

	NES::IO::FDS::CONTEXT context;

	if (PDX_SUCCEEDED(nes.GetFdsContext( context )))
	{
		const UINT disk = wParam - IDM_FDS_INSERT_DISK_1;

		if (!context.DiskInserted || context.CurrentDisk != disk)
		{
			context.DiskInserted = TRUE;
			context.CurrentDisk = disk;
			nes.SetFdsContext( context );
			UpdateFdsMenu();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnFdsEjectDisk()
{
	NES::IO::FDS::CONTEXT context;

	if (PDX_SUCCEEDED(nes.GetFdsContext( context )))
	{
		if (context.DiskInserted)
		{
			context.DiskInserted = FALSE;
			nes.SetFdsContext( context );
			UpdateFdsMenu();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnFdsSide(const UINT wParam)
{
	NES::IO::FDS::CONTEXT context;

	if (PDX_SUCCEEDED(nes.GetFdsContext( context )))
	{
		const UINT side = (wParam == IDM_FDS_SIDE_A ? 0 : 1);

		if (context.CurrentSide != side)
		{
			context.CurrentSide = side;
			nes.SetFdsContext( context );
			UpdateFdsMenu();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::UpdateFdsMenu()
{
	HMENU hMenu = GetMenu();

	EnableMenuItem( hMenu, IDM_FDS_EJECT_DISK, MF_GRAYED );
	EnableMenuItem( hMenu, IDM_FDS_SIDE_A, MF_GRAYED );
	EnableMenuItem( hMenu, IDM_FDS_SIDE_B, MF_GRAYED );

	HMENU hSubMenu;
	hSubMenu = GetSubMenu( hMenu,  2 );
	hSubMenu = GetSubMenu( hSubMenu, 0 );

	while (GetMenuItemCount( hSubMenu ))
		DeleteMenu( hSubMenu, 0, MF_BYPOSITION );

	hSubMenu = GetSubMenu( hMenu, 2 );

	EnableMenuItem( hSubMenu, 0, MF_BYPOSITION | MF_GRAYED );
	EnableMenuItem( hSubMenu, 2, MF_BYPOSITION | MF_GRAYED );

	if (nes.IsFds())
	{
		NES::IO::FDS::CONTEXT context;

		if (PDX_SUCCEEDED(nes.GetFdsContext( context )))
		{
			EnableMenuItem( hSubMenu, 2, MF_BYPOSITION | MF_ENABLED );
			
			EnableMenuItem( hMenu, IDM_FDS_SIDE_A, context.CurrentSide == 0 ? MF_GRAYED : MF_ENABLED );
			EnableMenuItem( hMenu, IDM_FDS_SIDE_B, context.CurrentSide == 1 ? MF_GRAYED : MF_ENABLED );

			if (!context.DiskInserted || context.NumDisks > 1)
			{
				EnableMenuItem( hSubMenu, 0, MF_BYPOSITION | MF_ENABLED );

				PDXSTRING name;

				hSubMenu = GetSubMenu( hSubMenu, 0 );

				const UINT NumDisks = PDX_MIN( 16, context.NumDisks );

				for (UINT i=0; i < NumDisks; ++i)
				{
					AppendMenu
					( 
				     	hSubMenu, 
						MF_BYPOSITION, 
						IDM_FDS_INSERT_DISK_1 + i, 
						( name = (i+1) ).String()
					);

					EnableMenuItem
					( 
				     	hSubMenu, 
						IDM_FDS_INSERT_DISK_1 + i, 
						(context.CurrentDisk == i && context.DiskInserted) ? MF_GRAYED : MF_ENABLED
					);
				}
			}
			
			if (context.DiskInserted)
				EnableMenuItem( hMenu, IDM_FDS_EJECT_DISK, MF_ENABLED );
  		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::UpdateWindowItems()
{
	HMENU hMenu = GetMenu();

	const NES::IO::CARTRIDGE::INFO* const info = nes.GetCartridgeInfo();

	PDXSTRING name("Nestopia");

	const BOOL IsOn        = nes.IsOn();
	const BOOL IsImage     = nes.IsImage();
	const BOOL IsImageOn   = IsOn && IsImage;
	const BOOL IsNsf       = nes.IsNsf();
	const BOOL IsNsfOn     = IsOn && IsNsf;
	const BOOL IsLoaded    = IsImage || IsNsf;
	const BOOL IsLoadedOn  = IsOn && IsLoaded;
	const BOOL IsCartridge = nes.IsCartridge();

	if (IsOn)
	{
		if (IsCartridge)
		{
			if (info && info->name != "unknown")
				name = info->name;
		}
		else if (IsNsf)
		{
			NES::IO::NSF::CONTEXT context;
			nes.GetNsfContext( context );
			name = context.name;
		}
	}

	UpdateNsf();
	ResetSaveSlots( IsImageOn );
	SetWindowText( hWnd, name.String() ); 

	const BOOL ExportBitmaps = GraphicManager.CanExportBitmaps();

	EnableMenuItem( hMenu, IDM_FILE_CLOSE,           NST_MENUSTATE( IsLoaded                        ) );
	EnableMenuItem( hMenu, IDM_FILE_LOAD_NST,        NST_MENUSTATE( IsImageOn                       ) );
	EnableMenuItem( hMenu, IDM_FILE_SAVE_NST,        NST_MENUSTATE( IsImageOn                       ) );
	EnableMenuItem( hMenu, IDM_FILE_SAVE_NSP,        NST_MENUSTATE( IsImage                         ) );
	EnableMenuItem( hMenu, IDM_FILE_SAVE_SCREENSHOT, NST_MENUSTATE( IsImageOn && ExportBitmaps      ) );
	EnableMenuItem( hMenu, IDM_MACHINE_RESET_SOFT,   NST_MENUSTATE( IsOn                            ) );
	EnableMenuItem( hMenu, IDM_MACHINE_RESET_HARD,   NST_MENUSTATE( IsOn                            ) );
	EnableMenuItem( hMenu, IDM_MACHINE_POWER_ON,     NST_MENUSTATE( !IsOn && IsLoaded               ) );
	EnableMenuItem( hMenu, IDM_MACHINE_POWER_OFF,    NST_MENUSTATE( IsOn                            ) );
	EnableMenuItem( hMenu, IDM_MACHINE_PAUSE,        NST_MENUSTATE( IsLoadedOn                      ) );
	EnableMenuItem( hMenu, IDM_NSF_PLAY,             NST_MENUSTATE( IsNsfOn                         ) );
	EnableMenuItem( hMenu, IDM_NSF_STOP,             NST_MENUSTATE( IsNsfOn                         ) );
	EnableMenuItem( hMenu, IDM_NSF_PREV,             NST_MENUSTATE( IsNsfOn                         ) );
	EnableMenuItem( hMenu, IDM_NSF_NEXT,             NST_MENUSTATE( IsNsfOn                         ) );
	EnableMenuItem( hMenu, IDM_VIEW_ROM_INFO,        NST_MENUSTATE( nes.IsCartridge()               ) );
	EnableMenuItem( hMenu, IDM_OPTIONS_DIP_SWITCHES, NST_MENUSTATE( nes.GetNumVsSystemDipSwitches() ) );

	CheckMenuItem( hMenu, IDM_MACHINE_PAUSE, nes.IsPaused() ? MF_CHECKED : MF_UNCHECKED );

	{
		HMENU hSubMenu;
		
		hSubMenu = GetSubMenu( hMenu, 0 );
		EnableMenuItem( hSubMenu, 6,  MF_BYPOSITION | NST_MENUSTATE( IsImageOn                     ) );
		EnableMenuItem( hSubMenu, 7,  MF_BYPOSITION | NST_MENUSTATE( IsImageOn                     ) );
		EnableMenuItem( hSubMenu, 13, MF_BYPOSITION | NST_MENUSTATE( FileManager.NumRecentFiles() ) );
		
		hSubMenu = GetSubMenu( hMenu, 1 );
		EnableMenuItem( hSubMenu, 0, MF_BYPOSITION | NST_MENUSTATE( IsOn || (IsLoaded && !IsOn) ) );
		EnableMenuItem( hSubMenu, 1, MF_BYPOSITION | NST_MENUSTATE( IsOn                        ) );
	}
  
	UpdateSoundRecorderMenu();
	UpdateFdsMenu();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::UpdateSoundRecorderMenu()
{
	const BOOL yeah = 
	(
     	SoundManager.IsSoundRecordingFilePresent() && 
		nes.IsOn() && (nes.IsImage() || nes.IsNsf())
	);

	HMENU hMenu = GetMenu();

	EnableMenuItem( hMenu, IDM_FILE_SOUND_CAPTURE_RECORD, NST_MENUSTATE( yeah && !SoundManager.IsSoundRecorderRecording() ) );
	EnableMenuItem( hMenu, IDM_FILE_SOUND_CAPTURE_STOP,   NST_MENUSTATE( yeah &&  SoundManager.IsSoundRecorderRecording() ) );
	EnableMenuItem( hMenu, IDM_FILE_SOUND_CAPTURE_RESET,  NST_MENUSTATE( yeah ) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::UpdateMovieMenu()
{
	HMENU hMenu = GetMenu();

	EnableMenuItem( hMenu, IDM_FILE_MOVIE_STOP,	   NST_MENUSTATE( MovieManager.CanStop()    ) );
	EnableMenuItem( hMenu, IDM_FILE_MOVIE_PLAY,    NST_MENUSTATE( MovieManager.CanPlay()    ) );
	EnableMenuItem( hMenu, IDM_FILE_MOVIE_RECORD,  NST_MENUSTATE( MovieManager.CanRecord()  ) );
	EnableMenuItem( hMenu, IDM_FILE_MOVIE_REWIND,  NST_MENUSTATE( MovieManager.CanRewind()  ) );
	EnableMenuItem( hMenu, IDM_FILE_MOVIE_FORWARD, NST_MENUSTATE( MovieManager.CanForward() ) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::UpdateNsf()
{
	NsfInfo.Clear();

	if (nes.IsNsf())
	{
		NES::IO::NSF::CONTEXT context;

		if (PDX_SUCCEEDED(nes.GetNsfContext(context)))
		{
			NsfInfo.name  = context.name;
			NsfInfo.name += (context.pal ? ", PAL" : ", NTSC");

			if (context.artist != "<?>" && context.artist != "< ? >" && context.artist != "?")
				NsfInfo.artist = context.artist;

			if (context.copyright != "<?>" && context.copyright != "< ? >" && context.copyright != "?")
				NsfInfo.copyright = context.copyright;

			if (context.chip.Length())
			{
				NsfInfo.copyright += ", ";
				NsfInfo.copyright += context.chip;
				NsfInfo.copyright += " chip";
			}

			NsfInfo.song  = "Song: ";
			NsfInfo.song += (context.song+1);
			NsfInfo.song += "/";
			NsfInfo.song += context.NumSongs;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnClose()
{
	SoundManager.Stop();

	nes.Unload();

	SaveStateManager.Flush();
	GameGenieManager.ClearAllCodes();
	GraphicManager.ClearNesScreen();
	UpdateWindowItems();

	StartScreenMsg
	(
	    1500,
	    "Closed ",
		FileManager.GetRecentFile().GetFileName()
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnCloseWindow()
{
	if (preferences.PowerOffOnClose() && nes.IsOn())
	{
		if (!preferences.ConfirmExit() || OnQuestion("Detach Image File","Absolutely sure?"))
			OnPower( FALSE );
	}
	else
	{
		if (!preferences.ConfirmExit() || OnQuestion("Exit Nestopia","Absolutely sure?"))
			PostQuitMessage(0);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnRecent(const UINT idm)
{
	PDX_ASSERT( idm >= IDM_FILE_RECENT_0 && idm <= IDM_FILE_RECENT_9 );
	OnOpen( FILE_ALL, idm - IDM_FILE_RECENT_0 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnPower(const BOOL state)
{
	if (bool(nes.IsOn()) != bool(state))
	{
		SoundManager.Clear();
		nes.Power( state );
		UpdateWindowItems();

		if (!state) 
			GraphicManager.ClearNesScreen();

		StartScreenMsg
		(
		    1500,
		    "Power ",
			(state ? "On.." : "Off..")
		);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnReset(const BOOL state)
{
	SoundManager.Clear();
	nes.Reset( state );

	StartScreenMsg
	(
	    1500,
	    (state ? "Hard" : "Soft"),
		" reset.."
	);

	UpdateFdsMenu();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnMode(const UINT NewIdm)
{
	NES::MODE NewMode;

	switch (NewIdm)
	{
    	case IDM_MACHINE_MODE_AUTO: NewMode = NES::MODE_AUTO; break;
		case IDM_MACHINE_MODE_NTSC: NewMode = NES::MODE_NTSC; break;
		default:                    NewMode = NES::MODE_PAL;  break;
	}

	if (NesMode != NewMode)
	{
		nes.SetMode( NewMode );
		GraphicManager.UpdateDirectDraw();

		const BOOL IsPAL = nes.IsPAL();
		TimerManager.EnablePAL( IsPAL );

		if (windowed)
		{
			const UINT factor = GetAspectRatio();

			if (factor != UINT_MAX)
				OnWindowSize( factor );
		}

		UINT OldIdm;

		switch (NesMode)
		{
       		case NES::MODE_AUTO: OldIdm = IDM_MACHINE_MODE_AUTO; break;
     		case NES::MODE_NTSC: OldIdm = IDM_MACHINE_MODE_NTSC; break;
    		default:             OldIdm = IDM_MACHINE_MODE_PAL;  break;
		}

		NesMode = NewMode;

		HMENU hMenu = GetMenu();
		CheckMenuItem( hMenu, OldIdm, MF_UNCHECKED );
		CheckMenuItem( hMenu, NewIdm, MF_CHECKED   );

		StartScreenMsg
		(
		    1500,
			"Switched to ",
			(IsPAL ? "PAL.." : "NTSC..")
		);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnPaint()
{
	{
    	PAINTSTRUCT ps;
    	BeginPaint( hWnd, &ps );
    	EndPaint( hWnd, &ps );

		if ((ps.rcPaint.right - ps.rcPaint.left) <= 0 || (ps.rcPaint.bottom - ps.rcPaint.top) <= 0)
			return;
	}

	if (GraphicManager.IsReady())
		GraphicManager.Repaint();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnMouseMove(const LPARAM lParam)
{
	if (nes.IsAnyControllerConnected( NES::CONTROLLER_PADDLE ))
	{
		POINT point =
		{
			GET_X_LPARAM( lParam ),
			GET_Y_LPARAM( lParam )
		};

		InputManager.OnMouseButtonDown( point );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnLeftMouseButtonDown(const LPARAM lParam)
{
	if (nes.IsAnyControllerConnected( NES::CONTROLLER_ZAPPER, NES::CONTROLLER_PADDLE ))
	{
		POINT point =
		{
			GET_X_LPARAM( lParam ),
			GET_Y_LPARAM( lParam )
		};

		InputManager.OnMouseButtonDown( point );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnLeftMouseButtonUp()
{															  
	if (nes.IsAnyControllerConnected( NES::CONTROLLER_ZAPPER, NES::CONTROLLER_PADDLE ))
		InputManager.OnMouseButtonUp();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnRightMouseButtonDown()
{
	if (!windowed)
	{
		if (hMenu) OnShowMenu();
		else       OnHideMenu();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnShowMenu()
{
	PDX_ASSERT( !windowed && bool(hMenu) != bool(::GetMenu(hWnd)) );

	if (hMenu)
	{
		LockWindowUpdate( hWnd );
		GraphicManager.EnableGDI( TRUE );
		SetMenu( hWnd, hMenu );
		hMenu = NULL;	
		RefreshCursor();
		LockWindowUpdate( NULL );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnHideMenu()
{
	PDX_ASSERT( !windowed && bool(hMenu) != bool(::GetMenu(hWnd)) );

	if (!hMenu)
	{
		LockWindowUpdate( hWnd );
		GraphicManager.EnableGDI( FALSE );
		hMenu = ::GetMenu( hWnd );
		SetMenu( hWnd, NULL );	
		RefreshCursor();
		LockWindowUpdate( NULL );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnMove(const LPARAM lParam)
{
	if (windowed) 
	{							 
		SetRect
		(
			&rcScreen,
			SHORT(LOWORD(lParam)),
			SHORT(HIWORD(lParam)),
			SHORT(LOWORD(lParam)) + (rcScreen.right - rcScreen.left),
			SHORT(HIWORD(lParam)) + (rcScreen.bottom - rcScreen.top) 
		);

		UpdateWindowRect( rcWindow, rcScreen );
		GraphicManager.UpdateScreenRect( rcScreen );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnSize(const LPARAM lParam)
{
	if (windowed)
	{
		rcScreen.right  = rcScreen.left + SHORT(LOWORD(lParam));
		rcScreen.bottom = rcScreen.top + SHORT(HIWORD(lParam));
		rcClient.right  = SHORT(LOWORD(lParam));
		rcClient.bottom = SHORT(HIWORD(lParam));

   		UpdateWindowRect( rcWindow, rcScreen );
   		GraphicManager.UpdateScreenRect( rcScreen );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnActivate(const WPARAM wParam)
{
	switch (LOWORD(wParam))
	{
   		case WA_INACTIVE: 

			OnInactive();
			return;

   		case WA_ACTIVE:
   		case WA_CLICKACTIVE: 

			OnActive();
			return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::UpdateWindowRect(RECT& rcTo,const RECT& rcFrom)
{
	rcTo = rcFrom;
	UpdateWindowRect( rcTo );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::UpdateWindowRect(RECT& rcTo)
{
	AdjustWindowRect( &rcTo, NST_WINDOWSTYLE, hMenu ? FALSE : TRUE );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::UpdateWindowSizes(const UINT width,const UINT height)
{
	HMENU hMenu = GetMenu();

	HMENU hSubMenu;
	hSubMenu = GetSubMenu( hMenu, 4 );
	hSubMenu = GetSubMenu( hSubMenu, 3 );

	while (GetMenuItemCount( hSubMenu ))
		DeleteMenu( hSubMenu, 0, MF_BYPOSITION );

	AppendMenu
	( 
		GetSubMenu( GetSubMenu( hMenu, 4 ), 3 ),
		MF_BYPOSITION, 
		IDM_VIEW_WINDOWSIZE_MAX, 
		"&Max"
	);

	PDXSTRING MenuText;

	for (UINT j=0,i=1,x=NES::IO::GFX::WIDTH,y=NES::IO::GFX::HEIGHT; x <= width && y <= height && j < 4; ++j, i *= 2, x *= 2, y *= 2) 
	{
		MenuText  = "&";
		MenuText += i;
		MenuText += "X";

		AppendMenu
		( 
			GetSubMenu( GetSubMenu( hMenu, 4 ), 3 ),
			MF_BYPOSITION, 
			IDM_VIEW_WINDOWSIZE_1X + j, 
			MenuText.String()
		);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::ApplyWindowSizing()
{
	if (IsZoomed( hWnd ))
		SendMessage( hWnd, WM_SYSCOMMAND, SC_RESTORE, 0 );

	rcWindow.right  = rcWindow.left + (rcDefWindow.right  - rcDefWindow.left);
	rcWindow.bottom = rcWindow.top  + (rcDefWindow.bottom - rcDefWindow.top);
	rcScreen.right  = rcScreen.left + (rcDefClient.right  - rcDefClient.left);
	rcScreen.bottom = rcScreen.top  + (rcDefClient.bottom - rcDefClient.top);
	rcClient = rcDefClient;

	SetWindowPos
	(
		hWnd,
		HWND_NOTOPMOST,
		0,
		0,
		rcWindow.right - rcWindow.left,
		rcWindow.bottom - rcWindow.top,
		(IsWindowVisible( hWnd ) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW) | SWP_NOMOVE
	);		
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

UINT APPLICATION::GetAspectRatio() const
{
	const UINT width = windowed ? (rcScreen.right - rcScreen.left) : (rcRestoreWindow.right - rcRestoreWindow.left);
	const UINT height = windowed ? (rcScreen.bottom - rcScreen.top) : (rcRestoreWindow.bottom - rcRestoreWindow.top);

	if (width <= 256 + (256/3) && height <= 240 + (240/3))
	{
		return 0;
	}
	else if (width <= (256*2) + (256/2) && height <= (240*2) + (240/2))
	{
		return 1;
	}
	else if (width <= (256*3) + (256/4) && height <= (240*3) + (240/4))
	{
		return 2;
	}
	else if (width < GraphicManager.GetDisplayWidth() && height < GraphicManager.GetDisplayHeight())
	{
		return 3;
	}

	return UINT_MAX;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnWindowSize(const UINT factor,UINT width,UINT height)
{
	if (!width || !height)
	{
		width = GraphicManager.GetDisplayWidth();
		height = GraphicManager.GetDisplayHeight();
	}

	HMENU hMenu = GetMenu();

	if (windowed)
	{
		if (factor == UINT_MAX)
		{
			ShowWindow( hWnd, SW_MAXIMIZE );
		}
		else
		{
			SetRect
			( 
		     	&rcDefClient, 
				0, 
				0,
				GraphicManager.GetNesRect().right - GraphicManager.GetNesRect().left, 
				GraphicManager.GetNesRect().bottom - GraphicManager.GetNesRect().top 
			);

			UpdateWindowSizes( width, height );

			for (UINT i=0; i < factor; ++i)
			{
				if (rcDefClient.right * 2 > width || rcDefClient.bottom * 2 > height) 
					break;

				rcDefClient.right *= 2;
				rcDefClient.bottom *= 2;
			}

			rcDefWindow = rcDefClient;
			AdjustWindowRect( &rcDefWindow, NST_WINDOWSTYLE, FALSE );

			ApplyWindowSizing();

			if (::GetMenu(hWnd))
			{
				MENUBARINFO info;
				PDXMemZero( info );
				info.cbSize = sizeof(info);

				if (GetMenuBarInfo( hWnd, OBJID_MENU, 0, &info))
				{
					rcDefWindow.bottom += (info.rcBar.bottom - info.rcBar.top) + 1;
					ApplyWindowSizing();
				}
			}

			GraphicManager.UpdateScreenRect( rcScreen, TRUE );
		}
	}
	else
	{
		GraphicManager.SetScreenSize
		(
       		factor == UINT_MAX ? GRAPHICMANAGER::SCREEN_STRETCHED : GRAPHICMANAGER::SCREENTYPE(factor)		    
		);

		UpdateWindowSizes
		( 
			GraphicManager.GetDisplayWidth(), 
			GraphicManager.GetDisplayHeight()
		);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::ResetSaveSlots(const BOOL enabled)
{
	HMENU hMenu = GetMenu();

	for (UINT i=IDM_FILE_QUICK_LOAD_STATE_SLOT_LAST; i <= IDM_FILE_QUICK_LOAD_STATE_SLOT_9; ++i)
		EnableMenuItem( hMenu, i, enabled ? MF_ENABLED : MF_GRAYED );

	for (UINT i=IDM_FILE_QUICK_SAVE_STATE_SLOT_NEXT; i <= IDM_FILE_QUICK_SAVE_STATE_SLOT_9; ++i)
		EnableMenuItem( hMenu, i, enabled ? MF_ENABLED : MF_GRAYED );
}

////////////////////////////////////////////////////////////////////////////////////////
// switch to full screen / window
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::SwitchScreen()
{
	PDX_ASSERT(hWnd);

	active = ready = FALSE;

	SoundManager.Clear();

	if (windowed)
	{
		PushWindow();
		GraphicManager.SwitchToFullScreen();

		UpdateWindowSizes( GraphicManager.GetDisplayWidth(), GraphicManager.GetDisplayHeight() );

		if (!preferences.HideMenuInFullScreen())
			OnRightMouseButtonDown();
	}
	else
	{
		GraphicManager.SwitchToWindowed( rcScreen );
		PopWindow();
	}

	if (windowed)
		DrawMenuBar( hWnd );

	InvalidateRect( NULL, NULL, FALSE );

	active = ready = TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
// save the window size and style
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::PushWindow()
{
	PDX_ASSERT(hWnd);

	windowed = FALSE;

	GetWindowRect( hWnd, &rcRestoreWindow );

	SetWindowLong( hWnd, GWL_STYLE, WS_POPUP|WS_VISIBLE );
	SetWindowLong( hWnd, GWL_EXSTYLE, WS_EX_TOPMOST );
  
	ChangeMenuText( IDM_VIEW_SWITCH_SCREEN, "&Window\tAlt+Return" );	
	EnableMenuItem( GetMenu(), IDM_VIEW_HIDE_MENU, MF_ENABLED );

	if (!hMenu)
	{
		hMenu = ::GetMenu( hWnd );
		SetMenu( hWnd, NULL );
	}

	RefreshCursor();
}

////////////////////////////////////////////////////////////////////////////////////////
// restore the window size and style
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::PopWindow()
{
	PDX_ASSERT(hWnd);

	windowed = TRUE;
  
	ChangeMenuText( IDM_VIEW_SWITCH_SCREEN, "&Fullscreen\tCtrl+Shift+F" );
	EnableMenuItem( GetMenu(), IDM_VIEW_HIDE_MENU, MF_GRAYED );

	RefreshCursor( TRUE );

	SetWindowLong( hWnd, GWL_STYLE, NST_WINDOWSTYLE );
	SetWindowLong( hWnd, GWL_EXSTYLE, 0 );

	if (hMenu)
	{
		SetMenu( hWnd, hMenu );
		hMenu = NULL;
	}

	SetWindowPos
	(
		hWnd,
		HWND_NOTOPMOST,
		rcRestoreWindow.left,
		rcRestoreWindow.top,
		rcRestoreWindow.right - rcRestoreWindow.left,
		rcRestoreWindow.bottom - rcRestoreWindow.top,
		SWP_SHOWWINDOW
	);

	UpdateWindowSizes( GraphicManager.GetDisplayWidth(), GraphicManager.GetDisplayHeight() );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OutputScreenMsg()
{
	const UINT height = windowed ? GraphicManager.GetNesDesc().dwHeight - (PDX_MAX(1,GraphicManager.GetScaleFactor()) * 16) : GraphicManager.GetDisplayHeight();
	GraphicManager.Print( ScreenMsg.String(), 2, height - 16, RGB(0x20,0x20,0xA0), ScreenMsg.Length() );
	GraphicManager.Print( ScreenMsg.String(), 1, height - 17, RGB(0xFF,0x20,0x20), ScreenMsg.Length() );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OutputNsfInfo()
{
	UINT height = 12;

	if (NsfInfo.name.Length())
	{
		GraphicManager.Print( NsfInfo.name.String(), 2, height - 0, RGB(0x20,0x60,0x20), NsfInfo.name.Length() );
		GraphicManager.Print( NsfInfo.name.String(), 1, height - 1, RGB(0x20,0xFF,0x20), NsfInfo.name.Length() );
		height += 12;
	}

	if (NsfInfo.artist.Length())
	{
		GraphicManager.Print( NsfInfo.artist.String(), 2, height - 0, RGB(0x20,0x60,0x20), NsfInfo.artist.Length() );
		GraphicManager.Print( NsfInfo.artist.String(), 1, height - 1, RGB(0x20,0xFF,0x20), NsfInfo.artist.Length() );
		height += 12;
	}

	if (NsfInfo.copyright.Length())
	{
		GraphicManager.Print( NsfInfo.copyright.String(), 2, height - 0, RGB(0x20,0x60,0x20), NsfInfo.copyright.Length() );
		GraphicManager.Print( NsfInfo.copyright.String(), 1, height - 1, RGB(0x20,0xFF,0x20), NsfInfo.copyright.Length() );
		height += 12;
	}

	GraphicManager.Print( NsfInfo.song.String(), 2, height - 0, RGB(0x20,0x60,0x20), NsfInfo.song.Length() );
	GraphicManager.Print( NsfInfo.song.String(), 1, height - 1, RGB(0x20,0xFF,0x20), NsfInfo.song.Length() );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnLoadStateSlot(const UINT idm)
{
	PDXSTRING string;

	const UINT slot = idm - IDM_FILE_QUICK_LOAD_STATE_SLOT_LAST;

	if (PDX_SUCCEEDED(SaveStateManager.LoadState( slot )))
	{
		StartScreenMsg
		(
		    1000,
		    "Loaded from slot ",
			(slot ? slot : SaveStateManager.GetLastSlot()),
			".."
		);
	}
	else
	{
		StartScreenMsg
		(
			1000,
			"Failed to load from slot.."
		);
	}

	UpdateFdsMenu();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnSaveStateSlot(const UINT idm)
{
	PDXSTRING string;

	if (PDX_SUCCEEDED(SaveStateManager.SaveState( idm - IDM_FILE_QUICK_SAVE_STATE_SLOT_NEXT )))
	{
		StartScreenMsg
		(
		    1000,
		    "Saved to slot ",
			SaveStateManager.GetLastSlot(),
			".."
		);
	}
	else
	{
		StartScreenMsg
		(
			1000,
			"Failed to save to slot.."
		);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::UpdateRecentFiles()
{
	HMENU hSubMenu;
	hSubMenu = GetSubMenu( GetMenu(),  0 );
	hSubMenu = GetSubMenu( hSubMenu,  13 );

	while (GetMenuItemCount( hSubMenu ))
		DeleteMenu( hSubMenu, 0, MF_BYPOSITION );

	PDXSTRING MenuItemName;

	const UINT NumFiles = PDX_MIN( 10, FileManager.NumRecentFiles() );

	for (UINT i=0; i < NumFiles; ++i)
	{
		MenuItemName  = i;
		MenuItemName += " ";
		MenuItemName += FileManager.GetRecentFile(i);

		AppendMenu( hSubMenu, MF_BYPOSITION, IDM_FILE_RECENT_0 + i,  MenuItemName.String() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL APPLICATION::ChangeMenuText(const ULONG id,CHAR* const string) const
{
	PDX_ASSERT(string);

	MENUITEMINFO info;
	PDXMemZero( info );

	info.cbSize     = sizeof(info);
	info.fMask      = MIIM_STRING;
	info.dwTypeData	= string;

	return SetMenuItemInfo( GetMenu(), id, FALSE, &info );
}
