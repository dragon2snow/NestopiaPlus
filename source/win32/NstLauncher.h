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

#pragma once

#ifndef NST_LAUNCHER_H
#define NST_LAUNCHER_H

#include "NstLauncherColumns.h"
#include "NstLauncherPaths.h"
#include "NstLauncherFileSearch.h"
#include "NstLauncherItemSearch.h"
#include "NstLauncherColors.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class LAUNCHER : private MANAGER
{
public:

	LAUNCHER();
	~LAUNCHER();

	VOID Create  (CONFIGFILE* const);
	VOID Destroy (CONFIGFILE* const);
	VOID Open    ();
	VOID Close   ();

	inline BOOL IsEnabled() const
	{ return hDlg != NULL; }

	inline HWND GetHWnd()
	{ return hDlg; }

	VOID Activate();
	VOID Inactivate();

	BOOL IsDlgMessage(MSG&);

private:

	typedef LAUNCHERFILESEARCH::ENTRY ENTRY;

	static INT CALLBACK StaticEntryCompare(LPARAM,LPARAM,LPARAM);

	typedef VOID (LAUNCHER::*ENTRY_ADD)(LVITEM&,const ENTRY&);
	typedef INT (LAUNCHER::*ENTRY_COMPARE)(const ENTRY&,const ENTRY&) const;

	struct COLUMNENTRY
	{
		inline BOOL operator == (const COLUMNENTRY& ce) const
		{ return Add == ce.Add && Compare == ce.Compare; }

		ENTRY_ADD Add;
		ENTRY_COMPARE Compare;
	};

	typedef PDXARRAY<COLUMNENTRY> COLUMNENTRIES;

	INT ColumnSortFile      (const ENTRY&,const ENTRY&) const;
	INT ColumnSortSystem    (const ENTRY&,const ENTRY&) const;
	INT ColumnSortMapper    (const ENTRY&,const ENTRY&) const;
	INT ColumnSortPRom      (const ENTRY&,const ENTRY&) const;
	INT ColumnSortCRom      (const ENTRY&,const ENTRY&) const;
	INT ColumnSortWRam      (const ENTRY&,const ENTRY&) const;
	INT ColumnSortBattery   (const ENTRY&,const ENTRY&) const;
	INT ColumnSortTrainer   (const ENTRY&,const ENTRY&) const;
	INT ColumnSortMirroring (const ENTRY&,const ENTRY&) const;
	INT ColumnSortName      (const ENTRY&,const ENTRY&) const;
	INT ColumnSortCopyright (const ENTRY&,const ENTRY&) const;
	INT ColumnSortCondition (const ENTRY&,const ENTRY&) const;
	INT ColumnSortFolder    (const ENTRY&,const ENTRY&) const;

	VOID ColumnAddFile      (LVITEM&,const ENTRY&);
	VOID ColumnAddSystem    (LVITEM&,const ENTRY&);
	VOID ColumnAddMapper    (LVITEM&,const ENTRY&);
	VOID ColumnAddPRom      (LVITEM&,const ENTRY&);
	VOID ColumnAddCRom      (LVITEM&,const ENTRY&);
	VOID ColumnAddWRam      (LVITEM&,const ENTRY&);
	VOID ColumnAddBattery   (LVITEM&,const ENTRY&);
	VOID ColumnAddTrainer   (LVITEM&,const ENTRY&);
	VOID ColumnAddMirroring (LVITEM&,const ENTRY&);
	VOID ColumnAddName      (LVITEM&,const ENTRY&);
	VOID ColumnAddCopyright (LVITEM&,const ENTRY&);
	VOID ColumnAddCondition (LVITEM&,const ENTRY&);
	VOID ColumnAddFolder    (LVITEM&,const ENTRY&);

	VOID UpdateImageList();

	VOID BeginUpdate(const BOOL);
	VOID EndUpdate();

	INT GetListSelection() const;

	BOOL DialogProc(HWND,UINT,WPARAM,LPARAM);
	
	VOID UpdateColumns();
	VOID UpdateEntries();

	VOID OnRun();
	VOID OnDropFiles(WPARAM);
	VOID OnAddBrowse();
	VOID OnAdd(const PDXSTRING&);
	VOID OnRemove();
	VOID OnClearAll();
	VOID OnSize(WPARAM);
	BOOL OnNotify(WPARAM,LPARAM);
	BOOL OnContextMenu(WPARAM,LPARAM);
	VOID OnItemChanged(LPARAM);
	VOID OnColumnClick(LPARAM);
	VOID OnKeyDown(LPARAM);
	VOID OnColors();
	VOID OnSelectionChange(LPARAM);
	VOID OnEnterMenuLoop();
	VOID OnSort(const INT,const BOOL=FALSE);
	VOID OnShowGrid();
	VOID OnToggleDatabase();
	VOID OnPaths();
	VOID OnColumns();
	VOID OnAlignColumns();
	VOID OnRefresh();

	enum
	{
		TREE_ALL,
		TREE_NES,
		TREE_UNF,
		TREE_FDS,
		TREE_NSF,
		TREE_NSP,
		TREE_NUM
	};

	HWND hDlg;
	HWND hList;
	HWND hTree;
	HWND hStatusBar;
	HMENU hMenu;
	HIMAGELIST hImageList;
	HTREEITEM hTreeItems[TREE_NUM];

	UINT ListWidthMargin;
	UINT ListHeightMargin;
	BOOL ShowDatabase;
	UINT ColumnPriority;
	UINT UpdateFlags;

	HCURSOR hTmpCursor;

	LAUNCHERCOLUMNS columns;
	LAUNCHERPATHS paths;
	LAUNCHERFILESEARCH FileSearcher;
	LAUNCHERITEMSEARCH ItemSearcher;
	COLUMNENTRIES ColumnEntries;
	LAUNCHERCOLORS colors;

	enum {STATUSBAR_ID=1001};
	enum {IMAGE_OPEN,IMAGE_CLOSED};

	INT ImageIds[2];
};

#endif
