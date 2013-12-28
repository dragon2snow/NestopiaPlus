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

#include "resource/resource.h"
#include "NstApplication.h"
#include "NstGraphicManager.h"
#include "NstGameGenieManager.h"
#include "NstPreferences.h"
#include "NstFileManager.h"
#include "NstSaveStateManager.h"
#include "NstMovieManager.h"
#include "../NstZipFile.h"
#include "../core/NstIps.h"
#include "../core/NstNsp.h"
#include <WindowsX.h>
#include <ShlObj.h>
#include "../paradox/PdxFile.h"

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

PDXRESULT FILEMANAGER::GetExeFileName(PDXSTRING& name)
{
	CHAR buffer[NST_MAX_PATH];
	buffer[0] = '\0';

	if (GetModuleFileName( NULL, buffer, NST_MAX_PATH-1 ) && buffer[0] != '\0')
	{
		name = buffer;
		return PDX_OK;
	}

	name.Clear();

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FILEMANAGER::GetExeFilePath(PDXSTRING& path)
{
	CHAR buffer[NST_MAX_PATH];
	buffer[0] = '\0';

	if (GetModuleFileName( NULL, buffer, NST_MAX_PATH-1 ) && buffer[0] != '\0')
	{
		path = buffer;
		path = path.GetFilePath();

		if (path.Length())
		{
			if (path.Back() != '\\')
				path.InsertBack('\\');

			return PDX_OK;
		}
	}

	path.Clear();

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FILEMANAGER::GetCurrentPath(PDXSTRING& path)
{
	CHAR buffer[NST_MAX_PATH];
	buffer[0] = '\0';

	if (GetCurrentDirectory( NST_MAX_PATH-1, buffer ) && buffer[0] != '\0')
	{
		path = buffer;

		if (path.Back() != '\\')
			path.InsertBack('\\');

		return PDX_OK;
	}

	path.Clear();

	return PDX_FAILURE;
}

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

		if (RomPath.IsEmpty() || GetFileAttributes(RomPath.String()) == INVALID_FILE_ATTRIBUTES)
		{
			if (PDX_FAILED(GetExeFilePath(RomPath)))
				RomPath = "C:\\";
		}

		if (SavePath.IsEmpty()      || GetFileAttributes( SavePath.String()    ) == INVALID_FILE_ATTRIBUTES) SavePath    = RomPath;
		if (NstPath.IsEmpty()       || GetFileAttributes( NstPath.String()     ) == INVALID_FILE_ATTRIBUTES) NstPath     = RomPath;
		if (IpsPath.IsEmpty()       || GetFileAttributes( IpsPath.String()     ) == INVALID_FILE_ATTRIBUTES) IpsPath     = RomPath;
		if (NspPath.IsEmpty()       || GetFileAttributes( NspPath.String()     ) == INVALID_FILE_ATTRIBUTES) NspPath     = RomPath;
		if (RomPathLast.IsEmpty()   || GetFileAttributes( RomPathLast.String() ) == INVALID_FILE_ATTRIBUTES) RomPathLast = RomPath;
		if (NstPathLast.IsEmpty()   || GetFileAttributes( NstPathLast.String() ) == INVALID_FILE_ATTRIBUTES) NstPathLast = NstPath;
		if (NspPathLast.IsEmpty()   || GetFileAttributes( NspPathLast.String() ) == INVALID_FILE_ATTRIBUTES) NspPathLast = NspPath;

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

		string = &file["files recent 0"]; if (string && GetFileAttributes(string->String()) != INVALID_FILE_ATTRIBUTES) RecentFiles.InsertBack(*string);
		string = &file["files recent 1"]; if (string && GetFileAttributes(string->String()) != INVALID_FILE_ATTRIBUTES) RecentFiles.InsertBack(*string);
		string = &file["files recent 2"]; if (string && GetFileAttributes(string->String()) != INVALID_FILE_ATTRIBUTES) RecentFiles.InsertBack(*string);
		string = &file["files recent 3"]; if (string && GetFileAttributes(string->String()) != INVALID_FILE_ATTRIBUTES) RecentFiles.InsertBack(*string);
		string = &file["files recent 4"]; if (string && GetFileAttributes(string->String()) != INVALID_FILE_ATTRIBUTES) RecentFiles.InsertBack(*string);
		string = &file["files recent 5"]; if (string && GetFileAttributes(string->String()) != INVALID_FILE_ATTRIBUTES) RecentFiles.InsertBack(*string);
		string = &file["files recent 6"]; if (string && GetFileAttributes(string->String()) != INVALID_FILE_ATTRIBUTES) RecentFiles.InsertBack(*string);
		string = &file["files recent 7"]; if (string && GetFileAttributes(string->String()) != INVALID_FILE_ATTRIBUTES) RecentFiles.InsertBack(*string);
		string = &file["files recent 8"]; if (string && GetFileAttributes(string->String()) != INVALID_FILE_ATTRIBUTES) RecentFiles.InsertBack(*string);
		string = &file["files recent 9"]; if (string && GetFileAttributes(string->String()) != INVALID_FILE_ATTRIBUTES) RecentFiles.InsertBack(*string);
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

		file[ "files path image"                   ] = RomPath;
		file[ "files path battery"                 ] = SavePath;
		file[ "files path nst"                     ] = NstPath;
		file[ "files path ips"                     ] = IpsPath;
		file[ "files path nsp"                     ] = NspPath;
		file[ "files last path image"              ] = RomPathLast; 
		file[ "files last path nst"                ] = NstPathLast;
		file[ "files last path nsp"                ] = NspPathLast;
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

		if (RecentFiles.Size() > 0) file[ "files recent 0" ] = RecentFiles[0];
		if (RecentFiles.Size() > 1) file[ "files recent 1" ] = RecentFiles[1];
		if (RecentFiles.Size() > 2) file[ "files recent 2" ] = RecentFiles[2];
		if (RecentFiles.Size() > 3) file[ "files recent 3" ] = RecentFiles[3];
		if (RecentFiles.Size() > 4) file[ "files recent 4" ] = RecentFiles[4];
		if (RecentFiles.Size() > 5) file[ "files recent 5" ] = RecentFiles[5];
		if (RecentFiles.Size() > 6) file[ "files recent 6" ] = RecentFiles[6];
		if (RecentFiles.Size() > 7) file[ "files recent 7" ] = RecentFiles[7];
		if (RecentFiles.Size() > 8) file[ "files recent 8" ] = RecentFiles[8];
		if (RecentFiles.Size() > 9) file[ "files recent 9" ] = RecentFiles[9];
	}								
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FILEMANAGER::Reset()
{
	RomPath.Clear();

	if (PDX_FAILED(GetExeFilePath(RomPath)))
		RomPath = "C:\\";

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

VOID FILEMANAGER::ValidatePath(PDXSTRING& path)
{
	if (path.IsEmpty() || GetFileAttributes(path.String()) == INVALID_FILE_ATTRIBUTES)
	{
		if (PDX_FAILED(GetExeFilePath(path)))
			path = "C:\\";
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FILEMANAGER::UpdateSettings()
{
	PDX_ASSERT( hDlg );

	RomPath.  Buffer().Resize( NST_MAX_PATH );
	SavePath. Buffer().Resize( NST_MAX_PATH );
	NstPath.  Buffer().Resize( NST_MAX_PATH );
	IpsPath.  Buffer().Resize( NST_MAX_PATH );
	NspPath.  Buffer().Resize( NST_MAX_PATH );

	RomPath.  Buffer().Front() = '\0';
	SavePath. Buffer().Front() = '\0';
	NstPath.  Buffer().Front() = '\0';
	IpsPath.  Buffer().Front() = '\0';
	NspPath.  Buffer().Front() = '\0';

	GetDlgItemText( hDlg, IDC_PATHS_IMAGE,   RomPath.Begin(),   NST_MAX_PATH );
	GetDlgItemText( hDlg, IDC_PATHS_BATTERY, SavePath.Begin(),  NST_MAX_PATH );
	GetDlgItemText( hDlg, IDC_PATHS_NST,     NstPath.Begin(),   NST_MAX_PATH );
	GetDlgItemText( hDlg, IDC_PATHS_IPS,     IpsPath.Begin(),   NST_MAX_PATH );
	GetDlgItemText( hDlg, IDC_PATHS_NSP,     NspPath.Begin(),   NST_MAX_PATH );

	RomPath.   Validate();
	SavePath.  Validate();
	NstPath.   Validate();
	IpsPath.   Validate();
	NspPath.   Validate();

	if (RomPath.Length()   && RomPath.Back()   != '\\') RomPath  += "\\";
	if (SavePath.Length()  && SavePath.Back()  != '\\') SavePath += "\\";
	if (NstPath.Length()   && NstPath.Back()   != '\\') NstPath  += "\\";
	if (IpsPath.Length()   && IpsPath.Back()   != '\\') IpsPath  += "\\";
	if (NspPath.Length()   && NspPath.Back()   != '\\') NspPath  += "\\";

	ValidatePath( RomPath  );
	ValidatePath( SavePath );
	ValidatePath( NstPath  );
	ValidatePath( IpsPath  );
	ValidatePath( NspPath  );

	UseRomPathLast      = IsDlgButtonChecked( hDlg, IDC_PATHS_IMAGE_LAST            ) == BST_CHECKED;  
	DisableSaveRamWrite = IsDlgButtonChecked( hDlg, IDC_PATHS_BATTERY_PROTECT       ) == BST_CHECKED;  
	UseSavePathRom      = IsDlgButtonChecked( hDlg, IDC_PATHS_BATTERY_IN_IMAGE      ) == BST_CHECKED;  
	UseNspPathRom       = IsDlgButtonChecked( hDlg, IDC_PATHS_NSP_IN_IMAGE          ) == BST_CHECKED; 
	UseNstPathRom       = IsDlgButtonChecked( hDlg, IDC_PATHS_NST_IN_IMAGE          ) == BST_CHECKED; 
	UseIpsPathRom       = IsDlgButtonChecked( hDlg, IDC_PATHS_IPS_IN_IMAGE          ) == BST_CHECKED; 
	AutoApplyIps        = IsDlgButtonChecked( hDlg, IDC_PATHS_IPS_AUTO_APPLY        ) == BST_CHECKED; 
	AutoImportNst       = IsDlgButtonChecked( hDlg, IDC_PATHS_NST_AUTO_IMPORT       ) == BST_CHECKED; 
	AutoExportNst       = IsDlgButtonChecked( hDlg, IDC_PATHS_NST_AUTO_EXPORT       ) == BST_CHECKED; 
	AutoApplyNsp        = IsDlgButtonChecked( hDlg, IDC_PATHS_NSP_AUTO_APPLY        ) == BST_CHECKED; 
	UseNspPathLast      = IsDlgButtonChecked( hDlg, IDC_PATHS_NSP_LAST              ) == BST_CHECKED; 
	UseNstPathLast      = IsDlgButtonChecked( hDlg, IDC_PATHS_NST_LAST              ) == BST_CHECKED; 

	application.GetSaveStateManager().EnableFileImport( AutoImportNst );
	application.GetSaveStateManager().EnableFileExport( AutoExportNst );

	if (!UseRomPathLast || RomPathLast.IsEmpty() || GetFileAttributes( RomPathLast.String() ) == INVALID_FILE_ATTRIBUTES)
		RomPathLast = RomPath;

	if (!UseNstPathLast || NstPathLast.IsEmpty() || GetFileAttributes( NstPathLast.String() ) == INVALID_FILE_ATTRIBUTES)
		NstPathLast = NstPath;

	if (!UseNspPathLast || NspPathLast.IsEmpty() || GetFileAttributes( NspPathLast.String() ) == INVALID_FILE_ATTRIBUTES)
		NspPathLast = NspPath;

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
					
					if (SelectPath( path, "Select your image file (*.nes,*.unf,*.fds,*.nsf) directory.." ))
						SetDlgItemText( hDlg, IDC_PATHS_IMAGE, path.String() );

					return TRUE;
				}
		
				case IDC_PATHS_BATTERY_BROWSE:
				{		
					PDXSTRING path;
					
					if (SelectPath( path, "Select your save ram (*.sav) directory.." ))
						SetDlgItemText( hDlg, IDC_PATHS_BATTERY, path.String() );

					return TRUE;
				}
		
				case IDC_PATHS_NST_BROWSE:
				{
					PDXSTRING path;
					
					if (SelectPath( path, "Select your save state (*.nst) directory.." ))
						SetDlgItemText( hDlg, IDC_PATHS_NST, path.String() );

					return TRUE;
				}

				case IDC_PATHS_IPS_BROWSE:
				{
					PDXSTRING path;

					if (SelectPath( path, "Select your patch file (*.ips) directory.." ))
						SetDlgItemText( hDlg, IDC_PATHS_IPS, path.String() );

					return TRUE;
				}

				case IDC_PATHS_NSP_BROWSE:
				{
					PDXSTRING path;

					if (SelectPath( path, "Select your game config file (*.nsp) directory.." ))
						SetDlgItemText( hDlg, IDC_PATHS_NSP, path.String() );

					return TRUE;
				}

				case IDC_PATHS_DEFAULT:
		
					Reset();
					UpdateDialog();
					return TRUE;
		
				case IDC_PATHS_CANCEL:

					EndDialog( hDlg, 0 );
					return TRUE;

				case IDC_PATHS_OK:
		
					UpdateSettings();
					EndDialog( hDlg, 0 );
					return TRUE;
			}
			return FALSE;

		case WM_CLOSE:

			EndDialog( hDlg, 0 );
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

BOOL FILEMANAGER::SelectPath(PDXSTRING& path,const CHAR* const title)
{
	CHAR name[NST_MAX_PATH];
	name[0] = '\0';

	BROWSEINFO bi;
	PDXMemZero( bi );

	bi.hwndOwner	  = hWnd;
	bi.pszDisplayName = name;
	bi.lpszTitle	  = title;
	bi.ulFlags		  = BIF_RETURNONLYFSDIRS;

	LPITEMIDLIST idl = SHBrowseForFolder( &bi );

	BOOL yep = FALSE;

	if (idl)
	{
		if (SHGetPathFromIDList( idl, name ))
		{
			path = name;
			yep = TRUE;
		}

		LPMALLOC pMalloc;

		if (SUCCEEDED(SHGetMalloc( &pMalloc ))) 
		{
			pMalloc->Free( idl );
			pMalloc->Release();
		}
	}

	return yep;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FILEMANAGER::UpdateDialog()
{
	SetDlgItemText( hDlg, IDC_PATHS_IMAGE,                 RomPath.String()    );
	SetDlgItemText( hDlg, IDC_PATHS_BATTERY,               SavePath.String()   );
	SetDlgItemText( hDlg, IDC_PATHS_NST,                   NstPath.String()    );
	SetDlgItemText( hDlg, IDC_PATHS_IPS,                   IpsPath.String()    );
	SetDlgItemText( hDlg, IDC_PATHS_NSP,                   NspPath.String()    );
	CheckDlgButton( hDlg, IDC_PATHS_IMAGE_LAST,            UseRomPathLast      ); 
	CheckDlgButton( hDlg, IDC_PATHS_BATTERY_IN_IMAGE,      UseSavePathRom      );
	CheckDlgButton( hDlg, IDC_PATHS_NST_LAST,              UseNstPathLast      );
	CheckDlgButton( hDlg, IDC_PATHS_NST_IN_IMAGE,          UseNstPathRom       );
	CheckDlgButton( hDlg, IDC_PATHS_NST_AUTO_IMPORT,       AutoImportNst       );
	CheckDlgButton( hDlg, IDC_PATHS_NST_AUTO_EXPORT,       AutoExportNst       );
	CheckDlgButton( hDlg, IDC_PATHS_BATTERY_PROTECT,       DisableSaveRamWrite );
	CheckDlgButton( hDlg, IDC_PATHS_IPS_IN_IMAGE,          UseIpsPathRom       );
	CheckDlgButton( hDlg, IDC_PATHS_IPS_AUTO_APPLY,        AutoApplyIps        );
	CheckDlgButton( hDlg, IDC_PATHS_NSP_IN_IMAGE,          UseNspPathRom       );
	CheckDlgButton( hDlg, IDC_PATHS_NSP_AUTO_APPLY,        AutoApplyNsp        );
	CheckDlgButton( hDlg, IDC_PATHS_NSP_LAST,              UseNspPathLast      );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FILEMANAGER::Load(INT RecentFileIndex,const BOOL power)
{
	PDX_ASSERT(RecentFileIndex >= -1 && RecentFileIndex < LONG(RecentFiles.Size()));

	application.GetGraphicManager().BeginDialogMode();

	UpdatedRecentFile = FALSE;

	PDXFILE file;
	PDXRESULT result;

	{
     	PDXARRAY<PDXSTRING> extensions;	
       	extensions.Resize( 9 );

     	extensions[0] = "nes";
		extensions[1] = "unf";
		extensions[2] = "unif";
		extensions[3] = "fds";
     	extensions[4] = "nsf";
     	extensions[5] = "nst";
		extensions[6] = "nsv";
		extensions[7] = "nsp";
		extensions[8] = "ips";

     	result = LoadFile
		( 
     		file,
			(GetFileAttributes(GetRomPath().String()) != INVALID_FILE_ATTRIBUTES ? GetRomPath().String() : NULL),
     		"Open..",
			(
				"All supported files\0"
				"*.nes;*.unf;*.fds;*.nsf;*.nst;*.nsp;*.nsv;*.ips;*.zip\0"
				"iNes ROM Images (*.nes)\0"
				"*.nes\0"
				"UNIF ROM Images (*.unf)\0"
				"*.unf\0"
				"Famicom Disk System Images (*.fds)\0"
				"*.fds\0"
				"NES Sound Files (*.nsf)\0"
				"*.nsf\0"
				"Nestopia State Files (*.nst)\0"
				"*.nst\0"
				"Nestopia Game Configuration Files (*.nsp)\0"
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
     		TRUE,
       		RecentFileIndex
		);
	}

	if (PDX_SUCCEEDED(result))
	{
		PDX_ASSERT(file.IsOpen());
		RomPathLast = file.FilePath();

		if (PDX_FAILED(LoadNesFile( file, power )))
		{
			UpdatedRecentFile = TRUE;
			RecentFiles.Erase( RecentFiles.Begin() );
			result = PDX_FAILURE;
			nes.Power( FALSE );
		}
	}

	application.GetGraphicManager().EndDialogMode();

	return result;														
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FILEMANAGER::LoadNSP(INT RecentFileIndex,const BOOL power)
{
	PDX_ASSERT(RecentFileIndex >= -1 && RecentFileIndex < LONG(RecentFiles.Size()));

	application.GetGraphicManager().BeginDialogMode();

	UpdatedRecentFile = FALSE;

	PDXFILE file;
	PDXRESULT result;

	{
		PDXARRAY<PDXSTRING> extension;
		extension.InsertBack(PDXSTRING("nsp"));

		result = LoadFile
		( 
			file,
			(GetFileAttributes(GetNspPath().String()) != INVALID_FILE_ATTRIBUTES ? GetNspPath().String() : NULL),
			"Load Game Configuration",
			(
				"All supported files (*.nsp, *.zip)\0"
				"*.nsp;*.zip\0"
				"Nestopia Game Configuration Files (*.nsp)\0"
				"*.nsp\0"
				"Zip Files (*.zip)\0"
				"*.zip\0"
				"All files (*.*)\0"
				"*.*\0"
			),
			extension,
			TRUE,
			RecentFileIndex
		);
	}

	if (PDX_SUCCEEDED(result))
	{
		PDX_ASSERT(file.IsOpen());
		NspPathLast = file.FilePath();

		if (PDX_FAILED(LoadNesFile( file, power )))
		{
			UpdatedRecentFile = TRUE;
			RecentFiles.Erase( RecentFiles.Begin() );
			result = PDX_FAILURE;
			nes.Power( FALSE );
		}
	}

	application.GetGraphicManager().EndDialogMode();

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

	application.GetGraphicManager().BeginDialogMode();

	PDXFILE file;
	PDXRESULT result;

	{
		PDXARRAY<PDXSTRING> extension;
		extension.InsertBack(PDXSTRING("nst"));

		result = LoadFile
		( 
			file,
			(GetFileAttributes(GetNstPath().String()) != INVALID_FILE_ATTRIBUTES ? GetNstPath().String() : NULL),
			"Load State",
			(
	     		"All supported files (*.nst, *.zip)\0"
	     		"*.nst;*.zip\0"
	       		"Nestopia State Files (*.nst)\0"
	     		"*.nst\0"
				"Zip Files (*.zip)\0"
				"*.zip\0"
	     		"All files (*.*)\0"
	       		"*.*\0"
			),
			extension
		);
	}

	if (PDX_SUCCEEDED(result))
	{
		PDX_ASSERT(file.IsOpen());

		NstPathLast = file.FilePath();

		if (file.Readable(sizeof(U32)))
		{
			if (file.Peek<U32>() == NES_MAGIC_NST)
				result = LoadNesFile( file, TRUE );
			else
				result = application.OnWarning("Not a state file!");
		}
		else
		{
			result = application.OnWarning("Corrupt file!");
		}
	}

	application.GetGraphicManager().EndDialogMode();

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FILEMANAGER::LoadNesFile(PDXFILE& file,const BOOL power)
{
	if (!file.Readable(sizeof(U32)))
		return application.OnWarning("Corrupt file!");
	
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
				return application.OnWarning("IPS patching failed!");

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
			{
				NES::NSP nsp;

				if (PDX_FAILED(nsp.Load( file )))
				{
					return application.OnWarning
					(
					    file.FileExtension() == "nsp" ?
						"Parse error!" :
					    "Invalid file!"
					);
				}

				NspLoaded = TRUE;
				nsp.GetContext( NspContext );
			}

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

				PDXSTRING msg;

				msg  = "NSV file: \"";
				msg += NspContext.MovieFile;
				msg += " was referenced but loading failed!";

				application.OnWarning( msg.String() );
				
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
		application.GetGameGenieManager().ClearAllCodes();

		for (UINT i=0; i < NspContext.GenieCodes.Size(); ++i)
		{
			application.GetGameGenieManager().AddCode
			(
			    NspContext.GenieCodes[i].code,
				TRUE,
				(
    				NspContext.GenieCodes[i].comment.Length() ? 
					&NspContext.GenieCodes[i].comment : NULL
				)
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
		if (application.OnQuestion("Image file not found!","A matching image file couldn't be found. Do you want to specify it?"))
		{
			PDXARRAY<PDXSTRING> extensions;	
			extensions.Resize( 4 );

			extensions[0] = "nes";
			extensions[1] = "unf";
			extensions[2] = "unif";
			extensions[3] = "fds";

			PDX_TRY(LoadFile
			( 
				ImageFile,
				(GetFileAttributes(GetRomPath().String()) != INVALID_FILE_ATTRIBUTES ? GetRomPath().String() : NULL),
				"Open..",
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
				extensions
			));
		}
	}

	if (!ImageFile.IsOpen())
		return PDX_FAILURE;

	if (!ImageFile.Readable(sizeof(U32)))
		return application.OnWarning("File is corrupt!");

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

				PDXSTRING msg;

				msg  = "NSP file: \"";
				msg += NspName;
				msg += " was found but parsing failed!";

				application.OnWarning( msg.String() );
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

				PDXSTRING msg;

				msg  = "IPS file: \"";
				msg += IpsName;
				msg += " was found but the image file couldn't be patched!";

				application.OnWarning( msg.String() );
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

			PDXSTRING msg;

			msg  = "State file: \"";
			msg += StateName;
			msg += " was referenced but not found!";

			application.OnWarning( msg.String() );
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

	PDXFILE ImageFile;

	for (UINT i=0; i < 4; ++i)
	{
		ImageName.ReplaceFileExtension( types[i] );

		if (PDX_SUCCEEDED(ImageFile.Open( ImageName, PDXFILE::INPUT )))
			return TRUE;
	}

	ImageName.ReplaceFilePath( RomPath );

	for (UINT i=0; i < 4; ++i)
	{
		ImageName.ReplaceFileExtension( types[i] );

		if (PDX_SUCCEEDED(ImageFile.Open( ImageName, PDXFILE::INPUT )))
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

		PDXFILE IpsFile;

		if (UseIpsPathRom)
		{
			if (PDX_SUCCEEDED(IpsFile.Open( IpsName, PDXFILE::INPUT )))
				return TRUE;
		}

		IpsName.ReplaceFilePath( IpsPath );

		if (PDX_SUCCEEDED(IpsFile.Open( IpsName, PDXFILE::INPUT )))
			return TRUE;
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

	PDXFILE SaveFile;

	if (UseSavePathRom)
	{
		if (PDX_SUCCEEDED(SaveFile.Open( SaveName, PDXFILE::INPUT )))
			return TRUE;
	}

	SaveName.ReplaceFilePath( SavePath );

	if (PDX_SUCCEEDED(SaveFile.Open( SaveName, PDXFILE::INPUT )))
		return TRUE;

	return FALSE;
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

		PDXFILE NspFile;

		if (UseNspPathRom)
		{
			if (PDX_SUCCEEDED(NspFile.Open( NspName, PDXFILE::INPUT )))
				return TRUE;
		}

		NspName.ReplaceFilePath( UseNspPathLast ? NspPathLast : NspPath );

		if (PDX_SUCCEEDED(NspFile.Open( NspName, PDXFILE::INPUT )))
			return TRUE;
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

	PDXFILE NstFile;

	if (PDX_SUCCEEDED(NstFile.Open( NstName, PDXFILE::INPUT )))
		return TRUE;

	NstName.ReplaceFilePath( UseNstPathLast ? NstPathLast : NstPath );

	if (PDX_SUCCEEDED(NstFile.Open( NstName, PDXFILE::INPUT )))
		return TRUE;

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL FILEMANAGER::FindSlt(const UINT index,const PDXSTRING& ImageName,PDXSTRING& SlotName) const
{
	SlotName = ImageName;
	SlotName.ReplaceFileExtension( "ns" );
	SlotName << index;

	PDXFILE SlotFile;

	if (UseNstPathRom)
	{
		if (PDX_SUCCEEDED(SlotFile.Open( SlotName, PDXFILE::INPUT )))
			return TRUE;
	}

	SlotName.ReplaceFilePath( UseNstPathLast ? NstPathLast : NstPath );

	if (PDX_SUCCEEDED(SlotFile.Open( SlotName, PDXFILE::INPUT )) && SlotFile.Size())
		return TRUE;

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FILEMANAGER::LoadFile
(
    PDXFILE& file,
    const CHAR* const path,
	const CHAR* const title,
	const CHAR* const filter,
	const PDXARRAY<PDXSTRING>& extensions,
	const BOOL SaveRecentFiles,
	const INT RecentFileIndex
)
{
	PDX_ASSERT(!file.IsOpen());

	PDXSTRING filename;

	if (RecentFileIndex == -1)
	{
		filename.Buffer().Resize( NST_MAX_PATH );
		filename.Buffer().Front() = '\0';

		OPENFILENAME ofn;
		PDXMemZero( ofn );

		ofn.lStructSize     = sizeof(ofn);
		ofn.hwndOwner       = hWnd;
		ofn.lpstrFilter     = filter;
		ofn.nFilterIndex    = 1;
		ofn.lpstrInitialDir	= path;
		ofn.lpstrFile       = filename.Begin();
		ofn.lpstrTitle      = title;
		ofn.nMaxFile        = NST_MAX_PATH;
		ofn.Flags           = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;

		if (!GetOpenFileName( &ofn ))
			return PDX_FAILURE;
		
		filename.Validate();

		if (filename.GetFileExtension() == "zip")
		{
			if (!OpenZipFile( NULL, filename.String(), extensions, file ))
				return application.OnWarning("Found nothing in zip file!");
		}
		else
		{
			if (PDX_FAILED(file.Open( filename, PDXFILE::INPUT )))
				return application.OnWarning("Couldn't open file!");
		}
	}
	else
	{
		filename = RecentFiles[RecentFileIndex];
		const BOOL IsZip = filename.GetFileExtension() == "zip";

		if (RecentFileIndex == 0 && nes.IsOn())
		{
			if (IsZip && OpenZipFile( NULL, filename.String(), extensions, file, TRUE ) > 1)
			{
				if (!OpenZipFile( NULL, filename.String(), extensions, file ))
				{
					RecentFiles.Erase( RecentFiles.Begin() );
					UpdatedRecentFile = TRUE;
					return application.OnWarning("Found nothing in zip file!");
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
				return application.OnWarning( IsZip ? "Found nothing in zip file!" : "Couldn't open file!" );
			}
		}
	}

	if (SaveRecentFiles)
		AddRecentFile(filename.String());

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

	PDXFILE file;

	PDXRESULT result = SaveFile
	( 
	    file,
		(GetFileAttributes(GetNstPath().String()) != INVALID_FILE_ATTRIBUTES ? GetNstPath().String() : NULL),
		"Save State",
		"Nestopia State Files (*.nst)\0*.nst\0All Files (*.*)\0*.*\0",
		"nst"
	);

	if (PDX_SUCCEEDED(result))
	{
		PDX_ASSERT(file.IsOpen());
		NstPathLast = file.FilePath();
		result = nes.SaveNST( file );
	}

	application.GetGraphicManager().EndDialogMode();

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FILEMANAGER::SaveNSP()
{
	if (!nes.IsImage())
		return PDX_FAILURE;

	application.GetGraphicManager().BeginDialogMode();

	PDXFILE file;

	PDXRESULT result = SaveFile
	( 
		file,
		(GetFileAttributes(GetNspPath().String()) != INVALID_FILE_ATTRIBUTES ? GetNspPath().String() : NULL),
		"Save Game Configuration",
		"Nestopia Game Configuration Files (*.nsp)\0*.nsp\0All Files (*.*)\0*.*\0",
		"nsp"
	);

	if (PDX_SUCCEEDED(result))
	{
		PDX_ASSERT(file.IsOpen());
		NspPathLast = file.FilePath();
		
		NES::IO::NSP::CONTEXT context;
		
		context.ImageFile = LastImageFile;
		context.IpsFile = LastIpsFile;

		if (nes.GetCartridgeInfo()->battery)
			context.SaveFile = LastSaveFile;

		{
			PDXFILE file;

			for (UINT i=0; i < SAVESTATEMANAGER::MAX_SLOTS; ++i)
			{
				const PDXSTRING& SlotFile = application.GetSaveStateManager().GetSlotFile(i+1);

				if (SlotFile.Length() && PDX_SUCCEEDED(file.Open(SlotFile,PDXFILE::INPUT) && file.Size()))
					context.StateSlots[i] = SlotFile;
			}
		}

		if (application.GetGraphicManager().IsCustomPalette())
			context.PaletteFile = application.GetGraphicManager().GetPaletteFile();

		const UINT NumCodes = application.GetGameGenieManager().NumCodes();

		if (NumCodes)
		{
			context.GenieCodes.Resize( NumCodes );

			for (UINT i=0; i < NumCodes; ++i)
				application.GetGameGenieManager().GetCode( i, context.GenieCodes[i].code, &context.GenieCodes[i].comment );
		}

		context.pal = nes.IsPAL() ? 1 : 0;

		for (UINT i=0; i < 5; ++i)
			context.controllers[i] = nes.ConnectedController(i);

		NES::NSP nsp;
		nsp.SetContext( context );
		result = nsp.Save( file );
	}

	application.GetGraphicManager().EndDialogMode();

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FILEMANAGER::SaveFile
(
    PDXFILE& file,
	const CHAR* const path,
	const CHAR* const title,
	const CHAR* const filter,
	const CHAR* const extension
)
{
	PDXSTRING filename;
	filename.Buffer().Resize( NST_MAX_PATH );
	filename.Buffer().Front() = '\0';

	OPENFILENAME ofn;
	PDXMemZero( ofn );

	ofn.lStructSize     = sizeof(ofn);
	ofn.hwndOwner       = hWnd;
	ofn.lpstrFilter     = filter;
	ofn.nFilterIndex    = 1;
	ofn.lpstrInitialDir	= path;
	ofn.lpstrFile       = filename.Begin();
	ofn.lpstrTitle      = title;
	ofn.nMaxFile        = NST_MAX_PATH;
	ofn.Flags           = OFN_HIDEREADONLY|OFN_PATHMUSTEXIST;

	if (GetSaveFileName(&ofn))
	{
		filename.Validate();

		if (filename.GetFileExtension().IsEmpty())
		{
			filename += ".";
			filename += extension;
		}
		else
		{
			filename.ReplaceFileExtension( extension );
		}

		return file.Open( filename, PDXFILE::OUTPUT );
	}

	return PDX_FAILURE;
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

	ZipDialogTitle = title ? title : "Choose File";

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
			GetModuleHandle(NULL),
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

			SetWindowText( hDlg, fm->ZipDialogTitle.String() );

			HWND hList = GetDlgItem( hDlg, IDC_COMPRESSED_FILE_LIST );

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
					fm->CompressedFileIndex = ListBox_GetCurSel( GetDlgItem( hDlg, IDC_COMPRESSED_FILE_LIST ) );
		
				case IDC_COMPRESSED_FILE_CANCEL:
		
					EndDialog( hDlg, 0 );
					return TRUE;
			}
			return FALSE;

		case WM_CLOSE:

			EndDialog( hDlg, 0 );
			return TRUE;

		case WM_DESTROY:

			PDX_ASSERT( fm );
			fm = NULL;
			return TRUE;
	}

	return FALSE;
}
