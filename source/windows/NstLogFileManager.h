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

#ifndef NST_LOGFILEMANAGER_H
#define NST_LOGFILEMANAGER_H

#include "NstManager.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class LOGFILEMANAGER : public MANAGER
{
public:

	LOGFILEMANAGER()
	: MANAGER(IDD_LOGFILE) {}

	VOID Close(const BOOL);

	static VOID LogSeparator();

	template<class T> 
	static PDX_NO_INLINE VOID Output(const T& m1)
	{
		LogString << m1;
		LogString << "\r\n";

		if (LogString.Length() >= 0x100000UL)
			LogString.Clear();
	}

	template<class T,class U> 
	static PDX_NO_INLINE VOID Output(const T& m1,const U& m2)
	{
		LogString << m1;
		LogString << m2;
		LogString << "\r\n";

		if (LogString.Length() >= 0x100000UL)
			LogString.Clear();
	}

	template<class T,class U,class V> 
	static PDX_NO_INLINE VOID Output(const T& m1,const U& m2,const V& m3)
	{
		LogString << m1;
		LogString << m2;
		LogString << m3;
		LogString << "\r\n";

		if (LogString.Length() >= 0x100000UL)
			LogString.Clear();
	}

	template<class T,class U,class V,class W> 
	static PDX_NO_INLINE VOID Output(const T& m1,const U& m2,const V& m3,const W& m4)
	{
		LogString << m1;
		LogString << m2;
		LogString << m3;
		LogString << m4;
		LogString << "\r\n";

		if (LogString.Length() >= 0x100000UL)
			LogString.Clear();
	}

private:

	BOOL DialogProc(HWND,UINT,WPARAM,LPARAM);

	static PDXSTRING LogString;
};

#endif
