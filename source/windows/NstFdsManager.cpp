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

#include <Windows.h>
#include "../paradox/PdxString.h"
#include "resource/resource.h"
#include "NstApplication.h"
#include "NstFdsManager.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FDSMANAGER::Create(PDXFILE* const file)
{
	if (file)
	{
		WriteProtect = file->Read<U8>();
		file->Text().Read( bios );

		if (bios.IsEmpty() || GetFileAttributes(bios) == INVALID_FILE_ATTRIBUTES)
		{
			bios.Clear();
			Reset();
		}
	}
	else
	{
		Reset();
	}

	SubmitBios();
	
	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT FDSMANAGER::Destroy(PDXFILE* const file)
{
	if (file)
	{
		if (bios.IsEmpty() || GetFileAttributes(bios) == INVALID_FILE_ATTRIBUTES)
			bios.Clear();

		file->Write( U8(WriteProtect ? 1 : 0) );
		file->Text().Write( bios );
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FDSMANAGER::Reset()
{
	WriteProtect = FALSE;

	bios.Clear();

	{
		PDXSTRING path;
		path.Buffer().Resize( NST_MAX_PATH );

		if (GetModuleFileName( NULL, path.Begin(), NST_MAX_PATH-1 ))
		{
			bios  = path.GetFilePath();
			bios += "disksys.rom";
		}
	}

	PDXFILE file;

	if (bios.IsEmpty() || PDX_FAILED(file.Open(bios,PDXFILE::INPUT)))
	{
		bios.Buffer().Resize( NST_MAX_PATH );

		if (GetCurrentDirectory( NST_MAX_PATH-1, bios.Begin() ))
		{
			bios.Validate();
			bios += "\\disksys.rom";

			if (PDX_SUCCEEDED(file.Open(bios,PDXFILE::INPUT)))
				return;
		}

		bios  = application.GetRomPath();
		bios +=	"disksys.rom";

		if (PDX_FAILED(file.Open(bios,PDXFILE::INPUT)))
			bios.Clear();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FDSMANAGER::SubmitBios(const BOOL SkipBios)
{
	NES::IO::FDS::CONTEXT context;

	if (PDX_SUCCEEDED(nes->GetFdsContext( context )))
	{
		PDXFILE file( PDXFILE::INPUT );

		context.bios = NULL;

		if (!SkipBios && bios.Length() && PDX_SUCCEEDED(file.Open( bios )))
		{
			if (bios.GetFileExtension() == "zip")
			{
				PDXARRAY<PDXSTRING> extensions;
				extensions.Resize(2);

				extensions[0] = "nes";
				extensions[1] = "rom";

				file.Close();

				if (application.GetFileManager()->OpenZipFile( "Choose BIOS Rom", bios, extensions, file ) == 0)
				{
					file.Close();
					application.OnWarning("Either the zip file is corrupt or the bios file could not be found inside it!");
				}
			}

			if (file.IsOpen() && file.Size() >= NES::n8k)
				context.bios = &file;
		}

		context.WriteProtected = WriteProtect;

		nes->SetFdsContext( context );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FDSMANAGER::UpdateDialog(HWND hDlg)
{
	SetDlgItemText( hDlg, IDC_FDS_BIOS, bios );	
	CheckDlgButton( hDlg, IDC_FDS_WRITE_PROTECT, WriteProtect ? BST_CHECKED : BST_UNCHECKED );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FDSMANAGER::UpdateSettings(HWND hDlg)
{
	const PDXSTRING old( bios );
	bios.Buffer().Resize( NST_MAX_PATH );
	GetDlgItemText( hDlg, IDC_FDS_BIOS, bios.Begin(), NST_MAX_PATH );	
	WriteProtect = IsDlgButtonChecked(hDlg, IDC_FDS_WRITE_PROTECT ) == BST_CHECKED ? TRUE : FALSE;
	bios.Validate();
	SubmitBios( old == bios );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FDSMANAGER::OnBrowse(HWND hDlg)
{
	PDXSTRING file;
	file.Buffer().Resize( NST_MAX_PATH );
	file.Buffer().Front() = '\0';

	OPENFILENAME ofn;
	PDXMemZero( ofn );

	ofn.lStructSize     = sizeof(ofn);
	ofn.hwndOwner       = hWnd;
	ofn.hInstance       = application.GetHInstance();
	ofn.nFilterIndex    = 1;
	ofn.lpstrInitialDir	= application.GetStatePath();
	ofn.lpstrFile       = file.Begin();
	ofn.lpstrTitle      = "Famicom Disk System Bios Rom";
	ofn.nMaxFile        = NST_MAX_PATH;
	ofn.Flags           = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;

	ofn.lpstrFilter = 
	(
     	"All supported files (*.rom, *.nes, *.zip)\0"
		"*.rom;*.nes;*.zip\0"
		"Rom Files (*.rom)\0"
		"*.rom\0"
		"iNes Files (*.nes)\0"
		"*.ines\0"
		"Zip Files (*.zip)\0"
		"*.zip\0"
		"All Files (*.*)\0"
		"*.*\0"
	);

	if (GetOpenFileName(&ofn))
	{
		file.Validate();
		SetDlgItemText( hDlg, IDC_FDS_BIOS, file );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FDSMANAGER::OnClear(HWND hDlg)
{
	SetDlgItemText( hDlg, IDC_FDS_BIOS, "" );
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
				case IDC_FDS_CANCEL: EndDialog( hDlg, 0 ); return TRUE;
			}		
			return FALSE;

     	case WM_CLOSE:

     		EndDialog( hDlg, 0 );
     		return TRUE;
	}

	return FALSE;
}
