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

#ifndef NST_IMAGEFILE_H
#define NST_IMAGEFILE_H

#ifndef NES_NO_ROM_DATABASE
#if defined(_WIN32) && defined(_MSC_VER)
#undef NES_USE_ROM_DATABASE
#define NES_USE_ROM_DATABASE
#endif
#endif

#ifdef NES_USE_ROM_DATABASE
#include "../paradox/PdxMap.h"
#endif

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_NAMESPACE_BEGIN

class IMAGEFILE
{
protected:

  #ifdef NES_USE_ROM_DATABASE

	VOID ImportDatabase();	
	
   #pragma pack(push,1)

	struct IMAGE
	{
		PDXSTRING name;
		PDXSTRING copyright;
		U32       pRomCrc;
		U8        pRomSize;
		U8        cRomSize;
		U8        wRamSize;
		U8        mapper;
		UCHAR     pal         : 1;
		UCHAR     ntsc        : 1;
		UCHAR     vs          : 1;
		UCHAR     p10         : 1;
		UCHAR     mirroring   : 2;
		UCHAR     battery     : 1;
		UCHAR     trainer     : 1;
		UCHAR     bad         : 1;
		UCHAR     hack        : 1;
		UCHAR     translation : 1;
		UCHAR     unlicensed  : 1;
		UCHAR     bootleg     : 1;
	};

	struct DBCHUNK
	{
		U8    copyright;
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

   #pragma pack(pop)

	static PDXMAP<IMAGE,U32> database;

  #endif
};

NES_NAMESPACE_END

#endif
