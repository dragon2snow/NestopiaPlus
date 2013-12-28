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
#include "NstImageFile.h"
#include "NstINes.h"

NES_NAMESPACE_BEGIN

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

	cartridge->info.name       = "unknown";
	cartridge->info.copyright  = "unknown";
	cartridge->info.crc        = PDXCRC32::Compute( file.At(16), file.Size() - 16 );
	cartridge->info.pRomCrc    = PDXCRC32::Compute( cartridge->pRom.Begin(), cartridge->pRom.Size() );
	cartridge->info.cRomCrc    = PDXCRC32::Compute( cartridge->cRom.Begin(), cartridge->cRom.Size() );
	cartridge->info.condition  = IO::CARTRIDGE::GOOD;
	cartridge->info.translated = IO::CARTRIDGE::UNKNOWN;
	cartridge->info.hacked     = IO::CARTRIDGE::UNKNOWN;
	cartridge->info.licensed   = IO::CARTRIDGE::UNKNOWN;
	cartridge->info.bootleg    = IO::CARTRIDGE::UNKNOWN;
	cartridge->info.board      = MAPPER::boards[cartridge->info.mapper];

  #ifdef NES_USE_ROM_DATABASE

	if (context.UseRomDatabase)
		CheckDatabase( cartridge, file, context );

  #endif

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

#ifdef NES_USE_ROM_DATABASE

VOID INES::CheckDatabase(CARTRIDGE* const cartridge,PDXFILE& file,const IO::GENERAL::CONTEXT& context)
{
	if (database.IsEmpty())
		ImportDatabase();

	PDXMAP<IMAGE,KEY>::CONSTITERATOR iterator(database.Find(KEY(cartridge->info.crc,cartridge->info.pRomCrc)));

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
			LogOutput( log.String() );

			cartridge->info.mapper = image.mapper;
		}

		if      (image.vs)                 cartridge->info.system = SYSTEM_VS;
		else if (image.p10)                cartridge->info.system = SYSTEM_PC10;
		else if (image.pal && !image.ntsc) cartridge->info.system = SYSTEM_PAL;
		else                               cartridge->info.system = SYSTEM_NTSC;

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
			LogOutput( log.String() );

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
			LogOutput( log.String() );

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
			LogOutput( log.String() );
  
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
			LogOutput( log.String() );

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
}

#endif

NES_NAMESPACE_END
