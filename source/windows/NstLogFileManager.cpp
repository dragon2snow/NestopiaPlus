///////////////////////////////////////////////////////////////////////////////////////
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

#include <windows.h>
#include "../paradox/PdxFile.h"
#include "resource/resource.h"
#include "NstLogFileManager.h"
#include "NstFileManager.h"

PDXSTRING LOGFILEMANAGER::LogString;

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LOGFILEMANAGER::LogSeparator()
{
	LogString << "-----------------------------------------------------------------------------------------------------\r\n";
}

/////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LOGFILEMANAGER::Close(const BOOL WriteToFile)
{
	if (WriteToFile && LogString.Length())
	{
		PDXSTRING name;

		if (PDX_SUCCEEDED(FILEMANAGER::GetExeFileName(name)))
		{
			name.ReplaceFileExtension("log");

			PDXFILE file(name,PDXFILE::OUTPUT);

			if (file.IsOpen())
				file.Text() << LogString;
		}
	}

	LogString.Destroy();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL LOGFILEMANAGER::DialogProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM)
{
	switch (uMsg) 
	{
     	case WM_INITDIALOG:
		
			SetDlgItemText( hDlg, IDC_LOGFILE_EDIT, LogString.String() );
			return TRUE;
		    
		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
       			case IDC_LOGFILE_CLEAR:  
					
					SetDlgItemText( hDlg, IDC_LOGFILE_EDIT, "" ); 
					LogString.Clear(); 
					return TRUE;

     			case IDC_LOGFILE_OK:
					
					EndDialog( hDlg, 0 ); 
					return TRUE;
			}		
			return FALSE;

     	case WM_CLOSE:

     		EndDialog( hDlg, 0 );
     		return TRUE;
	}

	return FALSE;
}
