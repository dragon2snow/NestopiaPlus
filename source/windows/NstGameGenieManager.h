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

	GAMEGENIEMANAGER(const INT id)
	: MANAGER(id) {}

	PDXRESULT ClearAllCodes();
	PDXRESULT AddCode(const PDXSTRING&,const BOOL=TRUE,const PDXSTRING* const=NULL);
	PDXRESULT GetCode(const UINT,PDXSTRING&,PDXSTRING* const) const;

	inline UINT NumCodes() const
	{ return codes.Size(); }

private:

	PDXRESULT Create(CONFIGFILE* const);

	struct CODE
	{
		inline BOOL operator == (const CODE& a) const { return code == a.code; }
		inline BOOL operator <  (const CODE& a) const { return code <  a.code; }
		inline BOOL operator == (const ULONG a) const { return code == a;      }
		inline BOOL operator <  (const ULONG a) const { return code <  a;      }

		inline friend BOOL operator == (const ULONG a,const CODE& b) { return a == b.code; }
		inline friend BOOL operator <  (const ULONG a,const CODE& b) { return a <  b.code; }

		ULONG code;
		PDXSTRING comment;
	};

	typedef PDXSET<CODE> CODES;

	VOID UpdateDialog();
	VOID CloseDialog();
	VOID UpdateColumns();
	BOOL AddCode(CODE&,const BOOL=FALSE);
	VOID UpdateCodes();
	VOID RemoveCode();
	VOID ClearCodes();
	VOID ImportCodes();
	VOID ExportCodes();
	VOID CreateCodeDialog();
	VOID SubmitCode(HWND);
	VOID ValidateCode(HWND) const;

	BOOL      NesIsEnabled (const ULONG) const;
	PDXRESULT NesEncode    (const ULONG,PDXSTRING&) const;
	PDXRESULT NesDecode    (const CHAR* const,ULONG&) const;
	PDXRESULT NesPack      (const UINT,const UINT,const UINT,const BOOL,ULONG&) const;
	PDXRESULT NesUnpack    (const ULONG,UINT&,UINT&,UINT&,BOOL&) const;
	PDXRESULT NesEnable    (const ULONG,const NES::IO::GAMEGENIE::STATE);
	PDXRESULT NesSet       (const ULONG,const NES::IO::GAMEGENIE::STATE);
	PDXRESULT NesDestroy   ();

	BOOL DialogProc(HWND,UINT,WPARAM,LPARAM);

	static BOOL CALLBACK StaticCodeDialogProc(HWND,UINT,WPARAM,LPARAM);

	HWND hDlg;
	HWND hList;
	CODES codes;
};

#endif
