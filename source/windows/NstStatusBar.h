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

#ifndef NST_STATUSBAR_H
#define NST_STATUSBAR_H

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class STATUSBAR
{
public:

	STATUSBAR();

	VOID Create(HINSTANCE,HWND);
	VOID Destroy();
	VOID Resize();
	UINT GetHeight() const;
	VOID DisplayMsg(const CHAR*);
	VOID DisplayFPS(const BOOL,DOUBLE=0.0);

	inline BOOL IsEnabled() const
	{ return hWnd != NULL; }

	inline HWND GetHWnd()
	{ return hWnd; }

	enum {CHILD_ID=1000};

private:

	enum {FPS_OFFSET=5};

	VOID UpdateParts();

	HWND hParent;
	HWND hWnd;

	struct WIDTHS
	{
		WIDTHS()
		: 
		character (7),
		fps       (7*10)
		{}

		LONG character;
		LONG fps;
	};

	WIDTHS widths;
	CHAR sFPS[16];
};

#endif
