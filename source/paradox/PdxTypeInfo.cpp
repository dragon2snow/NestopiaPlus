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

#include "PdxLibrary.h"

#define PDX_SET_INFO_(type,min_,max_)            \
									             \
const type PDXTYPE<type>::MIN          = min_; 	 \
const type PDXTYPE<type>::MAX          = max_;	 \
const type PDXTYPE<const type>::MIN    = min_;	 \
const type PDXTYPE<const type>::MAX    = max_;	 \
const CHAR PDXTYPE<type>::NAME[]       = #type;  \
const CHAR PDXTYPE<const type>::NAME[] = #type; 

PDX_SET_INFO_( CHAR,       CHAR_MIN,       CHAR_MAX       )
PDX_SET_INFO_( UCHAR,      UCHAR_MIN,      UCHAR_MAX      )
PDX_SET_INFO_( SHORT,      SHORT_MIN,      SHORT_MAX      )
PDX_SET_INFO_( USHORT,     USHORT_MIN,     USHORT_MAX     )
PDX_SET_INFO_( LONG,       LONG_MIN,       LONG_MAX       )
PDX_SET_INFO_( ULONG,      ULONG_MIN,      ULONG_MAX      )
PDX_SET_INFO_( INT,        INT_MIN,        INT_MAX        )
PDX_SET_INFO_( UINT,       UINT_MIN,       UINT_MAX       )
PDX_SET_INFO_( FLOAT,      FLOAT_MIN,      FLOAT_MAX      )
PDX_SET_INFO_( DOUBLE,     DOUBLE_MIN,     DOUBLE_MAX     )
PDX_SET_INFO_( LONGDOUBLE, LONGDOUBLE_MIN, LONGDOUBLE_MAX )

#ifdef PDX_I64_SUPPORT

 PDX_SET_INFO_( I64, I64_MIN, I64_MAX )
 PDX_SET_INFO_( U64, U64_MIN, U64_MAX )

#endif

#undef PDX_SET_INFO_
