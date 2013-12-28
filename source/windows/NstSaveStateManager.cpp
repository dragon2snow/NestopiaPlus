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
#include "../paradox/PdxString.h"
#include "../paradox/PdxMap.h"
#include "NstSaveStateManager.h"
#include "NstFileManager.h"
#include "NstApplication.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#define NST_TO_MILLI(x)   ((x) * (1000 * 60))
#define NST_FROM_MILLI(x) ((x) / (1000 * 60))

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

SAVESTATEMANAGER::SAVESTATEMANAGER() 
: 
MANAGER    (IDD_AUTO_SAVE),
LastSlot   (0),
ImportFile (FALSE),
ExportFile (FALSE)
{}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SAVESTATEMANAGER::Create(CONFIGFILE* const)
{
	Reset();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SAVESTATEMANAGER::Reset()
{
	LastSlot = 0;

	for (UINT i=0; i < MAX_SLOTS; ++i)
		slots[i].Reset();

	AutoSave.filename.Clear();
	AutoSave.enabled = FALSE;
	AutoSave.time = NST_TO_MILLI(1);
	AutoSave.msg = TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SAVESTATEMANAGER::SetFileName(const UINT index,const PDXSTRING& filename)
{ 
	slots[IndexToSlot(index)].filename = filename; 

	LastSlot = 0;

	if (ImportFile)
	{
		PDXMAP<UINT,U64> tree;

		for (UINT i=0; i < MAX_SLOTS; ++i)
		{
			if (slots[i].filename.Length())
			{
				WIN32_FILE_ATTRIBUTE_DATA attribute;

				if (::GetFileAttributesEx(slots[i].filename.String(),GetFileExInfoStandard,PDX_CAST(LPVOID,&attribute)))
					tree[PDX_CAST_REF(const U64,attribute.ftLastWriteTime.dwLowDateTime)] = i;
			}
		}

		if (tree.Size())
			LastSlot = (*tree.Last()).Second();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SAVESTATEMANAGER::CacheSlots()
{
	PDXFILE file;

	for (UINT i=0; i < MAX_SLOTS; ++i)
	{
		if
		(
			slots[i].filename.Length() &&
			PDX_SUCCEEDED(file.Open(slots[i].filename,PDXFILE::INPUT)) &&
			file.Size()
		)
		{
			slots[i].data = file.Buffer();

			LOGFILE::Output
			(
     			"SAVESTATE: imported ",
				slots[i].filename,
				" to slot ",
				(i+1)
			);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SAVESTATEMANAGER::FlushSlots() const
{
	PDXFILE file;

	for (UINT i=0; i < MAX_SLOTS; ++i)
	{
		if
		(
			slots[i].data.Size() &&
			slots[i].filename.Length() &&
			PDX_SUCCEEDED(file.Open(slots[i].filename,PDXFILE::OUTPUT))
		)
		{
		    file.Write(slots[i].data.Begin(),slots[i].data.End());

			if (PDX_SUCCEEDED(file.Close()))
			{
				LOGFILE::Output
				(
					"SAVESTATE: exported slot ",
					(i+1),
					" to ",
					slots[i].filename
				);
			}
			else
			{
				LOGFILE::Output
				(
					"SAVESTATE: failed to export slot ",
					(i+1),
					" to ",
					slots[i].filename
				);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SAVESTATEMANAGER::LoadState(UINT index)
{
	if (!application.IsRunning())
		return PDX_FAILURE;

	index = IndexToSlot( index );
	SLOT& slot = slots[index];

	PDXFILE file;

	if (slot.data.Size())
	{
		file.Hook(slot.data,PDXFILE::INPUT);
		const PDXRESULT result = nes.LoadNST(file);
		file.UnHook();
		return result;
	}

	if (!ImportFile || !slot.filename.Length())
		return PDX_FAILURE;

	if (PDX_SUCCEEDED(file.Open(slot.filename,PDXFILE::INPUT)) && PDX_SUCCEEDED(nes.LoadNST(file)))
	{
		slot.data = file.Buffer();

		LOGFILE::Output
		(
			"SAVESTATE: imported ",
			slot.filename,
			" to slot ",
			index
		);

		return PDX_OK;
	}

	LOGFILE::Output
	(
		"SAVESTATE: failed to import ",
		slot.filename,
		" to slot ",
		index
	);

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SAVESTATEMANAGER::SaveState(UINT index)
{
	PDX_ASSERT( index <= MAX_SLOTS );

	if (!application.IsRunning())
		return PDX_FAILURE;

	if (index == NEXT_SLOT)
	{
		if (++LastSlot == MAX_SLOTS)
			LastSlot = 0;
	}
	else
	{
		LastSlot = index - 1;
	}

	SLOT& slot = slots[LastSlot];

	PDXFILE file("x",PDXFILE::OUTPUT);

	if (PDX_FAILED(nes.SaveNST(file)))
	{
		file.Abort();

		LOGFILE::Output
		(
			"SAVESTATE: failed to save to slot ",
			LastSlot
		);

		return PDX_FAILURE;
	}

	slot.data = file.Buffer();

	if (ExportFile && slot.filename.Length())
	{
		file.ChangeName(slot.filename.String());

		if (PDX_FAILED(file.Close()))
		{
			LOGFILE::Output
			(
				"SAVESTATE: exported slot ",
				LastSlot,
				" to ",
				slot.filename
			);
		}
		else
		{
			LOGFILE::Output
			(
				"SAVESTATE: failed to export slot ",
				LastSlot,
				" to ",
				slot.filename
			);
		}
	}

	file.Abort();

	return PDX_OK;
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
	if (application.IsRunning())
	{
		UINT NoFile;

		if (NoFile = AutoSave.filename.Length())
		{
			PDXFILE file( AutoSave.filename, PDXFILE::OUTPUT );

			if (file.IsOpen() && PDX_SUCCEEDED( nes.SaveNST(file) ) && PDX_SUCCEEDED( file.Close() ))
			{
				if (AutoSave.msg)
					application.StartScreenMsg( 1000, "Auto-Saved to file.." );

				return;
			}

			file.Abort();
		}

		if (PDX_SUCCEEDED(SaveState(1)))
		{
			if (AutoSave.msg)
			{
				if (NoFile)
					application.StartScreenMsg( 1000, "Auto-Save to file failed, wrote to slot 1 instead.." );
				else
					application.StartScreenMsg( 1000, "Auto-Saved to slot 1.." );
			}
		}
		else
		{
			if (AutoSave.msg)
				application.StartScreenMsg( 1000, "Auto-Save failed.." );
		}
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
		::SetTimer
		( 
    		hWnd, 
    		APPLICATION::TIMER_ID_AUTO_SAVE, 
    		AutoSave.time, 
    		OnAutoSaveProc
    	);
	}
	else
	{
		::KillTimer(hWnd,APPLICATION::TIMER_ID_AUTO_SAVE); 
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SAVESTATEMANAGER::UpdateDialog(HWND hDlg)
{
	::SetDlgItemText( hDlg, IDC_AUTOSAVE_FILE, AutoSave.filename.String() );	
	::SetDlgItemInt( hDlg, IDC_AUTOSAVE_TIME, NST_FROM_MILLI(AutoSave.time), FALSE );	
	::CheckDlgButton( hDlg, IDC_AUTOSAVE_ENABLED, AutoSave.enabled );
	::CheckDlgButton( hDlg, IDC_AUTOSAVE_ENABLE_MSG, AutoSave.msg );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SAVESTATEMANAGER::UpdateDialogTime(HWND hDlg,const WPARAM wParam)
{
	if (HIWORD(wParam) == EN_KILLFOCUS)
	{
		UINT value = ::GetDlgItemInt( hDlg, IDC_AUTOSAVE_TIME, NULL, FALSE );
		
		if (value < 1)
			value = 1;

		::SetDlgItemInt( hDlg, IDC_AUTOSAVE_TIME, value, FALSE );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SAVESTATEMANAGER::UpdateSettings(HWND hDlg)
{
	AutoSave.time    = NST_TO_MILLI(::GetDlgItemInt( hDlg, IDC_AUTOSAVE_TIME, NULL, FALSE ));
	AutoSave.enabled = (::IsDlgButtonChecked( hDlg, IDC_AUTOSAVE_ENABLED    ) == BST_CHECKED);
	AutoSave.msg     = (::IsDlgButtonChecked( hDlg, IDC_AUTOSAVE_ENABLE_MSG ) == BST_CHECKED);

	if (AutoSave.time < NST_TO_MILLI(1))
		AutoSave.time = NST_TO_MILLI(1);

	AutoSave.filename.Clear();

	PDXSTRING filename;

	if (MANAGER::GetDlgItemText( hDlg, IDC_AUTOSAVE_FILE, filename ))
	{
		AutoSave.filename = filename;

		if (AutoSave.filename.Length() && AutoSave.filename.GetFileExtension().IsEmpty())
			AutoSave.filename.Append( ".nst" );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SAVESTATEMANAGER::OnBrowse(HWND hDlg)
{
	PDXSTRING filename;

	const BOOL succeeded = UTILITIES::BrowseSaveFile
	(
	    filename,
		hDlg,
		IDS_AUTOSAVER_SELECT_NST,
   		"NES State (*.nst)\0"
		"*.nst\0"
		"All Files (*.*)\0"
		"*.*\0",
		application.GetFileManager().GetNstPath().String(),
		"nst"
	);

	if (succeeded)
		::SetDlgItemText( hDlg, IDC_AUTOSAVE_FILE, filename.String() );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SAVESTATEMANAGER::OnClear(HWND hDlg)
{
	::SetDlgItemText( hDlg, IDC_AUTOSAVE_FILE, "" );
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
				case IDC_AUTOSAVE_CANCEL: ::EndDialog( hDlg, 0 ); return TRUE;
			}		
			return FALSE;

     	case WM_CLOSE:

     		::EndDialog( hDlg, 0 );
     		return TRUE;

		case WM_DESTROY:

			UpdateAutoSaveTimer();
			return TRUE;
	}

	return FALSE;
}
