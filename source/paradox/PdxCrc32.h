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

#ifndef PDXCRC32_H
#define PDXCRC32_H

#include "PdxLibrary.h"	

//////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////

class PDXCRC32
{
public:

	template<class T>
	static inline ULONG Compute(const T* const block,const TSIZE size)
	{ return ComputeCrc( PDX_CAST(const UCHAR*,block), size ); }

	template<class T>
	static inline ULONG Update(const T* const block,const TSIZE size,const ULONG crc)
	{ return UpdateCrc( PDX_CAST(const UCHAR*,block), size, crc ); }

private:

	static ULONG PDX_STDCALL ComputeCrc(const UCHAR* const,const TSIZE);
	static ULONG PDX_STDCALL UpdateCrc(const UCHAR* const,const TSIZE,ULONG);

	static const ULONG table[256];
};

#endif

