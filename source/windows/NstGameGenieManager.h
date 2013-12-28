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

#ifndef NST_GAMEGENIEMANAGER_H
#define NST_GAMEGENIEMANAGER_H

#include "../paradox/PdxSet.h"
#include "NstManager.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class GAMEGENIEMANAGER : public MANAGER
{
public:

	GAMEGENIEMANAGER();

	VOID Create  (CONFIGFILE* const);
	VOID Destroy (CONFIGFILE* const);

	PDXRESULT GetCode(const TSIZE,PDXSTRING&,BOOL&,PDXSTRING* const) const;
	PDXRESULT AddCode(const PDXSTRING&,const BOOL,const PDXSTRING* const);
	PDXRESULT ClearCodes(const BOOL);

	inline TSIZE NumCodes() const
	{ return codes.Size(); }

private:

	struct CODE
	{
		CODE(const ULONG p=0,const BOOL e=FALSE,const PDXSTRING* const c=NULL)
		: packed(p), enabled(e) { if (c && c->Length()) comment = *c; }

		inline BOOL operator == (const CODE& a) const { return packed == a.packed; }
		inline BOOL operator <  (const CODE& a) const { return packed <  a.packed; }
		inline BOOL operator == (const ULONG a) const { return packed == a;        }
		inline BOOL operator <  (const ULONG a) const { return packed <  a;        }

		inline friend BOOL operator == (const ULONG a,const CODE& b) { return a == b.packed; }
		inline friend BOOL operator <  (const ULONG a,const CODE& b) { return a <  b.packed; }

		ULONG packed;
		BOOL enabled;
		PDXSTRING comment;
	};

	typedef PDXSET<CODE> CODES;

	VOID InitDialog();
	VOID CloseDialog();
	BOOL AddToList(CODE&);
	VOID RemoveCode();
	VOID ClearCodes();
	VOID SetCodeStates(const BOOL);
	VOID ImportCodes();
	VOID ExportCodes();
	VOID CreateCodeDialog();
	VOID SubmitCode(HWND);
	VOID ValidateCode(HWND) const;

	BOOL DialogProc(HWND,UINT,WPARAM,LPARAM);

	static BOOL CALLBACK StaticCodeDialogProc(HWND,UINT,WPARAM,LPARAM);

	NES::IO::GAMEGENIE GameGenie;

	HWND hDlg;
	HWND hList;
	CODES codes;

	class GAMEGENIECLOSE : public MANAGER
	{
	public:

		GAMEGENIECLOSE(GAMEGENIEMANAGER& m)
		: MANAGER(IDD_GAMEGENIECLOSE), ggm(m) {}

	private:

		BOOL DialogProc(HWND,UINT,WPARAM,LPARAM);

		GAMEGENIEMANAGER& ggm;
	};

	friend class GAMEGENIECLOSE;
};

#endif
