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

#include <cstdio>
#include <Windows.h>
#include <WindowsX.h>
#include <CommCtrl.h>
#include <ShLwApi.h>
#include <ShellAPI.h>
#include <WinAble.h>
#include "resource/resource.h"
#include "NstApplication.h"
#include "NstPreferences.h"
#include "NstFileManager.h"
#include "NstGraphicManager.h"
#include "NstSoundManager.h"
#include "../NstNes.h"
#include "NstLauncher.h"

#define NST_DEF_LISTVIEW_STYLE (LVS_EX_FULLROWSELECT|LVS_EX_TWOCLICKACTIVATE)

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

LAUNCHER::LAUNCHER()
: 
MANAGER         (IDD_LAUNCHER),
hDlg            (NULL),
hStatusBar      (NULL),
hList           (NULL),
hTree           (NULL),
hMenu           (NULL),
hTmpCursor      (NULL),
hImageList      (ImageList_Create(16,16,ILC_COLOR16,0,2)),
ShowDatabase    (TRUE),
UpdateFlags     (ENTRY::TYPE_ANY),
ColumnPriority  (0)
{
	if (!hImageList)
		throw ("ImageList_Create() failed!");

	PDXMemZero( hTreeItems, TREE_NUM );

	HINSTANCE hInstance = UTILITIES::GetInstance();
	HBITMAP hBitmap;

	hBitmap = ::LoadBitmap( hInstance, MAKEINTRESOURCE( IDB_FOLDER_OPEN ) );
	ImageIds[IMAGE_OPEN] = ImageList_Add( hImageList, hBitmap, NULL );
	::DeleteObject( hBitmap ); 

	hBitmap = ::LoadBitmap( hInstance, MAKEINTRESOURCE( IDB_FOLDER_CLOSED ) );
	ImageIds[IMAGE_CLOSED] = ImageList_Add( hImageList, hBitmap, NULL );
	::DeleteObject( hBitmap ); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

LAUNCHER::~LAUNCHER()
{
	if (hImageList)
		ImageList_Destroy( hImageList );
  
	if (hDlg)
		::DestroyWindow( hDlg );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::Create(CONFIGFILE* const ConfigFile)
{
	columns.Create      ( ConfigFile );
	paths.Create        ( ConfigFile );
	FileSearcher.Create ( ConfigFile );
	colors.Create       ( ConfigFile );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::Destroy(CONFIGFILE* const ConfigFile)
{
	CONFIGFILE* const SaveLauncher = application.GetPreferences().SaveLauncher() ? ConfigFile : NULL;

	columns.Destroy      ( ConfigFile   );
	paths.Destroy        ( ConfigFile   );
	FileSearcher.Destroy ( SaveLauncher );
	colors.Destroy       ( ConfigFile   );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL LAUNCHER::IsDlgMessage(MSG& msg)
{
	if (hDlg)
	{
		if (::IsDialogMessage( hDlg, &msg ))
			return TRUE;

		if (ItemSearcher.IsDlgMessage( msg ))
			return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::Open()
{
	if (hDlg)
		return;

	application.GetSoundManager().Clear();

	if (application.GetGraphicManager().GetDisplayWidth() < 640 || application.GetGraphicManager().GetDisplayHeight() < 480)
	{
		UI::MsgInfo(IDS_LAUNCHER_UNSUPPORTED_RES);
		return;
	}

	MANAGER::StartModelessDialog();

	if (!hDlg)
		throw ("CreateDialog() failed!");

	hList = ::GetDlgItem( hDlg, IDC_LAUNCHER_LIST );
	hTree = ::GetDlgItem( hDlg, IDC_LAUNCHER_TREE );
	hMenu = ::GetMenu( hDlg );

	ListView_SetExtendedListViewStyle( hList, NST_DEF_LISTVIEW_STYLE );

	RECT rcWindow;
	::GetWindowRect( hDlg, &rcWindow );

	{
		RECT rcList;
		::GetWindowRect( ::GetDlgItem( hDlg, IDC_LAUNCHER_LIST ), &rcList );

		ListWidthMargin = rcWindow.right - rcList.right;
		ListHeightMargin = rcWindow.bottom - rcList.bottom;
	}

	hStatusBar = CreateWindowEx
	( 
		0,                       
		STATUSCLASSNAME,         
		NULL,          
		WS_CHILD|WS_VISIBLE|SBARS_SIZEGRIP,
		0,0,0,0,
		hDlg,
		HMENU(STATUSBAR_ID),
		UTILITIES::GetInstance(),
		NULL
	);

	if (!hStatusBar)
		throw ("CreateWindowEx() failed!");

	::SendMessage( hStatusBar, SB_SIMPLE, WPARAM(TRUE), 0 );

	UpdateColumns();
	OnAlignColumns();

	{
		TreeView_SetImageList( hTree, hImageList, TVSIL_NORMAL );

		TVINSERTSTRUCT tv; 
		PDXMemZero(tv);

		tv.hParent             = TVI_ROOT;
		tv.hInsertAfter        = TVI_LAST;
		tv.item.mask           = TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
		tv.item.iImage         = ImageIds[ IMAGE_CLOSED ];
		tv.item.iSelectedImage = ImageIds[ IMAGE_OPEN   ];

		HTREEITEM hItem;

		tv.item.pszText = "All Files"; 
		tv.item.cchTextMax = strlen("All Files");  
		hTreeItems[TREE_ALL] = TreeView_InsertItem( hTree, &tv );

		tv.item.pszText = "NES Files"; 
		tv.item.cchTextMax = strlen("NES Files");  
		hTreeItems[TREE_NES] = TreeView_InsertItem( hTree, &tv );

		tv.item.pszText = "UNIF Files"; 
		tv.item.cchTextMax = strlen("UNIF Files");  
		hTreeItems[TREE_UNF] = TreeView_InsertItem( hTree, &tv );

		tv.item.pszText = "FDS Files"; 
		tv.item.cchTextMax = strlen("FDS Files");  
		hTreeItems[TREE_FDS] = TreeView_InsertItem( hTree, &tv );

		tv.item.pszText = "NSF Files"; 
		tv.item.cchTextMax = strlen("NSF Files");  
		hTreeItems[TREE_NSF] = TreeView_InsertItem( hTree, &tv );

		tv.item.pszText = "NSP Files"; 
		tv.item.cchTextMax = strlen("NSP Files");  
		hTreeItems[TREE_NSP] = TreeView_InsertItem( hTree, &tv );

		TreeView_SelectItem( hTree, hTreeItems[TREE_ALL] );
	}

	UpdateFlags = ENTRY::TYPE_ANY;

	ListView_SetBkColor     ( hList, colors.GetBgColor() );
	ListView_SetTextBkColor ( hList, colors.GetBgColor() );
	ListView_SetTextColor   ( hList, colors.GetFgColor() );
	TreeView_SetBkColor     ( hTree, colors.GetBgColor() );
	TreeView_SetTextColor   ( hTree, colors.GetFgColor() );

	::ShowWindow( hDlg, SW_SHOW );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::Activate()
{
	PDX_ASSERT( hDlg );
	::ShowWindow( hDlg, SW_SHOWNA );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::Inactivate()
{
	PDX_ASSERT( hDlg );
	::ShowWindow( hDlg, SW_HIDE );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::BeginUpdate(const BOOL hide)
{
	application.GetSoundManager().Clear();

	::EnableWindow( hWnd, FALSE );
	::EnableWindow( hDlg, FALSE );

	hTmpCursor = ::SetCursor( ::LoadCursor( NULL, IDC_WAIT ) );

	if (hide)
		::ShowWindow( hList, SW_HIDE );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::EndUpdate()
{
	::EnableWindow( hWnd, TRUE );
	::EnableWindow( hDlg, TRUE );
	::ShowWindow( hList, SW_SHOWNA );

	if (hTmpCursor)
	{
		::SetCursor( hTmpCursor );
		hTmpCursor = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::UpdateColumns()
{
	ListView_DeleteAllItems( hList );

	while (ListView_DeleteColumn( hList, 0 ));

	ColumnEntries.Clear();

	LVCOLUMN lvColumn;
	lvColumn.mask = LVCF_FMT|LVCF_TEXT|LVCF_WIDTH;
	lvColumn.fmt = LVCFMT_LEFT;

	PDXSTRING ColumnText;
	COLUMNENTRY entry;

	for (UINT i=0; i < columns.NumSelectedTypes(); ++i)
	{
		switch (columns.GetType(i))
		{
     		case LAUNCHERCOLUMNS::TYPE_FILE:      entry.Add = ColumnAddFile;      entry.Compare = ColumnSortFile;      break;
			case LAUNCHERCOLUMNS::TYPE_SYSTEM:    entry.Add = ColumnAddSystem;    entry.Compare = ColumnSortSystem;    break;
			case LAUNCHERCOLUMNS::TYPE_MAPPER:    entry.Add = ColumnAddMapper;    entry.Compare = ColumnSortMapper;    break;
			case LAUNCHERCOLUMNS::TYPE_PROM:      entry.Add = ColumnAddPRom;      entry.Compare = ColumnSortPRom;      break;
			case LAUNCHERCOLUMNS::TYPE_CROM:      entry.Add = ColumnAddCRom;      entry.Compare = ColumnSortCRom;      break;
			case LAUNCHERCOLUMNS::TYPE_WRAM:      entry.Add = ColumnAddWRam;      entry.Compare = ColumnSortWRam;      break;
			case LAUNCHERCOLUMNS::TYPE_BATTERY:   entry.Add = ColumnAddBattery;   entry.Compare = ColumnSortBattery;   break;
			case LAUNCHERCOLUMNS::TYPE_TRAINER:   entry.Add = ColumnAddTrainer;   entry.Compare = ColumnSortTrainer;   break;
			case LAUNCHERCOLUMNS::TYPE_MIRRORING: entry.Add = ColumnAddMirroring; entry.Compare = ColumnSortMirroring; break;
			case LAUNCHERCOLUMNS::TYPE_NAME:      entry.Add = ColumnAddName;      entry.Compare = ColumnSortName;      break;
			case LAUNCHERCOLUMNS::TYPE_COPYRIGHT: entry.Add = ColumnAddCopyright; entry.Compare = ColumnSortCopyright; break;
			case LAUNCHERCOLUMNS::TYPE_CONDITION: entry.Add = ColumnAddCondition; entry.Compare = ColumnSortCondition; break;
     		case LAUNCHERCOLUMNS::TYPE_FOLDER:    entry.Add = ColumnAddFolder;    entry.Compare = ColumnSortFolder;    break;
		}

		ColumnEntries.InsertBack( entry );

		ColumnText = columns.GetString(i);
		lvColumn.pszText = ColumnText.Begin();

		ListView_InsertColumn( hList, i, &lvColumn );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::OnAlignColumns()
{
	for (INT i=0; ListView_SetColumnWidth( hList, i, LVSCW_AUTOSIZE_USEHEADER ); ++i)
	{
		const INT header = ListView_GetColumnWidth( hList, i );		

		ListView_SetColumnWidth( hList, i, LVSCW_AUTOSIZE );

		const INT text = ListView_GetColumnWidth( hList, i );

		if (text < header)
			ListView_SetColumnWidth( hList, i, header );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

INT CALLBACK LAUNCHER::StaticEntryCompare(LPARAM lParam1,LPARAM lParam2,LPARAM lParam3)
{
	PDX_ASSERT( lParam1 && lParam2 && lParam3 );

	INT compare = 0;

	const LAUNCHER* const launcher = PDX_CAST(const LAUNCHER*,lParam3);
	const UINT num = launcher->ColumnEntries.Size();

	for (UINT i=0; i < num; ++i)
	{
		compare = (*launcher.*launcher->ColumnEntries[i].Compare)
		( 
			*PDX_CAST(const ENTRY*,lParam1),
			*PDX_CAST(const ENTRY*,lParam2)
		);

		if (compare)
			break;
	}

	return compare;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

INT LAUNCHER::ColumnSortFile(const ENTRY& a,const ENTRY& b) const 
{ 
	return StrCmpI( a.file.String(), b.file.String() );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

INT LAUNCHER::ColumnSortSystem(const ENTRY& a,const ENTRY& b) const 
{ 
	UINT s1 = UINT_MAX;

	if (a.dBaseHandle)
		s1 = nes.GetRomDatabase().System( a.dBaseHandle );

	if (!a.dBaseHandle || !ShowDatabase)
	{
		if (s1 == NES::SYSTEM_VS)
			s1 = NES::SYSTEM_NTSC;

		if      ( a.header.vs   ) s1 = NES::SYSTEM_VS;
		else if ( a.header.ntsc ) s1 = NES::SYSTEM_NTSC;
		else if ( a.header.pal  ) s1 = NES::SYSTEM_PAL;
	}

	UINT s2 = UINT_MAX;

	if (b.dBaseHandle)
		s2 = nes.GetRomDatabase().System( b.dBaseHandle );

	if (!b.dBaseHandle || !ShowDatabase)
	{
		if (s2 == NES::SYSTEM_VS)
			s2 = NES::SYSTEM_NTSC;

		if      ( b.header.vs   ) s2 = NES::SYSTEM_VS;
		else if ( b.header.ntsc ) s2 = NES::SYSTEM_NTSC;
		else if ( b.header.pal  ) s2 = NES::SYSTEM_PAL;
	}

	return (s1 < s2 ? -1 : s1 > s2 ? +1 : 0);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

INT LAUNCHER::ColumnSortMapper(const ENTRY& a,const ENTRY& b) const 
{
	INT m1 = -1;

	if (a.type & ENTRY::TYPE_NES)
	{
		m1 = (ShowDatabase && a.dBaseHandle ? nes.GetRomDatabase().Mapper( a.dBaseHandle ) : a.header.mapper);
	}
	else if (a.type & ENTRY::TYPE_UNF)
	{
		m1 = INT_MAX;
	}

	INT m2 = -1;

	if (b.type & ENTRY::TYPE_NES)
	{
		m2 = (ShowDatabase && b.dBaseHandle ? nes.GetRomDatabase().Mapper( b.dBaseHandle ) : b.header.mapper);
	}
	else if (b.type & ENTRY::TYPE_UNF)
	{
		m2 = INT_MAX;
	}
	
	return (m1 < m2 ? -1 : m1 > m2 ? +1 : 0);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

INT LAUNCHER::ColumnSortPRom(const ENTRY& a,const ENTRY& b) const 
{ 
	const UINT p1 = (ShowDatabase && a.dBaseHandle ? nes.GetRomDatabase().pRomSize( a.dBaseHandle ) : a.header.pRomSize);
	const UINT p2 = (ShowDatabase && b.dBaseHandle ? nes.GetRomDatabase().pRomSize( b.dBaseHandle ) : b.header.pRomSize);

	return (p1 < p2 ? -1 : p1 > p2 ? +1 : 0);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

INT LAUNCHER::ColumnSortCRom(const ENTRY& a,const ENTRY& b) const 
{ 
	INT c1 = -1;

	if (a.type & (ENTRY::TYPE_NES|ENTRY::TYPE_UNF))
		c1 = (ShowDatabase && a.dBaseHandle ? nes.GetRomDatabase().cRomSize( a.dBaseHandle ) : a.header.cRomSize);

	INT c2 = -1;

	if (b.type & (ENTRY::TYPE_NES|ENTRY::TYPE_UNF))
		c2 = (ShowDatabase && b.dBaseHandle ? nes.GetRomDatabase().cRomSize( b.dBaseHandle ) : b.header.cRomSize);

	return (c1 < c2 ? -1 : c1 > c2 ? +1 : 0);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

INT LAUNCHER::ColumnSortWRam(const ENTRY& a,const ENTRY& b) const 
{ 
	INT w1 = INT_MAX;

	if (a.type & (ENTRY::TYPE_NES|ENTRY::TYPE_FDS|ENTRY::TYPE_NSF))
		w1 = (ShowDatabase && a.dBaseHandle ? nes.GetRomDatabase().wRamSize( a.dBaseHandle ) : a.header.wRamSize);

	INT w2 = INT_MAX;

	if (b.type & (ENTRY::TYPE_NES|ENTRY::TYPE_FDS|ENTRY::TYPE_NSF))
		w2 = (ShowDatabase && b.dBaseHandle ? nes.GetRomDatabase().wRamSize( b.dBaseHandle ) : b.header.wRamSize);

	return (w1 < w2 ? -1 : w1 > w2 ? +1 : 0);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

INT LAUNCHER::ColumnSortBattery(const ENTRY& a,const ENTRY& b) const 
{ 
	const UINT b1 = (ShowDatabase && a.dBaseHandle ? nes.GetRomDatabase().HasBattery( a.dBaseHandle ) : a.header.battery);
	const UINT b2 = (ShowDatabase && b.dBaseHandle ? nes.GetRomDatabase().HasBattery( b.dBaseHandle ) : b.header.battery);

	return (b1 < b2 ? -1 : b1 > b2 ? +1 : 0);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

INT LAUNCHER::ColumnSortTrainer(const ENTRY& a,const ENTRY& b) const 
{ 
	const UINT t1 = (ShowDatabase && a.dBaseHandle ? nes.GetRomDatabase().HasTrainer( a.dBaseHandle ) : a.header.trainer);
	const UINT t2 = (ShowDatabase && b.dBaseHandle ? nes.GetRomDatabase().HasTrainer( b.dBaseHandle ) : b.header.trainer);

	return (t1 < t2 ? -1 : t1 > t2 ? +1 : 0);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

INT LAUNCHER::ColumnSortMirroring(const ENTRY& a,const ENTRY& b) const 
{ 
	INT m1 = -1;

	if (a.type & (ENTRY::TYPE_NES|ENTRY::TYPE_UNF))
		m1 = (ShowDatabase && a.dBaseHandle ? nes.GetRomDatabase().Mirroring( a.dBaseHandle ) : a.header.mirroring);

	INT m2 = -1;

	if (b.type & (ENTRY::TYPE_NES|ENTRY::TYPE_UNF))
		m2 = (ShowDatabase && b.dBaseHandle ? nes.GetRomDatabase().Mirroring( b.dBaseHandle ) : b.header.mirroring);

	return (m1 < m2 ? -1 : m1 > m2 ? +1 : 0);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

INT LAUNCHER::ColumnSortName(const ENTRY& a,const ENTRY& b) const 
{ 
	const CHAR* const n1 = (a.name.IsEmpty() && a.dBaseHandle ? nes.GetRomDatabase().Name( a.dBaseHandle ) : a.name.String());
	const CHAR* const n2 = (b.name.IsEmpty() && b.dBaseHandle ? nes.GetRomDatabase().Name( b.dBaseHandle ) : b.name.String());

	if (*n1 != '-' && *n2 == '-') return -1;
	if (*n1 == '-' && *n2 != '-') return +1;

	return StrCmpI( n1, n2 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

INT LAUNCHER::ColumnSortCopyright(const ENTRY& a,const ENTRY& b) const 
{ 
	const CHAR* const c1 = (a.copyright.IsEmpty() && a.dBaseHandle ? nes.GetRomDatabase().Copyright( a.dBaseHandle ) : a.copyright.String());
	const CHAR* const c2 = (b.copyright.IsEmpty() && b.dBaseHandle ? nes.GetRomDatabase().Copyright( b.dBaseHandle ) : b.copyright.String());

	if (*c1 != '-' && *c2 == '-') return -1;
	if (*c1 == '-' && *c2 != '-') return +1;

	return StrCmpI( c1, c2 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

INT LAUNCHER::ColumnSortCondition(const ENTRY& a,const ENTRY& b) const 
{ 
	const UINT b1 = (a.dBaseHandle ? (nes.GetRomDatabase().IsBad( a.dBaseHandle ) ? 1 : 0) : 2);
	const UINT b2 = (b.dBaseHandle ? (nes.GetRomDatabase().IsBad( b.dBaseHandle ) ? 1 : 0) : 2);

	return (b1 < b2 ? -1 : b1 > b2 ? +1 : 0);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

INT LAUNCHER::ColumnSortFolder(const ENTRY& a,const ENTRY& b) const 
{
	return StrCmpI( a.path.String(), b.path.String() );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::ColumnAddFile(LVITEM& lvItem,const ENTRY& entry) 
{ 
	lvItem.pszText = const_cast<LPSTR>( entry.file.String() );
	::SendMessage( hList, LVM_SETITEMTEXT, WPARAM(lvItem.iItem), PDX_CAST(LPARAM,&lvItem) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::ColumnAddSystem(LVITEM& lvItem,const ENTRY& entry)
{ 
	lvItem.pszText = "unknown";

	if (entry.dBaseHandle)
	{
		switch (nes.GetRomDatabase().System( entry.dBaseHandle ))
		{
     		case NES::SYSTEM_NTSC: lvItem.pszText = "ntsc"; break;
     		case NES::SYSTEM_PAL:  lvItem.pszText = "pal";  break;
     		case NES::SYSTEM_VS:   lvItem.pszText = "vs";   break;
     		case NES::SYSTEM_PC10: lvItem.pszText = "pc10"; break;
		}
	}

	if (!entry.dBaseHandle || !ShowDatabase)
	{
		if (lvItem.pszText[0] == 'v')
			lvItem.pszText = "ntsc";

		if      ( entry.header.vs   ) lvItem.pszText = "vs";
		else if ( entry.header.ntsc ) lvItem.pszText = "ntsc";
		else if ( entry.header.pal  ) lvItem.pszText = "pal";
	}

	::SendMessage( hList, LVM_SETITEMTEXT, WPARAM(lvItem.iItem), PDX_CAST(LPARAM,&lvItem) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::ColumnAddMapper(LVITEM& lvItem,const ENTRY& entry) 
{
	lvItem.pszText = "none";

	CHAR text[8];

	if (entry.type & ENTRY::TYPE_NES)
	{
		UINT mapper;

		if (ShowDatabase && entry.dBaseHandle)
			mapper = nes.GetRomDatabase().Mapper( entry.dBaseHandle );
		else
			mapper = entry.header.mapper;

		PDX_ASSERT( mapper <= 255 );

		if (mapper > 0)
		{
			sprintf( text, "%u", mapper );
			lvItem.pszText = text;
		}
	}
	else if (entry.type & ENTRY::TYPE_UNF)
	{
		lvItem.pszText = "unknown";
	}

	::SendMessage( hList, LVM_SETITEMTEXT, WPARAM(lvItem.iItem), PDX_CAST(LPARAM,&lvItem) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::ColumnAddPRom(LVITEM& lvItem,const ENTRY& entry)  
{
	lvItem.pszText = "none";

	UINT size;

	if (ShowDatabase && entry.dBaseHandle)
		size = nes.GetRomDatabase().pRomSize( entry.dBaseHandle );
	else
		size = entry.header.pRomSize;

	CHAR text[16];

	if (size)
	{
		sprintf( text, "%uk", (size / 1024) );
		lvItem.pszText = text;
	}

	::SendMessage( hList, LVM_SETITEMTEXT, WPARAM(lvItem.iItem), PDX_CAST(LPARAM,&lvItem) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::ColumnAddCRom(LVITEM& lvItem,const ENTRY& entry)  
{
	lvItem.pszText = "none";

	CHAR text[16];

	if (entry.type & (ENTRY::TYPE_NES|ENTRY::TYPE_UNF))
	{
		UINT size;

		if (ShowDatabase && entry.dBaseHandle)
			size = nes.GetRomDatabase().cRomSize( entry.dBaseHandle );
		else
			size = entry.header.cRomSize;

		if (size)
		{
			sprintf( text, "%uk", (size / 1024) );
			lvItem.pszText = text;
		}
	}

	::SendMessage( hList, LVM_SETITEMTEXT, WPARAM(lvItem.iItem), PDX_CAST(LPARAM,&lvItem) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::ColumnAddWRam(LVITEM& lvItem,const ENTRY& entry)  
{
	CHAR text[16];

	if (entry.type & (ENTRY::TYPE_NES|ENTRY::TYPE_FDS|ENTRY::TYPE_NSF))
	{
		UINT size;

		if (ShowDatabase && entry.dBaseHandle)
			size = nes.GetRomDatabase().wRamSize( entry.dBaseHandle );
		else
			size = entry.header.wRamSize;

		if (size)
		{
			sprintf( text, "%uk", (size / 1024) );
			lvItem.pszText = text;
		}
		else
		{
			lvItem.pszText = "none";
		}
	}
	else
	{
		lvItem.pszText = "unknown";
	}

	::SendMessage( hList, LVM_SETITEMTEXT, WPARAM(lvItem.iItem), PDX_CAST(LPARAM,&lvItem) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::ColumnAddBattery(LVITEM& lvItem,const ENTRY& entry)  
{
	lvItem.pszText = "no";

	if (entry.type & (ENTRY::TYPE_NES|ENTRY::TYPE_UNF))
	{
		if (ShowDatabase && entry.dBaseHandle)
		{
			if (nes.GetRomDatabase().HasBattery( entry.dBaseHandle ))
				lvItem.pszText = "yes";
		}
		else if (entry.header.battery)
		{
			lvItem.pszText = "yes";
		}
	}

	::SendMessage( hList, LVM_SETITEMTEXT, WPARAM(lvItem.iItem), PDX_CAST(LPARAM,&lvItem) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::ColumnAddTrainer(LVITEM& lvItem,const ENTRY& entry)  
{ 
	lvItem.pszText = "no";

	if (entry.type & ENTRY::TYPE_NES)
	{
		if (ShowDatabase && entry.dBaseHandle)
		{
			if (nes.GetRomDatabase().HasTrainer( entry.dBaseHandle ))
				lvItem.pszText = "yes";
		}
		else if (entry.header.trainer)
		{
			lvItem.pszText = "yes";
		}
	}

	::SendMessage( hList, LVM_SETITEMTEXT, WPARAM(lvItem.iItem), PDX_CAST(LPARAM,&lvItem) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::ColumnAddMirroring(LVITEM& lvItem,const ENTRY& entry)  
{
	lvItem.pszText = "none";

	if (entry.type & (ENTRY::TYPE_NES|ENTRY::TYPE_UNF))
	{
		NES::MIRRORING mirroring;

		if (ShowDatabase && entry.dBaseHandle)
			mirroring = nes.GetRomDatabase().Mirroring( entry.dBaseHandle );
		else
			mirroring = NES::MIRRORING(entry.header.mirroring);

		switch (mirroring)
		{
      		case NES::MIRROR_VERTICAL:   lvItem.pszText = "vertical";    break;
       		case NES::MIRROR_FOURSCREEN: lvItem.pszText = "four-screen"; break;
     		case NES::MIRROR_ZERO:       lvItem.pszText = "zero";        break;
       		case NES::MIRROR_ONE:        lvItem.pszText = "one";         break;
			case NES::MIRROR_TWO:        lvItem.pszText = "two";         break;
			case NES::MIRROR_THREE:      lvItem.pszText = "three";       break;
			default:				     lvItem.pszText = "horizontal";  break;
		}
	}

	::SendMessage( hList, LVM_SETITEMTEXT, WPARAM(lvItem.iItem), PDX_CAST(LPARAM,&lvItem) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::ColumnAddName(LVITEM& lvItem,const ENTRY& entry)  
{ 
	if (entry.name.Length())
	{
		lvItem.pszText = const_cast<LPSTR>(entry.name.String());
	}
	else if (entry.dBaseHandle)
	{
		lvItem.pszText = const_cast<LPSTR>(nes.GetRomDatabase().Name( entry.dBaseHandle ));
	}

	::SendMessage( hList, LVM_SETITEMTEXT, WPARAM(lvItem.iItem), PDX_CAST(LPARAM,&lvItem) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::ColumnAddCopyright(LVITEM& lvItem,const ENTRY& entry)  
{ 
	if (entry.copyright.Length())
	{
		lvItem.pszText = const_cast<LPSTR>(entry.copyright.Begin());
	}
	else if (entry.dBaseHandle)
	{
		lvItem.pszText = const_cast<LPSTR>(nes.GetRomDatabase().Copyright( entry.dBaseHandle ));
	}

	::SendMessage( hList, LVM_SETITEMTEXT, WPARAM(lvItem.iItem), PDX_CAST(LPARAM,&lvItem) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::ColumnAddCondition(LVITEM& lvItem,const ENTRY& entry)
{
	if (entry.dBaseHandle)
		lvItem.pszText = (nes.GetRomDatabase().IsBad( entry.dBaseHandle ) ? "bad" : "good");

	::SendMessage( hList, LVM_SETITEMTEXT, WPARAM(lvItem.iItem), PDX_CAST(LPARAM,&lvItem) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::ColumnAddFolder(LVITEM& lvItem,const ENTRY& entry) 
{ 
	lvItem.pszText = const_cast<LPSTR>(entry.path.Begin());
	::SendMessage( hList, LVM_SETITEMTEXT, WPARAM(lvItem.iItem), PDX_CAST(LPARAM,&lvItem) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::UpdateEntries()
{
	const TSIZE NumEntries = FileSearcher.NumEntries();

	if (!NumEntries)
		return;

	ListView_SetItemCount( hList, NumEntries );

	const UINT NumColumns = ColumnEntries.Size();

	LVITEM lvItem;
	PDXMemZero( lvItem );

	for (TSIZE i=0; i < NumEntries; ++i)
	{
		const ENTRY& entry = FileSearcher.GetEntry(i);

		if (UpdateFlags & entry.type)
		{
			lvItem.mask = LVIF_PARAM;
			lvItem.lParam = PDX_CAST(LPARAM,&entry);
			lvItem.iSubItem = 0;
			lvItem.iItem = INT_MAX;

			if ((lvItem.iItem = ListView_InsertItem( hList, &lvItem )) == -1)
				break;

			lvItem.mask = LVIF_TEXT;

			for (UINT j=0; j < NumColumns; ++j)
			{
				lvItem.iSubItem = j;
				lvItem.pszText = "";
				(*this.*ColumnEntries[j].Add)( lvItem, entry );
			}
		}
	}

	ListView_SortItems( hList, StaticEntryCompare, PDX_CAST(LPARAM,this) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::Close()
{
	ItemSearcher.Close();

	if (hDlg)
	{
		::DestroyWindow( hDlg );
		hDlg = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL LAUNCHER::DialogProc(HWND h,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
     	case WM_INITDIALOG:

			hDlg = h;
			return TRUE;

		case WM_NOTIFY:        
			
			return OnNotify( wParam, lParam );

		case WM_CONTEXTMENU:
			
			return OnContextMenu ( wParam, lParam );		
		
		case WM_MOVE:

			application.GetSoundManager().Clear();
			return FALSE;

		case WM_SIZE:          
			
			OnSize( wParam ); 
			return TRUE;

		case WM_ENTERMENULOOP: 
			
			OnEnterMenuLoop(); 
			return TRUE;

		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDM_LAUNCHER_ADD:               OnAddBrowse();      return TRUE;
				case IDM_LAUNCHER_REMOVE:            OnRemove();         return TRUE;
				case IDM_LAUNCHER_CLEAR_ALL:         OnClearAll();       return TRUE;
				case IDM_LAUNCHER_RUN:               OnRun();            return TRUE;
     			case IDM_LAUNCHER_REFRESH:           OnRefresh();        return TRUE;
				case IDM_LAUNCHER_DATABASECORRECTED: OnToggleDatabase(); return TRUE;

				case IDM_LAUNCHER_FIND:              
					
					ItemSearcher.Open( hDlg );
					return TRUE;

				case IDM_LAUNCHER_ALIGNCOLUMNS:      
				{
					BeginUpdate( FALSE );
					OnAlignColumns();   
					EndUpdate();
					return TRUE;
				}
					
     			case IDM_LAUNCHER_SHOWGRID: OnShowGrid(); return TRUE;
				case IDM_LAUNCHER_PATHS:    OnPaths();    return TRUE;
				case IDM_LAUNCHER_COLUMNS:  OnColumns();  return TRUE;
				case IDM_LAUNCHER_COLORS:   OnColors();   return TRUE;
			}
			return FALSE;

		case WM_DROPFILES:

			OnDropFiles( wParam );
			return TRUE;

		case WM_CLOSE:

			Close();
			return TRUE;
	}

	if (ItemSearcher.IsDlgMessage( uMsg ))
	{
		ItemSearcher.OnMsg( hList, lParam );
		return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::OnDropFiles(WPARAM wParam)
{
	PDX_ASSERT( wParam );

	application.GetSoundManager().Clear();

	{
		POINT pt;

		if (!DragQueryPoint( HDROP(wParam), &pt ) || !::ClientToScreen( hDlg, &pt ))
			return;

		RECT rc;

		if (!::GetWindowRect( hList, &rc ))
			return;

		if (pt.x <= rc.left || pt.x >= rc.right || pt.y <= rc.top || pt.y >= rc.bottom)
			return;
	}
	
	PDXSTRING filename;

	filename.Resize(MAX_PATH);
	filename.Front() = '\0';

	if (::DragQueryFile( HDROP(wParam), 0, filename.Begin(), MAX_PATH ))
	{
		filename.Validate();

		if (filename.Length())
			OnAdd( filename );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::OnAddBrowse()
{
	application.GetSoundManager().Clear();

	const CHAR* const path = 
	(
       	(UpdateFlags == ENTRY::TYPE_NSP) ? 
		application.GetFileManager().GetNspPath().String() :
	    application.GetFileManager().GetRomPath().String()
	);

	PDXSTRING filename;

	const BOOL success = UTILITIES::BrowseOpenFile
	(
	    filename,
		hDlg,
		UTILITIES::IdToString(IDS_FILE_ADD).String(),
		(
			"All supported files\0"
			"*.nes;*.unf;*.fds;*.nsf;*.nsp;*.zip\0"
			"iNes ROM Images (*.nes)\0"
			"*.nes\0"
			"UNIF ROM Images (*.unf)\0"
			"*.unf\0"
			"Famicom Disk System Images (*.fds)\0"
			"*.fds\0"
			"NES Sound Files (*.nsf)\0"
			"*.nsf\0"
			"Nestopia Script Files (*.nsp)\0"
			"*.nsp\0"
			"Zip Files (*.zip)\0"
			"*.zip\0"
			"All files (*.*)\0"
			"*.*\0"									   
		),
		path
	);

	if (success)
		OnAdd( filename );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::OnAdd(const PDXSTRING& filename)
{
	if (FileSearcher.AddEntry( filename ))
	{
		BeginUpdate( TRUE );
		ListView_DeleteAllItems( hList );	
		UpdateEntries();
		EndUpdate();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::OnRemove()
{
	INT iCount = ListView_GetSelectedCount( hList );

	if (iCount < 1)
		return;

	BeginUpdate( FALSE );

	LVITEM lvItem;
	PDXMemZero( lvItem );
	lvItem.mask = LVIF_PARAM;

	do
	{
		lvItem.iItem = ListView_GetNextItem( hList, -1, LVNI_SELECTED );

		if (ListView_GetItem( hList, &lvItem ))
		{
			ENTRY* const entry = PDX_CAST(ENTRY*,lvItem.lParam);
			PDX_ASSERT( entry );

			ListView_DeleteItem( hList, lvItem.iItem );
			FileSearcher.RemoveEntry( entry );
		}
	}
	while (--iCount);

	if (!ListView_GetItemCount( hList ))
	{
		FileSearcher.RemoveAllEntries();
	}
	else if ((lvItem.iItem = ListView_GetSelectionMark( hList )) != -1)
	{
		ListView_SetItemState( hList, lvItem.iItem, LVIS_SELECTED, LVIS_SELECTED );
	}

	EndUpdate();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::OnClearAll()
{
	BeginUpdate( FALSE );
	ListView_DeleteAllItems( hList );
	FileSearcher.RemoveAllEntries();
	EndUpdate();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::OnRun()
{
	application.GetSoundManager().Clear();

	INT iItem = GetListSelection();

	if (iItem == -1)
		return;
	
	LVITEM lvItem;
	PDXMemZero( lvItem );

	lvItem.mask = LVIF_PARAM;
	lvItem.iItem = iItem;

	if (!ListView_GetItem( hList, &lvItem ) || !lvItem.lParam)
		return;
		
	const ENTRY& entry = *PDX_CAST(ENTRY*,lvItem.lParam);
	const TSIZE length = entry.file.Length() + entry.path.Length();

	if (!length)
		return;
	
	COPYDATASTRUCT cds;

	PDXSTRING filename;
	PDXPAIR<PDXSTRING,PDXSTRING> pair;

	if (entry.type & ENTRY::TYPE_ZIP)
	{
		PDXSTRING& first = pair.First();
		PDXSTRING& second = pair.Second();

		first.Reserve( length );
		first << entry.path << entry.file;
					
		const CHAR* const end = first.AtFirstOf( "<" ) + 1;
		PDX_ASSERT(end < first.End());

		second << end;
		PDX_ASSERT(second.Back() == '>');

		second.EraseBack();
		first.Resize( (end - 1) - first.Begin() );
		PDX_ASSERT(first.Length());

		cds.dwData = NST_WM_LAUNCHER_ZIPPED_FILE;
		cds.lpData = PDX_CAST(PVOID,&pair);
		cds.cbData = sizeof(pair);
	}
	else
	{
		filename.Reserve( length );
		filename << entry.path << entry.file;

		cds.dwData = NST_WM_LAUNCHER_FILE;
		cds.lpData = PDX_CAST(PVOID,filename.Begin());
		cds.cbData = filename.End() + 1 - filename.Begin();
	}

	::SendMessage( hWnd, WM_COPYDATA, WPARAM(hWnd), LPARAM(LPVOID(&cds)) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::OnEnterMenuLoop()
{
	application.GetSoundManager().Clear();

	UINT state;

	state = (GetListSelection() == -1 ? MF_GRAYED : MF_ENABLED);
	::EnableMenuItem( hMenu, IDM_LAUNCHER_RUN, state );
	::EnableMenuItem( hMenu, IDM_LAUNCHER_REMOVE, state );

	state = (ListView_GetItemCount( hList ) ? MF_ENABLED : MF_GRAYED);
	::EnableMenuItem( hMenu, IDM_LAUNCHER_CLEAR_ALL, state );
	::EnableMenuItem( hMenu, IDM_LAUNCHER_FIND, state );

	state = ((ListView_GetExtendedListViewStyle( hList ) & LVS_EX_GRIDLINES) ? MF_CHECKED : MF_UNCHECKED);
	::CheckMenuItem( hMenu, IDM_LAUNCHER_SHOWGRID, state );

	state = (ShowDatabase ? MF_CHECKED : MF_UNCHECKED);
	::CheckMenuItem( hMenu, IDM_LAUNCHER_DATABASECORRECTED, state );

	state = (paths.Folders().Size() ? MF_ENABLED : MF_GRAYED);
	::EnableMenuItem( hMenu, IDM_LAUNCHER_REFRESH, state );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL LAUNCHER::OnContextMenu(WPARAM wParam,LPARAM lParam)
{
	if (wParam != WPARAM(hList))
		return FALSE;
	
	LVHITTESTINFO lvHitTestInfo;

	lvHitTestInfo.pt.x = LOWORD(lParam);
	lvHitTestInfo.pt.y = HIWORD(lParam);
	
	::ScreenToClient( hList, &lvHitTestInfo.pt );

	HMENU hMenu = ::LoadMenu( UTILITIES::GetInstance(), MAKEINTRESOURCE(IDR_LAUNCHER_POPUP_MENU) );

 	if (ListView_HitTest( hList, &lvHitTestInfo ) == -1)
	{
		::EnableMenuItem( hMenu, IDM_LAUNCHER_RUN, MF_GRAYED );
		::EnableMenuItem( hMenu, IDM_LAUNCHER_REMOVE, MF_GRAYED );
	}

	if (ListView_GetExtendedListViewStyle( hList ) & LVS_EX_GRIDLINES)
		::CheckMenuItem( hMenu, IDM_LAUNCHER_SHOWGRID, MF_CHECKED );

	if (paths.Folders().IsEmpty())
		::EnableMenuItem( hMenu, IDM_LAUNCHER_REFRESH, MF_GRAYED );

	if (!ListView_GetItemCount( hList ))
		::EnableMenuItem( hMenu, IDM_LAUNCHER_FIND, MF_GRAYED );

	::TrackPopupMenu
	( 
	 	::GetSubMenu( hMenu, 0 ), 
		TPM_LEFTALIGN | TPM_TOPALIGN, 
		LOWORD(lParam), 
		HIWORD(lParam), 
		0, 
		hDlg, 
		NULL 
	);

	::DestroyMenu( hMenu );
	
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::OnToggleDatabase()
{
	ShowDatabase = !ShowDatabase;

	const INT NumEntries = ListView_GetItemCount( hList );

	if (NumEntries < 1)
		return;

	BeginUpdate( TRUE );

	PDXARRAY<UINT> ChangeColumns;
	ChangeColumns.Reserve( ColumnEntries.Size() );

	for (UINT i=0; i < ColumnEntries.Size(); ++i)
	{
		ENTRY_ADD type = ColumnEntries[i].Add;

		const BOOL NeedChange = 
		(
	     	type != ColumnAddFile      &&
			type != ColumnAddName      &&
			type != ColumnAddCopyright &&
			type != ColumnAddFolder
		);

		if (NeedChange)
			ChangeColumns.InsertBack( i );
	}

	const UINT NumChangeColumns = ChangeColumns.Size();

	if (NumChangeColumns)
	{
		LVITEM lvItem;
		PDXMemZero( lvItem );

		for (UINT i=0; i < NumEntries; ++i)
		{
			lvItem.mask = LVIF_PARAM;
			lvItem.iItem = i;

			if (ListView_GetItem( hList, &lvItem ) && lvItem.lParam)
			{
				const ENTRY& entry = *PDX_CAST(ENTRY*,lvItem.lParam);

				lvItem.mask = LVIF_TEXT;
				lvItem.lParam = 0;

				for (UINT j=0; j < NumChangeColumns; ++j)
				{
					lvItem.iSubItem = ChangeColumns[j];
					lvItem.pszText = "";
					(*this.*ColumnEntries[ChangeColumns[j]].Add)( lvItem, entry );
				}
			}
		}

		OnSort( ColumnPriority, TRUE );
	}

	EndUpdate();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL LAUNCHER::OnNotify(WPARAM wParam,LPARAM lParam)
{
	PDX_ASSERT( lParam );

	switch (wParam)
	{
     	case IDC_LAUNCHER_LIST:
		
			switch (LPNMLISTVIEW(lParam)->hdr.code)
			{
		     	case LVN_COLUMNCLICK:  OnColumnClick( lParam ); return TRUE;
				case LVN_ITEMACTIVATE: OnRun();                 return TRUE;
				case LVN_KEYDOWN:      OnKeyDown( lParam );		return TRUE;
				case LVN_ITEMCHANGED:  OnItemChanged( lParam ); return TRUE;
			}
			break;

		case IDC_LAUNCHER_TREE:
		
			switch (LPNMTREEVIEW(lParam)->hdr.code)
			{
     			case TVN_SELCHANGING: 
					
					OnSelectionChange( lParam );
					return TRUE;
			}
			break;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::OnItemChanged(LPARAM lParam)
{
	PDX_ASSERT( lParam );

	NMLISTVIEW& nmlv = *PDX_CAST(LPNMLISTVIEW,lParam);

	if (!(nmlv.uOldState & LVIS_SELECTED) && (nmlv.uNewState & LVIS_SELECTED))
	{
		LVITEM lvItem;
		PDXMemZero( lvItem );

		lvItem.mask = LVIF_PARAM;
		lvItem.iItem = nmlv.iItem;

		if (ListView_GetItem( hList, &lvItem ) && lvItem.lParam)
		{
			const ENTRY& entry = *PDX_CAST(const ENTRY*,lvItem.lParam);

			PDXSTRING filepath;
			filepath.Reserve( entry.path.Length() + entry.file.Length() );

			filepath << entry.path;
			filepath << entry.file;

			::SendMessage( hStatusBar, SB_SETTEXT, WPARAM(SB_SIMPLEID), LPARAM(filepath.Begin()) );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

INT LAUNCHER::GetListSelection() const
{
	INT iItem = ListView_GetSelectionMark( hList );

	if (iItem != -1 && !ListView_GetItemState( hList, iItem, LVIS_SELECTED ))
		iItem = -1;

	return iItem;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::OnColumnClick(LPARAM lParam)
{
	BeginUpdate( FALSE );
	OnSort( LPNMLISTVIEW(lParam)->iSubItem );
	EndUpdate();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::OnKeyDown(LPARAM lParam)
{
	switch (LPNMLVKEYDOWN(lParam)->wVKey)
	{
		case VK_INSERT: OnAddBrowse(); break;
		case VK_DELETE: OnRemove();    break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::OnSelectionChange(LPARAM lParam)
{
	BeginUpdate( TRUE );

	LPNMTREEVIEW pTree = LPNMTREEVIEW(lParam);

	if      (pTree->itemNew.hItem == hTreeItems[TREE_ALL]) UpdateFlags = ENTRY::TYPE_ANY;
	else if (pTree->itemNew.hItem == hTreeItems[TREE_NES]) UpdateFlags = ENTRY::TYPE_NES;
	else if (pTree->itemNew.hItem == hTreeItems[TREE_UNF]) UpdateFlags = ENTRY::TYPE_UNF;
	else if (pTree->itemNew.hItem == hTreeItems[TREE_FDS]) UpdateFlags = ENTRY::TYPE_FDS;
	else if (pTree->itemNew.hItem == hTreeItems[TREE_NSF]) UpdateFlags = ENTRY::TYPE_NSF;
	else                                                   UpdateFlags = ENTRY::TYPE_NSP;

	ListView_DeleteAllItems( hList );
	UpdateEntries();

	EndUpdate();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::OnSort(const INT p,const BOOL force)
{
	if (p < 0 || (!force && ColumnPriority == p))
		return;

	ColumnPriority = p;

	if (ListView_GetItemCount( hList ) < 1)
		return;

	const COLUMNENTRIES tmp( ColumnEntries );
	const COLUMNENTRY first( ColumnEntries[p] );

	ColumnEntries.Erase( ColumnEntries.At(p) );
	ColumnEntries.Insert( ColumnEntries.Begin(), first );

	ListView_SortItems( hList, StaticEntryCompare, PDX_CAST(LPARAM,this) );

	ColumnEntries = tmp;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::OnShowGrid()
{
	DWORD style = ListView_GetExtendedListViewStyle( hList );

	if (style & LVS_EX_GRIDLINES)
		style &= ~LVS_EX_GRIDLINES;
	else
		style |= LVS_EX_GRIDLINES;

	ListView_SetExtendedListViewStyle( hList, style );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::OnColumns()
{
	const LAUNCHERCOLUMNS::TYPES types( columns.SelectedTypes() );

	columns.StartDialog( hDlg );

	if (!(types == columns.SelectedTypes()))
	{
		BeginUpdate( TRUE );
		UpdateColumns();
		OnAlignColumns();
		UpdateEntries();
		EndUpdate();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::OnPaths()
{
	paths.StartDialog( hDlg );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::OnSize(WPARAM wParam)
{
	application.GetSoundManager().Clear();

	{
		RECT rcWindow;
		::GetWindowRect( hDlg, &rcWindow );

		{
			RECT rcList;
			::GetWindowRect( hList, &rcList );

			const INT width = (rcWindow.right - rcList.left) - ListWidthMargin;
			const INT height = (rcWindow.bottom - rcList.top) - ListHeightMargin;

			::SetWindowPos( hList, NULL, 0, 0, width, height, SWP_NOMOVE|SWP_NOZORDER );
		}

		{
			RECT rcTree;
			::GetWindowRect( hTree, &rcTree );

			const INT width = rcTree.right - rcTree.left;
			const INT height = (rcWindow.bottom - rcTree.top) - ListHeightMargin;

			::SetWindowPos( hTree, NULL, 0, 0, width, height, SWP_NOMOVE|SWP_NOZORDER );
		}
	}

	::SendMessage( hStatusBar, WM_SIZE, 0, 0 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::OnColors()
{
	colors.StartDialog( hDlg );

	ListView_SetBkColor     ( hList, colors.GetBgColor() );
	ListView_SetTextBkColor ( hList, colors.GetBgColor() );
	ListView_SetTextColor   ( hList, colors.GetFgColor() );
	TreeView_SetBkColor     ( hTree, colors.GetBgColor() );
	TreeView_SetTextColor   ( hTree, colors.GetFgColor() );

	::InvalidateRect( hTree, NULL, TRUE );
	::InvalidateRect( hList, NULL, TRUE );
	
	::UpdateWindow( hTree );
	::UpdateWindow( hList );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHER::OnRefresh()
{
	application.GetSoundManager().Clear();

	const LAUNCHERPATHS::FOLDERS& folders = paths.Folders();

	if (folders.IsEmpty())
		return;

	{
		UINT flags = 0;

		if ( paths.IncludeNes()   ) flags |= LAUNCHERFILESEARCH::READ_NES;
		if ( paths.IncludeUnf()	  ) flags |= LAUNCHERFILESEARCH::READ_UNF;
		if ( paths.IncludeFds()	  ) flags |= LAUNCHERFILESEARCH::READ_FDS;
		if ( paths.IncludeNsf()	  ) flags |= LAUNCHERFILESEARCH::READ_NSF;
		if ( paths.IncludeNsp()	  ) flags |= LAUNCHERFILESEARCH::READ_NSP;
		if ( paths.IncludeZip()	  ) flags |= LAUNCHERFILESEARCH::READ_ZIP;
		if ( paths.SearchAll()	  ) flags |= LAUNCHERFILESEARCH::READ_ANY;
		if ( paths.NoDublicates() ) flags |= LAUNCHERFILESEARCH::PATH_NO_DUBLICATES;

		FileSearcher.SetFlags( flags );
	}

	{
		LAUNCHERFILESEARCH::PATHS SearchPaths;
		SearchPaths.Resize( folders.Size() );

		for (TSIZE i=0; i < folders.Size(); ++i)
		{
			SearchPaths[i].First() = folders[i].folder;
			SearchPaths[i].Second() = folders[i].IncSubFolders;
		}

		FileSearcher.Refresh( SearchPaths, hDlg );
	}

	BeginUpdate( TRUE );
	ListView_DeleteAllItems( hList );	
	UpdateEntries();	
	EndUpdate();
}
