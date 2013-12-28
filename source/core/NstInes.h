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

#ifndef NST_INES_H
#define NST_INES_H

#include "NstRomDatabase.h"

class PDXFILE;

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_NAMESPACE_BEGIN

class INES
{
public:

	PDXRESULT Import
	(
     	CARTRIDGE* const,
		PDXFILE&,
		const ROMDATABASE&,
		const IO::GENERAL::CONTEXT&
	);
	
private:

   #ifdef NES_USE_ROM_DATABASE

	BOOL TryDatabase
	(
       	PDXFILE&,
		const ROMDATABASE&,
		PDXSTRING*
	);

	ROMDATABASE::HANDLE FindInDatabase
	(
    	const ROMDATABASE&,
       	PDXFILE&,
		const TSIZE,
		const TSIZE,
		ULONG&
	) const;

   #endif

	enum
	{
		INES_VERTICAL   = b16( 00000000, 00000001 ),
		INES_BATTERY    = b16( 00000000, 00000010 ),
		INES_TRAINER    = b16( 00000000, 00000100 ),
		INES_FOURSCREEN = b16( 00000000, 00001000 ),
		INES_MAPPER_LO  = b16( 00000000, 11110000 ),
		INES_VS         = b16( 00000001, 00000000 ),
		INES_MAPPER_HI  = b16( 11110000, 00000000 )
	};

	enum
	{
		INES_DIRT_POS = 7,
		INES_DIRT_SIZE = sizeof(U8) * 9,
		INES_RESERVED_LENGTH = 7
	};

   #pragma pack(push,1)

	struct HEADER
	{
		U32 signature;
		U8  Num16kPRomBanks;
		U8  Num8kCRomBanks;
		U16 flags;
		U8  Num8kWRamBanks;
		U8  reserved[INES_RESERVED_LENGTH];
	};

   #pragma pack(pop)

	PDX_COMPILE_ASSERT(sizeof(HEADER) == sizeof(U8) * 16);

	VOID MessWithTheHeader(HEADER&);

	CARTRIDGE* cartridge;
};

NES_NAMESPACE_END

#endif
