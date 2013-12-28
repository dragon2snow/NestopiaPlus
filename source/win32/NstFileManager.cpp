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
#include <WindowsX.h>
#include "../paradox/PdxFile.h"
#include "../NstZipFile.h"
#include "../NstNes.h"
#include "../core/NstIps.h"
#include "../core/NstNsp.h"
#include "NstFileManager.h"
#include "NstSaveStateManager.h"
#include "NstGraphicManager.h"
#include "NstGameGenieManager.h"
#include "NstMovieManager.h"
#include "NstPreferences.h"
#include "NstApplication.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#define NES_MAGIC_INES 0x1A53454EUL
#define NES_MAGIC_UNIF 0x46494E55UL
#define NES_MAGIC_FDS  0x1A534446UL
#define NES_MAGIC_NSF  0x4D53454EUL
#define NES_MAGIC_IPS  0x43544150UL
#define NES_MAGIC_NST  0x1A54534EUL
#define NES_MAGIC_NSV  0x1A564D4EUL

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

FILEMANAGER::FILEMANAGER()
: 
MANAGER             (IDD_PATHS), 
ZipFile             (NULL),
UseRomPathLast      (TRUE),
UseSavePathRom      (TRUE),
DisableSaveRamWrite	(FALSE),
UseNstPathRom		(TRUE),
UseNstPathLast		(TRUE),
AutoImportNst		(TRUE),
AutoExportNst       (TRUE),
UseIpsPathRom		(TRUE),
AutoApplyIps		(FALSE),
UseNspPathRom		(TRUE),
UseNspPathLast		(TRUE),
AutoApplyNsp		(TRUE),
UpdatedRecentFile	(FALSE),
CompressedFileIndex	(0)
{}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FILEMANAGER::Create(CONFIGFILE* const ConfigFile)
{
	UpdatedRecentFile = FALSE;
	RecentFiles.Reserve( MAX_RECENT_FILES );

	if (ConfigFile)
	{
		CONFIGFILE& file = *ConfigFile;

		RomPath     = file[ "files path image"      ];
		SavePath    = file[ "files path battery"    ];
		NstPath     = file[ "files path nst"        ];
		IpsPath     = file[ "files path ips"        ];
		NspPath     = file[ "files path nsp"        ];
		RomPathLast = file[ "files last path image" ];
		NstPathLast = file[ "files last path nst"   ];
		NspPathLast = file[ "files last path nsp"   ];

		UpdatePaths();

		UseRomPathLast      = (file[ "files use last image path"          ] == "no"  ? FALSE : TRUE);
		UseNstPathLast      = (file[ "files use last nst path"            ] == "no"  ? FALSE : TRUE);
		UseNspPathLast      = (file[ "files use last nsp path"            ] == "no"  ? FALSE : TRUE);
		UseSavePathRom      = (file[ "files search battery in image path" ] == "no"  ? FALSE : TRUE);
		UseIpsPathRom       = (file[ "files search ips in image path"     ] == "no"  ? FALSE : TRUE);
		UseNspPathRom       = (file[ "files search nsp in image path"     ] == "no"  ? FALSE : TRUE);
		UseNstPathRom       = (file[ "files search nst in image path"     ] == "no"  ? FALSE : TRUE);
		DisableSaveRamWrite	= (file[ "files write protect battery"        ] == "yes" ? TRUE : FALSE);
		AutoApplyIps        = (file[ "files auto apply ips"               ] == "yes" ? TRUE : FALSE);
		AutoApplyNsp        = (file[ "files auto apply nsp"               ] == "no"  ? FALSE : TRUE);
		AutoImportNst       = (file[ "files auto import nst"              ] == "no"  ? FALSE : TRUE);
		AutoExportNst       = (file[ "files auto export nst"              ] == "no"  ? FALSE : TRUE);

		const PDXSTRING* string;

		string = &file["files recent 0"]; if (UTILITIES::FileExist( *string )) RecentFiles.InsertBack( *string );
		string = &file["files recent 1"]; if (UTILITIES::FileExist( *string )) RecentFiles.InsertBack( *string );
		string = &file["files recent 2"]; if (UTILITIES::FileExist( *string )) RecentFiles.InsertBack( *string );
		string = &file["files recent 3"]; if (UTILITIES::FileExist( *string )) RecentFiles.InsertBack( *string );
		string = &file["files recent 4"]; if (UTILITIES::FileExist( *string )) RecentFiles.InsertBack( *string );
		string = &file["files recent 5"]; if (UTILITIES::FileExist( *string )) RecentFiles.InsertBack( *string );
		string = &file["files recent 6"]; if (UTILITIES::FileExist( *string )) RecentFiles.InsertBack( *string );
		string = &file["files recent 7"]; if (UTILITIES::FileExist( *string )) RecentFiles.InsertBack( *string );
		string = &file["files recent 8"]; if (UTILITIES::FileExist( *string )) RecentFiles.InsertBack( *string );
		string = &file["files recent 9"]; if (UTILITIES::FileExist( *string )) RecentFiles.InsertBack( *string );
	}
	else
	{
		Reset();
	}

	application.GetSaveStateManager().EnableFileImport( AutoImportNst );
	application.GetSaveStateManager().EnableFileExport( AutoExportNst );

	UpdateContext();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FILEMANAGER::Destroy(CONFIGFILE* const ConfigFile)
{
	if (ConfigFile)
	{
		CONFIGFILE& file = *ConfigFile;

		file[ "files path image"      ] = RomPath.Quoted();
		file[ "files path battery"    ] = SavePath.Quoted();
		file[ "files path nst"        ] = NstPath.Quoted();
		file[ "files path ips"        ] = IpsPath.Quoted();
		file[ "files path nsp"        ] = NspPath.Quoted();
		file[ "files last path image" ] = RomPathLast.Quoted(); 
		file[ "files last path nst"   ] = NstPathLast.Quoted();
		file[ "files last path nsp"   ] = NspPathLast.Quoted();

		file[ "files use last image path"          ] = (UseRomPathLast      ? "yes" : "no");
		file[ "files use last nst path"            ] = (UseNstPathLast      ? "yes" : "no");
		file[ "files use last nsp path"            ] = (UseNspPathLast      ? "yes" : "no");
		file[ "files search battery in image path" ] = (UseSavePathRom      ? "yes" : "no");
		file[ "files search ips in image path"     ] = (UseIpsPathRom       ? "yes" : "no");
		file[ "files search nsp in image path"     ] = (UseNspPathRom       ? "yes" : "no");
		file[ "files search nst in image path"     ] = (UseNstPathRom       ? "yes" : "no");
		file[ "files write protect battery"        ] = (DisableSaveRamWrite ? "yes" : "no");
		file[ "files auto apply ips"               ] = (AutoApplyIps        ? "yes" : "no");
		file[ "files auto apply nsp"               ] = (AutoApplyNsp        ? "yes" : "no");
		file[ "files auto import nst"              ] = (AutoImportNst       ? "yes" : "no");
		file[ "files auto export nst"              ] = (AutoExportNst       ? "yes" : "no");

		if (RecentFiles.Size() > 0) file[ "files recent 0" ] = RecentFiles[0].Quoted();
		if (RecentFiles.Size() > 1) file[ "files recent 1" ] = RecentFiles[1].Quoted();
		if (RecentFiles.Size() > 2) file[ "files recent 2" ] = RecentFiles[2].Quoted();
		if (RecentFiles.Size() > 3) file[ "files recent 3" ] = RecentFiles[3].Quoted();
		if (RecentFiles.Size() > 4) file[ "files recent 4" ] = RecentFiles[4].Quoted();
		if (RecentFiles.Size() > 5) file[ "files recent 5" ] = RecentFiles[5].Quoted();
		if (RecentFiles.Size() > 6) file[ "files recent 6" ] = RecentFiles[6].Quoted();
		if (RecentFiles.Size() > 7) file[ "files recent 7" ] = RecentFiles[7].Quoted();
		if (RecentFiles.Size() > 8) file[ "files recent 8" ] = RecentFiles[8].Quoted();
		if (RecentFiles.Size() > 9) file[ "files recent 9" ] = RecentFiles[9].Quoted();
	}								
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FILEMANAGER::Reset()
{
	UTILITIES::GetCurrentDir( RomPath );

	SavePath    = RomPath;
	NstPath     = RomPath;
	IpsPath     = RomPath;
	NspPath     = RomPath;
	RomPathLast = RomPath;
	NstPathLast = NstPath;
	NspPathLast = NspPath;

	UseRomPathLast      = TRUE;
	UseSavePathRom      = TRUE;
	UseNstPathLast      = TRUE;
	DisableSaveRamWrite	= FALSE;
	UseIpsPathRom       = TRUE;
	AutoApplyIps        = FALSE;
	UseNspPathRom       = TRUE;
	UseNspPathLast      = TRUE;
	AutoApplyNsp        = TRUE;
	AutoImportNst       = TRUE;
	AutoExportNst       = TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FILEMANAGER::UpdateContext()
{
	NES::IO::GENERAL::CONTEXT context;
	nes.GetGeneralContext( context );

	context.WriteProtectBattery = DisableSaveRamWrite;

	nes.SetGeneralContext( context );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FILEMANAGER::UpdatePaths()
{
	if (!UTILITIES::ValidateDir( RomPath ))
		UTILITIES::GetCurrentDir( RomPath );

	if (!UTILITIES::ValidateDir( SavePath    )) SavePath    = RomPath;
	if (!UTILITIES::ValidateDir( NstPath     )) NstPath     = RomPath;
	if (!UTILITIES::ValidateDir( IpsPath     )) IpsPath     = RomPath;
	if (!UTILITIES::ValidateDir( NspPath     )) NspPath     = RomPath;
	if (!UTILITIES::ValidateDir( RomPathLast )) RomPathLast = RomPath;
	if (!UTILITIES::ValidateDir( NstPathLast )) NstPathLast = NstPath;
	if (!UTILITIES::ValidateDir( NspPathLast )) NspPathLast = NspPath;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FILEMANAGER::UpdateSettings()
{
	PDX_ASSERT( hDlg );

	MANAGER::GetDlgItemText( hDlg, IDC_PATHS_IMAGE,   RomPath  );
	MANAGER::GetDlgItemText( hDlg, IDC_PATHS_BATTERY, SavePath );
	MANAGER::GetDlgItemText( hDlg, IDC_PATHS_NST,     NstPath  );
	MANAGER::GetDlgItemText( hDlg, IDC_PATHS_IPS,     IpsPath  );
	MANAGER::GetDlgItemText( hDlg, IDC_PATHS_NSP,     NspPath  );

	UpdatePaths();

	UseRomPathLast      = ( ::IsDlgButtonChecked( hDlg, IDC_PATHS_IMAGE_LAST       ) == BST_CHECKED );  
	DisableSaveRamWrite = ( ::IsDlgButtonChecked( hDlg, IDC_PATHS_BATTERY_PROTECT  ) == BST_CHECKED );  
	UseSavePathRom      = ( ::IsDlgButtonChecked( hDlg, IDC_PATHS_BATTERY_IN_IMAGE ) == BST_CHECKED );  
	UseNspPathRom       = ( ::IsDlgButtonChecked( hDlg, IDC_PATHS_NSP_IN_IMAGE     ) == BST_CHECKED ); 
	UseNstPathRom       = ( ::IsDlgButtonChecked( hDlg, IDC_PATHS_NST_IN_IMAGE     ) == BST_CHECKED ); 
	UseIpsPathRom       = ( ::IsDlgButtonChecked( hDlg, IDC_PATHS_IPS_IN_IMAGE     ) == BST_CHECKED ); 
	AutoApplyIps        = ( ::IsDlgButtonChecked( hDlg, IDC_PATHS_IPS_AUTO_APPLY   ) == BST_CHECKED ); 
	AutoImportNst       = ( ::IsDlgButtonChecked( hDlg, IDC_PATHS_NST_AUTO_IMPORT  ) == BST_CHECKED ); 
	AutoExportNst       = ( ::IsDlgButtonChecked( hDlg, IDC_PATHS_NST_AUTO_EXPORT  ) == BST_CHECKED ); 
	AutoApplyNsp        = ( ::IsDlgButtonChecked( hDlg, IDC_PATHS_NSP_AUTO_APPLY   ) == BST_CHECKED ); 
	UseNspPathLast      = ( ::IsDlgButtonChecked( hDlg, IDC_PATHS_NSP_LAST         ) == BST_CHECKED ); 
	UseNstPathLast      = ( ::IsDlgButtonChecked( hDlg, IDC_PATHS_NST_LAST         ) == BST_CHECKED ); 

	application.GetSaveStateManager().EnableFileImport( AutoImportNst );
	application.GetSaveStateManager().EnableFileExport( AutoExportNst );

	UpdateContext();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL FILEMANAGER::DialogProc(HWND h,UINT uMsg,WPARAM wParam,LPARAM)
{
	switch (uMsg) 
	{
		case WM_INITDIALOG:

			hDlg = h;
			UpdateDialog();
			return TRUE;

		case WM_COMMAND:

         	switch (LOWORD(wParam))
			{
				case IDC_PATHS_IMAGE_BROWSE:
				{
					PDXSTRING path;
					
					if (UTILITIES::BrowseFolder( path, hDlg, IDS_FILE_SELECT_IMAGE ))
						::SetDlgItemText( hDlg, IDC_PATHS_IMAGE, path.String() );

					return TRUE;
				}
		
				case IDC_PATHS_BATTERY_BROWSE:
				{		
					PDXSTRING path;

					if (UTILITIES::BrowseFolder( path, hDlg, IDS_FILE_SELECT_BATTERY ))
						::SetDlgItemText( hDlg, IDC_PATHS_BATTERY, path.String() );

					return TRUE;
				}
		
				case IDC_PATHS_NST_BROWSE:
				{
					PDXSTRING path;

					if (UTILITIES::BrowseFolder( path, hDlg, IDS_FILE_SELECT_NST ))
						::SetDlgItemText( hDlg, IDC_PATHS_NST, path.String() );

					return TRUE;
				}

				case IDC_PATHS_IPS_BROWSE:
				{
					PDXSTRING path;

					if (UTILITIES::BrowseFolder( path, hDlg, IDS_FILE_SELECT_IPS ))
						::SetDlgItemText( hDlg, IDC_PATHS_IPS, path.String() );

					return TRUE;
				}

				case IDC_PATHS_NSP_BROWSE:
				{
					PDXSTRING path;

					if (UTILITIES::BrowseFolder( path, hDlg, IDS_FILE_SELECT_NSP ))
						::SetDlgItemText( hDlg, IDC_PATHS_NSP, path.String() );

					return TRUE;
				}

				case IDC_PATHS_DEFAULT:
		
					Reset();
					UpdateDialog();
					return TRUE;
		
				case IDC_PATHS_OK:
		
					UpdateSettings();
					::EndDialog( hDlg, 0 );
					return TRUE;
			}
			return FALSE;

		case WM_CLOSE:

			UpdateSettings();
			::EndDialog( hDlg, 0 );
			return TRUE;

		case WM_DESTROY:

			hDlg = NULL;
			return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

const PDXSTRING& FILEMANAGER::GetDefaultRomPath()
{
	if (!UTILITIES::PathExist( RomPath ))
		UTILITIES::GetCurrentDir( RomPath );

	return RomPath;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

const PDXSTRING& FILEMANAGER::GetRomPath()
{
	if (!UTILITIES::PathExist( RomPath ))
		UTILITIES::GetCurrentDir( RomPath );

	if (!UseRomPathLast)
		return RomPath;

	if (!UTILITIES::PathExist( RomPathLast ))
		RomPathLast = RomPath;

	return RomPathLast;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

const PDXSTRING& FILEMANAGER::GetSavePath()
{
	if (UseSavePathRom)
		return GetRomPath();

	if (!UTILITIES::PathExist( SavePath ))
		SavePath = GetRomPath();

	return SavePath; 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

const PDXSTRING& FILEMANAGER::GetNstPath()
{ 
	if (!UTILITIES::PathExist( NstPath ))
		NstPath = GetRomPath();

	if (!UseNstPathLast)
		return NstPath;

	if (!UTILITIES::PathExist( NstPathLast ))
		NstPathLast = NstPath;

	return NstPathLast;  
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

const PDXSTRING& FILEMANAGER::GetIpsPath()
{
	if (UseIpsPathRom)
		return GetRomPath();

	if (!UTILITIES::PathExist( IpsPath ))
		IpsPath = GetRomPath();

	return IpsPath; 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

const PDXSTRING& FILEMANAGER::GetNspPath()
{
	if (!UTILITIES::PathExist( NspPath ))
		NspPath = GetRomPath();

	if (!UseNspPathLast)
		return NspPath;

	if (!UTILITIES::PathExist( NspPathLast ))
		NspPathLast = NspPath;

	return NspPathLast;  
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FILEMANAGER::UpdateDialog()
{
	::SetDlgItemText( hDlg, IDC_PATHS_IMAGE,   RomPath.String()  );
	::SetDlgItemText( hDlg, IDC_PATHS_BATTERY, SavePath.String() );
	::SetDlgItemText( hDlg, IDC_PATHS_NST,     NstPath.String()  );
	::SetDlgItemText( hDlg, IDC_PATHS_IPS,     IpsPath.String()  );
	::SetDlgItemText( hDlg, IDC_PATHS_NSP,     NspPath.String()  );

	::CheckDlgButton( hDlg, IDC_PATHS_IMAGE_LAST,       UseRomPathLast      ); 
	::CheckDlgButton( hDlg, IDC_PATHS_BATTERY_IN_IMAGE, UseSavePathRom      );
	::CheckDlgButton( hDlg, IDC_PATHS_NST_LAST,         UseNstPathLast      );
	::CheckDlgButton( hDlg, IDC_PATHS_NST_IN_IMAGE,     UseNstPathRom       );
	::CheckDlgButton( hDlg, IDC_PATHS_NST_AUTO_IMPORT,  AutoImportNst       );
	::CheckDlgButton( hDlg, IDC_PATHS_NST_AUTO_EXPORT,  AutoExportNst       );
	::CheckDlgButton( hDlg, IDC_PATHS_BATTERY_PROTECT,  DisableSaveRamWrite );
	::CheckDlgButton( hDlg, IDC_PATHS_IPS_IN_IMAGE,     UseIpsPathRom       );
	::CheckDlgButton( hDlg, IDC_PATHS_IPS_AUTO_APPLY,   AutoApplyIps        );
	::CheckDlgButton( hDlg, IDC_PATHS_NSP_IN_IMAGE,     UseNspPathRom       );
	::CheckDlgButton( hDlg, IDC_PATHS_NSP_AUTO_APPLY,   AutoApplyNsp        );
	::CheckDlgButton( hDlg, IDC_PATHS_NSP_LAST,         UseNspPathLast      );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FILEMANAGER::Load(const COMMAND command,const VOID* const param,const BOOL power)
{
	UpdatedRecentFile = FALSE;

	PDXFILE file;
	PDXRESULT result;

	{
		PDXARRAY<PDXSTRING> extensions;	
		extensions.Resize( 18 );

     	extensions[0]  = "nes";
		extensions[1]  = "unf";
		extensions[2]  = "unif";
		extensions[3]  = "fds";
     	extensions[4]  = "nsf";
     	extensions[5]  = "nst";
		extensions[6]  = "nsv";
		extensions[7]  = "nsp";
		extensions[8]  = "ips";
		extensions[9]  = "ns1";
		extensions[10] = "ns2";
		extensions[11] = "ns3";
		extensions[12] = "ns4";
		extensions[13] = "ns5";
		extensions[14] = "ns6";
		extensions[15] = "ns7";
		extensions[16] = "ns8";
		extensions[17] = "ns9";

     	result = LoadFile
		( 
		    command,
			param,
     		file,
			&RomPathLast,
			GetRomPath().String(),
     		UTILITIES::IdToString(IDS_FILE_OPEN).String(),
			(
				"All supported files\0"
				"*.nes;*.unf;*.fds;*.nsf;*.nst;*.nsp;*.nsv;*.ips;*.zip;*.ns1;*.ns2;*.ns3;*.ns4;*.ns5;*.ns6;*.ns7;*.ns8;*.ns9\0"
				"iNes ROM Images (*.nes)\0"
				"*.nes\0"
				"UNIF ROM Images (*.unf)\0"
				"*.unf\0"
				"Famicom Disk System Images (*.fds)\0"
				"*.fds\0"
				"NES Sound Files (*.nsf)\0"
				"*.nsf\0"
				"Nestopia State Files (*.nst,*.ns?)\0"
				"*.nst;*.ns1;*.ns2;*.ns3;*.ns4;*.ns5;*.ns6;*.ns7;*.ns8;*.ns9\0"
				"Nestopia Script Files (*.nsp)\0"
				"*.nsp\0"
				"Nestopia Movie Files (*.nsv)\0"
				"*.nsv\0"
				"IPS Files (*.ips)\0"
				"*.ips\0"
				"Zip Files (*.zip)\0"
				"*.zip\0"
				"All files (*.*)\0"
				"*.*\0"									   
			),
       		extensions,
     		ADD_RECENT_FILE
		);
	}

	if (PDX_SUCCEEDED(result))
	{
		PDX_ASSERT(file.IsOpen());

		if (PDX_FAILED(LoadNesFile( file, power )))
		{
			UpdatedRecentFile = TRUE;
			RecentFiles.Erase( RecentFiles.Begin() );
			result = PDX_FAILURE;
			nes.Power( FALSE );
		}
	}

	return result;														
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FILEMANAGER::LoadNSP(const COMMAND command,const VOID* const param,const BOOL power)
{
	UpdatedRecentFile = FALSE;

	PDXFILE file;
	PDXRESULT result;

	{
		PDXARRAY<PDXSTRING> extension;
		extension.InsertBack(PDXSTRING("nsp"));

		result = LoadFile
		( 
		    command,
			param,
			file,
			&NspPathLast,
			GetNspPath().String(),
			UTILITIES::IdToString(IDS_FILE_LOAD_NSP).String(),
			(
				"All supported files (*.nsp, *.zip)\0"
				"*.nsp;*.zip\0"
				"Nestopia Script Files (*.nsp)\0"
				"*.nsp\0"
				"Zip Files (*.zip)\0"
				"*.zip\0"
				"All files (*.*)\0"
				"*.*\0"
			),
			extension,
			ADD_RECENT_FILE
		);
	}

	if (PDX_SUCCEEDED(result))
	{
		PDX_ASSERT(file.IsOpen());

		if (PDX_FAILED(LoadNesFile( file, power )))
		{
			UpdatedRecentFile = TRUE;
			RecentFiles.Erase( RecentFiles.Begin() );
			result = PDX_FAILURE;
			nes.Power( FALSE );
		}
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FILEMANAGER::LoadNST()
{
	if (!nes.IsImage())
		return PDX_FAILURE;

	if (nes.IsOff())
		nes.Power( TRUE );

	PDXFILE file;
	PDXRESULT result;

	{
		PDXARRAY<PDXSTRING> extensions;
		extensions.Resize( 10 );

		extensions[0] = "nst";
		extensions[1] = "ns1";
		extensions[2] = "ns2";
		extensions[3] = "ns3";
		extensions[4] = "ns4";
		extensions[5] = "ns5";
		extensions[6] = "ns6";
		extensions[7] = "ns7";
		extensions[8] = "ns8";
		extensions[9] = "ns9";

		result = LoadFile
		( 
		    COMMAND_CHOOSE_FILE,
			NULL,
			file,
			&NstPathLast,
			GetNstPath().String(),
			UTILITIES::IdToString(IDS_FILE_LOAD_NST).String(),
			(
	     		"All supported files (*.nst, *ns?, *.zip)\0"
	     		"*.nst;*.ns1;*.ns2;*.ns3;*.ns4;*.ns5;*.ns6;*.ns7;*.ns8;*.ns9;*.zip\0"
	       		"Nestopia State Files (*.nst, *ns?)\0"
	     		"*.nst;*.ns1;*.ns2;*.ns3;*.ns4;*.ns5;*.ns6;*.ns7;*.ns8;*.ns9\0"
				"Zip Files (*.zip)\0"
				"*.zip\0"
	     		"All files (*.*)\0"
	       		"*.*\0"
			),
			extensions,
			DONT_ADD_RECENT_FILE
		);
	}

	if (PDX_SUCCEEDED(result))
	{
		PDX_ASSERT(file.IsOpen());

		if (file.Readable(sizeof(U32)))
		{
			if (file.Peek<U32>() == NES_MAGIC_NST)
				result = LoadNesFile( file, TRUE );
			else
				result = UI::MsgWarning(IDS_FILE_INVALID_NST);
		}
		else
		{
			result = UI::MsgWarning(IDS_FILE_CORRUPT);
		}
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FILEMANAGER::LoadNesFile(PDXFILE& file,const BOOL power)
{
	if (!file.Readable(sizeof(U32)))
		return UI::MsgWarning(IDS_FILE_CORRUPT);
	
	PDXRESULT result = PDX_FAILURE;

	BOOL NspLoaded = FALSE;
	NES::IO::NSP::CONTEXT NspContext;

	const U32 magic = file.Peek<U32>();

	switch (magic)
	{
       	case NES_MAGIC_NST:
       	case NES_MAGIC_NSV:
			break;

		default:

			LastImageFile.Clear();
			LastIpsFile.Clear();
			LastSaveFile.Clear();

			application.GetSaveStateManager().Reset();
	}

	BOOL ShowWarnings = TRUE;

	switch (magic)
	{
    	case NES_MAGIC_INES:
		case NES_MAGIC_UNIF:
		case NES_MAGIC_FDS:
		{
			if (nes.IsImage())
				application.GetGameGenieManager().ClearCodes( TRUE );

			NspLoaded = PDX_SUCCEEDED(ApplyNsp( file.Name(), NspContext, ShowWarnings ));

			if (magic != NES_MAGIC_UNIF)
				ApplyIps( file, NspContext.IpsFile, ShowWarnings );

			const PDXSTRING* save = NULL;

			if (magic != NES_MAGIC_FDS)
			{
				ApplySav( file.Name(), NspContext.SaveFile );
				save = &NspContext.SaveFile;
			}

			{
				PDXSTRING SlotFile;

				for (UINT i=1; i < SAVESTATEMANAGER::MAX_SLOTS+1; ++i)
				{
					FindSlt( i, file.Name(), SlotFile );
					application.GetSaveStateManager().SetFileName( i, SlotFile );
				}
			}

			if (PDX_FAILED(nes.LoadRom( file, save )))
				return PDX_FAILURE;

			nes.Power( power );
			LastImageFile = file.Name();
			break;
		}

		case NES_MAGIC_IPS:
		{
			PDXFILE ImageFile;
			PDXSTRING ImageName;

			if (PDX_FAILED(ApplyRom( file.Name(), ImageFile, ImageName )))
				return PDX_FAILURE;

			if (PDX_FAILED(NES::IPS::Load( ImageFile, file )))
				return UI::MsgWarning(IDS_FILE_IPS_FAILED);

			LastIpsFile = file.Name();

			PDXSTRING SaveName;
			const PDXSTRING* save = NULL;

			if (ImageFile.Peek<U32>() != NES_MAGIC_FDS)
			{
				ApplySav( file.Name(), SaveName );
				save = &SaveName;
			}

			{
				PDXSTRING SlotFile;

				for (UINT i=1; i < SAVESTATEMANAGER::MAX_SLOTS+1; ++i)
				{
					FindSlt( i, ImageFile.Name(), SlotFile );
					application.GetSaveStateManager().SetFileName( i, SlotFile );
				}
			}

			if (PDX_FAILED(nes.LoadRom( ImageFile, save )))
				return PDX_FAILURE;

			nes.Power( power );
			LastImageFile = ImageFile.Name();
			break;
		}

		case NES_MAGIC_NST:

			if (!nes.IsImage() || nes.IsOff() || PDX_FAILED(nes.LoadNST( file )))
				return PDX_FAILURE;

			break;

		case NES_MAGIC_NSV:

			if (!nes.IsImage() || nes.IsOff() || PDX_FAILED(nes.LoadMovie( file.Name() )))
				return PDX_FAILURE;

			break;

		case NES_MAGIC_NSF:

			if (PDX_FAILED(nes.LoadNSF( file )))
				return PDX_FAILURE;

			nes.Power( power );
			break;

		default: // NSP
		{
			if (!file.Name().IsFileExtension( "nsp", 3 ))
				return UI::MsgError(IDS_FILE_INVALID);

			{
				NES::NSP nsp;

				if (PDX_FAILED(nsp.Load( file )))
					return UI::MsgWarning(IDS_FILE_PARSE_ERROR);

				NspLoaded = TRUE;
				nsp.GetContext( NspContext );
			}

			if (nes.IsImage())
				application.GetGameGenieManager().ClearCodes( TRUE );

			PDXFILE ImageFile;

			if (PDX_FAILED(ApplyRom( file.Name(), ImageFile, NspContext.ImageFile )))
				return PDX_FAILURE;

			const U32 ImageMagic = ImageFile.Peek<U32>();

			if (ImageMagic != NES_MAGIC_UNIF)
				ApplyIps( file, NspContext.IpsFile, ShowWarnings );

			const PDXSTRING* save = NULL;

			if (ImageMagic != NES_MAGIC_FDS)
			{
				ApplySav( file.Name(), NspContext.SaveFile );
				save = &NspContext.SaveFile;
			}

			if (PDX_FAILED(nes.LoadRom( ImageFile, save )))
				return PDX_FAILURE;

			nes.Power( power );
			LastImageFile = ImageFile.Name();
			break;
		}
	}

	if (NspLoaded)
	{
		if (NspContext.MovieFile.Length())
		{
			application.GetMovieManager().Load( NspContext.MovieFile );

			if (application.GetMovieManager().CanPlay())
			{
				application.GetMovieManager().Play();
			}
			else
			{
				ShowWarnings = FALSE;

				UI::MsgWarning
				(  
				    UTILITIES::IdToString(IDS_FILE_NSV_FAILED_1_2) <<
					NspContext.MovieFile <<
					UTILITIES::IdToString(IDS_FILE_NSV_FAILED_2_2)
				);
				
				nes.CloseMovie();
			}
		}
		else
		{
			PDXFILE StateFile;
			ApplyNst( StateFile, NspContext.StateFile, ShowWarnings );

			if (StateFile.IsOpen() && PDX_FAILED(nes.LoadNST( StateFile )))
				return PDX_FAILURE;
		}

		for (UINT i=0; i < SAVESTATEMANAGER::MAX_SLOTS; ++i)
		{
			PDXFILE SlotFile;
			
			if (PDX_SUCCEEDED(ApplyNst( SlotFile, NspContext.StateSlots[i], ShowWarnings )))
			{
				application.GetSaveStateManager().SetFileName( i+1, SlotFile.Name() );
			}
			else
			{
				PDXSTRING SlotName;
				FindSlt( i+1, LastImageFile, SlotName );
				application.GetSaveStateManager().SetFileName( i+1, SlotName );
			}
		}
  
		application.GetSaveStateManager().CacheSlots();

		for (UINT i=0; i < NspContext.GenieCodes.Size(); ++i)
		{
			application.GetGameGenieManager().AddCode
			(
			    NspContext.GenieCodes[i].code,
				NspContext.GenieCodes[i].enabled,
				&NspContext.GenieCodes[i].comment
			);
		}

		if (NspContext.PaletteFile.Length())
			application.GetGraphicManager().LoadPalette( NspContext.PaletteFile );

		if (NspContext.pal != -1 && application.GetNesMode() == NES::MODE_AUTO)
			nes.SetMode( NspContext.pal ? NES::MODE_PAL : NES::MODE_NTSC );
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FILEMANAGER::ApplyRom(const PDXSTRING& AnyFileName,PDXFILE& ImageFile,PDXSTRING& ImageName)
{
	if (ImageName.Length())
		ImageFile.Open( ImageName, PDXFILE::INPUT );

	if (!ImageFile.IsOpen() && FindRom( AnyFileName, ImageName ))
		ImageFile.Open( ImageName, PDXFILE::INPUT );

	if (!ImageFile.IsOpen())
	{
		const BOOL yeah = UI::MsgQuestion
		(
		    IDS_FILE_IMAGE_NOT_FOUND_1_2,
			IDS_FILE_IMAGE_NOT_FOUND_2_2
		);

		if (yeah)
		{
			PDXARRAY<PDXSTRING> extensions;	
			extensions.Resize( 4 );

			extensions[0] = "nes";
			extensions[1] = "unf";
			extensions[2] = "unif";
			extensions[3] = "fds";

			const PDXRESULT result = LoadFile
			( 
			    COMMAND_CHOOSE_FILE,
				NULL,
				ImageFile,
				NULL,
				GetRomPath().String(),
				UTILITIES::IdToString(IDS_FILE_OPEN).String(),
				(
					"All supported files\0"
					"*.nes;*.unf;*.fds;*.zip\0"
					"iNes ROM Images (*.nes)\0"
					"*.nes\0"
					"UNIF ROM Images (*.unf)\0"
					"*.unf\0"
					"Famicom Disk System Images (*.fds)\0"
					"*.fds\0"
					"Zip Files (*.zip)\0"
					"*.zip\0"
					"All files (*.*)\0"
					"*.*\0"									   
				),
				extensions,
				DONT_ADD_RECENT_FILE
			);

			if (PDX_FAILED(result))
				return PDX_FAILURE;
		}
	}

	if (!ImageFile.IsOpen())
		return PDX_FAILURE;

	if (!ImageFile.Readable(sizeof(U32)))
		return UI::MsgWarning(IDS_FILE_CORRUPT);

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FILEMANAGER::ApplyNsp(const PDXSTRING& ImageName,NES::IO::NSP::CONTEXT& context,BOOL& ShowWarning) const
{
	PDXSTRING NspName;

	if (FindNsp( ImageName, NspName ))
	{
		PDXFILE NspFile( NspName, PDXFILE::INPUT );

		NES::NSP nsp;

		if (PDX_SUCCEEDED(nsp.Load( NspFile )))
		{
			nsp.GetContext( context );
			return PDX_OK;
		}
		else 
		{
			if (application.GetPreferences().ShowWarnings() && ShowWarning)
			{
				ShowWarning = FALSE;

				UI::MsgWarning
				( 
					PDXSTRING("NSP file: \"") << NspName << " was found but parsing failed!"
				);
			}
		} 
	}

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FILEMANAGER::ApplyIps(PDXFILE& ImageFile,PDXSTRING& IpsName,BOOL& ShowWarning)
{
	LastIpsFile.Clear();

	PDXFILE IpsFile;

	if (IpsName.Length())
		IpsFile.Open( IpsName, PDXFILE::INPUT );

	if (!IpsFile.IsOpen() && FindIps( ImageFile.FileName(), IpsName ))
		IpsFile.Open( IpsName, PDXFILE::INPUT );

	if (IpsFile.IsOpen())
	{
		if (PDX_SUCCEEDED(NES::IPS::Load( ImageFile, IpsFile )))
		{
			LastIpsFile = IpsName;
			return PDX_OK;
		}
		else 
		{
			if (application.GetPreferences().ShowWarnings() && ShowWarning)
			{
				ShowWarning = FALSE;

				UI::MsgWarning
				( 
				    PDXSTRING("IPS file: \"") << IpsName << " was found but the image file couldn't be patched!"
				);
			}
		} 
	}

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FILEMANAGER::ApplySav(const PDXSTRING& ImageName,PDXSTRING& SaveName)
{
	LastSaveFile.Clear();

	PDXFILE SaveFile;

	if (SaveName.Length())
		SaveFile.Open( SaveName, PDXFILE::INPUT );

	if (!SaveFile.IsOpen())
		FindSav( ImageName, SaveName );

	LastSaveFile = SaveName;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FILEMANAGER::ApplyNst(PDXFILE& StateFile,PDXSTRING& StateName,BOOL& ShowWarning) const
{
	if (StateName.Length())
	{
		StateFile.Open( StateName, PDXFILE::INPUT );

		if (!StateFile.IsOpen() && application.GetPreferences().ShowWarnings() && ShowWarning)
		{
			ShowWarning = FALSE;

			UI::MsgWarning
			( 
    			PDXSTRING("State file: \"") << StateName << " was referenced but not found!"
			);
		}
	}

	return StateFile.IsOpen() ? PDX_OK : PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL FILEMANAGER::FindRom(const PDXSTRING& FileName,PDXSTRING& ImageName) const
{
	const CHAR* types[4] = {"nes","unf","unif","fds"}; 

	ImageName = FileName;

	for (UINT i=0; i < 4; ++i)
	{
		ImageName.ReplaceFileExtension( types[i] );

		if (UTILITIES::FileExist( ImageName ))
			return TRUE;
	}

	ImageName.ReplaceFilePath( RomPath );

	for (UINT i=0; i < 4; ++i)
	{
		ImageName.ReplaceFileExtension( types[i] );

		if (UTILITIES::FileExist( ImageName ))
			return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL FILEMANAGER::FindIps(const PDXSTRING& ImageName,PDXSTRING& IpsName) const
{
	if (AutoApplyIps)
	{
		IpsName = ImageName;
		IpsName.ReplaceFileExtension( "ips" );

		if (UseIpsPathRom)
		{
			if (UTILITIES::FileExist( IpsName ))
				return TRUE;
		}

		IpsName.ReplaceFilePath( IpsPath );

		return UTILITIES::FileExist( IpsName );
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL FILEMANAGER::FindSav(const PDXSTRING& ImageName,PDXSTRING& SaveName) const
{
	SaveName = ImageName;
	SaveName.ReplaceFileExtension( "sav" );

	if (UseSavePathRom)
	{
		if (UTILITIES::FileExist( SaveName ))
			return TRUE;
	}

	SaveName.ReplaceFilePath( SavePath );

	return UTILITIES::FileExist( SaveName );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL FILEMANAGER::FindNsp(const PDXSTRING& ImageName,PDXSTRING& NspName) const
{
	if (AutoApplyNsp)
	{
		NspName = ImageName;
		NspName.ReplaceFileExtension( "nsp" );

		if (UseNspPathRom)
		{
			if (UTILITIES::FileExist( NspName ))
				return TRUE;
		}

		NspName.ReplaceFilePath( UseNspPathLast ? NspPathLast : NspPath );

		return UTILITIES::FileExist( NspName );
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL FILEMANAGER::FindNst(const PDXSTRING& ImageName,PDXSTRING& NstName) const
{
	NstName = ImageName;
	NstName.ReplaceFileExtension( "nst" );

	if (UTILITIES::FileExist( NstName ))
		return TRUE;

	NstName.ReplaceFilePath( UseNstPathLast ? NstPathLast : NstPath );

	return UTILITIES::FileExist( NstName );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL FILEMANAGER::FindSlt(const UINT index,const PDXSTRING& ImageName,PDXSTRING& SlotName) const
{
	SlotName = ImageName;
	SlotName.ReplaceFileExtension( "ns" );
	SlotName << index;

	if (UseNstPathRom)
	{
		if (UTILITIES::FileExist( SlotName ))
			return TRUE;
	}

	SlotName.ReplaceFilePath( UseNstPathLast ? NstPathLast : NstPath );

	return UTILITIES::FileExist( SlotName );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FILEMANAGER::LoadFile
(
    COMMAND command,
	const VOID* const param,
    PDXFILE& file,
	PDXSTRING* const TargetDir,
    const CHAR* const path,
	const CHAR* const title,
	const CHAR* const filter,
	const PDXARRAY<PDXSTRING>& extensions,
	const RECENT_FILE_COMMAND RecentFileCommand
)
{
	PDX_ASSERT( !file.IsOpen() );

	PDXSTRING filename;

	switch (command)
	{
     	case COMMAND_CHOOSE_FILE:
		{
			application.GetGraphicManager().BeginDialogMode();
			const BOOL success = UTILITIES::BrowseOpenFile( filename, hWnd, title, filter, path );
			application.GetGraphicManager().EndDialogMode();

			if (!success)
				return PDX_FAILURE;

			if (TargetDir)
			{
				*TargetDir = filename;
				UTILITIES::ValidateDir( *TargetDir );
			}
			break;
		}

		case COMMAND_INPUT_FILE:
		{
			PDX_ASSERT( param );
			filename = PDX_CAST(const CHAR*,param);
			break;
		}

		case COMMAND_ZIPPED_FILE:
		{
			PDX_ASSERT( param );
			
			typedef PDXPAIR<PDXSTRING,PDXSTRING> PAIR;
			const PAIR* const pair = PDX_CAST(const PAIR*,param);
			
			filename = pair->First();

			ZIPFILE ZipFile;

			if (PDX_FAILED(ZipFile.Open( filename.String() )))
				return PDX_FAILURE;

			BOOL success = FALSE;
			const TSIZE NumFiles = ZipFile.NumFiles();

			PDXARRAY<CHAR> buffer;

			for (TSIZE i=0; i < NumFiles; ++i)
			{
				if (pair->Second() == ZipFile.FileName( i ))
				{
					success = PDX_SUCCEEDED(ZipFile.Uncompress( i, buffer ));
					break;
				}
			}

			if (!success)
				return PDX_FAILURE;

			file.Hook( buffer, PDXFILE::INPUT, filename.String() );
			buffer.UnHook();
			break;
		}

		case COMMAND_RECENT_FILE:
		{
			PDX_ASSERT( param );
			const UINT RecentFileIndex = *PDX_CAST(const UINT*,param);

			filename = RecentFiles[RecentFileIndex];
			const BOOL IsZip = filename.IsFileExtension( "zip", 3 );

			if (RecentFileIndex == 0 && nes.IsOn())
			{
				if (IsZip && OpenZipFile( NULL, filename.String(), extensions, file, TRUE ) > 1)
				{
					if (!OpenZipFile( NULL, filename.String(), extensions, file ))
					{
						RecentFiles.Erase( RecentFiles.Begin() );
						UpdatedRecentFile = TRUE;
						return UI::MsgWarning(IDS_FILE_ZIP_EMPTY);
					}
				}
				else
				{
					return PDX_NOT_UPDATED;
				}
			}
			else
			{
				if ((IsZip && !OpenZipFile(NULL,filename.String(),extensions,file)) || (!IsZip && PDX_FAILED(file.Open(filename,PDXFILE::INPUT))))
				{
					RecentFiles.Erase( RecentFiles.At(RecentFileIndex) );
					UpdatedRecentFile = TRUE;
					return UI::MsgWarning(IsZip ? IDS_FILE_ZIP_EMPTY : IDS_FILE_OPEN_ERROR);
				}
			}
		}
	}

	switch (command)
	{
     	case COMMAND_CHOOSE_FILE:
       	case COMMAND_INPUT_FILE:

			if (filename.IsFileExtension( "zip", 3 ))
			{
				if (!OpenZipFile( NULL, filename.String(), extensions, file ))
					return UI::MsgWarning(IDS_FILE_ZIP_EMPTY);
			}
			else
			{
				if (PDX_FAILED(file.Open( filename, PDXFILE::INPUT )))
					return UI::MsgWarning(IDS_FILE_OPEN_ERROR);
			}
			break;
	}

	if (RecentFileCommand == ADD_RECENT_FILE)
		AddRecentFile( filename.String() );

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FILEMANAGER::AddRecentFile(const CHAR* const filename)
{
	for (UINT i=0; i < RecentFiles.Size(); ++i)
	{
		if (RecentFiles[i] == filename)
		{
			if (i)
			{
				RecentFiles.Erase( RecentFiles.At(i) );
				break;
			}
			else
			{
				return;
			}
		}
	}

	UpdatedRecentFile = TRUE;

	if (RecentFiles.Size() > MAX_RECENT_FILES)
		RecentFiles.EraseBack();

	RecentFiles.Insert( RecentFiles.Begin(), PDXSTRING(filename) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FILEMANAGER::SaveNST()
{
	if (nes.IsOff() || !nes.IsImage())
		return PDX_FAILURE;

	application.GetGraphicManager().BeginDialogMode();

	PDXSTRING filename;

	BOOL succeeded = UTILITIES::BrowseSaveFile
	(
	    filename,
		hWnd,
		IDS_FILE_SAVE_NST,
     	"Nestopia State Files (*.nst)\0"
		"*.nst\0"
		"All Files (*.*)\0"
		"*.*\0",
		GetNstPath().String(),
		"nst"
	);

	if (succeeded)
	{
		NstPathLast = filename;
		UTILITIES::ValidateDir( NstPathLast );

		PDXFILE file( filename, PDXFILE::OUTPUT );

		if (succeeded = file.IsOpen())
			succeeded = PDX_SUCCEEDED(nes.SaveNST( file ));
	}

	application.GetGraphicManager().EndDialogMode();

	return succeeded ? PDX_OK : PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FILEMANAGER::SaveNSP()
{
	if (!nes.IsImage())
		return PDX_FAILURE;

	application.GetGraphicManager().BeginDialogMode();

	PDXSTRING filename;

	BOOL succeeded = UTILITIES::BrowseSaveFile
	(
	    filename,
		hWnd,
		IDS_FILE_SAVE_NSP,
   		"Nestopia Script Files (*.nsp)\0"
		"*.nsp\0"
		"All Files (*.*)\0"
		"*.*\0",
		GetNspPath().String(),
		"nsp"
	);

	if (succeeded)
	{
		NspPathLast = filename;
		UTILITIES::ValidateDir( NspPathLast );
	}

	PDXFILE file;

	if (succeeded && (succeeded = PDX_SUCCEEDED(file.Open( filename, PDXFILE::OUTPUT ))))
	{
		NES::NSP::CONTEXT context;

		context.ImageFile = LastImageFile;
		context.IpsFile = LastIpsFile;

		if (nes.IsCartridge() && nes.GetCartridgeInfo()->battery)
			context.SaveFile = LastSaveFile;

		{
			PDXFILE SlotFile;

			for (UINT i=0; i < SAVESTATEMANAGER::MAX_SLOTS; ++i)
			{
				const PDXSTRING& name = application.GetSaveStateManager().GetSlotFile(i+1);

				if (name.Length() && PDX_SUCCEEDED(SlotFile.Open( name, PDXFILE::INPUT )) && SlotFile.Size())
					context.StateSlots[i] = name;
			}
		}

		if (application.GetGraphicManager().IsCustomPalette())
			context.PaletteFile = application.GetGraphicManager().GetPaletteFile();

		const UINT NumCodes = application.GetGameGenieManager().NumCodes();

		if (NumCodes)
		{
			context.GenieCodes.Resize( NumCodes );

			for (UINT i=0; i < NumCodes; ++i)
			{
				application.GetGameGenieManager().GetCode
				(
					i, 
					context.GenieCodes[i].code, 
					context.GenieCodes[i].enabled,
					&context.GenieCodes[i].comment 
				);
			}
		}

		context.pal = nes.IsPAL() ? 1 : 0;

		for (UINT i=0; i < 5; ++i)
			context.controllers[i] = nes.ConnectedController(i);

		NES::NSP nsp;
		nsp.SetContext( context );
		succeeded = PDX_SUCCEEDED(nsp.Save( file ));
	}

	application.GetGraphicManager().EndDialogMode();

	return succeeded ? PDX_OK : PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

UINT FILEMANAGER::OpenZipFile
(
	const CHAR* const title,
    const CHAR* const name,
	const PDXARRAY<PDXSTRING>& extensions,
	PDXFILE& file,
	const BOOL JustCheck
)
{
	PDX_ASSERT(name && !ZipFile);

	ZipDialogTitle = (title ? title : UTILITIES::IdToString(IDS_FILE_ZIP_TITLE).String());

	ZipFile = new ZIPFILE();

	if (PDX_FAILED(ZipFile->Open( name, &extensions )))
	{
		delete ZipFile;
		ZipFile = NULL;
		return 0;
	}

	if (JustCheck)
	{
		const UINT NumFiles = ZipFile->NumFiles();

		delete ZipFile;
		ZipFile = NULL;

		return NumFiles;
	}

	CompressedFileIndex = -1;

	if (ZipFile->NumFiles() == 1)
	{
		CompressedFileIndex = 0;
	}
	else if (ZipFile->NumFiles() > 1)
	{
		DialogBoxParam
		(
			::GetModuleHandle(NULL),
			MAKEINTRESOURCE(IDD_COMPRESSED_FILE),
			hWnd,
			CompressedFileDialogProc,
			PDX_CAST(LPARAM,this)
		); 
	}

	if (CompressedFileIndex != -1)
	{
		PDXARRAY<CHAR> buffer;

		if (PDX_FAILED(ZipFile->Uncompress( CompressedFileIndex, buffer )))
		{
			delete ZipFile;
			ZipFile = NULL;
			return 0;
		}

		file.Hook( buffer, PDXFILE::INPUT, ZipFile->FileName( CompressedFileIndex ).String() );
		buffer.UnHook();
	}

	delete ZipFile;
	ZipFile = NULL;

	return CompressedFileIndex == -1 ? 0 : 1;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK FILEMANAGER::CompressedFileDialogProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	static FILEMANAGER* fm = NULL;

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{
			PDX_ASSERT( lParam && !fm );
			fm = PDX_CAST(FILEMANAGER*,lParam);

			::SetWindowText( hDlg, fm->ZipDialogTitle.String() );

			HWND hList = ::GetDlgItem( hDlg, IDC_COMPRESSED_FILE_LIST );

			for (UINT i=0; i < fm->ZipFile->NumFiles(); ++i)
				ListBox_AddString( hList, fm->ZipFile->FileName(i).String() );

			ListBox_SetCurSel( hList, 0 );
			return TRUE;
		}

		case WM_COMMAND:

         	switch (LOWORD(wParam))
			{
				case IDC_COMPRESSED_FILE_OK:
		
					PDX_ASSERT( fm );
					fm->CompressedFileIndex = ListBox_GetCurSel( ::GetDlgItem( hDlg, IDC_COMPRESSED_FILE_LIST ) );
		
				case IDC_COMPRESSED_FILE_CANCEL:
		
					::EndDialog( hDlg, 0 );
					return TRUE;
			}
			return FALSE;

		case WM_CLOSE:

			::EndDialog( hDlg, 0 );
			return TRUE;

		case WM_DESTROY:

			PDX_ASSERT( fm );
			fm = NULL;
			return TRUE;
	}

	return FALSE;
}
