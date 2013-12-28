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

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT INES::Import
(
    CARTRIDGE* const c,
	PDXFILE& file,
	const ROMDATABASE& RomDatabase,
	const IO::GENERAL::CONTEXT& context
)
{
	PDX_ASSERT( c );

	cartridge = c;

	try
	{
		{
			HEADER header;

			if (!file.Read(header))
				throw 1;

			MessWithTheHeader( header );
		}

		BOOL UsedDatabase = FALSE;

      #ifdef NES_USE_ROM_DATABASE

		PDXSTRING msg;

		if (context.UseRomDatabase)
			UsedDatabase = TryDatabase( file, RomDatabase, (context.DisableWarnings ? NULL : &msg) );

      #endif

		if (!cartridge->info.pRom || cartridge->info.pRom > file.Size() - 0x10)
			throw 1;

		cartridge->wRam.Resize( PDX_MAX(cartridge->info.wRam,n8k) );

		cartridge->wRam.Fill( cartridge->wRam.At(0x0000), cartridge->wRam.At(0x1000), 0x60 );
		cartridge->wRam.Fill( cartridge->wRam.At(0x1000), cartridge->wRam.At(0x2000), 0x70 );
		cartridge->wRam.Fill( cartridge->wRam.At(0x2000), cartridge->wRam.End(),      0x00 );

		if (cartridge->info.trained)
		{
			if (!file.Read( cartridge->wRam.At(0x1000), cartridge->wRam.At(0x1200) ))
				throw 1;
		}

		cartridge->pRom.Resize( cartridge->info.pRom );

		if (!file.Read( cartridge->pRom.Begin(), cartridge->pRom.End() ))
			throw 1;

		cartridge->cRom.Resize( cartridge->info.cRom );

		if (!file.Read( cartridge->cRom.Begin(), cartridge->cRom.End() ))
			throw 1;

		cartridge->info.board   = MAPPER::boards[ cartridge->info.mapper ];
		cartridge->info.pRomCrc = PDXCRC32::Compute( cartridge->pRom.Begin(), cartridge->pRom.Size() );
		cartridge->info.cRomCrc = PDXCRC32::Compute( cartridge->cRom.Begin(), cartridge->cRom.Size() );

		if (!UsedDatabase)
		{
			cartridge->info.name       = "unknown";
			cartridge->info.copyright  = "unknown";
			cartridge->info.condition  = IO::CARTRIDGE::GOOD;
			cartridge->info.hacked     = IO::CARTRIDGE::UNKNOWN;
			cartridge->info.translated = IO::CARTRIDGE::UNKNOWN;
			cartridge->info.licensed   = IO::CARTRIDGE::UNKNOWN;
			cartridge->info.bootleg    = IO::CARTRIDGE::UNKNOWN;
			cartridge->info.crc        = PDXCRC32::Compute( file.At(0x10), file.Size() - 0x10 ); 
		}

      #ifdef NES_USE_ROM_DATABASE

		if (msg.Length())
			MsgWarning( msg.String() );

      #endif
	}
	catch (...)
	{
		return MsgError( "iNes file is corrupt!" );
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID INES::MessWithTheHeader(HEADER& header)
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

	cartridge->info.pRom = n16k * header.Num16kPRomBanks;
	cartridge->info.cRom = n8k * header.Num8kCRomBanks;
	cartridge->info.wRam = n8k * header.Num8kWRamBanks;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#ifdef NES_USE_ROM_DATABASE

ROMDATABASE::HANDLE INES::FindInDatabase
(
    const ROMDATABASE& RomDatabase,
    PDXFILE& file,
	const TSIZE offset,
	const TSIZE length,
	ULONG& crc
) const
{
	if (file.Size() >= offset + length)
	{
		crc = PDXCRC32::Compute( file.At(offset), length );
		return RomDatabase.GetHandle( crc );
	}

	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL INES::TryDatabase(PDXFILE& file,const ROMDATABASE& RomDatabase,PDXSTRING* msg)
{
	ULONG crc;

	ROMDATABASE::HANDLE handle = NULL;

	TSIZE length = file.Size() - 0x10;

	if (length)
		handle = FindInDatabase( RomDatabase, file, 0x10, length, crc );

	if (!handle)
	{
		length = cartridge->info.pRom + cartridge->info.cRom;

		if (length && length <= file.Size() - 0x10)
			handle = FindInDatabase( RomDatabase, file, 0x10, length, crc );
	}

	if (!handle)
		return FALSE;

	{
		const CHAR* const copyright = RomDatabase.Copyright( handle );

		if (copyright)
			cartridge->info.copyright = copyright;
	}

	cartridge->info.name = RomDatabase.Name( handle );
	cartridge->info.crc = crc;

	cartridge->info.condition  = ( RomDatabase.IsBad        ( handle ) ? IO::CARTRIDGE::BAD : IO::CARTRIDGE::GOOD );
	cartridge->info.hacked     = ( RomDatabase.IsHacked     ( handle ) ? IO::CARTRIDGE::YES : IO::CARTRIDGE::NO   );
	cartridge->info.translated = ( RomDatabase.IsTranslated ( handle ) ? IO::CARTRIDGE::YES : IO::CARTRIDGE::NO   );
	cartridge->info.licensed   = ( RomDatabase.IsUnlicenced ( handle ) ? IO::CARTRIDGE::NO  : IO::CARTRIDGE::YES  );
	cartridge->info.bootleg    = ( RomDatabase.IsBootleg    ( handle ) ? IO::CARTRIDGE::YES : IO::CARTRIDGE::NO   );

	if (cartridge->info.condition == IO::CARTRIDGE::BAD)
	{
		LogOutput("INES DATABASE: warning, possibly bad dump!");

		if (msg)
		{
			*msg = "Possibly bad dump! Image may not run properly!";
			msg = NULL;
		}
	}

	{
		PDXSTRING log("INES DATABASE: warning, ");

		const TSIZE pRom = RomDatabase.pRomSize( handle );
		PDX_ASSERT( pRom );

		const BOOL HasTrainer = RomDatabase.HasTrainer( handle );

		if (cartridge->info.pRom != pRom)
		{	
			log.Resize( 24 );

			const TSIZE total = (HasTrainer ? 0x210 : 0x10) + pRom;

			if (cartridge->info.pRom > pRom || !cartridge->info.pRom || file.Size() >= total)
			{
				log << "changed PRG-ROM size: ";
				log << (cartridge->info.pRom / 1024);
				log << "k to: ";
				log << (pRom / 1024);
				log < "k";

				cartridge->info.pRom = pRom;
			}
			else
			{
				log << "wanted to change PRG-ROM size: ";
				log << (cartridge->info.pRom / 1024);
				log << "k to: ";
				log << (pRom / 1024);
				log << "k but couldn't";
			}

			LogOutput( log );

			cartridge->info.condition = IO::CARTRIDGE::BAD;

			if (msg)
			{
				*msg = "Wrong PRG-ROM size! Image may not run properly!";
				msg = NULL;
			}
		}

		const TSIZE cRom = RomDatabase.cRomSize( handle );

		if (cartridge->info.cRom != cRom)
		{
			log.Resize( 24 );

			const TSIZE total = (HasTrainer ? 0x210 : 0x10) + cartridge->info.pRom + cRom;

			if (cartridge->info.cRom > cRom || file.Size() >= total)
			{
				log << "changed CHR-ROM size: ";
				log << (cartridge->info.cRom / 1024);
				log << "k to: ";
				log << (cRom / 1024);
				log << "k";

				cartridge->info.cRom = cRom;
			}
			else
			{
				log << "wanted to change CHR-ROM size: ";
				log << (cartridge->info.cRom / 1024);
				log << "k to: ";
				log << (cRom / 1024);
				log << "k but couldn't";
			}

			LogOutput( log );

			cartridge->info.condition = IO::CARTRIDGE::BAD;

			if (msg)
			{
				*msg = "Wrong CHR-ROM size! Image may not run properly!";
				msg = NULL;
			}
		}

		const TSIZE wRam = RomDatabase.wRamSize( handle );

		if (cartridge->info.wRam != wRam)
		{
			log.Resize( 24 );
			log << "changed WRAM size: ";
			log << (cartridge->info.wRam / 1024);
			log << "k to: ";
			log << (wRam / 1024);
			log << "k";
			LogOutput( log );

			cartridge->info.wRam = wRam;
		}

		const UINT mapper = RomDatabase.Mapper( handle );

		if (cartridge->info.mapper != mapper)
		{
			log.Resize( 24 );
			log << "changed mapper ";
			log << cartridge->info.mapper;
			log << " to ";
			log << mapper;
			LogOutput( log );

			cartridge->info.mapper = mapper;
		}

		const BOOL HasBattery = RomDatabase.HasBattery( handle );

		if (bool(cartridge->info.battery) != bool(HasBattery))
		{
			log.Resize( 24 );
			log << (HasBattery ? "enabled" : "disabled");
			log << " battery RAM";
			LogOutput( log );
			
			cartridge->info.battery = HasBattery;
		}

		const MIRRORING mirroring = RomDatabase.Mirroring( handle );

		if (cartridge->info.mirroring != mirroring)
		{
			PDX_COMPILE_ASSERT(MIRROR_HORIZONTAL < 3 && MIRROR_VERTICAL < 3 && MIRROR_FOURSCREEN < 3);

			const CHAR* types[3];

			types[ MIRROR_HORIZONTAL ] = "horizontal mirroring";
			types[ MIRROR_VERTICAL   ] = "vertical mirroring";
			types[ MIRROR_FOURSCREEN ] = "four-screen mirroring";

			log.Resize( 24 );
			log << "changed ";
			log << types[cartridge->info.mirroring];
			log << " to ";
			log << types[UINT(mirroring)];
			LogOutput( log );
			
			cartridge->info.mirroring = mirroring;
		}
  
		if (bool(cartridge->info.trained) != bool(HasTrainer))
		{
			log.Resize( 24 );
			log << (HasTrainer ? "enabled" : "disabled");
			log << " trainer";
			LogOutput( log );

			cartridge->info.trained = HasTrainer;
		}
	}

	if (cartridge->info.copyright.IsEmpty())
		cartridge->info.copyright = "unknown";

	cartridge->info.system = RomDatabase.System( handle );

	return TRUE;
}

#endif

NES_NAMESPACE_END
