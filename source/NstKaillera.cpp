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
#include "NstKaillera.h"
#include "windows/resource/resource.h"
#include "windows/NstUtilities.h"
#include "windows/NstApplication.h"
#include "windows/NstSoundManager.h"
#include "windows/NstPreferences.h"
#include "windows/NstNetplayManager.h"
#include "windows/NstGraphicManager.h"
#include "core/NstRomDatabase.h"
#include "paradox/PdxCrc32.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#define NST_IDM_NETPLAY_POS_MACHINE 1
#define NST_IDM_NETPLAY_POS_FDS     2
#define NST_IDM_NETPLAY_POS_VIEW    3

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

KAILLERA::KAILLERA()
: connected(FALSE), hAccel(NULL)
{
	if (hDLL = LoadLibrary("KailleraClient.dll"))
	{
		const BOOL AllSet = 
		(
			ImportFunction( kailleraGetVersion,		    "_kailleraGetVersion@4"         ) &&
			ImportFunction( kailleraInit,			    "_kailleraInit@0"               ) &&
			ImportFunction( kailleraShutdown,		    "_kailleraShutdown@0"           ) &&
			ImportFunction( kailleraSetInfos,		    "_kailleraSetInfos@4"           ) &&
			ImportFunction( kailleraSelectServerDialog, "_kailleraSelectServerDialog@4" ) &&
			ImportFunction( kailleraModifyPlayValues,   "_kailleraModifyPlayValues@8"   ) &&
			ImportFunction( kailleraChatSend,		    "_kailleraChatSend@4"           ) &&
			ImportFunction( kailleraEndGame,		    "_kailleraEndGame@0"            )
		);

		if (AllSet)
		{
			kailleraInit();

			CHAR version[16+1];
			version[0] = '\0';

			kailleraGetVersion( version );

			if (strlen( version ))
			{
				LOGFILE::Output("KAILLERA: loaded \"KailleraClient.dll\" version ",version,".");

				if (strcmpi( version, "0.9" ))
					LOGFILE::Output("KAILLERA: warning, the loaded dll file may be incompatible with Nestopia!");
			}
			else
			{
				LOGFILE::Output("KAILLERA: warning, dll file version couldn't be detected!");
			}

			return;
		}

		::FreeLibrary( hDLL );
		hDLL = NULL;
	}

	LOGFILE::Output("KAILLERA: file \"KailleraClient.dll\" not found or initialization failed. netplay will be disabled.");
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

template<class FPTR> 
BOOL KAILLERA::ImportFunction(FPTR& fptr,const CHAR* const address) const
{
	return (fptr = PDX_CAST(FPTR,::GetProcAddress( hDLL, address ))) != NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

KAILLERA::~KAILLERA()
{
	PDX_ASSERT( !hAccel );

	if (hDLL)
	{
		kailleraShutdown();
		::FreeLibrary( hDLL );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID KAILLERA::Launch(FILELIST& FileList,const BOOL UseDatabaseNames,const BOOL p)
{
	PDX_ASSERT( hDLL );

	PlayFullscreen = p;

	UpdateGameList( FileList, UseDatabaseNames );

	if (GameList.IsEmpty())
	{
		UI::MsgError( IDS_NETPLAY_NO_GAMELIST );
		return;
	}

	application.GetGraphicManager().EnableGDI( TRUE );

	{
		PDXARRAY<CHAR> GameStrings;

		for (GAMELIST::CONSTITERATOR it(GameList.Begin()); it != GameList.End(); ++it)
			GameStrings.InsertBack( (*it).First().Begin(), (*it).First().End() + 1 );

		GameStrings.InsertBack( '\0' );

		kailleraInfos kInfos;

		kInfos.appName               = "Nestopia 1.09";
		kInfos.gameList              = GameStrings.Begin();
		kInfos.gameCallback          = GameProc;
		kInfos.chatReceivedCallback  = ChatRecievedProc;
		kInfos.clientDroppedCallback = ClientDroppedProc;
		kInfos.moreInfosCallback     = NULL;

		kailleraSetInfos( &kInfos );
	}

	// Add a hook for monitoring the Kaillera windows activity. The bug seems to be located in the Kaillera 
	// code so I have to resolve to some dirty hacks to prevent the message queue from entering an infinite 
	// loop. This will happen if the user tries to close the main server list window when others are open.

	hHook = ::SetWindowsHookEx
	( 
       	WH_GETMESSAGE, 
		GetMsgProc, 
		UTILITIES::GetInstance(), 
		::GetCurrentThreadId() 
	);

	try
	{
		kailleraSelectServerDialog( application.GetHWnd() );
	}
	catch (...)
	{
		UI::MsgError( IDS_NETPLAY_KAILLERA_ERROR );
	}

	if (hHook)
		::UnhookWindowsHookEx( hHook );

	application.GetGraphicManager().EnableGDI( FALSE );

	GameList.Destroy();
}

////////////////////////////////////////////////////////////////////////////////////////
// Kaillera bug remover
////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK KAILLERA::GetMsgProc(INT iCode,WPARAM wParam,LPARAM lParam)
{
	if (iCode == HC_ACTION)
	{
		MSG& msg = *PDX_CAST(LPMSG,lParam);

		if (msg.message == WM_CLOSE && msg.hwnd != application.GetHWnd())
		{
			KAILLERAWINDOWS KailleraWindows;
			FindKailleraWindows( KailleraWindows );

			if (KailleraWindows.Size() > 1)
			{
				for (TSIZE i=0; i < KailleraWindows.Size(); ++i)
				{
					// see if this is the parent window
					if (msg.hwnd != KailleraWindows[i] && msg.hwnd == ::GetParent( KailleraWindows[i] ))
					{
						// stop the insanity!
						msg.message = WM_NULL;
						break;
					}
				}
			}
		}
	}

	return ::CallNextHookEx
	( 
       	application.GetNetplayManager().Kaillera().hHook, 
		iCode, 
		wParam, 
		lParam
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID KAILLERA::UpdateGameList(FILELIST& FileList,const BOOL UseDatabaseNames)
{
	GameList.Destroy();

	PDXSTRING GameName;

	for (TSIZE i=0; i < FileList.Size(); ++i)
	{
		GameName.Clear();

		if (UseDatabaseNames)
			FindDatabaseName( FileList[i], GameName );

		if (GameName.IsEmpty())
			FileList[i].GetFileName( GameName );

		if (GameName.Length())
			GameList[GameName] = FileList[i].String();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID KAILLERA::FindDatabaseName(const PDXSTRING& FileName,PDXSTRING& GameName)
{
	PDXFILE file( FileName, PDXFILE::INPUT );

	if (!file.IsOpen())
		return;

   #pragma pack(push,1)

	struct HEADER
	{
		U32 signature;
		U8  Num16kPRomBanks;
		U8  Num8kCRomBanks;
		U16 other1;
		U32 other2;
		U32 other3;
	};

   #pragma pack(pop)

	if (file.Size() < sizeof(HEADER))
		return;

	const HEADER& header = *PDX_CAST(const HEADER*,file.At(0));

	if (header.signature != 0x1A53454EUL)
		return;

	TSIZE length = file.Size() - sizeof(HEADER);

	if (!length)
		return;
	
	NES::ROMDATABASE::HANDLE dBaseHandle = application.GetNes().GetRomDatabase().GetHandle
	( 
     	PDXCRC32::Compute( file.At(sizeof(HEADER)), length )
	);

	if (!dBaseHandle)
	{
		length = (NES::n16k * header.Num16kPRomBanks) + (NES::n8k * header.Num8kCRomBanks);

		if (length && length <= (file.Size() - sizeof(HEADER)))
		{
			dBaseHandle = application.GetNes().GetRomDatabase().GetHandle
			( 
		     	PDXCRC32::Compute( file.At(sizeof(HEADER)), length ) 
			);
		}
	}

	if (dBaseHandle)
		GameName = application.GetNes().GetRomDatabase().Name( dBaseHandle );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

INT KAILLERA::StartNetPlay(CHAR* game,const INT player,const INT num)
{
	InitInput( player, num );

	SETTINGS settings;
	KAILLERAWINDOWS KailleraWindows;

	HWND hWnd = application.GetHWnd();

	try
	{
		PushKaillera( KailleraWindows );

		if (PlayFullscreen && application.IsWindowed())
		{
			settings.SwitchedScreen = TRUE;
			::SendMessage( hWnd, WM_COMMAND, IDM_VIEW_SWITCH_SCREEN, 0 );
		}

		LoadGame( game );
		PushSettings( settings, player );
		PushAccelerator( player );

		::EnableWindow( hWnd, TRUE );
		::SetForegroundWindow( hWnd );

		connected = TRUE;
		UpdateCounter = 0;
		command = 0;

		MainLoop();
	}
	catch (KAILLERAERROR error)
	{
		if (error.code == KAILLERAERROR::GENERIC)
			UI::MsgError( IDS_NETPLAY_INIT_ERROR );
	}
	catch (...)
	{
		UI::MsgError( IDS_NETPLAY_KAILLERA_ERROR );
	}

	connected = FALSE;

	ChatWindow.Close();
	
	PopAccelerator();
	PopSettings( settings, player );

	if (settings.SwitchedScreen)
		::SendMessage( hWnd, WM_COMMAND, IDM_VIEW_SWITCH_SCREEN, 0 );

	PopKaillera( KailleraWindows );
	kailleraEndGame();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID KAILLERA::PushAccelerator(const INT player)
{
	HACCEL hOld = application.GetHAccel();
	
	INT count = ::CopyAcceleratorTable( hOld, NULL, 0 );
	PDX_ASSERT( count > 0 );

	PDXARRAY<ACCEL> OldTable;
	OldTable.Resize( count );

	count = ::CopyAcceleratorTable( hOld, OldTable.Begin(), count );
	PDX_ASSERT( count > 0 );

	PDXARRAY<ACCEL>	NewTable;

	const BOOL UseFDS = (application.GetNes().IsFds() && player == 1);

	for (UINT i=0; i < count; ++i)
	{
		UINT cmd = OldTable[i].cmd;

		switch (cmd)
		{
     		case IDM_FILE_QUIT:
       		case IDM_VIEW_WINDOWSIZE_MAX:
       		case IDM_VIEW_WINDOWSIZE_1X: 
       		case IDM_VIEW_WINDOWSIZE_2X: 
     		case IDM_VIEW_WINDOWSIZE_4X: 
       		case IDM_VIEW_WINDOWSIZE_8X: 
       		case IDM_VIEW_MENU: 
			case IDM_NETPLAY_CHATWINDOW:
			case IDM_NETPLAY_DISCONNECT:
				break;

			case IDM_MACHINE_RESET_SOFT: 

				cmd = (player == 1);
				break;

			case IDM_FDS_EJECT_DISK:     
			case IDM_FDS_SIDE_A:         
			case IDM_FDS_SIDE_B:         
			case IDM_FDS_OPTIONS:        

				cmd = UseFDS;
				break;

			default:

				cmd = (UseFDS && (cmd >= IDM_FDS_INSERT_DISK_1 && cmd <= IDM_FDS_INSERT_DISK_16));
				break;
		}

		if (cmd)
			NewTable.InsertBack( OldTable[i] );
	}

	PDX_ASSERT( !hAccel );

	if (NewTable.Size())
		hAccel = ::CreateAcceleratorTable( NewTable.Begin(), NewTable.Size() ); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID KAILLERA::PopAccelerator()
{
	if (hAccel)
	{
		::DestroyAcceleratorTable( hAccel );
		hAccel = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID KAILLERA::MainLoop()
{
	MSG msg;	
	msg.message = WM_NULL;
	::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE );

	while (msg.message != WM_QUIT)
	{
		if (::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ))
		{
			const BOOL translate =
			(
       			!ChatWindow.IsDlgMsg( msg ) && 
				!::TranslateAccelerator( application.GetHWnd(), hAccel, &msg )
			);

			if (translate)
			{
				::TranslateMessage( &msg );
				::DispatchMessage( &msg );
			}
		}
		else
		{
			application.ExecuteImage();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID KAILLERA::InitInput(const INT player,const INT num)
{
	NumPlayers = PDX_CLAMP(num,1,4);

	NES::MACHINE& nes = application.GetNes();

	GetInput = GetNone;

	if (player >= 1 && player <= 4)
	{
		switch (nes.ConnectedController( player-1 ))
		{
     		case NES::CONTROLLER_PAD1: GetInput = GetPad0; break;
     		case NES::CONTROLLER_PAD2: GetInput = GetPad1; break;       
     		case NES::CONTROLLER_PAD3: GetInput = GetPad2; break;
       		case NES::CONTROLLER_PAD4: GetInput = GetPad3; break;
		}
	}

	for (UINT i=0; i < 4; ++i)
	{
		switch (nes.ConnectedController( i ))
		{
     		case NES::CONTROLLER_PAD1: SetInput[i] = SetPad0; break;
     		case NES::CONTROLLER_PAD2: SetInput[i] = SetPad1; break;       
     		case NES::CONTROLLER_PAD3: SetInput[i] = SetPad2; break;
     		case NES::CONTROLLER_PAD4: SetInput[i] = SetPad3; break;
    		default:                   SetInput[i] = SetNone; break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID KAILLERA::PushKaillera(KAILLERAWINDOWS& KailleraWindows)
{
	PDX_ASSERT( KailleraWindows.IsEmpty() );

	FindKailleraWindows( KailleraWindows );

	if (KailleraWindows.IsEmpty())
		throw KAILLERAERROR(KAILLERAERROR::GENERIC);

	for (TSIZE i=0; i < KailleraWindows.Size(); ++i)
	{
		PDX_ASSERT(::IsWindow( KailleraWindows[i] ));
		::ShowWindow( KailleraWindows[i], SW_HIDE );
	}

	application.GetGraphicManager().EnableGDI( FALSE );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID KAILLERA::PopKaillera(const KAILLERAWINDOWS& KailleraWindows)
{
	if (KailleraWindows.IsEmpty())
		return;
	
	application.GetGraphicManager().EnableGDI( TRUE );

	for (TSIZE i=0; i < KailleraWindows.Size(); ++i)
	{
		PDX_ASSERT(::IsWindow( KailleraWindows[i] ));

		if (::IsWindow( KailleraWindows[i] ))
			::ShowWindow( KailleraWindows[i], SW_SHOW );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID KAILLERA::PushSettings(SETTINGS& settings,const INT player)
{
	settings.menu = application.GetMenu();

	settings.NoStatusBar = 
	(
     	application.IsWindowed() &&
		settings.menu.IsUnchecked( IDM_VIEW_STATUSBAR )
	);

	HWND hWnd = application.GetHWnd();

	if (settings.NoStatusBar)
		::SendMessage( hWnd, WM_COMMAND, IDM_VIEW_STATUSBAR, 0 );

	settings.RunInBackground = (application.GetPreferences().SetRunInBackground( TRUE ) ? 1 : 0);
	settings.ConfirmReset = (application.GetPreferences().SetConfirmReset( FALSE ) ? 1 : 0);
	settings.WinProc = ::SetWindowLongPtr( hWnd, GWLP_WNDPROC, PDX_CAST(LONG_PTR,WndProc) );

	if (!settings.WinProc)
		throw KAILLERAERROR(KAILLERAERROR::GENERIC);

   #define NST_M_(a_,b_) a_ = (settings.menu.Disable( b_ ) ? 1 : 0)

	if (player == 1)
	{
		NST_M_( settings.MenuItems[0x00], IDM_MACHINE_RESET_HARD           );
		NST_M_( settings.MenuItems[0x01], IDM_MACHINE_PAUSE                );
		NST_M_( settings.MenuItems[0x02], IDM_MACHINE_AUTOSELECTCONTROLLER );
		NST_M_( settings.MenuItems[0x03], IDM_FDS_OPTIONS                  );
	}

	NST_M_( settings.MenuItems[0x04], IDM_VIEW_ROM_INFO      );
	NST_M_( settings.MenuItems[0x05], IDM_VIEW_LOGFILE       );
	NST_M_( settings.MenuItems[0x06], IDM_VIEW_SWITCH_SCREEN );

   #undef NST_M_
   #define NST_M_(a_,b_,c_) a_ = (settings.menu.GetSub( b_ ).Disable( c_, NSTMENU::POS ) ? 1 : 0)

	if (player == 1)
	{
		NST_M_( settings.MenuItems[0x07], NST_IDM_POS_MACHINE, NST_IDM_POS_MACHINE_POWER   );
		NST_M_( settings.MenuItems[0x08], NST_IDM_POS_MACHINE, NST_IDM_POS_MACHINE_PORT_1  );
		NST_M_( settings.MenuItems[0x09], NST_IDM_POS_MACHINE, NST_IDM_POS_MACHINE_PORT_2  );
		NST_M_( settings.MenuItems[0x0A], NST_IDM_POS_MACHINE, NST_IDM_POS_MACHINE_PORT_3  );
		NST_M_( settings.MenuItems[0x0B], NST_IDM_POS_MACHINE, NST_IDM_POS_MACHINE_PORT_4  );
		NST_M_( settings.MenuItems[0x0C], NST_IDM_POS_MACHINE, NST_IDM_POS_MACHINE_PORT_5  );
		NST_M_( settings.MenuItems[0x0D], NST_IDM_POS_MACHINE, NST_IDM_POS_MACHINE_MODE    );
		NST_M_( settings.MenuItems[0x0E], NST_IDM_POS_MACHINE, NST_IDM_POS_MACHINE_OPTIONS );
	}
	
	NST_M_( settings.MenuItems[0x0F], NST_IDM_POS_VIEW, NST_IDM_POS_VIEW_SHOW );

   #undef NST_M_

	menu.Load( IDR_NETPLAY_MENU );
	menu.MakeModeless();	

	{
		PDXSTRING title;

		if (player == 1)
		{
			menu.InsertSub( settings.menu.GetSub( NST_IDM_POS_MACHINE ), settings.menu.GetText( NST_IDM_POS_MACHINE, title, NSTMENU::POS ) );
			menu.InsertSub( settings.menu.GetSub( NST_IDM_POS_FDS     ), settings.menu.GetText( NST_IDM_POS_FDS,     title, NSTMENU::POS ) );
		}

		menu.InsertSub( settings.menu.GetSub( NST_IDM_POS_VIEW ), settings.menu.GetText( NST_IDM_POS_VIEW, title, NSTMENU::POS ) );
	}

	application.SwitchMenu( menu );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID KAILLERA::PopSettings(SETTINGS& settings,const INT player)
{
	if (menu.IsSet())
	{
		PDX_ASSERT( settings.menu.IsSet() );

		if (player == 1)
		{
			menu.Remove( NST_IDM_NETPLAY_POS_MACHINE - 0, NSTMENU::POS );
			menu.Remove( NST_IDM_NETPLAY_POS_FDS - 1, NSTMENU::POS );
		}

		menu.Remove( NST_IDM_NETPLAY_POS_VIEW - 2, NSTMENU::POS );

       #define NST_M_(a_,b_) if (a_ != -1) settings.menu.Enable( b_, a_ )

		if (player == 1)
		{
			NST_M_( settings.MenuItems[0x00], IDM_MACHINE_RESET_HARD           );
			NST_M_( settings.MenuItems[0x01], IDM_MACHINE_PAUSE                );
			NST_M_( settings.MenuItems[0x02], IDM_MACHINE_AUTOSELECTCONTROLLER );
			NST_M_( settings.MenuItems[0x03], IDM_FDS_OPTIONS                  );
		}

		NST_M_( settings.MenuItems[0x04], IDM_VIEW_ROM_INFO      );
		NST_M_( settings.MenuItems[0x05], IDM_VIEW_LOGFILE       );
		NST_M_( settings.MenuItems[0x06], IDM_VIEW_SWITCH_SCREEN );

       #undef NST_M_
       #define NST_M_(a_,b_,c_) settings.menu.GetSub( b_ ).Enable( c_, a_, NSTMENU::POS )

		if (player == 1)
		{
			NST_M_( settings.MenuItems[0x07], NST_IDM_POS_MACHINE, NST_IDM_POS_MACHINE_POWER   );
			NST_M_( settings.MenuItems[0x08], NST_IDM_POS_MACHINE, NST_IDM_POS_MACHINE_PORT_1  );
			NST_M_( settings.MenuItems[0x09], NST_IDM_POS_MACHINE, NST_IDM_POS_MACHINE_PORT_2  );
			NST_M_( settings.MenuItems[0x0A], NST_IDM_POS_MACHINE, NST_IDM_POS_MACHINE_PORT_3  );
			NST_M_( settings.MenuItems[0x0B], NST_IDM_POS_MACHINE, NST_IDM_POS_MACHINE_PORT_4  );
			NST_M_( settings.MenuItems[0x0C], NST_IDM_POS_MACHINE, NST_IDM_POS_MACHINE_PORT_5  );
			NST_M_( settings.MenuItems[0x0D], NST_IDM_POS_MACHINE, NST_IDM_POS_MACHINE_MODE    );
			NST_M_( settings.MenuItems[0x0E], NST_IDM_POS_MACHINE, NST_IDM_POS_MACHINE_OPTIONS );
		}

		NST_M_( settings.MenuItems[0x0F], NST_IDM_POS_VIEW, NST_IDM_POS_VIEW_SHOW );

       #undef NST_M_

		application.SwitchMenu( settings.menu );	
		menu.Destroy();
	}

	HWND hWnd = application.GetHWnd();

	if (settings.WinProc)
		::SetWindowLongPtr( hWnd, GWLP_WNDPROC, settings.WinProc );

	if (settings.NoStatusBar)
		::SendMessage( hWnd, WM_COMMAND, IDM_VIEW_STATUSBAR, 0 );

	if (settings.RunInBackground != -1)
		application.GetPreferences().SetRunInBackground( settings.RunInBackground );

	if (settings.ConfirmReset != -1)
		application.GetPreferences().SetConfirmReset( settings.ConfirmReset );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID KAILLERA::LoadGame(const CHAR* const game)
{
	PDXSTRING file(game);
	GAMELIST::CONSTITERATOR it(GameList.Find(file));

	if (it == GameList.End())
		throw KAILLERAERROR(KAILLERAERROR::LOADGAME);

	file = (*it).Second();

	COPYDATASTRUCT cds;

	cds.dwData = NST_WM_OPEN_FILE;
	cds.lpData = PDX_CAST(PVOID,file.Begin());
	cds.cbData = file.Length();

	HWND hWnd = application.GetHWnd();
	::SendMessage( hWnd, WM_COPYDATA, PDX_CAST(WPARAM,hWnd), PDX_CAST(LPARAM,&cds) );

	if (!application.GetNes().IsOn() || application.GetNes().IsNsf())
		throw KAILLERAERROR(KAILLERAERROR::LOADGAME);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID KAILLERA::FindKailleraWindows(KAILLERAWINDOWS& KailleraWindows)
{
	struct ENUMKAILLERA
	{
		static BOOL CALLBACK Find(HWND hWnd,LPARAM lParam)
		{
			if (application.GetHWnd() == hWnd)
				return TRUE;

			CHAR name[9+1];
			name[0] = '\0';

			if (::GetWindowText( hWnd, name, 9+1 ) == 9 && !strcmpi( "Kaillera ", name ))
				PDX_CAST(KAILLERAWINDOWS*,lParam)->InsertBack( hWnd );

			return TRUE;
		}
	};

	::EnumThreadWindows
	( 
       	::GetCurrentThreadId(), 
		ENUMKAILLERA::Find, 
		PDX_CAST(LPARAM,&KailleraWindows) 
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

UINT KAILLERA::GetPad0     (const NES::IO::INPUT& input) { return input.pad[0].buttons; }
UINT KAILLERA::GetPad1     (const NES::IO::INPUT& input) { return input.pad[1].buttons; }
UINT KAILLERA::GetPad2     (const NES::IO::INPUT& input) { return input.pad[2].buttons; }
UINT KAILLERA::GetPad3     (const NES::IO::INPUT& input) { return input.pad[3].buttons; }
UINT KAILLERA::GetZapper   (const NES::IO::INPUT& input) { return 0;                    }
UINT KAILLERA::GetPaddle   (const NES::IO::INPUT& input) { return 0;                    }
UINT KAILLERA::GetPowerPad (const NES::IO::INPUT& input) { return 0;                    }
UINT KAILLERA::GetNone     (const NES::IO::INPUT& input) { return 0;                    }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID KAILLERA::SetPad0     (NES::IO::INPUT& input,const UINT data) { input.pad[0].buttons = data; }
VOID KAILLERA::SetPad1     (NES::IO::INPUT& input,const UINT data) { input.pad[1].buttons = data; }
VOID KAILLERA::SetPad2     (NES::IO::INPUT& input,const UINT data) { input.pad[2].buttons = data; }
VOID KAILLERA::SetPad3     (NES::IO::INPUT& input,const UINT data) { input.pad[3].buttons = data; }
VOID KAILLERA::SetZapper   (NES::IO::INPUT& input,const UINT data) {                              }
VOID KAILLERA::SetPaddle   (NES::IO::INPUT& input,const UINT data) {                              }
VOID KAILLERA::SetPowerPad (NES::IO::INPUT& input,const UINT data) {                              }
VOID KAILLERA::SetNone     (NES::IO::INPUT& input,const UINT data) {                              }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID KAILLERA::Update(NES::IO::INPUT& input)
{
	PDX_ASSERT( connected );

	if (input.vs.InsertCoin)
	{
		if (!command)
			OnInsertCoin( input.vs.InsertCoin );

		input.vs.InsertCoin = 0;
	}

	const UINT values = command ? command : GetInput( input );
	
	for (;;)
	{
		packets[0].data[0] = values;
		packets[0].data[1] = UpdateCounter;

		if (command)
			packets[0].data[1] |= PACKET::COMMAND_MODE;

		const INT length = kailleraModifyPlayValues( packets, PACKET::SIZE );

		if (length > 0)
		{
			for (UINT i=0; i < NumPlayers; ++i)
			{
				if ((packets[i].data[1] & PACKET::COUNTER) != UpdateCounter)
					continue;
			}
			break;
		}
		else if (length == -1)
		{
			Unconnect();
			break;
		}
	}

	UpdateCounter = (UpdateCounter + 1) & PACKET::COUNTER;

	if (packets[0].data[1] & PACKET::COMMAND_MODE)
	{
		command = 0;
		OnPacketCommand( input );
	}
	else
	{
		for (UINT i=0; i < 4; ++i)
			SetInput[i]( input, packets[i].data[0] );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID KAILLERA::OnPacketCommand(NES::IO::INPUT& input)
{
	UINT data = packets[0].data[0] >> PACKET::COMMAND_DATA_SHIFT;

	switch (packets[0].data[0] & PACKET::COMMAND)
	{
     	case PACKET::COMMAND_RESET:       
			
			application.OnReset( FALSE );        
			break;

     	case PACKET::COMMAND_INSERT_DISK: 
			
			data += IDM_FDS_INSERT_DISK_1;
			application.OnFdsInsertDisk( data ); 
			break;

    	case PACKET::COMMAND_EJECT_DISK:  
			
			application.OnFdsEjectDisk();      
			break;

     	case PACKET::COMMAND_DISK_SIDE:   
	
			data = data ? IDM_FDS_SIDE_B : IDM_FDS_SIDE_A;
			application.OnFdsSide( data );       
			break;

		case PACKET::COMMAND_INSERT_COIN:

			input.vs.InsertCoin = 
			(
     			((data & 0x1) ? NES::IO::INPUT::VS::COIN_1 : 0) |
				((data & 0x2) ? NES::IO::INPUT::VS::COIN_2 : 0)
			);
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID KAILLERA::OnInsertCoin(const UINT data)
{
	PDX_ASSERT( data );

	command = 
	(
     	PACKET::COMMAND_INSERT_COIN |
		((data & NES::IO::INPUT::VS::COIN_1) ? (0x1 << PACKET::COMMAND_DATA_SHIFT): 0x0) |
		((data & NES::IO::INPUT::VS::COIN_2) ? (0x2 << PACKET::COMMAND_DATA_SHIFT): 0x0) 
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID KAILLERA::Unconnect()
{
	connected = FALSE;
	::PostQuitMessage(0);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK KAILLERA::WndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return application.GetNetplayManager().Kaillera().MsgProc( hWnd, uMsg, wParam, lParam );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

LRESULT KAILLERA::MsgProc(const HWND hWnd,const UINT uMsg,const WPARAM wParam,const LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_COMMAND:

			if (OnCommand( wParam ))
				return 0;

			break;

		case WM_RBUTTONDOWN:

			OnToggleMenu();
			return 0;

		case WM_ENTERMENULOOP:

			menu.Check( IDM_NETPLAY_CHATWINDOW, ChatWindow.IsActive() );

		case WM_ENTERSIZEMOVE:
		case WM_EXITSIZEMOVE:
		case WM_EXITMENULOOP:
		case WM_COPYDATA:
		case WM_DROPFILES:
			return 0;

		case WM_CLOSE:

			Unconnect();
			return 0;
	}

	return application.MsgProc( hWnd, uMsg, wParam, lParam );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID KAILLERA::OnToggleMenu()
{
	application.OnToggleMenu();

	if (!application.IsWindowed())
	{
		HWND hChat = ChatWindow.GetHWnd();

		if (hChat)
			::ShowWindow( hChat, ::IsWindowVisible( hChat ) ? SW_HIDE : SW_SHOW );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL KAILLERA::OnCommand(const WPARAM wParam)
{
	const UINT idm = LOWORD(wParam);

	switch (idm)
	{
     	case IDM_VIEW_MENU:

			OnToggleMenu();
			return TRUE;

       	case IDM_NETPLAY_CHATWINDOW:

       		if (ChatWindow.IsActive())
       			ChatWindow.Close();
     		else
       			ChatWindow.Open( ChatSendProc );

	     	return TRUE;

       	case IDM_MACHINE_RESET_SOFT:

			command = PACKET::COMMAND_RESET;
			return TRUE;

		case IDM_FDS_EJECT_DISK:

			command = PACKET::COMMAND_EJECT_DISK;
			return TRUE;

		case IDM_FDS_SIDE_A:    

			command = PACKET::COMMAND_DISK_SIDE | (0 << PACKET::COMMAND_DATA_SHIFT);
			return TRUE;

		case IDM_FDS_SIDE_B:    

			command = PACKET::COMMAND_DISK_SIDE | (1 << PACKET::COMMAND_DATA_SHIFT);
			return TRUE;

     	case IDM_NETPLAY_DISCONNECT:

			Unconnect();
     		return TRUE;
	}

	if (idm >= IDM_FDS_INSERT_DISK_1 && idm <= IDM_FDS_INSERT_DISK_16)
	{
		command = 
		(
	     	PACKET::COMMAND_INSERT_DISK |
			((idm - IDM_FDS_INSERT_DISK_1) << PACKET::COMMAND_DATA_SHIFT)
		);
		return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
// Callback Procedures
////////////////////////////////////////////////////////////////////////////////////////

VOID KAILLERA::ChatSendProc(PDXSTRING& text)
{
	application.GetNetplayManager().Kaillera().kailleraChatSend( text.Begin() );
}

INT WINAPI KAILLERA::GameProc(CHAR* game,INT player,INT NumPlayers)
{
	return application.GetNetplayManager().Kaillera().StartNetPlay( game, player, NumPlayers );
}

VOID WINAPI KAILLERA::ChatRecievedProc(CHAR* nick,CHAR* text)
{
	application.StartScreenMsg( 5000, nick, " says: ", text );
}

VOID WINAPI KAILLERA::ClientDroppedProc(CHAR* nick,INT player)
{
	application.StartScreenMsg( 5000, "Player ", player, "(", nick, ") dropped out.." );
}

