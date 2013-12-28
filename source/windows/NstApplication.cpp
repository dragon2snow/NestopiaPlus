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

#include "../paradox/PdxFile.h"
#include "resource/resource.h"
#include "NstApplication.h"
#include "NstSoundManager.h"
#include "NstInputManager.h"
#include "NstFileManager.h"
#include "NstFdsManager.h"
#include "NstSaveStateManager.h"
#include "NstMovieManager.h"
#include "NstGameGenieManager.h"
#include "NstVsDipSwitchManager.h"
#include "NstPreferences.h"
#include "NstLogFileManager.h"
#include "NstRomInfo.h"
#include "NstHelpManager.h"
#include <WindowsX.h>

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXSTRING APPLICATION::ScreenMsg;

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_NAMESPACE_BEGIN

	PDXRESULT MsgError(const CHAR* const text) 
    { return application.OnError( text ); }

	PDXRESULT MsgWarning(const CHAR* const text) 
	{ return application.OnWarning( text ); }
	
	BOOL MsgQuestion(const CHAR* const text,const CHAR* const head) 
	{ return application.OnQuestion( text, head ); }

	VOID MsgOutput(const CHAR* const text)
	{ application.StartScreenMsg( text, 1500 ); }

	VOID LogOutput(const CHAR* const text)
	{ LOGFILEMANAGER::LogOutput( text ); }

NES_NAMESPACE_END

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#define NST_MENUSTATE(x) ((x) ? MF_ENABLED : MF_GRAYED)
#define NST_WINDOWSTYLE	(WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_THICKFRAME)

////////////////////////////////////////////////////////////////////////////////////////
// constructor
////////////////////////////////////////////////////////////////////////////////////////

APPLICATION::APPLICATION()
: 
hWnd                 (NULL),
hInstance            (NULL),
hMenu                (NULL),
hCursor              (NULL),
active               (FALSE),
ready                (FALSE),
locked               (FALSE),
error                (FALSE),
UseZapper            (FALSE),
windowed             (TRUE),
InBackground         (FALSE),
ScreenInvisible      (FALSE),
AutoSelectController (TRUE),
AcceleratorEnabled   (TRUE),
NesMode              (NES::MODE_AUTO),
hMenuFullscreenBrush (NULL),
GraphicManager       (new GRAPHICMANAGER     ( IDD_GRAPHICS,    CHUNK_GRAPHICMANAGER   )),
SoundManager         (new SOUNDMANAGER       ( IDD_SOUND,       CHUNK_SOUNDMANAGER     )),
InputManager         (new INPUTMANAGER       ( IDD_INPUT,       CHUNK_INPUTMANAGER     )),
FileManager          (new FILEMANAGER        ( IDD_PATHS,       CHUNK_FILEMANAGER      )),
FdsManager           (new FDSMANAGER         ( IDD_FDS,         CHUNK_FDSMANAGER       )),
preferences          (new PREFERENCES        ( IDD_PREFERENCES, CHUNK_PREFERENCES      )),
RomInfo              (new ROMINFO            ( IDD_ROM_INFO                            )),
VsDipSwitchManager   (new VSDIPSWITCHMANAGER ( IDD_DIPSWITCHES                         )),
GameGenieManager     (new GAMEGENIEMANAGER   ( IDD_GAMEGENIE,   CHUNK_GAMEGENIEMANAGER )),
SaveStateManager     (new SAVESTATEMANAGER   ( IDD_AUTO_SAVE                           )),
MovieManager         (new MOVIEMANAGER       ( IDD_MOVIE                               )),
LogFileManager       (new LOGFILEMANAGER     ( IDD_LOGFILE                             )),
HelpManager          (new HELPMANAGER),
hMenuWindowBrush     (NULL)
{}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

