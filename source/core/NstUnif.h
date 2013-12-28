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

#ifndef NST_UNIF_H
#define NST_UNIF_H

class PDXFILE;

#include "../paradox/PdxMap.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_NAMESPACE_BEGIN

class UNIF : public IMAGEFILE
{
public:

	UNIF();

	PDXRESULT Import
	(
       	CARTRIDGE* const,
		PDXFILE&,
		const IO::GENERAL::CONTEXT&
	);
	
private:

	typedef PDXMAP<U8,PDXSTRING> BOARDS;

	BOARDS boards;

	U32 pRomCrcs[16];
	U32 cRomCrcs[16];

   #ifdef NES_USE_ROM_DATABASE

	PDXRESULT CheckDatabase
	(
		CARTRIDGE* const,
		PDXFILE&,
		const IO::GENERAL::CONTEXT&
	);

   #endif

   #pragma pack(push,1)

	struct HEADER
	{
		CHAR signature[4];
		U32  revision;
		U8   reserved[24];
	};

	struct CHUNK
	{
		CHAR id[4];
		U32 length;
	};

   #pragma pack(pop)
};

NES_NAMESPACE_END

#endif
