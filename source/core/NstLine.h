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

#ifndef NST_LINE_H
#define NST_LINE_H

#include "../paradox/PdxLibrary.h"	

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_NAMESPACE_BEGIN

class LINE
{
public:

	LINE()
	: 
	pin   (NULL),
	lo    (NULL),
	hi    (NULL),
	reset (NULL)
	{}

	struct PIN {};
	typedef VOID (PIN::*SIGNAL)();

	inline VOID SignalLo()
	{ (*pin.*lo)(); }

	inline VOID SignalHi()
	{ (*pin.*hi)(); }

	inline VOID Reset()
	{ (*pin.*reset)(); }

	template<class A,class B,class C,class D>
	VOID Connect(A* a,B b,C c,D d)
	{
		PDX_ASSERT( a && b && c && d );

		pin   = PDX_CAST_REF( PIN*,a   );
		lo    = PDX_CAST_REF( SIGNAL,b ); 
		hi    = PDX_CAST_REF( SIGNAL,c );
		reset = PDX_CAST_REF( SIGNAL,d );
	}

	VOID Unconnect()
	{
		pin   = NULL;
		lo    = NULL;
		hi    = NULL;
		reset = NULL;
	}

	inline BOOL IsConnected() const
	{ return pin != NULL; }

private:

	PIN* pin;

	SIGNAL lo;
	SIGNAL hi;
	SIGNAL reset;
};

NES_NAMESPACE_END

#endif

