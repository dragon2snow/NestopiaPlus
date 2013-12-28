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

#include "../paradox/PdxCrc32.h"
#include "../paradox/PdxFile.h"
#include "mapper/NstMappers.h"
#include "NstCartridge.h"
#include "NstINes.h"

#ifdef NES_USE_ROM_DATABASE
 
 #include <Windows.h>
 #include "../windows/resource/resource.h"

#endif

NES_NAMESPACE_BEGIN

#ifdef NES_USE_ROM_DATABASE

 PDXMAP<INES::IMAGE,U32> INES::database;

#endif

#define NES_INES_SIGNATURE 0x1A53454EUL

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT INES::Import(CARTRIDGE* const cartridge,PDXFILE& file,const IO::GENERAL::CONTEXT& context)
{
	{
		HEADER header;

		if (!file.Read(header))
			return MsgWarning( "Not a valid rom image format!" );

		if (header.signature != NES_INES_SIGNATURE)
			return MsgWarning( "Invalid file type or file is corrupt!" );

		if (!header.Num16kPRomBanks)
			return MsgWarning( "INES file is corrupt!" );

		MessWithTheHeader( cartridge, header );
	}

	if (cartridge->info.trained)
	{
		if (!file.Read( cartridge->wRam.At(0x1000), cartridge->wRam.At(0x1200) ))
			return MsgWarning( "INES file is corrupt!" );
	}

	if (!file.Read( cartridge->pRom.Begin(), cartridge->pRom.End() ))
		return MsgWarning( "INES file is corrupt!" );

	if (!file.Read( cartridge->cRom.Begin(), cartridge->cRom.End() ))
		return MsgWarning( "INES file is corrupt!" );

	cartridge->info.crc     = PDXCRC32::Compute( file.At(16), file.Size() - 16 );
	cartridge->info.pRomCrc = PDXCRC32::Compute( cartridge->pRom.Begin(), cartridge->pRom.Size() );
	cartridge->info.cRomCrc = PDXCRC32::Compute( cartridge->cRom.Begin(), cartridge->cRom.Size() );

  #ifdef NES_USE_ROM_DATABASE

	PDX_TRY(CheckDatabase( cartridge, file, context ));

  #endif

	DoFinalAdjustments( cartridge );

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID INES::MessWithTheHeader(CARTRIDGE* const cartridge,HEADER& header)
{
	for (UINT i=0; i < INES_RESERVED_LENGTH; ++i)
	{
		if (header.reserved[i])
		{
			PDXMemZero( PDX_CAST_PTR(U8,header) + INES_DIRT_POS, INES_DIRT_SIZE );
			break;
		}
	}

	cartridge->info.mirroring = 
	(
     	(header.flags & INES_FOURSCREEN) ? MIRROR_FOURSCREEN : 
     	(header.flags & INES_VERTICAL) ? MIRROR_VERTICAL : 
		MIRROR_HORIZONTAL
	);

	cartridge->info.mapper =
	( 
     	((header.flags & INES_MAPPER_LO) >> 4) +
		((header.flags & INES_MAPPER_HI) >> 8)
	);

	cartridge->info.trained = (header.flags & INES_TRAINER) ? TRUE : FALSE;
	cartridge->info.battery = (header.flags & INES_BATTERY) ? TRUE : FALSE;
	cartridge->info.system  = (header.flags & INES_VS) ? SYSTEM_VS : SYSTEM_NTSC;

	cartridge->info.pRom = header.Num16kPRomBanks * n16k;
	cartridge->info.cRom = header.Num8kCRomBanks * n8k;
	cartridge->info.wRam = header.Num8kWRamBanks * n8k;

	cartridge->pRom.Resize( cartridge->info.pRom );
	cartridge->cRom.Resize( cartridge->info.cRom );
	cartridge->wRam.Resize( PDX_MAX(cartridge->info.wRam,n8k) );	

	cartridge->wRam.Fill( cartridge->wRam.At(0x0000), cartridge->wRam.At(0x1000), 0x60 );
	cartridge->wRam.Fill( cartridge->wRam.At(0x1000), cartridge->wRam.At(0x2000), 0x70 );
	cartridge->wRam.Fill( cartridge->wRam.At(0x2000), cartridge->wRam.End(),      0x00 );

	cartridge->pRom.Defrag();
	cartridge->cRom.Defrag();
	cartridge->wRam.Defrag();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID INES::DoFinalAdjustments(CARTRIDGE* const cartridge)
{
	if (cartridge->info.mirroring == MIRROR_FOURSCREEN)
	{
		switch (cartridge->info.mapper)
		{
    		case  24:
     		case  26:
       		case 118:

     			cartridge->info.mirroring = MIRROR_HORIZONTAL;
       			break;
		}
	}

	if (!cartridge->info.battery)
	{
		switch (cartridge->info.pRomCrc)
		{
     		case 0xB17574F3UL: // AD&D Heroes of the Lance
     		case 0x25952141UL: // AD&D Pool of Radiance
			case 0x1335CB05UL: // Crystalis
			case 0x2545214CUL: // Dragon Warrior PRG(0,1)
			case 0x45F03D2EUL: // Faria
     		case 0xE1383DEBUL: // Mouryou Senki Madara
          	case 0x3B3F88F0UL: // -||-
			case 0x889129CBUL: // Startropics
			case 0xD054FFB0UL: // Startropics 2
			case 0x7CAB2E9BUL: // -||-
			case 0x3EE43CDAUL: // -||-
		
				cartridge->info.battery = TRUE;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#ifdef NES_USE_ROM_DATABASE

VOID INES::ImportDatabase()
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

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT INES::CheckDatabase(CARTRIDGE* const cartridge,PDXFILE& file,const IO::GENERAL::CONTEXT& context)
{
	if (database.IsEmpty())
		ImportDatabase();

	PDXMAP<IMAGE,U32>::CONSTITERATOR iterator( database.Find(cartridge->info.pRomCrc) );

	PDXSTRING log("INES: ");

	if (iterator != database.End())
	{
		const IMAGE& image = (*iterator).Second();

		cartridge->info.condition = (image.bad ? IO::CARTRIDGE::BAD : IO::CARTRIDGE::GOOD);

		BOOL DisableWarning = context.DisableWarnings;

		if (cartridge->info.condition == IO::CARTRIDGE::BAD)
		{
			LogOutput("INES: warning, PRG-ROM or CHR-ROM may be broken");

			if (!DisableWarning)
			{
				DisableWarning = TRUE;
				MsgWarning( "Possibly bad dump! Image may not run properly!" );
			}
		}

		if (cartridge->info.mapper != image.mapper)
		{
			log.Resize( 6 );
			log += "warning, mapper ";
			log += cartridge->info.mapper;
			log += " as defined in the file header is wrong and should be ";
			log += image.mapper;
			LogOutput( log );

			cartridge->info.mapper = image.mapper;
		}

		if      (image.vs)  cartridge->info.system = SYSTEM_VS;
		else if (image.p10) cartridge->info.system = SYSTEM_PC10;
		else if (image.pal) cartridge->info.system = SYSTEM_PAL;
		else                cartridge->info.system = SYSTEM_NTSC;

		if (cartridge->info.mirroring != image.mirroring)
			cartridge->info.mirroring = MIRRORING(image.mirroring);

		if (cartridge->info.battery != image.battery)
		{
			log.Resize( 6 );
			log += "warning, battery save ram should be ";
			log += (image.battery ? "enabled" : "disabled");
			log += " and not ";
			log += (image.battery ? "disabled" : "enabled");
			log += " as defined in the file header";
			LogOutput( log );

			cartridge->info.battery = image.battery;
		}

		cartridge->info.pRom = image.pRomSize * n16k;
		cartridge->info.cRom = image.cRomSize * n8k;

		if (cartridge->pRom.Size() != cartridge->info.pRom)
		{	
			log.Resize( 6 );
			log += "warning, PRG-ROM size ";
			log += (cartridge->pRom.Size() / 1024);
			log += "k as defined in the file header is wrong and should be ";
			log += (cartridge->info.pRom / 1024);
			log += "k";
			LogOutput( log );

			if (!DisableWarning)
			{
				DisableWarning = TRUE;
				cartridge->info.condition = IO::CARTRIDGE::BAD;
				MsgWarning( "Wrong PRG-ROM size! Image may not run properly!" );
			}

			if (cartridge->pRom.Size() > cartridge->info.pRom)
			{
				cartridge->pRom.Resize( cartridge->info.pRom );
				cartridge->info.pRomCrc = PDXCRC32::Compute( cartridge->pRom.Begin(), cartridge->info.pRom );
			}
		}

		if (cartridge->cRom.Size() != cartridge->info.cRom)
		{
			log.Resize( 6 );
			log += "warning, CHR-ROM size ";
			log += (cartridge->cRom.Size() / 1024);
			log += "k as defined in the file header is wrong and should be ";
			log += (cartridge->info.cRom / 1024);
			log += "k";
			LogOutput( log );

			if (!DisableWarning)
			{
				DisableWarning = TRUE;
				cartridge->info.condition = IO::CARTRIDGE::BAD;
				MsgWarning( "Wrong CHR-ROM size! Image may not run properly!" );
			}

			if (cartridge->cRom.Size() > cartridge->info.cRom)
			{
				cartridge->cRom.Resize( cartridge->info.cRom );
				cartridge->info.cRomCrc = PDXCRC32::Compute( cartridge->cRom.Begin(), cartridge->info.cRom );
			}
		}

		if (cartridge->wRam.Size() < image.wRamSize)
		{
			log.Resize( 6 );
			log += "warning, WRAM size ";
			log += (cartridge->info.wRam / 1024);
			log += "k as defined in the file header is wrong and should be ";
			log += (image.wRamSize / 1024);
			log += "k";
			LogOutput( log );

			cartridge->info.wRam = image.wRamSize;
			cartridge->wRam.Resize( image.wRamSize );
		}

		cartridge->info.name       = image.name;
		cartridge->info.copyright  = ( image.copyright.Size() ? image.copyright.String() : "unknown");
		cartridge->info.translated = ( image.translation ? IO::CARTRIDGE::YES : IO::CARTRIDGE::NO  );
		cartridge->info.hacked     = ( image.hack        ? IO::CARTRIDGE::YES : IO::CARTRIDGE::NO  );
		cartridge->info.licensed   = ( image.unlicensed  ? IO::CARTRIDGE::NO  : IO::CARTRIDGE::YES );
		cartridge->info.bootleg    = ( image.bootleg     ? IO::CARTRIDGE::YES : IO::CARTRIDGE::NO  );
	}
	else
	{
		cartridge->info.name       = "unknown";
		cartridge->info.copyright  = "unknown";
		cartridge->info.condition  = IO::CARTRIDGE::GOOD;
		cartridge->info.translated = IO::CARTRIDGE::UNKNOWN;
		cartridge->info.hacked     = IO::CARTRIDGE::UNKNOWN;
		cartridge->info.licensed   = IO::CARTRIDGE::UNKNOWN;
		cartridge->info.bootleg    = IO::CARTRIDGE::UNKNOWN;
	}

	cartridge->info.board = MAPPER::boards[cartridge->info.mapper];

	return PDX_OK;
}

#endif

NES_NAMESPACE_END