APPLICATION::~APPLICATION()
{
	if (hWnd)
		OnExit();

	delete GraphicManager;
	delete SoundManager;
	delete InputManager;
	delete FileManager;
	delete FdsManager;
	delete GameGenieManager;
	delete MovieManager;
	delete SaveStateManager;
	delete VsDipSwitchManager;
	delete preferences;
	delete LogFileManager;
	delete RomInfo;
	delete HelpManager;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::LogOutput(const CHAR* const msg) const
{ 
	return LOGFILEMANAGER::LogOutput(msg); 
}

VOID APPLICATION::LogSeparator() const
{
	return LOGFILEMANAGER::LogSeparator(); 
}

////////////////////////////////////////////////////////////////////////////////////////
// creator
////////////////////////////////////////////////////////////////////////////////////////

#define NST_INIT_F(t,f) if (!t || PDX_FAILED(t->Init( hWnd, hInstance, &nes, f ))) return PDX_FAILURE
#define NST_INIT(t)     if (!t || PDX_FAILED(t->Init( hWnd, hInstance, &nes    ))) return PDX_FAILURE
#define NST_INIT_S(t)   if (!t || PDX_FAILED(t->Init( hWnd, hInstance          ))) return PDX_FAILURE

PDXRESULT APPLICATION::Init(HINSTANCE h,const CHAR* const RomImage,const INT iCmdShow)
{
	PDX_ASSERT(!hWnd);

	active = FALSE;
	hInstance = h;
	hCursor = LoadCursor(NULL,IDC_ARROW);

	const WNDCLASSEX WndClass = 
	{ 
		sizeof(WNDCLASSEX),
		0,
		WndProc, 
		0, 
		0, 
		hInstance,
		LoadIcon(hInstance,MAKEINTRESOURCE(IDI_GEEK_1)),
		hCursor,
		(HBRUSH) GetStockObject(NULL_BRUSH),
		NULL,
		TEXT("Nestopia"),
		LoadIcon(hInstance,MAKEINTRESOURCE(IDI_GEEK_2))
	};

	if (!RegisterClassEx(&WndClass))
		return OnError("RegisterClassEx() failed!");

	SetRect( &rcDefClient, 0, 0, 256*2, 224*2 );
	rcDefWindow = rcDefClient;
	AdjustWindowRect( &rcDefWindow, NST_WINDOWSTYLE, TRUE );

	hWnd = CreateWindow
	(
		TEXT("Nestopia"),
		TEXT("Nestopia"),
		NST_WINDOWSTYLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rcDefWindow.right - rcDefWindow.left,
		rcDefWindow.bottom - rcDefWindow.top,
		0,
		LoadMenu(hInstance,MAKEINTRESOURCE(IDR_MENU)),
		hInstance,
		NULL
	);

	if (!hWnd)
		return PDX_FAILURE;

	GetWindowRect( hWnd, &rcWindow );
	rcDefWindow = rcWindow;
	GetClientRect( hWnd, &rcClient );
	SetScreenRect( rcScreen );

	{
		PDXFILE cfgfile;
		CfgFileName.Clear();

		{
			CHAR PathExe[NST_MAX_PATH];
			PathExe[0] = '\0';

			if (GetModuleFileName( NULL, PathExe, NST_MAX_PATH-1 ))
			{
				CfgFileName = PathExe;
				CfgFileName.ReplaceFileExtension( "cfg" );
			}
		}

		cfgfile.Open( CfgFileName, PDXFILE::INPUT );
		PDXFILE* const file = cfgfile.IsOpen() ? &cfgfile : NULL;

		NST_INIT_F ( FileManager,      file );
		NST_INIT_F ( GraphicManager,   file );
		NST_INIT_F ( SoundManager,     file );
		NST_INIT_F ( InputManager,     file );
		NST_INIT_F ( FdsManager,       file );
		NST_INIT_F ( GameGenieManager, file );
		NST_INIT_F ( preferences,      file );
		NST_INIT_S ( HelpManager            );
		NST_INIT   ( SaveStateManager       );
		NST_INIT   ( MovieManager           );
		NST_INIT   ( VsDipSwitchManager     );
		NST_INIT   ( LogFileManager         );
		NST_INIT   ( RomInfo                );
	}

	UpdateRecentFiles();

	if (RomImage && strlen( RomImage ))
	{
		FileManager->AddRecentFile( RomImage );
		FileManager->Load( 0, preferences->EmulateImmediately() );
	}

	hMenuFullscreenBrush = CreateSolidBrush( RGB(0x70,0x75,0xC0) );

	SelectPort[0] = IDM_MACHINE_PORT1_PAD1;
	SelectPort[1] = IDM_MACHINE_PORT2_PAD2;
	SelectPort[2] = IDM_MACHINE_PORT3_UNCONNECTED;
	SelectPort[3] = IDM_MACHINE_PORT4_UNCONNECTED;
	SelectPort[4] = IDM_MACHINE_EXPANSION_UNCONNECTED;

	AutoSelectController = FALSE;
	OnAutoSelectController();

	UpdateWindowItems();

	PDX_TRY(GraphicManager->SwitchToWindowed( rcScreen ));

	if (preferences->StartUpFullScreen())
	{
		PDX_TRY(SwitchScreen());
	}
	else
	{
		ShowWindow( hWnd, iCmdShow );
	}
  
	ready = TRUE;
	active = TRUE;

	return PDX_OK;
}

#undef NST_INIT_F
#undef NST_INIT_S
#undef NST_INIT

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT APPLICATION::OnError(const CHAR* const text)
{
	if (!error)
	{
		error = TRUE;
		active = ready = FALSE;

		if (!windowed)
			SwitchScreen();

		ShowCursor( TRUE );

		GraphicManager->EnableGDI( TRUE );
		MessageBox( hWnd, text, "Nestopia Error!", MB_OK|MB_ICONERROR );
		GraphicManager->EnableGDI( FALSE );

		if (hWnd) 
			SendMessage( hWnd, WM_CLOSE, 0, 0 );
	}

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT APPLICATION::OnWarning(const CHAR* const text)
{
	if (!error)
	{
		const BOOL yourcardsuck = !windowed && !GraphicManager->CanRenderWindowed();

		if (yourcardsuck)
			SwitchScreen();

		GraphicManager->EnableGDI( TRUE );
		MessageBox( hWnd, text, "Nestopia Warning!", MB_OK|MB_ICONWARNING );
		GraphicManager->EnableGDI( FALSE );

		if (yourcardsuck)
			SwitchScreen();
	}

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL APPLICATION::OnQuestion(const CHAR* const head,const CHAR* const text)
{
	if (!error)
	{
		const BOOL yourcardsuck = !windowed && !GraphicManager->CanRenderWindowed();

		if (yourcardsuck)
			SwitchScreen();

		ShowCursor( TRUE );

		GraphicManager->EnableGDI( TRUE );		
		const BOOL yep = MessageBox( hWnd, text, head, MB_YESNO|MB_ICONQUESTION ) == IDYES;		
		GraphicManager->EnableGDI( FALSE );

		if (yourcardsuck)
			SwitchScreen();

		if (!windowed && hMenu)
			while (ShowCursor( TRUE ) >= 0);

		return yep;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT APPLICATION::BeginDialogMode()
{
	if (!windowed && hMenu)
		ShowCursor( TRUE );

	return GraphicManager->BeginDialogMode(); 

}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT APPLICATION::EndDialogMode()
{
	if (!windowed && hMenu)
		while (ShowCursor( FALSE ) >= 0);

	return GraphicManager->EndDialogMode(); 
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
       	case WM_PAINT:

			OnPaint();
			return 0;

		case WM_ERASEBKGND:
			return TRUE;

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
     			case PBT_APMQUERYSUSPEND:  OnInactive( TRUE ); return 0;
				case PBT_APMRESUMESUSPEND: OnActive(); return 0;
			}
			break;

		case WM_CLOSE:

			OnCloseWindow();
			return 0;

		case WM_DESTROY:

			PostQuitMessage(0); 
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
		case IDM_FILE_OPEN:                       OnOpen();                               return TRUE;
		case IDM_FILE_CLOSE:                      OnClose();                              return TRUE;
		case IDM_FILE_LOAD_STATE:                 OnLoadState();                          return TRUE;
		case IDM_FILE_SAVE_STATE:                 OnSaveState();                          return TRUE;
		case IDM_FILE_QUIT:			              OnCloseWindow();                        return TRUE;
		case IDM_FILE_SAVE_SCREENSHOT:            GraphicManager->SaveScreenShot();       return TRUE;
		case IDM_FILE_RECORD_SOUND_FILE:		  
		case IDM_FILE_RECORD_SOUND_START:		  
		case IDM_FILE_RECORD_SOUND_STOP:		  
		case IDM_FILE_RECORD_SOUND_RESET:         OnSoundRecorder( idm );                 return TRUE;
		case IDM_FILE_MOVIE_FILE:                 MovieManager->StartDialog();            return TRUE;
		case IDM_FILE_MOVIE_PLAY:                 MovieManager->Play();                   return TRUE;
		case IDM_FILE_MOVIE_RECORD:               MovieManager->Record();                 return TRUE;
		case IDM_FILE_MOVIE_STOP:                 MovieManager->Stop();                   return TRUE;
		case IDM_FILE_MOVIE_REWIND:               MovieManager->Rewind();                 return TRUE;
		case IDM_MACHINE_POWER_ON:		          OnPower( TRUE  );                       return TRUE;
		case IDM_MACHINE_POWER_OFF:               OnPower( FALSE );                       return TRUE;
		case IDM_MACHINE_RESET_SOFT:              OnReset( FALSE );                       return TRUE;
		case IDM_MACHINE_RESET_HARD:              OnReset( TRUE  );                       return TRUE;
		case IDM_MACHINE_AUTOSELECTCONTROLLER:    OnAutoSelectController();               return TRUE;                                
		case IDM_MACHINE_MODE_AUTO:                                    
		case IDM_MACHINE_MODE_NTSC:		  
		case IDM_MACHINE_MODE_PAL:	              OnMode( idm );                          return TRUE;
		case IDM_MACHINE_PAUSE:	                  OnPause();                              return TRUE;
		case IDM_FDS_EJECT_DISK:                  OnFdsEjectDisk();                       return TRUE;
		case IDM_FDS_SIDE_A:                      
		case IDM_FDS_SIDE_B:                      OnFdsSide( idm );                       return TRUE;
		case IDM_FDS_OPTIONS:                     FdsManager->StartDialog();;             return TRUE;
		case IDM_NSF_PLAY:                        OnNsfCommand( NES::IO::NSF::PLAY );     return TRUE;
		case IDM_NSF_STOP:                        OnNsfCommand( NES::IO::NSF::STOP );     return TRUE;
		case IDM_NSF_NEXT:                        OnNsfCommand( NES::IO::NSF::NEXT );     return TRUE;
		case IDM_NSF_PREV:                        OnNsfCommand( NES::IO::NSF::PREV );     return TRUE;
		case IDM_VIEW_ROM_INFO:                   RomInfo->StartDialog();                 return TRUE;
		case IDM_VIEW_LOGFILE:                    LogFileManager->StartDialog();          return TRUE;
		case IDM_VIEW_SWITCH_SCREEN:              SwitchScreen();                         return TRUE;
		case IDM_VIEW_NORMAL_WINDOW_SIZE:         OnRestoreWindow();                      return TRUE;
		case IDM_VIEW_HIDE_MENU:                  OnHideMenu();                           return TRUE;
		case IDM_OPTIONS_PREFERENCES:             preferences        ->StartDialog();     return TRUE;
		case IDM_OPTIONS_VIDEO:                   GraphicManager     ->StartDialog();     return TRUE;
		case IDM_OPTIONS_SOUND:                   SoundManager       ->StartDialog();     return TRUE;
		case IDM_OPTIONS_INPUT:                   InputManager       ->StartDialog();     return TRUE;
		case IDM_OPTIONS_PATHS:                   FileManager        ->StartDialog();     return TRUE;
		case IDM_OPTIONS_GAME_GENIE:              GameGenieManager   ->StartDialog();     return TRUE;
		case IDM_OPTIONS_AUTO_SAVE:               SaveStateManager   ->StartDialog();     return TRUE;
		case IDM_OPTIONS_DIP_SWITCHES:            VsDipSwitchManager ->StartDialog();     return TRUE;
		case IDM_HELP_ABOUT:                      HelpManager->StartAboutDialog();        return TRUE;
		case IDM_HELP_LICENCE:                    HelpManager->StartLicenceDialog();      return TRUE;
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
			AcceleratorEnabled = TRUE;
			UseZapper = nes.IsAnyControllerConnected(NES::CONTROLLER_ZAPPER);
		}

		if (UseZapper)
		{
			hCursor = LoadCursor(NULL,IDC_CROSS);
		}
		else
		{
			hCursor = (windowed || !hMenu) ? LoadCursor(NULL,IDC_ARROW) : NULL; 
		}

		SetCursor( hCursor );
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

VOID APPLICATION::StartScreenMsg(const CHAR* const msg,const UINT duration)
{
	ScreenMsg = msg;
	SetTimer( hWnd, TIMER_ID_SCREEN_MSG, duration, OnScreenMsgEnd );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL APPLICATION::OnSysCommand(const WPARAM wParam)
{
	if (windowed)
	{
		SoundManager->Clear();
		timer.Reset();
	}
	else
	{
		switch (wParam)
		{
     		case SC_MOVE:
       		case SC_SIZE:
     		case SC_MAXIMIZE:
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
		StartScreenMsg( "Resumed..", 1000 );
		OutputScreenMsg();
	}
	else
	{
		nes.Pause( TRUE );
		SoundManager->Clear();
		CheckMenuItem( GetMenu(), IDM_MACHINE_PAUSE, MF_CHECKED );
		StartScreenMsg( "Paused..", 1000 );
		OutputScreenMsg();
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

VOID APPLICATION::OnActive()
{
	InBackground = FALSE;
	active = ready;
	SoundManager->Start();
	timer.Reset();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnInactive(const BOOL force)
{
	InBackground = TRUE;

	if (!force && (preferences->RunInBackground() || (nes.IsNsf() && nes.IsOn() && preferences->RunNsfInBackground())))
		return;

	active = FALSE;
	SoundManager->Stop();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnOpen(const INT recent)
{
	const BOOL WasPAL = nes.IsPAL();

	if (PDX_SUCCEEDED(FileManager->Load( recent, preferences->EmulateImmediately() )))
	{
		const BOOL IsPAL = nes.IsPAL();

		if (NesMode == NES::MODE_AUTO)
		{
			GraphicManager->EnablePAL( IsPAL );
			SoundManager->EnablePAL( IsPAL );
			timer.EnablePAL( IsPAL );
		}

		if (AutoSelectController)
			UpdateControllerPorts();

		{
    		PDXSTRING string;

    		string  = "Loaded \"";
     		string += FileManager->GetRecentFile().GetFileName();

    		if (WasPAL != IsPAL)
     		{
    			string += "\" and switched to ";
     			string += (IsPAL ? "PAL.." : "NTSC..");
    		}
     		else
    		{
     			string += "\"..";
    		}

			StartScreenMsg( string, 1500 );
		}

		const PDXSTRING* const name = nes.GetMovieFileName();

		if (name)
			MovieManager->SetFile( *name );
	}

	UpdateWindowItems();

	if (FileManager->UpdatedRecentFiles())
		UpdateRecentFiles();

	DrawMenuBar( hWnd );
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

VOID APPLICATION::OnLoadState()
{
	FileManager->LoadState();
	UpdateFdsMenu();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnSaveState()
{
	FileManager->SaveState();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnSoundRecorder(const UINT wParam)
{
	switch (wParam)
	{
     	case IDM_FILE_RECORD_SOUND_FILE:  SoundManager->StartSoundRecordDialog(); break;
     	case IDM_FILE_RECORD_SOUND_START: SoundManager->StartSoundRecording();    break;
     	case IDM_FILE_RECORD_SOUND_STOP:  SoundManager->StopSoundRecording();     break;
     	case IDM_FILE_RECORD_SOUND_RESET: SoundManager->ResetSoundRecording();    break;
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
						( name = (i+1) )
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

	if (nes.IsOn())
	{
		if (nes.IsCartridge())
		{
			if (info && info->name != "unknown")
				name = info->name;
		}
		else if (nes.IsNsf())
		{
			NES::IO::NSF::CONTEXT context;
			nes.GetNsfContext( context );
			name = context.name;
		}
	}

	UpdateNsf();

	const BOOL IsGame = nes.IsCartridge() || nes.IsFds();
	const BOOL IsLoaded = IsGame || nes.IsNsf();

	ResetSaveSlots( nes.IsOn() && IsGame );

	SetWindowText( hWnd, name ); 

	const BOOL ExportBitmaps = GraphicManager->CanExportBitmaps();

	EnableMenuItem( hMenu, IDM_FILE_LOAD_STATE,      NST_MENUSTATE( nes.IsOn() && IsGame                   ) );
	EnableMenuItem( hMenu, IDM_FILE_SAVE_STATE,      NST_MENUSTATE( nes.IsOn() && IsGame                   ) );
	EnableMenuItem( hMenu, IDM_FILE_CLOSE,           NST_MENUSTATE( IsLoaded                               ) );
	EnableMenuItem( hMenu, IDM_FILE_SAVE_SCREENSHOT, NST_MENUSTATE( nes.IsOn() && IsGame && ExportBitmaps  ) );
	EnableMenuItem( hMenu, IDM_MACHINE_RESET_SOFT,   NST_MENUSTATE( nes.IsOn()                             ) );
	EnableMenuItem( hMenu, IDM_MACHINE_RESET_HARD,   NST_MENUSTATE( nes.IsOn()                             ) );
	EnableMenuItem( hMenu, IDM_MACHINE_POWER_ON,     NST_MENUSTATE( nes.IsOff() && IsLoaded                ) );
	EnableMenuItem( hMenu, IDM_MACHINE_POWER_OFF,    NST_MENUSTATE( nes.IsOn()                             ) );
	EnableMenuItem( hMenu, IDM_MACHINE_PAUSE,        NST_MENUSTATE( nes.IsOn() && IsLoaded                 ) );
	EnableMenuItem( hMenu, IDM_NSF_PLAY,             NST_MENUSTATE( nes.IsOn() && nes.IsNsf()              ) );
	EnableMenuItem( hMenu, IDM_NSF_STOP,             NST_MENUSTATE( nes.IsOn() && nes.IsNsf()              ) );
	EnableMenuItem( hMenu, IDM_NSF_PREV,             NST_MENUSTATE( nes.IsOn() && nes.IsNsf()              ) );
	EnableMenuItem( hMenu, IDM_NSF_NEXT,             NST_MENUSTATE( nes.IsOn() && nes.IsNsf()              ) );
	EnableMenuItem( hMenu, IDM_VIEW_ROM_INFO,        NST_MENUSTATE( nes.IsCartridge()                      ) );
	EnableMenuItem( hMenu, IDM_OPTIONS_DIP_SWITCHES, NST_MENUSTATE( info && info->system == NES::SYSTEM_VS ) );

	CheckMenuItem( hMenu, IDM_MACHINE_PAUSE, nes.IsPaused() ? MF_CHECKED : MF_UNCHECKED );

	{
		HMENU hSubMenu;
		
		hSubMenu = GetSubMenu( hMenu, 0 );
		EnableMenuItem( hSubMenu, 6,  MF_BYPOSITION | NST_MENUSTATE( nes.IsOn() && IsGame          ) );
		EnableMenuItem( hSubMenu, 7,  MF_BYPOSITION | NST_MENUSTATE( nes.IsOn() && IsGame          ) );
		EnableMenuItem( hSubMenu, 13, MF_BYPOSITION | NST_MENUSTATE( FileManager->NumRecentFiles() ) );
		
		hSubMenu = GetSubMenu( hMenu, 1 );
		EnableMenuItem( hSubMenu, 0, MF_BYPOSITION | NST_MENUSTATE( nes.IsOn() || (nes.IsOff() && IsLoaded) ) );
		EnableMenuItem( hSubMenu, 1, MF_BYPOSITION | NST_MENUSTATE( nes.IsOn() ) );
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
     	SoundManager->IsSoundRecordingFilePresent() && 
		nes.IsOn() && 
		(nes.IsCartridge() || nes.IsFds() || nes.IsNsf())
	);

	HMENU hMenu = GetMenu();

	EnableMenuItem( hMenu, IDM_FILE_RECORD_SOUND_START, NST_MENUSTATE( yeah && !SoundManager->IsSoundRecorderRecording() ) );
	EnableMenuItem( hMenu, IDM_FILE_RECORD_SOUND_STOP,  NST_MENUSTATE( yeah &&  SoundManager->IsSoundRecorderRecording() ) );
	EnableMenuItem( hMenu, IDM_FILE_RECORD_SOUND_RESET, NST_MENUSTATE( yeah ) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::UpdateMovieMenu()
{
	HMENU hMenu = GetMenu();

	EnableMenuItem( hMenu, IDM_FILE_MOVIE_STOP,	  NST_MENUSTATE( MovieManager->IsPlaying() || MovieManager->IsRecording() ) );
	EnableMenuItem( hMenu, IDM_FILE_MOVIE_PLAY,   NST_MENUSTATE( !MovieManager->IsPlaying() && MovieManager->CanPlay()    ) );
	EnableMenuItem( hMenu, IDM_FILE_MOVIE_RECORD, NST_MENUSTATE( !MovieManager->IsRecording() && MovieManager->IsLoaded() ) );
	EnableMenuItem( hMenu, IDM_FILE_MOVIE_REWIND, NST_MENUSTATE( MovieManager->IsLoaded()                                 ) );
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
	nes.Unload();

	GraphicManager->ClearNesScreen();
	SoundManager->Stop();

	UpdateWindowItems();

	{
    	PDXSTRING string;

    	string  = "Closed \"";
    	string += FileManager->GetRecentFile().GetFileName();
    	string += "\"..";

    	StartScreenMsg( string, 1500 );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnCloseWindow()
{
	if (preferences->PowerOffOnClose() && nes.IsOn())
	{
		if (!preferences->ConfirmExit() || OnQuestion("Detach Rom Image","Absolutely sure?"))
			OnPower( FALSE );
	}
	else
	{
		if (!preferences->ConfirmExit() || OnQuestion("Exit Nestopia","Absolutely sure?"))
			OnExit();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnRecent(const UINT idm)
{
	PDX_ASSERT( idm >= IDM_FILE_RECENT_0 && idm <= IDM_FILE_RECENT_9 );
	OnOpen( idm - IDM_FILE_RECENT_0 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnPower(const BOOL state)
{
	if (bool(nes.IsOn()) != bool(state))
	{
		nes.Power( state );

		UpdateWindowItems();

		if (state) 
		{
			SoundManager->Start();
		}
		else
		{
			SoundManager->Stop();
			GraphicManager->ClearNesScreen();
		}

		{
			PDXSTRING string;

			string = "Power ";
			string += (state ? "On.." : "Off..");

			StartScreenMsg( string, 1500 );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnReset(const BOOL state)
{
	nes.Reset( state );

	PDXSTRING string;

	string  = (state ? "Hard" : "Soft");
	string += " reset..";

	StartScreenMsg( string, 1500 );
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

		const BOOL IsPAL = nes.IsPAL();

		GraphicManager->EnablePAL( IsPAL );
		SoundManager->EnablePAL( IsPAL );
		timer.EnablePAL( IsPAL );

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

		{
			PDXSTRING string;

			string  = "Switched to ";
			string += (IsPAL ? "PAL.." : "NTSC..");

			StartScreenMsg( string, 1500 );
		}
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
	}

	if (GraphicManager->IsReady())
		GraphicManager->Repaint();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnMouseMove(const LPARAM lParam)
{
	if (nes.IsAnyControllerConnected( NES::CONTROLLER_PADDLE ))
	{
		POINT point;

		point.x = GET_X_LPARAM( lParam );
		point.y = GET_Y_LPARAM( lParam );

		InputManager->OnMouseButtonDown( point );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnLeftMouseButtonDown(const LPARAM lParam)
{
	if (nes.IsAnyControllerConnected( NES::CONTROLLER_ZAPPER, NES::CONTROLLER_PADDLE ))
	{
		POINT point;

		point.x = GET_X_LPARAM( lParam );
		point.y = GET_Y_LPARAM( lParam );

		InputManager->OnMouseButtonDown( point );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnLeftMouseButtonUp()
{															  
	if (nes.IsAnyControllerConnected( NES::CONTROLLER_ZAPPER, NES::CONTROLLER_PADDLE ))
		InputManager->OnMouseButtonUp();
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

		SetCursor( hCursor );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnShowMenu()
{
	PDX_ASSERT( !windowed && bool(hMenu) != bool(::GetMenu(hWnd)) );

	if (hMenu && GraphicManager->CanRenderWindowed())
	{
		GraphicManager->EnableGDI( TRUE );
		SetMenu( hWnd, hMenu );
		hMenu = NULL;	
		hCursor = LoadCursor( NULL, UseZapper ? IDC_CROSS : IDC_ARROW);
		ShowCursor( TRUE );
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
		GraphicManager->EnableGDI( FALSE );
		
		hMenu = ::GetMenu( hWnd );
		SetMenu( hWnd, NULL );	

		hCursor = UseZapper ? LoadCursor( NULL, IDC_CROSS ) : NULL;

		if (hCursor) ShowCursor( TRUE );
		else while (ShowCursor( FALSE ) >= 0);
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
			LOWORD(lParam),
			HIWORD(lParam),
			LOWORD(lParam) + (rcScreen.right - rcScreen.left),
			HIWORD(lParam) + (rcScreen.bottom - rcScreen.top) 
		);

		rcWindow = rcScreen;
		AdjustWindowRect( &rcWindow, NST_WINDOWSTYLE, TRUE );

		GraphicManager->UpdateScreenRect( rcScreen );
		InvalidateRect( hWnd, NULL, FALSE ); 
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnSize(const LPARAM lParam)
{
	if (windowed)
	{
		rcScreen.right  = rcScreen.left + LOWORD(lParam);
		rcScreen.bottom = rcScreen.top + HIWORD(lParam);
		rcClient.right  = LOWORD(lParam);
		rcClient.bottom = HIWORD(lParam);

		rcWindow = rcScreen;
		AdjustWindowRect( &rcWindow, NST_WINDOWSTYLE, TRUE );

		GraphicManager->UpdateScreenRect( rcScreen );
		InvalidateRect( hWnd, NULL, FALSE ); 

		ScreenInvisible = !(rcScreen.right - rcScreen.left) || !(rcScreen.bottom - rcScreen.top);
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

			InBackground = TRUE;

			if (!(preferences->RunInBackground() || (nes.IsNsf() && nes.IsOn() && preferences->RunNsfInBackground())))
			{
				SoundManager->Stop();
				active = FALSE; 
			}
   			return;

   		case WA_ACTIVE:
   		case WA_CLICKACTIVE: 

			InBackground = FALSE;
 			InputManager->AcquireDevices();
			active = ready;
			SoundManager->Start();
			timer.Reset();
   			return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnExit()
{
	nes.Power( FALSE );

	ready = FALSE;
	active = FALSE;

	KillTimer( hWnd, TIMER_ID_SCREEN_MSG );

	{
		PDXFILE file;

		if (CfgFileName.Size())
			file.Open( CfgFileName, PDXFILE::OUTPUT );

		PDXFILE* const SaveConfig = file.IsOpen() ? &file : NULL; 

		if ( FileManager      ) FileManager->Close      ( SaveConfig );
		if ( GraphicManager   ) GraphicManager->Close   ( SaveConfig );
		if ( SoundManager     ) SoundManager->Close     ( SaveConfig );
		if ( InputManager     ) InputManager->Close     ( SaveConfig );
		if ( FdsManager       )	FdsManager->Close       ( SaveConfig );
		if ( GameGenieManager ) GameGenieManager->Close ( SaveConfig );
		if ( preferences      ) preferences->Close      ( SaveConfig );
		if ( LogFileManager   )	LogFileManager->Close   ( preferences->LogFileEnabled() );
	}

	if (hMenuFullscreenBrush)
	{
		DeleteObject( hMenuFullscreenBrush );
		hMenuFullscreenBrush = NULL;
	}

	if (hMenu)
	{
		DestroyMenu( hMenu );
		hMenu = NULL;
	}

	if (hWnd)
	{
		DestroyWindow( hWnd );
		hWnd = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnRestoreWindow()
{
	if (windowed)
	{
		rcWindow.right  = rcWindow.left + (rcDefWindow.right  - rcDefWindow.left);
		rcWindow.bottom = rcWindow.top  + (rcDefWindow.bottom - rcDefWindow.top);
		rcScreen.right  = rcScreen.left + (rcDefClient.right  - rcDefClient.left);
		rcScreen.bottom = rcScreen.top  + (rcDefClient.bottom - rcDefClient.top);
		rcClient = rcDefClient;

		SetWindowPos
		(
			hWnd,
			HWND_NOTOPMOST,
			rcWindow.left,
			rcWindow.top,
			rcWindow.right - rcWindow.left,
			rcWindow.bottom - rcWindow.top,
			SWP_SHOWWINDOW
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
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnError()
{
	ready = active = FALSE;
	SendMessage( hWnd, WM_CLOSE, 0, 0 );
}

////////////////////////////////////////////////////////////////////////////////////////
// main loop
////////////////////////////////////////////////////////////////////////////////////////

INT APPLICATION::Loop()
{
	PDX_ASSERT(hWnd && !locked);

	HACCEL hAccel = LoadAccelerators( hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR) );

	MSG msg;	
	msg.message = WM_NULL;
	PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE );

	timer.Reset();

	while (msg.message != WM_QUIT)
	{
		if ((active ? PeekMessage(&msg,NULL,0,0,PM_REMOVE) : GetMessage(&msg,NULL,0,0)))
		{
			if (!AcceleratorEnabled || !hAccel || !TranslateAccelerator( hWnd, hAccel, &msg ))
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		}
		else if (active)
		{
			ExecuteFrame();
		}
	}

	if (hAccel)
		DestroyAcceleratorTable( hAccel );

	return INT(msg.wParam);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::ExecuteFrame()
{
	PDXRESULT result = PDX_OK;

	if (nes.IsCartridge() || nes.IsFds())
	{
		result = GraphicManager->TryClearScreen();

		InputManager->Poll();

		if (PDX_FAILED(result))
			GraphicManager->ClearScreen();

		result = nes.Execute
		(
			ScreenInvisible ? NULL : GraphicManager->GetFormat(),
			SoundManager->GetFormat(),
			InputManager->GetFormat()
		);
	}
	else
	{
		GraphicManager->ClearScreen();

		result = nes.Execute
		(
			NULL,
			SoundManager->GetFormat(),
			NULL
		);

		if (nes.IsNsf())
			OutputNsfInfo();
	}

	if (ScreenMsg.Length())
		OutputScreenMsg();

	if (PDX_SUCCEEDED(result))
	{
		if (!ScreenInvisible && GraphicManager->DoVSync())
		{
			GraphicManager->Present();
		}
		else
		{
			INT SkipFrames = timer.SynchRefreshRate( GraphicManager->AutoFrameSkip(), InBackground );

			if (!ScreenInvisible)
				GraphicManager->Present();

			while (--SkipFrames >= 0)
				nes.Execute( NULL, SoundManager->GetFormat(), NULL );
		}
	}
	else
	{
		GraphicManager->Repaint();
		timer.Reset();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// detect the absolute desktop rectangle coordinates
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT APPLICATION::SetScreenRect(RECT& rc)
{
	PDX_ASSERT(hWnd);

	rc = rcClient;
	ClientToScreen( hWnd, LPPOINT( &rc.left  ) );
	ClientToScreen( hWnd, LPPOINT( &rc.right ) );

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// switch to full screen / window
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT APPLICATION::SwitchScreen()
{
	PDX_ASSERT(hWnd);

	active = FALSE;
	
	PDX_TRY(SoundManager->Stop());

	if (windowed)
	{
		PDX_TRY(PushWindow());
		PDX_TRY(GraphicManager->SwitchToFullScreen());

		if (!preferences->HideMenuInFullScreen())
			OnRightMouseButtonDown();
	}
	else
	{
		PDX_TRY(GraphicManager->SwitchToWindowed( rcScreen ));
		PDX_TRY(PopWindow());
	}

	PDX_TRY(SoundManager->Start());

	active = TRUE;

	if (windowed || GraphicManager->CanRenderWindowed())
		DrawMenuBar( hWnd );

	Sleep(300);

	timer.Reset();

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// save the window size and style
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT APPLICATION::PushWindow()
{
	PDX_ASSERT(hWnd);

	windowed = FALSE;
	SetWindowLong( hWnd, GWL_STYLE, WS_POPUP|WS_VISIBLE );
	SetWindowLong( hWnd, GWL_EXSTYLE, WS_EX_TOPMOST );

	if (!hMenu)
	{
		hMenu = ::GetMenu( hWnd );
		SetMenu( hWnd, NULL );
	}

	ChangeMenuText( IDM_VIEW_SWITCH_SCREEN, "&Window\tCtrl+Shift+F" );	
	EnableMenuItem( GetMenu(), IDM_VIEW_HIDE_MENU, MF_ENABLED );

	hCursor = UseZapper ? LoadCursor(NULL,IDC_CROSS) : NULL; 
	SetCursor( hCursor );

	if (hCursor) ShowCursor( TRUE );
	else while (ShowCursor( FALSE ) >= 0);
  
	{
		MENUITEMINFO info;
		PDXMemZero( info );

		info.cbSize = sizeof(info);
		info.fMask  = MIIM_STATE;
		info.fState	= MFS_DISABLED;

		SetMenuItemInfo( GetMenu(), IDM_VIEW_NORMAL_WINDOW_SIZE, FALSE, &info );
	}

	{
		MENUINFO info;
		PDXMemZero( info );

		info.cbSize = sizeof(info);
		info.fMask  = MIM_BACKGROUND;

		GetMenuInfo( GetMenu(), &info );

		hMenuWindowBrush = info.hbrBack;

		info.fMask   = MIM_BACKGROUND | MIM_APPLYTOSUBMENUS;
		info.hbrBack = hMenuFullscreenBrush;

		SetMenuInfo( GetMenu(), &info );
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// restore the window size and style
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT APPLICATION::PopWindow()
{
	PDX_ASSERT(hWnd);

	windowed = TRUE;
	SetWindowLong( hWnd, GWL_STYLE, NST_WINDOWSTYLE );
	SetWindowLong( hWnd, GWL_EXSTYLE, 0 );

	SetWindowPos
	(
		hWnd,
		HWND_NOTOPMOST,
		rcWindow.left,
		rcWindow.top,
		rcWindow.right - rcWindow.left,
		rcWindow.bottom - rcWindow.top,
		SWP_SHOWWINDOW
	);

	if (hMenu)
	{
		SetMenu( hWnd, hMenu );
		hMenu = NULL;
	}

	ChangeMenuText( IDM_VIEW_SWITCH_SCREEN, "&Fullscreen\tCtrl+Shift+F" );

	hCursor = LoadCursor( NULL, UseZapper ? IDC_CROSS : IDC_ARROW ); 
	SetCursor( hCursor );
	ShowCursor( TRUE );

	EnableMenuItem( GetMenu(), IDM_VIEW_HIDE_MENU, MF_GRAYED );

	{
		MENUITEMINFO info;
		PDXMemZero( info );

		info.cbSize = sizeof(info);
		info.fMask  = MIIM_STATE;
		info.fState	= MFS_ENABLED;
	
		SetMenuItemInfo( GetMenu(), IDM_VIEW_NORMAL_WINDOW_SIZE, FALSE, &info );
	}

	{
		MENUINFO info;
		PDXMemZero( info );

		info.cbSize  = sizeof(info);
		info.fMask   = MIM_BACKGROUND | MIM_APPLYTOSUBMENUS;
		info.hbrBack = hMenuWindowBrush;

		SetMenuInfo( GetMenu(), &info );
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OutputScreenMsg()
{
	const UINT height = windowed ? 224 : GraphicManager->GetDisplayHeight();
	GraphicManager->Print( ScreenMsg, 2, height - 16, RGB(0x20,0x20,0xA0), ScreenMsg.Length() );
	GraphicManager->Print( ScreenMsg, 1, height - 17, RGB(0xFF,0x20,0x20), ScreenMsg.Length() );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OutputNsfInfo()
{
	UINT height = 12;

	if (NsfInfo.name.Length())
	{
		GraphicManager->Print( NsfInfo.name, 2, height - 0, RGB(0x20,0x60,0x20), NsfInfo.name.Length() );
		GraphicManager->Print( NsfInfo.name, 1, height - 1, RGB(0x20,0xFF,0x20), NsfInfo.name.Length() );
		height += 12;
	}

	if (NsfInfo.artist.Length())
	{
		GraphicManager->Print( NsfInfo.artist, 2, height - 0, RGB(0x20,0x60,0x20), NsfInfo.artist.Length() );
		GraphicManager->Print( NsfInfo.artist, 1, height - 1, RGB(0x20,0xFF,0x20), NsfInfo.artist.Length() );
		height += 12;
	}

	if (NsfInfo.copyright.Length())
	{
		GraphicManager->Print( NsfInfo.copyright, 2, height - 0, RGB(0x20,0x60,0x20), NsfInfo.copyright.Length() );
		GraphicManager->Print( NsfInfo.copyright, 1, height - 1, RGB(0x20,0xFF,0x20), NsfInfo.copyright.Length() );
		height += 12;
	}

	GraphicManager->Print( NsfInfo.song, 2, height - 0, RGB(0x20,0x60,0x20), NsfInfo.song.Length() );
	GraphicManager->Print( NsfInfo.song, 1, height - 1, RGB(0x20,0xFF,0x20), NsfInfo.song.Length() );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnLoadStateSlot(const UINT idm)
{
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

	PDXSTRING string;

	const UINT slot = idm - IDM_FILE_QUICK_LOAD_STATE_SLOT_LAST;

	if (PDX_SUCCEEDED(SaveStateManager->LoadState( slot )))
	{
		string  = "Loaded from slot ";
		string += slot ? slot : SaveStateManager->GetLastSlot();
		string += "..";
	}
	else
	{
		string  = "Failed to load from slot..";
	}

	StartScreenMsg( string, 1000 );

	UpdateFdsMenu();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::OnSaveStateSlot(const UINT idm)
{
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

	PDXSTRING string;

	if (PDX_SUCCEEDED(SaveStateManager->SaveState( idm - IDM_FILE_QUICK_SAVE_STATE_SLOT_NEXT )))
	{
		string  = "Saved to slot ";
		string += SaveStateManager->GetLastSlot();
		string += "..";
	}
	else
	{
		string = "Failed to save to slot..";
	}

	StartScreenMsg( string, 1000 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APPLICATION::UpdateRecentFiles()
{
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

	HMENU hSubMenu;
	hSubMenu = GetSubMenu( GetMenu(),  0 );
	hSubMenu = GetSubMenu( hSubMenu,  13 );

	while (GetMenuItemCount( hSubMenu ))
		DeleteMenu( hSubMenu, 0, MF_BYPOSITION );

	PDXSTRING MenuItemName;

	const UINT NumFiles = PDX_MIN( 10, FileManager->NumRecentFiles() );

	for (UINT i=0; i < NumFiles; ++i)
	{
		MenuItemName  = i;
		MenuItemName += " ";
		MenuItemName += FileManager->GetRecentFile(i);

		AppendMenu( hSubMenu, MF_BYPOSITION, IDM_FILE_RECENT_0 + i,  MenuItemName );
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
