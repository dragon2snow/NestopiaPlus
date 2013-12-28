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

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

#include <Windows.h>
#include <CommCtrl.h>
#include <ShLwApi.h>
#include "resource/resource.h"
#include "NstLauncher.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

LAUNCHERITEMSEARCH::LAUNCHERITEMSEARCH()
:
hFind   (NULL),
hParent (NULL),
uFlags  (FR_DOWN),
uMsg    (::RegisterWindowMessage(FINDMSGSTRING))
{
	StringBuffer[0] = '\0';
	PDXMemZero( fr );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

LAUNCHERITEMSEARCH::~LAUNCHERITEMSEARCH()
{
	Close();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERITEMSEARCH::Open(HWND hWnd)
{
	PDX_ASSERT( hWnd );

	if (hFind)
		return;

	hParent = hWnd;

	StringBuffer[0] = '\0';

	fr.lStructSize   = sizeof(fr);
	fr.hInstance     = UTILITIES::GetInstance();
	fr.hwndOwner     = hParent;
	fr.lpstrFindWhat = StringBuffer;
	fr.wFindWhatLen  = STRING_BUFFER_LENGTH;
	fr.Flags         = uFlags;

	hFind = ::FindText( &fr );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERITEMSEARCH::Close()
{
	if (hFind)
	{
		::DestroyWindow( hFind );
		hFind = NULL;
	}

	hParent = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERITEMSEARCH::OnMsg(HWND hList,const LPARAM lParam)
{
	PDX_ASSERT( lParam && hList );

	const FINDREPLACE& fr = *PDX_CAST(const FINDREPLACE*,lParam);
	uFlags = fr.Flags & (FR_WHOLEWORD|FR_MATCHCASE|FR_DOWN);

	if (fr.Flags & FR_DIALOGTERM)
	{
		hFind = NULL;
		hParent = NULL;
	}
	else if (fr.Flags & FR_FINDNEXT)
	{
		OnFind( hList, fr.lpstrFindWhat );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERITEMSEARCH::OnFind(HWND hList,const CHAR* const SearchString)
{
	PDX_ASSERT( hList && SearchString );

	const TSIZE SearchStringLength = strlen( SearchString );

	if (!SearchStringLength)
		return;

	const INT NumItems = ListView_GetItemCount( hList );

	if (NumItems < 1)
		return;

	CHAR buffer[512];
	buffer[0] = '\0';

	const INT iItemOld = ListView_GetNextItem( hList, -1, LVNI_SELECTED );

	LVITEM lvItem;
	PDXMemZero( lvItem );

	lvItem.mask = LVIF_TEXT;
	lvItem.pszText = buffer;
	lvItem.cchTextMax = 512-1;

	if (iItemOld == -1)
	{
		lvItem.iItem = (uFlags & FR_DOWN) ? 0 : (NumItems-1);
	}
	else
	{
		if (uFlags & FR_DOWN)
			lvItem.iItem = PDX_MIN(iItemOld+1,NumItems-1);
		else
			lvItem.iItem = PDX_MAX(iItemOld-1,0);
	}

	BOOL found = FALSE;

	for (;;)
	{
		if (!ListView_GetItem( hList, &lvItem ) || !lvItem.pszText)
			break;

		if (uFlags & FR_WHOLEWORD)
		{
			if (uFlags & FR_MATCHCASE)
				found = !StrCmp( lvItem.pszText, SearchString );
			else
				found = !StrCmpI( lvItem.pszText, SearchString );
		}
		else
		{
			if (uFlags & FR_MATCHCASE)
				found = !StrCmpN( lvItem.pszText, SearchString, SearchStringLength );
			else
				found = !StrCmpNI( lvItem.pszText, SearchString, SearchStringLength );
		}

		if (found)
		{
			if (iItemOld != -1)
				ListView_SetItemState( hList, iItemOld, 0, LVIS_SELECTED );

			ListView_SetItemState( hList, lvItem.iItem, LVIS_SELECTED, LVIS_SELECTED );
			ListView_EnsureVisible( hList, lvItem.iItem, FALSE );
			break;
		}

		if (uFlags & FR_DOWN)
		{
			if (++lvItem.iItem >= NumItems)
				break;
		}
		else
		{
			if (--lvItem.iItem <= -1)
				break;
		}
	}

	if (!found)
		UI::MsgInfo( IDS_LAUNCHER_SEARCH_TEXT_NOT_FOUND );
}
