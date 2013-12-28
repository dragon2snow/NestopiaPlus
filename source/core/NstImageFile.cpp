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

#include "NstTypes.h"
#include "NstImageFile.h"

#ifdef NES_USE_ROM_DATABASE

#include <Windows.h>
#include "../windows/resource/resource.h"
#include "../paradox/PdxArray.h"

NES_NAMESPACE_BEGIN

PDXMAP<IMAGEFILE::IMAGE,U32> IMAGEFILE::database;

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID IMAGEFILE::ImportDatabase()
{
	const CHAR* iterator = NULL;		
	const CHAR* end = NULL;

	HGLOBAL hGlobal;
	HMODULE hModule = GetModuleHandle( NULL );		
	HRSRC hRsrc = FindResource( hModule, MAKEINTRESOURCE(IDR_ROMDATABASE1),"RomDatabase" );

	if (hRsrc && (hGlobal = LoadResource( hModule, hRsrc )))
	{
		const DWORD size = SizeofResource( hModule, hRsrc );

		if (size)
		{
			iterator = (const CHAR*) LockResource(hGlobal);
			end = iterator + size;
		}
	}

	if (!iterator)
		return;

	PDXARRAY<const CHAR*> copyright;
	copyright.Resize(*PDX_CAST(const U8*,iterator));

	iterator += sizeof(U8);

	for (UINT i=0; i < copyright.Size(); ++i)
	{
		copyright[i] = iterator;
		iterator += strlen(iterator) + 1;
	}

	while (iterator < end)
	{
		const CHAR* const name = iterator;
		iterator += strlen(name) + 1;

		const DBCHUNK* const chunk = PDX_CAST(const DBCHUNK*,iterator);
		iterator += sizeof(DBCHUNK);

		IMAGE& image = database[chunk->pRomCrc];

		image.name        = name;
		image.copyright   = copyright[chunk->copyright];
		image.pRomCrc     = chunk->pRomCrc;
		image.pRomSize    = chunk->pRomSize;
		image.cRomSize    = chunk->cRomSize;
		image.wRamSize    = chunk->wRamSize;
		image.mapper      = chunk->mapper;
		image.pal         = chunk->pal;
		image.ntsc        = chunk->ntsc;
		image.vs          = chunk->vs;
		image.p10         = chunk->p10;
		image.mirroring   = chunk->mirroring;
		image.battery     = chunk->battery;
		image.trainer     = chunk->trainer;
		image.bad         = chunk->bad;
		image.hack        = chunk->hack;
		image.translation = chunk->translation;
		image.unlicensed  = chunk->unlicensed;
		image.bootleg     = chunk->bootleg;
	}
}

NES_NAMESPACE_END

#endif
