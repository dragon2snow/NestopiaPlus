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

#include <ShLwApi.h>
#include <ShlObj.h>
#include "../paradox/PdxFile.h"
#include "NstFileManager.h"
#include "NstPreferences.h"
#include "NstApplication.h"

#pragma comment(lib,"ShlwApi")

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PREFERENCES::PREFERENCES()
: 
MANAGER(IDD_PREFERENCES)
{
	DefaultPriority = ::GetThreadPriority( ::GetCurrentThread() );

	if (DefaultPriority == THREAD_PRIORITY_ERROR_RETURN)
		DefaultPriority = THREAD_PRIORITY_NORMAL;

	PDX_COMPILE_ASSERT
	(
		( IDC_PREFERENCES_ASSOCIATE_UNF - IDC_PREFERENCES_ASSOCIATE_NES ) == 1 &&  
		( IDC_PREFERENCES_ASSOCIATE_FDS - IDC_PREFERENCES_ASSOCIATE_NES ) == 2 &&  
		( IDC_PREFERENCES_ASSOCIATE_NSF - IDC_PREFERENCES_ASSOCIATE_NES ) == 3 &&  
		( IDC_PREFERENCES_ASSOCIATE_NSP - IDC_PREFERENCES_ASSOCIATE_NES ) == 4 &&
		NUM_FILE_TYPES == 5
	);

	associations[FILE_NES].Set( ".nes", "Nestopia iNes File",                2 );  
	associations[FILE_UNF].Set( ".unf", "Nestopia UNIF File",                3 );  
	associations[FILE_FDS].Set( ".fds", "Nestopia Famicom Disk System File", 4 );  
	associations[FILE_NSF].Set( ".nsf", "Nestopia NES Sound File",           5 );  
	associations[FILE_NSP].Set( ".nsp", "Nestopia Script File",              6 );  

	Reset();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PREFERENCES::Create(CONFIGFILE* const ConfigFile)
{
	if (ConfigFile)
	{
		CONFIGFILE& file = *ConfigFile;

		emulateimmediately = ( file[ "preferences emulate at once"          ] == "no"  ? FALSE : TRUE );
		background         = ( file[ "preferences run in background"        ] == "yes" ? TRUE : FALSE );
		prioritycontrol    = ( file[ "preferences auto priority control"    ] == "no"  ? FALSE : TRUE );
		fullscreen         = ( file[ "preferences fullscreen on start"      ] == "yes" ? TRUE : FALSE );
		nowarnings         = ( file[ "preferences warnings"                 ] == "no"  ? TRUE : FALSE );
		closepoweroff      = ( file[ "preferences power off on exit"        ] == "yes" ? TRUE : FALSE );
		confirmexit        = ( file[ "preferences confirm exit"             ] == "no"  ? FALSE : TRUE );
		confirmreset       = ( file[ "preferences confirm machine reset"    ] == "yes" ? TRUE : FALSE );
		usedatabase        = ( file[ "preferences use rom database"         ] == "no"  ? FALSE : TRUE );
		multipleinstances  = ( file[ "preferences allow multiple instances" ] == "yes" ? TRUE : FALSE );
		savelogfile        = ( file[ "preferences save logfile"             ] == "yes" ? TRUE : FALSE );
		savesettings       = ( file[ "preferences save settings"            ] == "no"  ? FALSE : TRUE );
		savelauncher       = ( file[ "preferences save launcher files"      ] == "no"  ? FALSE : TRUE );
	}
	else
	{
		Reset();
	}

	for (UINT i=0; i < NUM_FILE_TYPES; ++i)
		UpdateAssociation( associations[i] );

	SetContext();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PREFERENCES::Destroy(CONFIGFILE* const ConfigFile)
{
	if (ConfigFile)
	{
		CONFIGFILE& file = *ConfigFile;

		file[ "preferences emulate at once"          ] = ( emulateimmediately ? "yes" : "no" );
		file[ "preferences run in background"        ] = ( background         ? "yes" : "no" );	  
		file[ "preferences auto priority control"    ] = ( prioritycontrol    ? "yes" : "no" );
		file[ "preferences fullscreen on start"      ] = ( fullscreen         ? "yes" : "no" );
		file[ "preferences warnings"                 ] = ( nowarnings         ? "no" : "yes" );
		file[ "preferences power off on exit"        ] = ( closepoweroff      ? "yes" : "no" );
		file[ "preferences confirm exit"             ] = ( confirmexit        ? "yes" : "no" );
		file[ "preferences confirm machine reset"    ] = ( confirmreset       ? "yes" : "no" );
		file[ "preferences use rom database"         ] = ( usedatabase        ? "yes" : "no" );
		file[ "preferences allow multiple instances" ] = ( multipleinstances  ? "yes" : "no" );
		file[ "preferences save logfile"             ] = ( savelogfile        ? "yes" : "no" );
		file[ "preferences save settings"            ] = ( savesettings       ? "yes" : "no" );
		file[ "preferences save launcher files"      ] = ( savelauncher       ? "yes" : "no" );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PREFERENCES::Reset()
{
	emulateimmediately = TRUE;
	background         = FALSE;
	fullscreen         = FALSE;
	nowarnings         = FALSE;
	closepoweroff      = FALSE;
	confirmexit        = TRUE;
	confirmreset       = FALSE;
	prioritycontrol    = TRUE;
	usedatabase        = TRUE;
	multipleinstances  = FALSE;
	savelogfile        = FALSE;
	savesettings       = TRUE;
	savelauncher       = TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PREFERENCES::UpdateAssociation(ASSOCIATION& association)
{
	PDXSTRING IdString("Nestopia");
	IdString += association.extension;

	association.enabled = FALSE;

	HKEY hKey;

	// open the .<ext> key

	if (::RegOpenKeyEx( HKEY_CLASSES_ROOT, association.extension.String(), 0, KEY_READ, &hKey ) == ERROR_SUCCESS)
	{
		CHAR value[MAX_PATH+1];
		BYTE data[MAX_PATH+1];

		// enumerate all values

		for (DWORD i=0; ; ++i)
		{
			DWORD type;

			DWORD vSize = MAX_PATH;
			DWORD dSize = MAX_PATH;

			if (::RegEnumValue( hKey, i, value, &vSize, NULL, &type, data, &dSize ) != ERROR_SUCCESS)
				break;

			// if a match was found, mark as enabled

			if (!vSize && type == REG_SZ && IdString.Length() + 1 == dSize && !memcmp( IdString.String(), data, dSize ))
			{
				association.enabled = TRUE;
				break;
			}
		}

		::RegCloseKey( hKey );
	}

	// update the registry

	if (association.enabled)
		RegisterFile( association, FALSE );
	else
		::SHDeleteKey( HKEY_CLASSES_ROOT, IdString.String() );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PREFERENCES::UpdateDialog(HWND hDlg)
{
	::CheckDlgButton( hDlg, IDC_PREFERENCES_BEGIN_EMULATION,      emulateimmediately ? BST_CHECKED : BST_UNCHECKED );	
	::CheckDlgButton( hDlg, IDC_PREFERENCES_RUN_IN_BACKGROUND,    background         ? BST_CHECKED : BST_UNCHECKED );	
	::CheckDlgButton( hDlg, IDC_PREFERENCES_HIGH_PRIORITY,        prioritycontrol    ? BST_CHECKED : BST_UNCHECKED );	
	::CheckDlgButton( hDlg, IDC_PREFERENCES_STARTUP_FULLSCREEN,   fullscreen         ? BST_CHECKED : BST_UNCHECKED );	
	::CheckDlgButton( hDlg, IDC_PREFERENCES_DISABLE_ROM_WARNINGS, nowarnings         ? BST_CHECKED : BST_UNCHECKED );	
	::CheckDlgButton( hDlg, IDC_PREFERENCES_CLOSE_POWER_OFF,      closepoweroff      ? BST_CHECKED : BST_UNCHECKED );	
	::CheckDlgButton( hDlg, IDC_PREFERENCES_CONFIRM_EXIT,         confirmexit        ? BST_CHECKED : BST_UNCHECKED );	
	::CheckDlgButton( hDlg, IDC_PREFERENCES_CONFIRM_RESET,        confirmreset       ? BST_CHECKED : BST_UNCHECKED );	
	::CheckDlgButton( hDlg, IDC_PREFERENCES_SAVE_LOGFILE,         savelogfile        ? BST_CHECKED : BST_UNCHECKED );	
	::CheckDlgButton( hDlg, IDC_PREFERENCES_SAVE_SETTINGS,        savesettings       ? BST_CHECKED : BST_UNCHECKED );	
	::CheckDlgButton( hDlg, IDC_PREFERENCES_SAVE_LAUNCHER,        savelauncher       ? BST_CHECKED : BST_UNCHECKED );	
	::CheckDlgButton( hDlg, IDC_PREFERENCES_USE_ROM_DATABASE,     usedatabase        ? BST_CHECKED : BST_UNCHECKED );	
	::CheckDlgButton( hDlg, IDC_PREFERENCES_MULTIPLE_INSTANCES,   multipleinstances  ? BST_CHECKED : BST_UNCHECKED );	

	for (UINT i=0; i < NUM_FILE_TYPES; ++i)
	{
		UpdateAssociation( associations[i] );
		::CheckDlgButton( hDlg, IDC_PREFERENCES_ASSOCIATE_NES + i, associations[i].enabled ? BST_CHECKED : BST_UNCHECKED );	
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PREFERENCES::SetContext(HWND hDlg)
{
	if (hDlg)
	{
		const BOOL PrevPriority = prioritycontrol;

		emulateimmediately = ( ::IsDlgButtonChecked( hDlg, IDC_PREFERENCES_BEGIN_EMULATION      ) == BST_CHECKED );	
		background         = ( ::IsDlgButtonChecked( hDlg, IDC_PREFERENCES_RUN_IN_BACKGROUND    ) == BST_CHECKED );	
		prioritycontrol    = ( ::IsDlgButtonChecked( hDlg, IDC_PREFERENCES_HIGH_PRIORITY        ) == BST_CHECKED );	
		fullscreen         = ( ::IsDlgButtonChecked( hDlg, IDC_PREFERENCES_STARTUP_FULLSCREEN   ) == BST_CHECKED );	
		nowarnings         = ( ::IsDlgButtonChecked( hDlg, IDC_PREFERENCES_DISABLE_ROM_WARNINGS ) == BST_CHECKED );
		closepoweroff      = ( ::IsDlgButtonChecked( hDlg, IDC_PREFERENCES_CLOSE_POWER_OFF      ) == BST_CHECKED );
		confirmexit        = ( ::IsDlgButtonChecked( hDlg, IDC_PREFERENCES_CONFIRM_EXIT         ) == BST_CHECKED );
		confirmreset       = ( ::IsDlgButtonChecked( hDlg, IDC_PREFERENCES_CONFIRM_RESET        ) == BST_CHECKED );
		savelogfile        = ( ::IsDlgButtonChecked( hDlg, IDC_PREFERENCES_SAVE_LOGFILE         ) == BST_CHECKED );
		savesettings       = ( ::IsDlgButtonChecked( hDlg, IDC_PREFERENCES_SAVE_SETTINGS        ) == BST_CHECKED );
		savelauncher       = ( ::IsDlgButtonChecked( hDlg, IDC_PREFERENCES_SAVE_LAUNCHER        ) == BST_CHECKED );
		usedatabase        = ( ::IsDlgButtonChecked( hDlg, IDC_PREFERENCES_USE_ROM_DATABASE     ) == BST_CHECKED );
		multipleinstances  = ( ::IsDlgButtonChecked( hDlg, IDC_PREFERENCES_MULTIPLE_INSTANCES   ) == BST_CHECKED );

		if (PrevPriority && !prioritycontrol)
			::SetThreadPriority( ::GetCurrentThread(), DefaultPriority );

		BOOL updated = FALSE;

		for (UINT i=0; i < NUM_FILE_TYPES; ++i)
		{
			const bool IsChecked = (::IsDlgButtonChecked( hDlg, IDC_PREFERENCES_ASSOCIATE_NES + i ) == BST_CHECKED);

			if (bool(associations[i].enabled) != IsChecked) 
			{ 
				associations[i].enabled = IsChecked;
				updated = TRUE;

				if (IsChecked) RegisterFile( associations[i], TRUE ); 
				else           UnregisterFile( associations[i] );
			}
		}

		if (updated)
		{
			::SHChangeNotify( SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0 );	
			UI::MsgInfo(IDS_APP_REGISTRY_SUCCESS);
		}
	}

	NES::IO::GENERAL::CONTEXT context;
	nes.GetGeneralContext( context );	
	context.DisableWarnings = nowarnings;
	context.UseRomDatabase = usedatabase;	
	nes.SetGeneralContext( context );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PREFERENCES::RegisterFile(const ASSOCIATION& association,const BOOL IncludeExtension)
{
	PDX_ASSERT
	(
     	association.extension.Length() && 
		association.extension[0] == '.' && 
		association.desc.Length()
	);

	PDXSTRING path;
	UTILITIES::GetExeName( path );

	HKEY hExtKey     = NULL;
	HKEY hRegKey     = NULL;
	HKEY hIconKey    = NULL;
	HKEY hCommandKey = NULL;

	try
	{
		PDXSTRING string("Nestopia");
		string << association.extension;

		if (IncludeExtension)
		{
			// create the .<ext> key

			if (::RegCreateKeyEx( HKEY_CLASSES_ROOT, association.extension.String(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hExtKey, NULL ) != ERROR_SUCCESS)
				throw 1;

			// set the value to point to the ID key

			if (::RegSetValueEx( hExtKey, NULL, 0, REG_SZ, PDX_CAST(const BYTE*,string.String()), string.Length() + 1 ) != ERROR_SUCCESS)
				throw 1;

			::RegCloseKey( hExtKey );
			hExtKey = NULL;
		}

		// create the ID key

		if (::RegCreateKeyEx( HKEY_CLASSES_ROOT, string.String(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hRegKey, NULL ) != ERROR_SUCCESS)
			throw 1;

		// create the icon key

		if (::RegCreateKeyEx( hRegKey, "DefaultIcon", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hIconKey, NULL ) != ERROR_SUCCESS)
			throw 1;

		string = path;
		string << "," << association.icon;

		// set the icon string

		if (::RegSetValueEx( hIconKey, NULL, 0, REG_SZ, PDX_CAST(const BYTE*,string.String()), string.Length() + 1 ) != ERROR_SUCCESS)
			throw 1;

		::RegCloseKey( hIconKey );
		hIconKey = NULL;

		// write a description of the file extension format

		if (::RegSetValueEx( hRegKey, NULL, 0, REG_SZ, PDX_CAST(const BYTE*,association.desc.String()), association.desc.Length() + 1 ) != ERROR_SUCCESS)
			throw 1;

		// create the file association keys

		if (::RegCreateKeyEx( hRegKey, "Shell\\Open\\Command", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE|KEY_CREATE_SUB_KEY, NULL, &hCommandKey, NULL ) != ERROR_SUCCESS) 
			throw 1;

		string = path;
		string << " \"%1\"";

		// write the file path and executable

		if (::RegSetValueEx( hCommandKey, NULL, 0, REG_SZ, PDX_CAST(const BYTE*,string.String()), string.Length() + 1 ) != ERROR_SUCCESS)
			throw 1;

		::RegCloseKey( hCommandKey ); 
		hCommandKey = NULL;

		::RegCloseKey( hRegKey );	
		hRegKey = NULL;
	}
	catch (...)
	{
		if ( hExtKey     ) ::RegCloseKey( hExtKey     );
		if ( hIconKey    ) ::RegCloseKey( hIconKey    );
		if ( hCommandKey ) ::RegCloseKey( hCommandKey );
		if ( hRegKey     ) ::RegCloseKey( hRegKey     );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PREFERENCES::UnregisterFile(const ASSOCIATION& association)
{
	PDXSTRING IdString("Nestopia");
	IdString << association.extension;

	// delete the Nestopia.<ext> key

	::SHDeleteKey( HKEY_CLASSES_ROOT, IdString.String() );

	HKEY hKey;

	// open the .nes key

	if (::RegOpenKeyEx( HKEY_CLASSES_ROOT, association.extension.String(), 0, KEY_WRITE|KEY_QUERY_VALUE, &hKey ) == ERROR_SUCCESS)
	{
		CHAR value[MAX_PATH+1];
		BYTE data[MAX_PATH+1];

		// enumerate all values

		for (DWORD i=0; ; ++i)
		{
			DWORD type;

			DWORD vSize = MAX_PATH;
			DWORD dSize = MAX_PATH;

			if (::RegEnumValue( hKey, i, value, &vSize, NULL, &type, data, &dSize ) != ERROR_SUCCESS)
				break;

			// check for value mach

			if (type == REG_SZ && IdString.Length() + 1 == dSize && !memcmp( IdString.String(), data, dSize ))
			{
				if (vSize)
				{
					// Nestopia key is not the default, only delete the value

					::RegDeleteValue( hKey, value );
					::RegCloseKey( hKey );
				}
				else
				{
					// Nestopia key is the default, delete the whole key

					::RegCloseKey( hKey );
					::SHDeleteKey( HKEY_CLASSES_ROOT, association.extension.String() );
				}
				return;
			}
		}

		// registry was clean

		::RegCloseKey( hKey );
	}
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

					::EndDialog( hDlg, 0 );
					return TRUE;

				case IDC_PREFERENCES_DEFAULT:

					Reset();
					UpdateDialog( hDlg );
					return TRUE;
			}
			return FALSE;

     	case WM_CLOSE:

     		::EndDialog( hDlg, 0 );
     		return TRUE;
	}

	return FALSE;
}
