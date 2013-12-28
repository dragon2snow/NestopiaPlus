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

#include <windows.h>
#include "../paradox/PdxString.h"
#include "resource/resource.h"
#include "NstUserInputManager.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL USERINPUTMANAGER::Start(const CHAR* const t,const CHAR* const m,PDXSTRING& i)
{
	title.Clear();
	msg.Clear();

	if (t) title = t;
	if (m) msg = m;

	ok = FALSE;

	StartDialog();

	if (ok)
		i = input;

	input.Destroy();
	title.Destroy();
	msg.Destroy();
	
	return ok;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL USERINPUTMANAGER::DialogProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM)
{
	switch (uMsg) 
	{
       	case WM_INITDIALOG:

			ok = FALSE;

			input.Clear();
			
			if (msg.Length())
				SetDlgItemText( hDlg, IDC_USERINPUT_TEXT, msg );

			if (title.Length())
				SetWindowText( hDlg, title );

			return TRUE;

     	case WM_COMMAND:

     		switch (LOWORD(wParam))
     		{
        		case IDC_USERINPUT_OK:    

					input.Buffer().Resize( 256 );
					input.Buffer().Back() = '\0';
					GetDlgItemText( hDlg, IDC_USERINPUT_EDIT, input.Begin(), 256 );
					input.Validate();
					ok = TRUE;

          		case IDC_USERINPUT_ABORT: 
					
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
