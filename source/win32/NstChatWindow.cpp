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
#include <Richedit.h>
#include "NstChatWindow.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

CHATWINDOW::CHATWINDOW()
: 
MANAGER          (IDD_CHAT), 
hDlg             (NULL),
SendChatCallback (NULL)
{}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

CHATWINDOW::~CHATWINDOW()
{
	Close();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID CHATWINDOW::Open(CHATSENDCALLBACK callback)
{
	PDX_ASSERT( callback );

	SendChatCallback = callback;

	if (!hDlg)
	{
		MANAGER::StartModelessDialog();
		::ShowWindow( hDlg, SW_SHOW );
		::SendMessage( ::GetDlgItem( hDlg, IDC_CHAT ), EM_SETEVENTMASK, 0, ENM_KEYEVENTS );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID CHATWINDOW::Close()
{
	SendChatCallback = NULL;

	if (hDlg)
	{
		::DestroyWindow( hDlg );
		hDlg = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID CHATWINDOW::OnNotify(LPARAM lParam)
{
	NMHDR& nmhdr = *PDX_CAST(LPNMHDR,lParam);

	if (nmhdr.idFrom != IDC_CHAT || nmhdr.code != EN_MSGFILTER)
		return;
	
	MSGFILTER& MsgFilter = *PDX_CAST(MSGFILTER*,lParam);

	if (MsgFilter.msg != WM_KEYUP || MsgFilter.wParam != VK_RETURN)
		return;
	
	HWND hEdit = ::GetDlgItem( hDlg, IDC_CHAT );
	const INT length = ::GetWindowTextLength( hEdit );

	if (length <= 0)
		return;

	ChatText.Resize( length );
	::GetWindowText( hEdit, ChatText.Begin(), length );
	ChatText.RemoveSpaces();

	if (ChatText.Length())
		SendChatCallback( ChatText );

	::SetWindowText( hEdit, "" );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL CHATWINDOW::DialogProc(HWND h,UINT uMsg,WPARAM,LPARAM lParam)
{
	switch (uMsg)
	{
     	case WM_INITDIALOG:

			hDlg = h;
			return TRUE;

		case WM_NOTIFY:  

			OnNotify( lParam );
			return TRUE;
  
		case WM_CLOSE:

			Close();
			return TRUE;
	}

	return FALSE;
}
