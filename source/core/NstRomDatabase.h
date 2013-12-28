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

#ifndef NST_ROMDATABASE_H
#define NST_ROMDATABASE_H

#ifndef NES_NO_ROM_DATABASE
#if defined(_WIN32) && defined(_MSC_VER)
#undef NES_USE_ROM_DATABASE
#define NES_USE_ROM_DATABASE
#endif
#endif

#ifdef NES_USE_ROM_DATABASE
#include "../paradox/PdxArray.h"
#endif

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class ROMDATABASE
{
public:

 #ifdef NES_USE_ROM_DATABASE

	ROMDATABASE();

	typedef const VOID* HANDLE;

	HANDLE GetHandle(const ULONG) const;

	const CHAR* Name         (HANDLE) const;
	const CHAR* Copyright    (HANDLE) const;	
	ULONG       Crc          (HANDLE) const;
	ULONG       pRomCrc      (HANDLE) const;
	TSIZE       pRomSize     (HANDLE) const;
	TSIZE       cRomSize     (HANDLE) const;
	TSIZE       wRamSize     (HANDLE) const;	
	SYSTEM      System       (HANDLE) const;
	UINT        Mapper       (HANDLE) const;
	MIRRORING   Mirroring    (HANDLE) const;
	BOOL        HasBattery   (HANDLE) const;
	BOOL        HasTrainer   (HANDLE) const;
	BOOL        IsBad        (HANDLE) const;
	BOOL        IsHacked     (HANDLE) const;
	BOOL        IsTranslated (HANDLE) const;
	BOOL        IsUnlicenced (HANDLE) const;
	BOOL        IsBootleg    (HANDLE) const;

private:

   #pragma pack(push,1)

	struct INFO
	{
		U16   copyright;
		U32   crc;
		U32   pRomCrc;
		U8    pRomSize;
		U8    cRomSize;
		U8    wRamSize;
		U8    mapper;
		UCHAR pal         : 1;
		UCHAR ntsc        : 1;
		UCHAR vs          : 1;
		UCHAR p10         : 1;
		UCHAR mirroring   : 2;
		UCHAR battery     : 1;
		UCHAR trainer     : 1;
		UCHAR bad         : 1;
		UCHAR hack        : 1;
		UCHAR translation : 1;
		UCHAR unlicensed  : 1;
		UCHAR bootleg     : 1;
	};

   #pragma pack(pop,1)

	struct ENTRY
	{
		inline ENTRY(const CHAR* const n,const INFO& i)
		: name(n), info(i) {}

		inline operator ULONG () const
		{ return info.crc; }

		const CHAR* const name;
		const INFO& info;
	};

	typedef PDXARRAY<const CHAR* const> COPYRIGHTS;
	typedef PDXARRAY<ENTRY> DATABASE;

	DATABASE database;
	COPYRIGHTS copyrights;

 #endif
};

NES_NAMESPACE_END

#endif

