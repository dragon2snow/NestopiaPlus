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

#ifndef PDX_SINGLETON_H
#define PDX_SINGLETON_H

#include "PdxLibrary.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// Singleton class, designed for global access
//////////////////////////////////////////////////////////////////////////////////////////////

template<class T> class PDXSINGLETON
{
public:

	inline PDXSINGLETON()
	{
		PDX_ASSERT_MSG(!singleton,"Singleton has already been constructed!");
		singleton = static_cast<T*>(this);
	}

	inline ~PDXSINGLETON()
	{
		PDX_ASSERT_MSG(singleton,"Singleton was never constructed!");
		singleton = NULL;
	}

	static inline T& GetSingleton()
	{
		PDX_ASSERT_MSG(singleton,"Singleton has not been constructed!");
		return (*singleton);
	}

private:

	static T* singleton;
};

//////////////////////////////////////////////////////////////////////////////////////////////
// Null it
//////////////////////////////////////////////////////////////////////////////////////////////

template<class T> T* PDXSINGLETON<T>::singleton = NULL;

#endif
