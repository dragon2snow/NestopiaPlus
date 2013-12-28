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

#pragma once

#ifndef NST_MANAGER_H
#define NST_MANAGER_H

#include "resource/resource.h"
#include "../NstNes.h"
#include "NstUI.h"
#include "NstException.h"
#include "NstUtilities.h"
#include "NstConfigFile.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class MANAGER
{
public:

	enum {NO_FILE=0};
	enum {NO_DIALOG=INT_MAX};

	virtual VOID Create  (CONFIGFILE* const) {}
	virtual VOID Destroy (CONFIGFILE* const) {}

	INT_PTR StartDialog(HWND=NULL);

	static TSIZE GetDlgItemText(HWND,const INT,PDXSTRING&,const INT=MAX_PATH);

protected:

	MANAGER(const INT=NO_DIALOG);
	virtual ~MANAGER() {}

	VOID StartModelessDialog(HWND=NULL);

	HWND const hWnd;
	NES::MACHINE& nes;

private:

	virtual BOOL DialogProc(HWND,UINT,WPARAM,LPARAM) 
	{ return TRUE; }

	static BOOL CALLBACK StaticDialogProc(HWND,UINT,WPARAM,LPARAM);
	static BOOL CALLBACK StaticModelessDialogProc(HWND,UINT,WPARAM,LPARAM);

	const INT DialogID;
};

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class FILEEXISTDIALOG : public MANAGER
{
public:

	FILEEXISTDIALOG()				  
	: MANAGER(IDD_FILE_EXIST), choice(ULONG_MAX) {}

	inline ULONG Choice() const
	{ return choice; }

private:

	BOOL DialogProc(HWND,UINT,WPARAM,LPARAM);

	ULONG choice;
};

#include "NstLogFileManager.h"

#endif