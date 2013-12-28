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

#include <Windows.h>
#include "../paradox/PdxFile.h"
#include "resource/resource.h"
#include "NstPreferences.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT PREFERENCES::Create(CONFIGFILE* const ConfigFile)
{
	if (ConfigFile)
	{
		CONFIGFILE& file = *ConfigFile;

		emulateimmediately = ( file[ "preferences emulate at once"         ] == "no"  ? FALSE : TRUE );
		background         = ( file[ "preferences run in background"       ] == "yes" ? TRUE : FALSE );
		backgroundnsf      = ( file[ "preferences nsf in background"       ] == "no"  ? FALSE : TRUE );
		highpriority       = ( file[ "preferences high priority"           ] == "yes" ? TRUE : FALSE );
		fullscreen         = ( file[ "preferences fullscreen on start"     ] == "yes" ? TRUE : FALSE );
		nowarnings         = ( file[ "preferences warnings"                ] == "no"  ? TRUE : FALSE );
		closepoweroff      = ( file[ "preferences power off on exit"       ] == "yes" ? TRUE : FALSE );
		hidemenu           = ( file[ "preferences hide menu in fullscreen" ] == "yes" ? TRUE : FALSE );
		confirmexit        = ( file[ "preferences confirm exit"            ] == "no"  ? FALSE : TRUE );
		uselogfile         = ( file[ "preferences save logfile"            ] == "yes" ? TRUE : FALSE );
	}
	else
	{
		Reset();
	}

	if ((DefPriority = GetThreadPriority(GetCurrentThread())) == THREAD_PRIORITY_ERROR_RETURN)
		DefPriority = THREAD_PRIORITY_NORMAL;

	SetContext();

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT PREFERENCES::Destroy(CONFIGFILE* const ConfigFile)
{
	if (ConfigFile)
	{
		CONFIGFILE& file = *ConfigFile;

		file[ "preferences emulate at once"         ] = ( emulateimmediately ? "yes" : "no" );
		file[ "preferences run in background"       ] = ( background         ? "yes" : "no" );	  
		file[ "preferences nsf in background"       ] = ( backgroundnsf      ? "yes" : "no" );
		file[ "preferences high priority"           ] = ( highpriority       ? "yes" : "no" );
		file[ "preferences fullscreen on start"     ] = ( fullscreen         ? "yes" : "no" );
		file[ "preferences warnings"                ] = ( nowarnings         ? "no" : "yes" );
		file[ "preferences power off on exit"       ] = ( closepoweroff      ? "yes" : "no" );
		file[ "preferences hide menu in fullscreen" ] = ( hidemenu           ? "yes" : "no" );
		file[ "preferences confirm exit"            ] = ( confirmexit        ? "yes" : "no" );
		file[ "preferences save logfile"            ] = ( uselogfile         ? "yes" : "no" );
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PREFERENCES::Reset()
{
	emulateimmediately = TRUE;
	background         = FALSE;
	backgroundnsf      = TRUE;
	fullscreen         = FALSE;
	nowarnings         = FALSE;
	closepoweroff      = FALSE;
	hidemenu           = FALSE;
	confirmexit        = TRUE;
	uselogfile         = FALSE;
	highpriority       = FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PREFERENCES::UpdateDialog(HWND hDlg)
{
	CheckDlgButton( hDlg, IDC_PREFERENCES_BEGIN_EMULATION,      emulateimmediately ? BST_CHECKED : BST_UNCHECKED );	
	CheckDlgButton( hDlg, IDC_PREFERENCES_RUN_IN_BACKGROUND,    background         ? BST_CHECKED : BST_UNCHECKED );	
	CheckDlgButton( hDlg, IDC_PREFERENCES_NSF_IN_BACKGROUND,    backgroundnsf      ? BST_CHECKED : BST_UNCHECKED );	
	CheckDlgButton( hDlg, IDC_PREFERENCES_HIGH_PRIORITY,        highpriority       ? BST_CHECKED : BST_UNCHECKED );	
	CheckDlgButton( hDlg, IDC_PREFERENCES_STARTUP_FULLSCREEN,   fullscreen         ? BST_CHECKED : BST_UNCHECKED );	
	CheckDlgButton( hDlg, IDC_PREFERENCES_DISABLE_ROM_WARNINGS, nowarnings         ? BST_CHECKED : BST_UNCHECKED );	
	CheckDlgButton( hDlg, IDC_PREFERENCES_CLOSE_POWER_OFF,      closepoweroff      ? BST_CHECKED : BST_UNCHECKED );	
	CheckDlgButton( hDlg, IDC_PREFERENCES_HIDE_MENU_FULLSCREEN, hidemenu           ? BST_CHECKED : BST_UNCHECKED );	
	CheckDlgButton( hDlg, IDC_PREFERENCES_CONFIRM_EXIT,         confirmexit        ? BST_CHECKED : BST_UNCHECKED );	
	CheckDlgButton( hDlg, IDC_PREFERENCES_LOGFILE,              uselogfile         ? BST_CHECKED : BST_UNCHECKED );	
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PREFERENCES::SetContext(HWND hDlg)
{
	if (hDlg)
	{
		emulateimmediately = IsDlgButtonChecked( hDlg, IDC_PREFERENCES_BEGIN_EMULATION      ) == BST_CHECKED ? TRUE : FALSE;	
		background         = IsDlgButtonChecked( hDlg, IDC_PREFERENCES_RUN_IN_BACKGROUND    ) == BST_CHECKED ? TRUE : FALSE;	
		backgroundnsf      = IsDlgButtonChecked( hDlg, IDC_PREFERENCES_NSF_IN_BACKGROUND    ) == BST_CHECKED ? TRUE : FALSE;	
		highpriority       = IsDlgButtonChecked( hDlg, IDC_PREFERENCES_HIGH_PRIORITY        ) == BST_CHECKED ? TRUE : FALSE;	
		fullscreen         = IsDlgButtonChecked( hDlg, IDC_PREFERENCES_STARTUP_FULLSCREEN   ) == BST_CHECKED ? TRUE : FALSE;	
		nowarnings         = IsDlgButtonChecked( hDlg, IDC_PREFERENCES_DISABLE_ROM_WARNINGS ) == BST_CHECKED ? TRUE : FALSE;
		closepoweroff      = IsDlgButtonChecked( hDlg, IDC_PREFERENCES_CLOSE_POWER_OFF      ) == BST_CHECKED ? TRUE : FALSE;
		hidemenu           = IsDlgButtonChecked( hDlg, IDC_PREFERENCES_HIDE_MENU_FULLSCREEN ) == BST_CHECKED ? TRUE : FALSE;
		confirmexit        = IsDlgButtonChecked( hDlg, IDC_PREFERENCES_CONFIRM_EXIT         ) == BST_CHECKED ? TRUE : FALSE;
		uselogfile         = IsDlgButtonChecked( hDlg, IDC_PREFERENCES_LOGFILE              ) == BST_CHECKED ? TRUE : FALSE;
	}

	SetThreadPriority( GetCurrentThread(), highpriority ? THREAD_PRIORITY_HIGHEST : DefPriority );

	NES::IO::GENERAL::CONTEXT context;
	nes->GetGeneralContext( context );
	context.DisableWarnings = nowarnings;
	nes->SetGeneralContext( context );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL PREFERENCES::DialogProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM)
{
	switch (uMsg) 
	{
     	case WM_INITDIALOG:

			UpdateDialog( hDlg );
			return TRUE;
			
		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDC_PREFERENCES_OK:

					SetContext( hDlg );

				case IDC_PREFERENCES_CANCEL:

					EndDialog( hDlg, 0 );
					return TRUE;

				case IDC_PREFERENCES_DEFAULT:

					Reset();
					UpdateDialog( hDlg );
					return TRUE;
			}
			return FALSE;

     	case WM_CLOSE:

     		EndDialog( hDlg, 0 );
     		return TRUE;
	}

	return FALSE;
}
