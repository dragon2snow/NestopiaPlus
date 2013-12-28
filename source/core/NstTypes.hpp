////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2005 Martin Freij
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

#ifndef NST_TYPES_H
#define NST_TYPES_H

#include "api/NstApiCompiler.hpp"
#include <cstddef>
#include <climits>

#ifndef NST_X86
#if defined(_X86_) || defined(_M_IX86)
#define NST_X86
#endif
#endif

#ifdef _MSC_VER

 #pragma once

 #ifndef NST_PRAGMA_ONCE_SUPPORT
 #define NST_PRAGMA_ONCE_SUPPORT
 #endif

 #ifdef NDEBUG

  #define NST_PRAGMA_OPTIMIZE

  #pragma inline_depth( 255 )
  #pragma inline_recursion( on ) 

  #if _MSC_VER <= 1300
  #define NST_PRAGMA_OPTIMIZE_ALIAS
  #endif

  #ifndef NST_FORCE_INLINE
  #define NST_FORCE_INLINE __forceinline
  #endif
 
  #ifndef __INTEL_COMPILER
  #ifndef NST_TAILCALL_OPTIMIZE
  #define NST_TAILCALL_OPTIMIZE
  #endif
  #endif

 #endif

 #ifdef __INTEL_COMPILER 
 
  #ifndef NST_RESTRICT
  #define NST_RESTRICT restrict
  #endif
 
 #endif

 #if _MSC_VER >= 1300
						
  #ifndef NST_NO_INLINE
  #define NST_NO_INLINE __declspec(noinline)
  #endif

  #ifndef NST_ASSUME
  #define NST_ASSUME(x_) __assume(x_)
  #endif

  #define NST_UNREACHABLE default: __assume(0);
  #define NST_NO_VTABLE __declspec(novtable)

  #if _MSC_VER >= 1400
					  
   #ifndef NST_RESTRICT
   #define NST_RESTRICT __restrict
   #endif

  #endif

 #endif

#elif defined(__GNUC__)

 #ifndef NST_NO_INLINE
 #define NST_NO_INLINE __attribute__ ((noinline))
 #endif

 #ifndef NST_RESTRICT
 #define NST_RESTRICT __restrict__
 #endif

 #ifndef NST_TAILCALL_OPTIMIZE
 #define NST_TAILCALL_OPTIMIZE
 #endif

#elif defined(__MWERKS__)

 #pragma once

 #ifndef NST_PRAGMA_ONCE_SUPPORT
 #define NST_PRAGMA_ONCE_SUPPORT
 #endif

#endif

#ifndef NST_FORCE_INLINE
#define NST_FORCE_INLINE inline
#endif

#ifndef NST_NO_INLINE
#define NST_NO_INLINE
#endif

#ifndef NST_ASSUME
#define NST_ASSUME(x_) NST_NOP
#endif

#ifndef NST_NO_VTABLE
#define NST_NO_VTABLE
#endif

#ifndef NST_RESTRICT
#define NST_RESTRICT
#endif

#ifndef NST_CALL
#define NST_CALL
#endif
										   
#ifndef NST_UNREACHABLE
#define NST_UNREACHABLE
#endif

#ifndef NST_LINEBREAK
#ifdef _WIN32
#define NST_LINEBREAK "\r\n"
#else
#define NST_LINEBREAK "\n"
#endif
#endif

#if defined(NST_MIN) || defined(NST_MAX) || defined(NST_CLAMP)
#error Nestopia min/max/clamp macros collision!
#endif

#define NST_MIN(x_,y_) ((x_) < (y_) ? (x_) : (y_))
#define NST_MAX(x_,y_) ((x_) < (y_) ? (y_) : (x_))
#define NST_CLAMP(a_,b_,c_) NST_MAX(NST_MIN(a_,c_),b_)

#define NST_NOP ((void)0)

namespace Nes 
{
	typedef signed char schar;
	typedef unsigned char uchar;
	typedef unsigned short ushort;
	typedef unsigned int uint;
	typedef unsigned long ulong;
	typedef const char* cstring;
	typedef unsigned int ibool;

	template<typename T,size_t N> 
	char(& array_count(T(&)[N]))[N]; 
    #define NST_COUNT(array_) sizeof(Nes::array_count(array_))
}

#include "NstInteger.hpp"
#include "NstAssert.hpp"

namespace Nes
{
	typedef uint Data;
	typedef uint Address;
	typedef dword Cycle;
}

#endif
