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
#include "NstFdsManager.h"
#include "NstFileManager.h"
#include "NstApplication.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

FDSMANAGER::FDSMANAGER() 
: 
MANAGER      (IDD_FDS),
WriteProtect (FALSE)
{}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FDSMANAGER::Create(CONFIGFILE* const ConfigFile)
{
	if (ConfigFile)
	{
		CONFIGFILE& file = *ConfigFile;

		bios = file["files fds bios"];
		WriteProtect = (file["files write protect fds"] == "yes");

		PDXFILE BiosFile;

		if (bios.IsEmpty() || PDX_FAILED(BiosFile.Open( bios, PDXFILE::INPUT )))
			SearchBios();
	}
	else
	{
		Reset();
	}

	SubmitBios();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FDSMANAGER::Destroy(CONFIGFILE* const ConfigFile)
{
	if (ConfigFile)
	{
		CONFIGFILE& file = *ConfigFile;

		file["files fds bios"] = bios.Quoted();
		file["files write protect fds"] = (WriteProtect ? "yes" : "no");
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FDSMANAGER::Reset()
{
	WriteProtect = FALSE;
	SearchBios();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL FDSMANAGER::SearchBios()
{
	UTILITIES::GetExeDir( bios );
	bios += "disksys.rom";

	if (UTILITIES::FileExist( bios ))
		return TRUE;

	UTILITIES::GetCurrentDir( bios );
	bios += "disksys.rom";

	if (UTILITIES::FileExist( bios ))
		return TRUE;

	bios  = application.GetFileManager().GetDefaultRomPath();
	bios +=	"disksys.rom";

	if (UTILITIES::FileExist( bios ))
		return TRUE;

	bios.Clear();

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FDSMANAGER::SubmitBios(const BOOL SkipBios)
{
	NES::IO::FDS::CONTEXT context;

	if (PDX_SUCCEEDED(nes.GetFdsContext( context )))
	{
		PDXFILE file( PDXFILE::INPUT );

		context.bios = NULL;

		if (!SkipBios && bios.Length() && PDX_SUCCEEDED(file.Open( bios )))
		{
			if (bios.IsFileExtension( "zip", 3 ))
			{
				PDXARRAY<PDXSTRING> extensions;
				extensions.Resize(2);

				extensions[0] = "nes";
				extensions[1] = "rom";

				file.Close();

				if (application.GetFileManager().OpenZipFile( UTILITIES::IdToString(IDS_FDS_LOAD_BIOS).String(), bios.String(), extensions, file ) == 0)
				{
					file.Close();
					UI::MsgWarning( IDS_FDS_BIOS_ZIP_INVALID );
				}
			}

			if (file.IsOpen() && file.Size() >= NES::n8k)
				context.bios = &file;
		}

		context.WriteProtected = WriteProtect;

		nes.SetFdsContext( context );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FDSMANAGER::UpdateDialog(HWND hDlg)
{
	::SetDlgItemText( hDlg, IDC_FDS_BIOS, bios.String() );	
	::CheckDlgButton( hDlg, IDC_FDS_WRITE_PROTECT, WriteProtect ? BST_CHECKED : BST_UNCHECKED );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FDSMANAGER::UpdateSettings(HWND hDlg)
{
	WriteProtect = (::IsDlgButtonChecked( hDlg, IDC_FDS_WRITE_PROTECT ) == BST_CHECKED);

	const PDXSTRING old( bios );
	MANAGER::GetDlgItemText( hDlg, IDC_FDS_BIOS, bios );
	SubmitBios( old == bios );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FDSMANAGER::OnBrowse(HWND hDlg)
{
	PDXSTRING filename;

	const BOOL succeeded = UTILITIES::BrowseOpenFile
	( 
    	filename, 
		hDlg, 
		IDS_FDS_SELECT_BIOS,
		"All supported files (*.rom, *.nes, *.zip)\0"
		"*.rom;*.nes;*.zip\0"
		"Rom Files (*.rom)\0"
		"*.rom\0"
		"iNes Files (*.nes)\0"
		"*.ines\0"
		"Zip Files (*.zip)\0"
		"*.zip\0"
		"All Files (*.*)\0"
		"*.*\0",
		application.GetFileManager().GetRomPath().String()
	);

	if (succeeded)
		::SetDlgItemText( hDlg, IDC_FDS_BIOS, filename.String() );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FDSMANAGER::OnClear(HWND hDlg)
{
	::SetDlgItemText( hDlg, IDC_FDS_BIOS, "" );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL FDSMANAGER::DialogProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM)
{
	switch (uMsg) 
	{
     	case WM_INITDIALOG:

			UpdateDialog( hDlg );
			return TRUE;

		case WM_COMMAND:

         	switch (LOWORD(wParam))
			{
    			case IDC_FDS_CLEAR:  OnClear( hDlg ); return TRUE;
    			case IDC_FDS_BROWSE: OnBrowse( hDlg ); return TRUE;
				case IDC_FDS_OK:     UpdateSettings( hDlg );
				case IDC_FDS_CANCEL: ::EndDialog( hDlg, 0 ); return TRUE;
			}		
			return FALSE;

     	case WM_CLOSE:

     		::EndDialog( hDlg, 0 );
     		return TRUE;
	}

	return FALSE;
}
