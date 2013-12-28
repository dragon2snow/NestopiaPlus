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

#ifndef PDXLIBRARY_H
#define PDXLIBRARY_H

#ifndef __cplusplus
#pragma message("Sorry, C++ only!")
#endif

#include <climits>
#include <cfloat>
#include <cstring>

#if defined(__INTEL_COMPILER) || defined(_MSC_VER)

 #undef  PDX_PRAGMA_ONCE_SUPPORT
 #define PDX_PRAGMA_ONCE_SUPPORT
 #undef  PDX_PRAGMA_PACK_SUPPORT
 #define PDX_PRAGMA_PACK_SUPPORT

 #pragma intrinsic(abs)
 #pragma intrinsic(labs)
 #pragma intrinsic(fabs)
 #pragma intrinsic(memcmp)
 #pragma intrinsic(memcpy)
 #pragma intrinsic(memset)
 #pragma intrinsic(strcmp)
 #pragma intrinsic(strcpy)
 #pragma intrinsic(strlen)
 #pragma intrinsic(strcat)

#endif

#if defined(_X86_) || defined(_M_IX86)
#undef  PDX_X86
#define PDX_X86
#endif

#include "PdxTypes.h"
#include "PdxBinary.h"
#include "PdxAssert.h"
#include "PdxTypeInfo.h"

#endif

