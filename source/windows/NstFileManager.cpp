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

PDXRESULT FILEMANAGER::Create(PDXFILE* const file)
{
	UpdatedRecentFile = FALSE;
	RecentFiles.Reserve( MAX_RECENT_FILES );

	if (file)
	{
		file->Text().Read( RomPath       );
		file->Text().Read( RomPathLast   );
		file->Text().Read( SavePath      );
		file->Text().Read( StatePath     );
		file->Text().Read( StatePathLast );
		file->Text().Read( IpsPath       );
		file->Text().Read( NspPath       );
		file->Text().Read( NspPathLast   );

		ValidatePath( RomPath       );
		ValidatePath( RomPathLast   );
		ValidatePath( SavePath      );
		ValidatePath( StatePath     );
		ValidatePath( StatePathLast );
		ValidatePath( IpsPath       );
		ValidatePath( NspPath       );
		ValidatePath( NspPathLast   );

		const UINT states = file->Read<U16>();

		UseRomPathLast      = states & USE_ROM_PATH_LAST;
		UseSavePathRom	    = states & USE_SAVE_PATH_ROM;
		UseStatePathLast    = states & USE_STATE_PATH_LAST;
		DisableSaveRamWrite	= states & DISABLE_SAVERAM_WRITE;
		UseIpsPathRom       = states & USE_IPS_PATH_ROM;
		AutoApplyIps        = states & AUTO_APPLY_IPS;
		UseNspPathRom       = states & USE_NSP_PATH_ROM;
		AutoApplyNsp        = states & AUTO_APPLY_NSP;
		UseNspPathLast      = states & USE_NSP_PATH_LAST;

		const UINT NumFiles = file->Read<U8>();

		PDXSTRING string;

		for (UINT i=0; i < NumFiles; ++i)
		{
			file->Text().Read( string );

			if (GetFileAttributes( string ) != INVALID_FILE_ATTRIBUTES)
				RecentFiles.InsertBack( string );
		}
	}
	else
	{
		Reset();
	}

	UpdateContext();

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FILEMANAGER::Destroy(PDXFILE* const file)
{
	if (file)
	{
		file->Text().Write( RomPath       );
		file->Text().Write( RomPathLast   );
		file->Text().Write( SavePath      );
		file->Text().Write( StatePath     );
		file->Text().Write( StatePathLast );
		file->Text().Write( IpsPath       );
		file->Text().Write( NspPath       );
		file->Text().Write( NspPathLast   );

		const U16 state = 
		(
			( UseRomPathLast      ? USE_ROM_PATH_LAST     : 0x0 ) |
			( UseSavePathRom      ? USE_SAVE_PATH_ROM     : 0x0 ) |
			( UseStatePathLast    ? USE_STATE_PATH_LAST   : 0x0 ) |
			( DisableSaveRamWrite ? DISABLE_SAVERAM_WRITE : 0x0 ) |
			( UseIpsPathRom       ? USE_IPS_PATH_ROM      : 0x0 ) |
			( AutoApplyIps        ? AUTO_APPLY_IPS        : 0x0 ) |
			( UseNspPathRom       ? USE_NSP_PATH_ROM      : 0x0 ) |
			( AutoApplyNsp        ? AUTO_APPLY_NSP        : 0x0 ) |
			( UseNspPathLast      ? USE_NSP_PATH_LAST     : 0x0 ) 
		);

		file->Write( state );

		PDXARRAY<UINT> GoodFiles;
		GoodFiles.Reserve( RecentFiles.Size() );

		for (UINT i=0; i < RecentFiles.Size(); ++i)
		{
			if (GetFileAttributes( RecentFiles[i] ) != INVALID_FILE_ATTRIBUTES)
				GoodFiles.InsertBack( i );
		}

		file->Write( U8(GoodFiles.Size()) );

		for (UINT i=0; i < GoodFiles.Size(); ++i)
			file->Text().Write( RecentFiles[GoodFiles[i]] );
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FILEMANAGER::Reset()
{
	PDXSTRING path;
	path.Buffer().Resize( NST_MAX_PATH );
	path.Buffer().Front() = '\0';

	GetModuleFileName( NULL, path.Begin(), NST_MAX_PATH-1 );
	path.Validate();

	if (path.Size())
	{
		path.GetFilePath( RomPath );

		SavePath  = RomPath;
		StatePath = RomPath;
		IpsPath   = RomPath;
		NspPath   = RomPath;
	}
	else
	{
		SavePath.Clear();
		StatePath.Clear();
		IpsPath.Clear();
		NspPath.Clear();
	}

	RomPathLast   = RomPath;
	StatePathLast = StatePath;
	NspPathLast   = NspPath;

	UseRomPathLast      = TRUE;
	UseSavePathRom      = TRUE;
	UseStatePathLast    = TRUE;
	DisableSaveRamWrite	= FALSE;
	UseIpsPathRom       = TRUE;
	AutoApplyIps        = FALSE;
	UseNspPathRom       = TRUE;
	UseNspPathLast      = TRUE;
	AutoApplyNsp        = TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FILEMANAGER::UpdateContext()
{
	NES::IO::GENERAL::CONTEXT context;
	nes->GetGeneralContext( context );

	context.WriteProtectBattery = DisableSaveRamWrite;

	nes->SetGeneralContext( context );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FILEMANAGER::ValidatePath(PDXSTRING& path)
{
	if (path.IsEmpty() || GetFileAttributes( path ) == INVALID_FILE_ATTRIBUTES)
	{
		path.Clear();

		PDXSTRING string;
		string.Buffer().Resize( NST_MAX_PATH );
		string.Buffer().Front() = '\0';

		GetModuleFileName( NULL, string.Begin(), NST_MAX_PATH-1 );
		string.Validate();

		if (string.Size())
			string.GetFilePath( path );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FILEMANAGER::UpdateSettings()
{
	PDX_ASSERT( hDlg );

	RomPath.   Buffer().Resize( NST_MAX_PATH );
	SavePath.  Buffer().Resize( NST_MAX_PATH );
	StatePath. Buffer().Resize( NST_MAX_PATH );
	IpsPath.   Buffer().Resize( NST_MAX_PATH );
	NspPath.   Buffer().Resize( NST_MAX_PATH );

	RomPath.   Buffer().Front() = '\0';
	SavePath.  Buffer().Front() = '\0';
	StatePath. Buffer().Front() = '\0';
	IpsPath.   Buffer().Front() = '\0';
	NspPath.   Buffer().Front() = '\0';

	GetDlgItemText( hDlg, IDC_PATHS_ROM_IMAGES,  RomPath.Begin(),   NST_MAX_PATH );
	GetDlgItemText( hDlg, IDC_PATHS_SAVE_RAM,    SavePath.Begin(),  NST_MAX_PATH );
	GetDlgItemText( hDlg, IDC_PATHS_SAVE_STATES, StatePath.Begin(), NST_MAX_PATH );
	GetDlgItemText( hDlg, IDC_PATHS_IPS,         IpsPath.Begin(),   NST_MAX_PATH );
	GetDlgItemText( hDlg, IDC_PATHS_NSP,         NspPath.Begin(),   NST_MAX_PATH );

	RomPath.   Validate();
	SavePath.  Validate();
	StatePath. Validate();
	IpsPath.   Validate();
	NspPath.   Validate();

	if (RomPath.Length()   && RomPath.Back()   != '\\') RomPath   += "\\";
	if (SavePath.Length()  && SavePath.Back()  != '\\') SavePath  += "\\";
	if (StatePath.Length() && StatePath.Back() != '\\') StatePath += "\\";
	if (IpsPath.Length()   && IpsPath.Back()   != '\\') IpsPath   += "\\";
	if (NspPath.Length()   && NspPath.Back()   != '\\') NspPath   += "\\";

	ValidatePath( RomPath   );
	ValidatePath( SavePath  );
	ValidatePath( StatePath );
	ValidatePath( IpsPath   );
	ValidatePath( NspPath   );

	UseRomPathLast      = IsDlgButtonChecked( hDlg, IDC_PATHS_ROM_IMAGES_LAST       ) == BST_CHECKED;  
	UseSavePathRom      = IsDlgButtonChecked( hDlg, IDC_PATHS_SAVE_RAM_ROM          ) == BST_CHECKED;  
	DisableSaveRamWrite = IsDlgButtonChecked( hDlg, IDC_PATHS_DISABLE_SAVERAM_WRITE ) == BST_CHECKED;  
	UseStatePathLast    = IsDlgButtonChecked( hDlg, IDC_PATHS_SAVE_STATES_LAST      ) == BST_CHECKED; 
	UseIpsPathRom       = IsDlgButtonChecked( hDlg, IDC_PATHS_IPS_ROM               ) == BST_CHECKED; 
	AutoApplyIps        = IsDlgButtonChecked( hDlg, IDC_PATHS_IPS_AUTO_APPLY        ) == BST_CHECKED; 
	UseNspPathRom       = IsDlgButtonChecked( hDlg, IDC_PATHS_NSP_ROM               ) == BST_CHECKED; 
	UseNspPathLast      = IsDlgButtonChecked( hDlg, IDC_PATHS_NSP_LAST              ) == BST_CHECKED; 
	AutoApplyNsp        = IsDlgButtonChecked( hDlg, IDC_PATHS_NSP_AUTO_APPLY        ) == BST_CHECKED; 

	if (!UseRomPathLast || RomPathLast.IsEmpty() || GetFileAttributes( RomPathLast ) == INVALID_FILE_ATTRIBUTES)
		RomPathLast = RomPath;

	if (!UseStatePathLast || StatePathLast.IsEmpty() || GetFileAttributes( StatePathLast ) == INVALID_FILE_ATTRIBUTES)
		StatePathLast = StatePath;

	if (!UseNspPathLast || NspPathLast.IsEmpty() || GetFileAttributes( NspPathLast ) == INVALID_FILE_ATTRIBUTES)
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
				case IDC_PATHS_ROM_IMAGES_BROWSE:
				{
					PDXSTRING path;
					
					if (SelectPath( path, "Select your image file (*.nes,*.unf,*.fds,*.nsf) directory.." ))
						SetDlgItemText( hDlg, IDC_PATHS_ROM_IMAGES, path );

					return TRUE;
				}
		
				case IDC_PATHS_SAVE_RAM_BROWSE:
				{		
					PDXSTRING path;
					
					if (SelectPath( path, "Select your save ram (*.sav) directory.." ))
						SetDlgItemText( hDlg, IDC_PATHS_SAVE_RAM, path );

					return TRUE;
				}
		
				case IDC_PATHS_SAVE_STATES_BROWSE:
				{
					PDXSTRING path;
					
					if (SelectPath( path, "Select your save state (*.nst) directory.." ))
						SetDlgItemText( hDlg, IDC_PATHS_SAVE_STATES, path );

					return TRUE;
				}

				case IDC_PATHS_IPS_BROWSE:
				{
					PDXSTRING path;

					if (SelectPath( path, "Select your patch file (*.ips) directory.." ))
						SetDlgItemText( hDlg, IDC_PATHS_IPS, path );

					return TRUE;
				}

				case IDC_PATHS_NSP_BROWSE:
				{
					PDXSTRING path;

					if (SelectPath( path, "Select your game config file (*.nsp) directory.." ))
						SetDlgItemText( hDlg, IDC_PATHS_NSP, path );

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
	SetDlgItemText( hDlg, IDC_PATHS_ROM_IMAGES,            RomPath             );
	SetDlgItemText( hDlg, IDC_PATHS_SAVE_RAM,              SavePath            );
	SetDlgItemText( hDlg, IDC_PATHS_SAVE_STATES,           StatePath           );
	SetDlgItemText( hDlg, IDC_PATHS_IPS,                   IpsPath             );
	SetDlgItemText( hDlg, IDC_PATHS_NSP,                   NspPath             );
	CheckDlgButton( hDlg, IDC_PATHS_ROM_IMAGES_LAST,       UseRomPathLast      ); 
	CheckDlgButton( hDlg, IDC_PATHS_SAVE_RAM_ROM,          UseSavePathRom      );
	CheckDlgButton( hDlg, IDC_PATHS_SAVE_STATES_LAST,      UseStatePathLast    );
	CheckDlgButton( hDlg, IDC_PATHS_DISABLE_SAVERAM_WRITE, DisableSaveRamWrite );
	CheckDlgButton( hDlg, IDC_PATHS_IPS_ROM,               UseIpsPathRom       );
	CheckDlgButton( hDlg, IDC_PATHS_IPS_AUTO_APPLY,        AutoApplyIps        );
	CheckDlgButton( hDlg, IDC_PATHS_NSP_ROM,               UseNspPathRom       );
	CheckDlgButton( hDlg, IDC_PATHS_NSP_AUTO_APPLY,        AutoApplyNsp        );
	CheckDlgButton( hDlg, IDC_PATHS_NSP_LAST,              UseNspPathLast      );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL FILEMANAGER::UpdateAccess(const PDXSTRING& path)
{
	return GetFileAttributes( path ) != INVALID_FILE_ATTRIBUTES;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FILEMANAGER::Load(INT RecentFileIndex,const BOOL power)
{
	PDX_ASSERT(RecentFileIndex >= -1 && RecentFileIndex < LONG(RecentFiles.Size()));

	PDX_TRY(application.BeginDialogMode());

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
     		UpdateAccess( GetRomPath() ) ? GetRomPath().String() : NULL,
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
			nes->Power( FALSE );
		}
	}

	PDX_TRY(application.EndDialogMode());

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FILEMANAGER::LoadNSP(INT RecentFileIndex,const BOOL power)
{
	PDX_ASSERT(RecentFileIndex >= -1 && RecentFileIndex < LONG(RecentFiles.Size()));

	PDX_TRY(application.BeginDialogMode());

	UpdatedRecentFile = FALSE;

	PDXFILE file;
	PDXRESULT result;

	{
		PDXARRAY<PDXSTRING> extension;
		extension.InsertBack(PDXSTRING("nsp"));

		result = LoadFile
		( 
			file,
			UpdateAccess( GetNspPath() ) ? GetNspPath().String() : NULL,
			"Load Script",
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
			nes->Power( FALSE );
		}
	}

	PDX_TRY(application.EndDialogMode());

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FILEMANAGER::LoadNST()
{
	if (!nes->IsImage() || (nes->IsOff() && PDX_FAILED(nes->Power( TRUE ))))
		return PDX_FAILURE;

	PDX_TRY(application.BeginDialogMode());

	PDXFILE file;
	PDXRESULT result;

	{
		PDXARRAY<PDXSTRING> extension;
		extension.InsertBack(PDXSTRING("nst"));

		result = LoadFile
		( 
			file,
			UpdateAccess( GetNstPath() ) ? GetNstPath().String() : NULL,
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

		StatePathLast = file.FilePath();

		if (file.Readable(sizeof(U32)))
		{
			if (file.Peek<U32>() == NES_MAGIC_NST)
			{
				result = LoadNesFile( file, TRUE );
			}
			else
			{
				result = application.OnWarning("Not a state file!");
			}
		}
		else
		{
			result = application.OnWarning("Corrupt file!");
		}
	}

	PDX_TRY(application.EndDialogMode());

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
	}

	BOOL ShowWarnings = TRUE;

	switch (magic)
	{
    	case NES_MAGIC_INES:
		case NES_MAGIC_UNIF:
		case NES_MAGIC_FDS:
		{
			NspLoaded = PDX_SUCCEEDED(ApplyNps( file.Name(), NspContext, ShowWarnings ));

			if (magic != NES_MAGIC_UNIF)
				ApplyIps( file, NspContext.IpsFile, ShowWarnings );

			const PDXSTRING* save = NULL;

			if (magic != NES_MAGIC_FDS)
			{
				if (PDX_SUCCEEDED(ApplySav( file.Name(), NspContext.SaveFile )))
					save = &NspContext.SaveFile;
			}

			if (PDX_FAILED(nes->LoadRom( file, save )) || PDX_FAILED(nes->Power( power )))
				return PDX_FAILURE;

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
				if (PDX_SUCCEEDED(ApplySav( file.Name(), SaveName )))
					save = &SaveName;
			}

			if (PDX_FAILED(nes->LoadRom( ImageFile, save )) || PDX_FAILED(nes->Power( power )))
				return PDX_FAILURE;

			LastImageFile = ImageFile.Name();
			break;
		}

		case NES_MAGIC_NST:

			if (!nes->IsImage() || nes->IsOff() || PDX_FAILED(nes->LoadNST( file )))
				return PDX_FAILURE;

			break;

		case NES_MAGIC_NSV:

			if (!nes->IsImage() || nes->IsOff() || PDX_FAILED(nes->LoadMovie( file.Name() )))
				return PDX_FAILURE;

			break;

		case NES_MAGIC_NSF:

			if (PDX_FAILED(nes->LoadNSF( file )) || PDX_FAILED(nes->Power( power )))
				return PDX_FAILURE;

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
				if (PDX_SUCCEEDED(ApplySav( file.Name(), NspContext.SaveFile )))
					save = &NspContext.SaveFile;
			}

			if (PDX_FAILED(nes->LoadRom( ImageFile, save )) || PDX_FAILED(nes->Power( power )))
				return PDX_FAILURE;

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

				application.OnWarning( msg );
				
				nes->CloseMovie();
			}
		}
		else
		{
			PDXFILE StateFile;
			ApplyNst( StateFile, NspContext.StateFile, ShowWarnings );

			if (StateFile.IsOpen() && PDX_FAILED(nes->LoadNST( StateFile )))
				return PDX_FAILURE;
		}

		for (UINT i=0; i < 9; ++i)
		{
			PDXFILE SlotFile;
			
			if (PDX_SUCCEEDED(ApplyNst( SlotFile, NspContext.StateSlots[i], ShowWarnings )))
				application.GetSaveStateManager().SetFile( i+1, SlotFile );
		}

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
			nes->SetMode( NspContext.pal ? NES::MODE_PAL : NES::MODE_NTSC );
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
				UpdateAccess( GetRomPath() ) ? GetRomPath().String() : NULL,
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

PDXRESULT FILEMANAGER::ApplyNps(const PDXSTRING& ImageName,NES::IO::NSP::CONTEXT& context,BOOL& ShowWarning) const
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

				application.OnWarning( msg );
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

				application.OnWarning( msg );
			}
		} 
	}

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FILEMANAGER::ApplySav(const PDXSTRING& ImageName,PDXSTRING& SaveName)
{
	LastSaveFile.Clear();

	PDXFILE SaveFile;

	if (SaveName.Length())
		SaveFile.Open( SaveName, PDXFILE::INPUT );

	if (!SaveFile.IsOpen() && FindSav( ImageName, SaveName ))
		SaveFile.Open( SaveName, PDXFILE::INPUT );

	if (SaveFile.IsOpen())
	{
		LastSaveFile = SaveName;
		return PDX_OK;
	}

	return PDX_FAILURE;
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

			application.OnWarning( msg );
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

	NstName.ReplaceFilePath( UseStatePathLast ? StatePathLast : StatePath );

	if (PDX_SUCCEEDED(NstFile.Open( NstName, PDXFILE::INPUT )))
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
			if (!OpenZipFile( NULL, filename, extensions, file ))
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

		if (RecentFileIndex == 0 && nes->IsOn())
		{
			if (IsZip && OpenZipFile( NULL, filename, extensions, file, TRUE ) > 1)
			{
				if (!OpenZipFile( NULL, filename, extensions, file ))
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
			if ((IsZip && !OpenZipFile(NULL,filename,extensions,file)) || (!IsZip && PDX_FAILED(file.Open(filename,PDXFILE::INPUT))))
			{
				RecentFiles.Erase( RecentFiles.At(RecentFileIndex) );
				UpdatedRecentFile = TRUE;
				return application.OnWarning( IsZip ? "Found nothing in zip file!" : "Couldn't open file!" );
			}
		}
	}

	if (SaveRecentFiles)
		AddRecentFile(filename);

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
	if (nes->IsOff() || !nes->IsImage() || PDX_FAILED(application.BeginDialogMode()))
		return PDX_FAILURE;

	PDXFILE file;

	PDXRESULT result = SaveFile
	( 
	    file,
       	UpdateAccess( GetNstPath() ) ? GetNstPath().String() : NULL,
		"Save State",
		"Nestopia State Files (*.nst)\0*.nst\0All Files (*.*)\0*.*\0",
		"nst"
	);

	if (PDX_SUCCEEDED(result))
	{
		PDX_ASSERT(file.IsOpen());
		StatePathLast = file.FilePath();
		result = nes->SaveNST( file );
	}

	PDX_TRY(application.EndDialogMode());

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FILEMANAGER::SaveNSP()
{
	if (!nes->IsImage() || PDX_FAILED(application.BeginDialogMode()))
		return PDX_FAILURE;

	PDXFILE file;

	PDXRESULT result = SaveFile
	( 
		file,
		UpdateAccess( GetNspPath() ) ? GetNspPath().String() : NULL,
		"Save Script",
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

		if (nes->GetCartridgeInfo()->battery)
			context.SaveFile = LastSaveFile;

		for (UINT i=0; i < 9; ++i)
		{
			if (application.GetSaveStateManager().IsValidSlot(i+1))
			{
				static const CHAR* ext[9] = 
				{
					"ns0","ns1","ns2","ns3","ns4","ns5","ns6","ns7","ns8"
				};

				context.StateSlots[i]  = GetNstPath();
				context.StateSlots[i] += context.ImageFile.GetFileName();
				context.StateSlots[i].ReplaceFileExtension( ext[i] );

				PDXFILE file( context.StateSlots[i], PDXFILE::OUTPUT );

				if (file.IsOpen())
				{
					PDXFILE& SlotFile = application.GetSaveStateManager().GetFile(i+1);

					if (SlotFile.Size())
					{
						file.Write( SlotFile.Begin(), SlotFile.End() );

						if (PDX_SUCCEEDED(file.Close()))
							continue;
					}
				}

				context.StateSlots[i].Clear();
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

		context.pal = nes->IsPAL() ? 1 : 0;

		for (UINT i=0; i < 5; ++i)
			context.controllers[i] = nes->ConnectedController(i);

		NES::NSP nsp;
		nsp.SetContext( context );
		result = nsp.Save( file );
	}

	PDX_TRY(application.EndDialogMode());

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
			application.GetInstance(),
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

		file.Hook( buffer, PDXFILE::INPUT, ZipFile->FileName( CompressedFileIndex ) );
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

			SetWindowText( hDlg, fm->ZipDialogTitle );

			HWND hList = GetDlgItem( hDlg, IDC_COMPRESSED_FILE_LIST );

			for (UINT i=0; i < fm->ZipFile->NumFiles(); ++i)
				ListBox_AddString( hList, fm->ZipFile->FileName(i) );

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
