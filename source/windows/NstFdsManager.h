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

#ifndef NST_FDSMANAGER_H
#define NST_FDSMANAGER_H

#include "../paradox/PdxFile.h"
#include "NstManager.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class FDSMANAGER : public MANAGER
{
public:

	FDSMANAGER(const INT id,const UINT b) 
	: MANAGER(id,b) {}

	~FDSMANAGER() {}

private:

	PDXRESULT Create  (PDXFILE* const);
	PDXRESULT Destroy (PDXFILE* const);

	BOOL DialogProc(HWND,UINT,WPARAM,LPARAM);

	VOID Reset();
	VOID UpdateSettings (HWND);
	VOID UpdateDialog   (HWND);
	VOID OnBrowse       (HWND);
	VOID OnClear        (HWND);
	
	VOID SubmitBios(const BOOL=FALSE);

	BOOL WriteProtect;
	PDXSTRING bios;
};

#endif