////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
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

#ifndef NST_MAIN_H
#define NST_MAIN_H

#pragma once

#include "../core/NstTypes.hpp"
#include <tchar.h>

#ifdef UNICODE
#include <cwchar>
#elif defined(_MBCS)
#error Multibyte characters not supported!
#endif

#ifdef _MSC_VER

 #ifdef __INTEL_COMPILER

  #pragma warning( disable : 171 304 373 383 444 810 981 1683 1684 )

 #else

  #pragma warning( disable : 4244 4355 4511 4512 4800 4996 )

  #ifdef NDEBUG

   #ifdef NST_SHOW_INLINING
   #pragma warning( default : 4710 4711 )
   #endif

  #endif

 #endif

 #define NST_CDECL __cdecl
 #define NST_STDCALL __stdcall

 #define NST_SIGN_SHIFT

#elif defined(__GNUC__)

 #define NST_CDECL __cdecl
 #define NST_STDCALL __stdcall

#endif

// minimum operating system: Windows 98 2nd.edition

#define WINVER         0x0500
#define _WIN32_WINDOWS 0x0410
#define _WIN32_WINNT   0x0403
#define _WIN32_IE      0x0401

#define _WIN32_DCOM

#define STRICT
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

// <Windows.h>

#define NOGDICAPMASKS
#define NOICONS
#define NOKEYSTATES
#define NORASTEROPS
#define NOATOM
#define NOKERNEL
#define NOMEMMGR
#define NOMETAFILE
#define NOOPENFILE
#define NOSERVICE
#define NOSOUND
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX

#ifndef NOMINMAX
#define NOMINMAX
#endif

// <commctrl.h>

#define NOUPDOWN
#define NOMENUHELP
#define NODRAGLIST
#define NOPROGRESS
#define NOHOTKEY
#define NOHEADER
#define NOTABCONTROL
#define NOANIMATE
#define NOBUTTON
#define NOSTATIC
#define NOEDIT
#define NOLISTBOX
#define NOCOMBOBOX
#define NOSCROLLBAR

#define NST_FOURCC(a_,b_,c_,d_) uint((a_) | ((b_) << 8) | ((c_) << 16) | ((d_) << 24))

namespace Nestopia
{
	typedef signed char schar;
	typedef unsigned char uchar;
	typedef unsigned short ushort;
	typedef unsigned int uint;
	typedef unsigned long ulong;
	typedef TCHAR tchar;

	using Nes::u8;
	using Nes::i8;
	using Nes::u16;
	using Nes::i16;
	using Nes::u32;
	using Nes::i32;
	using Nes::u64;

	typedef uint ibool;
	typedef const char* cstring;
	typedef const wchar_t* wstring;
	typedef const TCHAR* tstring;

	namespace Application
	{
		class Configuration;
	}

	namespace Collection
	{
		template<typename> class Vector;
		typedef Vector<char> Buffer;
	}

	namespace Managers
	{
		class Emulator;
		class Paths;

		using Application::Configuration;
	}

	template<typename T> struct ConstParam
	{
		typedef const T& Type;
	};

	template<typename T> struct ConstParam<T*>
	{
		typedef const T* const Type;
	};

	template<> struct ConstParam< bool   > { typedef const bool   Type; };
	template<> struct ConstParam< char   > { typedef const int    Type; };
	template<> struct ConstParam< schar  > { typedef const int    Type; };
	template<> struct ConstParam< uchar  > { typedef const uint   Type; };
	template<> struct ConstParam< short  > { typedef const int    Type; };
	template<> struct ConstParam< ushort > { typedef const uint   Type; };
	template<> struct ConstParam< int    > { typedef const int    Type; };
	template<> struct ConstParam< uint   > { typedef const uint   Type; };
	template<> struct ConstParam< long   > { typedef const long   Type; };
	template<> struct ConstParam< ulong  > { typedef const ulong  Type; };

	class Sealed
	{
		Sealed(const Sealed&) {}
		void operator = (const Sealed&) {}

	protected:

		Sealed() {}
		~Sealed() {}
	};

	enum
	{
		IDM_POS_FILE = 0,
		IDM_POS_FILE_LOAD = 3,
		IDM_POS_FILE_SAVE = 4,
		IDM_POS_FILE_QUICKLOADSTATE = 6,
		IDM_POS_FILE_QUICKSAVESTATE = 7,
		IDM_POS_FILE_SOUNDRECORDER = 12,
		IDM_POS_FILE_MOVIE = 13,
		IDM_POS_FILE_RECENTFILES = 17,
		IDM_POS_FILE_RECENTDIRS = 18,
		IDM_POS_MACHINE = 1,
		IDM_POS_MACHINE_RESET = 1,
		IDM_POS_MACHINE_INPUT = 3,
		IDM_POS_MACHINE_INPUT_PORT1 = 2,
		IDM_POS_MACHINE_INPUT_PORT2 = 3,
		IDM_POS_MACHINE_INPUT_PORT3 = 4,
		IDM_POS_MACHINE_INPUT_PORT4 = 5,
		IDM_POS_MACHINE_INPUT_EXP = 6,
		IDM_POS_MACHINE_EXT = 4,
		IDM_POS_MACHINE_EXT_FDS = 0,
		IDM_POS_MACHINE_EXT_KEYBOARD = 1,
		IDM_POS_MACHINE_EXT_TAPE = 2,
		IDM_POS_MACHINE_EXT_FDS_INSERTDISK = 0,
		IDM_POS_MACHINE_SYSTEM = 6,
		IDM_POS_MACHINE_OPTIONS = 7,
		IDM_POS_VIEW = 3,
		IDM_POS_VIEW_SCREENSIZE = 3,
		IDM_POS_OPTIONS = 4,
		IDM_POS_OPTIONS_AUTOSAVER = 9,
		IDM_POS_OPTIONS_AUTOSAVER_OPTIONS = 0,
		IDM_POS_OPTIONS_AUTOSAVER_START
	};
}

#endif
