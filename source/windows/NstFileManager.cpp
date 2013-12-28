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
#include "NstFileManager.h"
#include "../NstZipFile.h"
#include <WindowsX.h>
#include <ShlObj.h>
#include "../paradox/PdxFile.h"

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

		ValidatePath( RomPath       );
		ValidatePath( RomPathLast   );
		ValidatePath( SavePath      );
		ValidatePath( StatePath     );
		ValidatePath( StatePathLast );
		ValidatePath( IpsPath       );

		const UINT states = file->Read<U8>();

		UseRomPathLast      = states & USE_ROM_PATH_LAST;
		UseSavePathRom	    = states & USE_SAVE_PATH_ROM;
		UseStatePathLast    = states & USE_STATE_PATH_LAST;
		DisableSaveRamWrite	= states & DISABLE_SAVERAM_WRITE;
		UseIpsPathRom       = states & USE_IPS_PATH_ROM;
		AutoApplyIps        = states & AUTO_APPLY_IPS;

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

		const U8 state = 
		(
			( UseRomPathLast      ? USE_ROM_PATH_LAST     : 0x0 ) |
			( UseSavePathRom      ? USE_SAVE_PATH_ROM     : 0x0 ) |
			( UseStatePathLast    ? USE_STATE_PATH_LAST   : 0x0 ) |
			( DisableSaveRamWrite ? DISABLE_SAVERAM_WRITE : 0x0 ) |
			( UseIpsPathRom       ? USE_IPS_PATH_ROM      : 0x0 ) |
			( AutoApplyIps        ? AUTO_APPLY_IPS        : 0x0 )
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
	}
	else
	{
		SavePath.Clear();
		StatePath.Clear();
		IpsPath.Clear();
	}

	RomPathLast   = RomPath;
	StatePathLast = StatePath;

	UseRomPathLast      = TRUE;
	UseSavePathRom      = TRUE;
	UseStatePathLast    = TRUE;
	DisableSaveRamWrite	= FALSE;
	UseIpsPathRom       = TRUE;
	AutoApplyIps        = FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FILEMANAGER::UpdateContext()
{
	NES::IO::GENERAL::CONTEXT context;
	nes->GetGeneralContext( context );

	context.RomPath                 = GetRomPath();
	context.SaveRamPath             = SavePath;
	context.LookInRomPathForSaveRam = UseSavePathRom;
	context.DisableSaveRamWrite     = DisableSaveRamWrite;
	context.IpsPath                 = IpsPath;
	context.LookInRomPathForIps     = UseIpsPathRom;
	context.ApplyIPS                = AutoApplyIps;

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

	RomPath.   Buffer().Front() = '\0';
	SavePath.  Buffer().Front() = '\0';
	StatePath. Buffer().Front() = '\0';
	IpsPath.   Buffer().Front() = '\0';

	GetDlgItemText( hDlg, IDC_PATHS_ROM_IMAGES,  RomPath.Begin(),   NST_MAX_PATH );
	GetDlgItemText( hDlg, IDC_PATHS_SAVE_RAM,    SavePath.Begin(),  NST_MAX_PATH );
	GetDlgItemText( hDlg, IDC_PATHS_SAVE_STATES, StatePath.Begin(), NST_MAX_PATH );
	GetDlgItemText( hDlg, IDC_PATHS_SAVE_STATES, IpsPath.Begin(),   NST_MAX_PATH );

	RomPath.   Validate();
	SavePath.  Validate();
	StatePath. Validate();
	IpsPath.   Validate();

	if (RomPath.Length()   && RomPath.Back()   != '\\') RomPath   += "\\";
	if (SavePath.Length()  && SavePath.Back()  != '\\') SavePath  += "\\";
	if (StatePath.Length() && StatePath.Back() != '\\') StatePath += "\\";
	if (IpsPath.Length()   && IpsPath.Back()   != '\\') IpsPath   += "\\";

	ValidatePath( RomPath   );
	ValidatePath( SavePath  );
	ValidatePath( StatePath );
	ValidatePath( IpsPath   );

	UseRomPathLast      = IsDlgButtonChecked( hDlg, IDC_PATHS_ROM_IMAGES_LAST       ) == BST_CHECKED;  
	UseSavePathRom      = IsDlgButtonChecked( hDlg, IDC_PATHS_SAVE_RAM_ROM          ) == BST_CHECKED;  
	DisableSaveRamWrite = IsDlgButtonChecked( hDlg, IDC_PATHS_DISABLE_SAVERAM_WRITE ) == BST_CHECKED;  
	UseStatePathLast    = IsDlgButtonChecked( hDlg, IDC_PATHS_SAVE_STATES_LAST      ) == BST_CHECKED; 
	UseIpsPathRom       = IsDlgButtonChecked( hDlg, IDC_PATHS_IPS_ROM               ) == BST_CHECKED; 
	AutoApplyIps        = IsDlgButtonChecked( hDlg, IDC_PATHS_IPS_AUTO_APPLY        ) == BST_CHECKED; 

	if (!UseRomPathLast || RomPathLast.IsEmpty() || GetFileAttributes( RomPathLast ) == INVALID_FILE_ATTRIBUTES)
		RomPathLast = RomPath;

	if (!UseStatePathLast || StatePathLast.IsEmpty() || GetFileAttributes( StatePathLast ) == INVALID_FILE_ATTRIBUTES)
		StatePathLast = StatePath;

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
					
					if (SelectPath( path, "Select your image (*.nes,*.unf,*.fds,*.nsf) directory.." ))
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

					if (SelectPath( path, "Select your rom patch (*.ips) directory.." ))
						SetDlgItemText( hDlg, IDC_PATHS_IPS, path );

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
	CheckDlgButton( hDlg, IDC_PATHS_ROM_IMAGES_LAST,       UseRomPathLast      ); 
	CheckDlgButton( hDlg, IDC_PATHS_SAVE_RAM_ROM,          UseSavePathRom      );
	CheckDlgButton( hDlg, IDC_PATHS_SAVE_STATES_LAST,      UseStatePathLast    );
	CheckDlgButton( hDlg, IDC_PATHS_DISABLE_SAVERAM_WRITE, DisableSaveRamWrite );
	CheckDlgButton( hDlg, IDC_PATHS_IPS_ROM,               UseIpsPathRom       );
	CheckDlgButton( hDlg, IDC_PATHS_IPS_AUTO_APPLY,        AutoApplyIps        );
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
       	extensions.Resize( 8 );

     	extensions[0] = "nes";
		extensions[1] = "unf";
		extensions[2] = "unif";
		extensions[3] = "fds";
     	extensions[4] = "nsf";
     	extensions[5] = "nst";
		extensions[6] = "nsv";
		extensions[7] = "ips";

     	result = LoadFile
		( 
     		file,
     		UpdateAccess( GetRomPath() ) ? GetRomPath().String() : NULL,
     		"Open..",
			(
				"All supported files\0"
				"*.nes;*.unf;*.fds;*.nsf;*.nst;*.nsv;*.ips;*.zip\0"
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

		if (PDX_FAILED(nes->Load( file )) || PDX_FAILED(nes->Power( power )))
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

PDXRESULT FILEMANAGER::LoadState()
{
	if (nes->IsOff() || PDX_FAILED(application.BeginDialogMode()))
		return PDX_FAILURE;

	PDXFILE file;
	PDXRESULT result;

	{
		PDXARRAY<PDXSTRING> extension;
		extension.InsertBack(PDXSTRING("nst"));

		result = LoadFile
		( 
			file,
			UpdateAccess( GetStatePath() ) ? GetStatePath().String() : NULL,
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
		result = nes->LoadNST( file );
	}

	PDX_TRY(application.EndDialogMode());

	return result;
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
		ofn.hInstance       = application.GetHInstance();
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

PDXRESULT FILEMANAGER::SaveState()
{
	if (nes->IsOff() || PDX_FAILED(application.BeginDialogMode()))
		return PDX_FAILURE;

	PDXFILE file;

	PDXRESULT result = SaveFile
	( 
	    file,
       	UpdateAccess( GetStatePath() ) ? GetStatePath().String() : NULL,
		"Save State",
		"NES State (*.nst)\0*.nst\0All Files (*.*)\0*.*\0",
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
	ofn.hInstance       = application.GetHInstance();
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
			filename.Append( "." );
			filename.Append( extension );
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
			application.GetHInstance(),
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
