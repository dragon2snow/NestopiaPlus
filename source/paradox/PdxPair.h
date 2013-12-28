//////////////////////////////////////////////////////////////////////////////////////////////
//
// Paradox Library - general purpose C++ utilities
//
// Copyright (C) 2003 Martin Freij
//
// This file is part of Paradox Library.
// 
// Paradox Library is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// Paradox Library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Paradox Library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef PDX_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#ifndef PDXPAIR_H
#define PDXPAIR_H

#include "PdxLibrary.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// Class for storing pair of objects
//////////////////////////////////////////////////////////////////////////////////////////////

template<class T,class U> class PDXPAIR
{
public:

	PDX_DEFAULT_CONSTRUCTOR(PDXPAIR)

	typedef T FIRST;
	typedef U SECOND;

	inline PDXPAIR(const T& t,const U& u)
	: first(t), second(u) {}

	template<class X,class Y> 
	inline PDXPAIR(const PDXPAIR<X,Y>& p)
	: first(p.First()), second(p.Second()) {}
  
	inline const VOID* operator [] (const UINT idx) const
	{
		PDX_ASSERT(idx < 2);
		return PDX_CAST(const CHAR*,&first)+(idx*sizeof(T));
	}

	inline VOID* operator [] (const UINT idx)
	{
		PDX_ASSERT(idx < 2);
		return PDX_CAST(CHAR*,&first)+(idx*sizeof(T));
	}

	inline const T& First() const  { return first;  }
	inline T& First()              { return first;  }
	inline const U& Second() const { return second; }
	inline U& Second()             { return second; }

private:

	T first;
	U second;
};

#include "PdxPair.inl"

#endif
