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
#include "NstSaveStateManager.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#define NST_TO_MILLI(x)   ((x) * (1000 * 60))
#define NST_FROM_MILLI(x) ((x) / (1000 * 60))

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SAVESTATEMANAGER::Create(CONFIGFILE* const)
{
	Reset();
	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SAVESTATEMANAGER::Destroy(CONFIGFILE* const)
{
	for (UINT i=0; i < MAX_SLOTS; ++i)
	{
		slots[i].file.Abort();
		slots[i].valid = FALSE;
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SAVESTATEMANAGER::Reset()
{
	LastSlot = MAX_SLOTS - 1;

	PDXSTRING name("StateSlot.ns");

	for (UINT i=0; i < MAX_SLOTS; ++i)
	{
		name.Resize(12);
		name += (i+1);

		slots[i].file.Open( name, PDXFILE::OUTPUT );
		slots[i].valid = FALSE;
	}

	AutoSave.name.Clear();
	AutoSave.file.Abort();

	AutoSave.enabled = FALSE;
	AutoSave.time = NST_TO_MILLI(1);
	AutoSave.msg = TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SAVESTATEMANAGER::SetFile(UINT index,PDXFILE& file)
{
	if (file.IsOpen() && file.Size())
	{
		index = IndexToSlot( index );

		slots[index].file.Seek( PDXFILE::BEGIN );
		slots[index].file.Write( file.Begin(), file.End() );
		slots[index].valid = TRUE;

		return PDX_OK;
	}

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SAVESTATEMANAGER::LoadState(UINT index)
{
	index = IndexToSlot( index );
	slots[index].file.Seek( PDXFILE::BEGIN );

	PDXRESULT result = PDX_FAILURE;

	if (slots[index].valid)
	{
		result = nes->LoadNST( slots[index].file );
		slots[index].valid = PDX_SUCCEEDED(result);
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SAVESTATEMANAGER::SaveState(UINT index)
{
	PDX_ASSERT( index <= MAX_SLOTS );

	if (index == NEXT_SLOT)
	{
		if (++LastSlot == MAX_SLOTS)
			LastSlot = 0;
	}
	else
	{
		LastSlot = index - 1;
	}

	slots[LastSlot].file.Seek( PDXFILE::BEGIN, 0 );
	
	const PDXRESULT result = nes->SaveNST( slots[LastSlot].file );
	slots[LastSlot].valid = PDX_SUCCEEDED(result);

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID CALLBACK SAVESTATEMANAGER::OnAutoSaveProc(HWND,UINT,UINT_PTR,DWORD)
{
	application.GetSaveStateManager().OnAutoSave();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SAVESTATEMANAGER::OnAutoSave()
{
	if (nes->IsOn() && !nes->IsPaused() && application.IsActive())
	{
		AutoSave.file.Open( AutoSave.name, PDXFILE::OUTPUT );
	
		if (PDX_SUCCEEDED(nes->SaveNST( AutoSave.file )))
		{
			if (AutoSave.name.Length())
			{
				if (PDX_SUCCEEDED(AutoSave.file.Close()))
				{
					if (AutoSave.msg)
						application.StartScreenMsg( "Auto-Saved to file..", 1000 );
				}
				else
				{
					slots[0].file.Seek( PDXFILE::BEGIN, 0 );
					slots[0].file.Buffer() = AutoSave.file.Buffer();
					slots[0].valid = TRUE;
	
					if (AutoSave.msg)
						application.StartScreenMsg( "Auto-Save to file failed, wrote to slot 1 instead..", 1000 );
				}
			}
			else
			{
				slots[0].file.Seek( PDXFILE::BEGIN, 0 );
				slots[0].file.Buffer() = AutoSave.file.Buffer();
				slots[0].valid = TRUE;

				if (AutoSave.msg)
					application.StartScreenMsg( "Auto-Saved to slot 1...", 1000 );
			}
		}
		else
		{
			if (AutoSave.msg)
				application.StartScreenMsg( "Auto-Save failed...", 1000 );
		}
	
		AutoSave.file.Abort();
	}
	
	UpdateAutoSaveTimer();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SAVESTATEMANAGER::UpdateAutoSaveTimer()
{
	if (AutoSave.enabled)
	{
		SetTimer
		( 
    		hWnd, 
    		APPLICATION::TIMER_ID_AUTO_SAVE, 
    		AutoSave.time, 
    		OnAutoSaveProc
    	);
	}
	else
	{
		KillTimer(hWnd,APPLICATION::TIMER_ID_AUTO_SAVE); 
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SAVESTATEMANAGER::UpdateDialog(HWND hDlg)
{
	SetDlgItemText( hDlg, IDC_AUTOSAVE_FILE, AutoSave.name.String() );	
	SetDlgItemInt( hDlg, IDC_AUTOSAVE_TIME, NST_FROM_MILLI(AutoSave.time), FALSE );	
	CheckDlgButton( hDlg, IDC_AUTOSAVE_ENABLED, AutoSave.enabled );
	CheckDlgButton( hDlg, IDC_AUTOSAVE_ENABLE_MSG, AutoSave.msg );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SAVESTATEMANAGER::UpdateDialogTime(HWND hDlg,const WPARAM wParam)
{
	if (HIWORD(wParam) == EN_KILLFOCUS)
	{
		UINT value = GetDlgItemInt( hDlg, IDC_AUTOSAVE_TIME, NULL, FALSE );
		
		if (value < 1)
			value = 1;

		SetDlgItemInt( hDlg, IDC_AUTOSAVE_TIME, value, FALSE );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SAVESTATEMANAGER::UpdateSettings(HWND hDlg)
{
	AutoSave.time    = NST_TO_MILLI(GetDlgItemInt( hDlg, IDC_AUTOSAVE_TIME, NULL, FALSE ));
	AutoSave.enabled = IsDlgButtonChecked( hDlg, IDC_AUTOSAVE_ENABLED    ) == BST_CHECKED ? TRUE : FALSE;
	AutoSave.msg     = IsDlgButtonChecked( hDlg, IDC_AUTOSAVE_ENABLE_MSG ) == BST_CHECKED ? TRUE : FALSE;

	if (AutoSave.time < NST_TO_MILLI(1))
		AutoSave.time = NST_TO_MILLI(1);

	AutoSave.name.Clear();

    CHAR buffer[NST_MAX_PATH];

	if (GetDlgItemText( hDlg, IDC_AUTOSAVE_FILE, buffer, NST_MAX_PATH ))
	{
		AutoSave.name = buffer;

		if (AutoSave.name.Length() && AutoSave.name.GetFileExtension().IsEmpty())
			AutoSave.name.Append( ".nst" );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SAVESTATEMANAGER::OnBrowse(HWND hDlg)
{
	PDXSTRING file;
	file.Buffer().Resize( NST_MAX_PATH );
	file.Buffer().Front() = '\0';

	OPENFILENAME ofn;
	PDXMemZero( ofn );

	ofn.lStructSize     = sizeof(ofn);
	ofn.hwndOwner       = hWnd;
	ofn.lpstrFilter     = "NES State (*.nst)\0*.nst\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrInitialDir	= application.GetFileManager().GetNstPath().String();
	ofn.lpstrFile       = file.Begin();
	ofn.lpstrTitle      = "Save State File";
	ofn.nMaxFile        = NST_MAX_PATH;
	ofn.Flags           = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;

	if (GetSaveFileName(&ofn))
	{
		file.Validate();

		if (file.Length() && file.GetFileExtension().IsEmpty())
			file.Append( ".nst" );

		SetDlgItemText( hDlg, IDC_AUTOSAVE_FILE, file.String() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SAVESTATEMANAGER::OnClear(HWND hDlg)
{
	SetDlgItemText( hDlg, IDC_AUTOSAVE_FILE, "" );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL SAVESTATEMANAGER::DialogProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM)
{
	switch (uMsg) 
	{
     	case WM_INITDIALOG:

			UpdateDialog( hDlg );
			return TRUE;

		case WM_COMMAND:

         	switch (LOWORD(wParam))
			{
    			case IDC_AUTOSAVE_CLEAR:  OnClear( hDlg ); return TRUE;
    			case IDC_AUTOSAVE_BROWSE: OnBrowse( hDlg ); return TRUE;
				case IDC_AUTOSAVE_TIME:   UpdateDialogTime( hDlg, wParam ); return TRUE;
				case IDC_AUTOSAVE_OK:     UpdateSettings( hDlg );
				case IDC_AUTOSAVE_CANCEL: EndDialog( hDlg, 0 ); return TRUE;
			}		
			return FALSE;

     	case WM_CLOSE:

     		EndDialog( hDlg, 0 );
     		return TRUE;

		case WM_DESTROY:

			UpdateAutoSaveTimer();
			return TRUE;
	}

	return FALSE;
}
