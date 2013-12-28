////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2006 Martin Freij
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
#error Do not include NstApiCompiler.hpp directly!
#endif

////////////////////////////////////////////////////////////////////////////////////////
//
// #define NST_X86 - if targeting x86 processors. 
//                   (has no effect if _M_IX86 or _X86_ is already defined)
//
// #define NST_PRAGMA_ONCE_SUPPORT - for compilers supporting "pragma once".
//                                   (automatically defined for MSVC, ICC and MCW)
//
// #define NST_U64_DEFINED - if native support for 64bit integers and can be reached from ::u64 in global namespace.
//                           (has no effect with MSVC and ICC, or any other compiler that support the long long integer type)
//
// #define NST_CALL	- compiler specific calling convention for global functions
//                    placed between return type and name e.g void NST_CALLBACK Function();
//                    standard conventions on win32 are cdecl, fastcall and stdcall
//
// #define NST_LINEBREAK - string for line breaks. if not defined defaults to "\r\n" on win32 or else "\n"
//
// #define NST_CALLBACK - same as above but for callback functions only
//
// #define NST_NO_INLINE x - keyword to prevent a function from getting inlined.
//                           (automatically enabled for MSVC, ICC and GCC)
//
// #define NST_RESTRICT x - restrict keyword - for non-aliasing pointers.
//                          (automatically enabled for ICC and GCC)
//
// #define NST_ASSUME(x) y(x) - optimization hint for the compiler, informs it that the condition will evaluate to true
//								(automatically enabled for MSVC)
//
// #define NST_HALT x - win32 debug mode only. halts program execution. 
//                      defaults to __debugbreak() or __asm {int 3} for MSVC and ICC, others: std::abort().
//
// #define NST_FUNCTION_NAME x - like __LINE__, expands to the current function name.
//                               (automatically defined for MSVC using __FUNCTION__)
//
// #define NST_FPTR_MEM_MAP - define this if the compiler/platform can't handle conversions between
//                            different class member function pointers.
//
// #define NST_TAILCALL_OPTIMIZE - define this if the compiler supports tail-call optimizations
// 								   (automatically defined for MSVC and GCC)
//
// #define NST_NO_ZLIB - omit ZLib support, warning: if you do, compressed states and movie files can't be saved/loaded!
//
// #define NST_NO_SCALE2X - omit Scale2x and Scale3x filter
//
// #define NST_NO_2XSAI - omit 2xSaI, Super 2xSaI and Super Eagle filter
//
// #define NST_NO_HQ2X - omit hq2x and hq3x filter
//
// #define NST_NO_NTSCVIDEO - omit NTSC composite video filter
//
// remarks: GCC = GNU Compiler
//          ICC = Intel C++ Compiler
//          MCW = Metrowerks CodeWarrior
//          MSVC = Microsoft Visual C++
//
////////////////////////////////////////////////////////////////////////////////////////
