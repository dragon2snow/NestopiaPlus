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
#include "NstUnif.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

UNIF::UNIF()
{	
	boards[ "SNROM"    ] = 1;  // MMC1
	boards[ "SKROM"    ] = 1;
	boards[ "SAROM"    ] = 1;
	boards[ "SBROM"    ] = 1;
	boards[ "SCEOROM"  ] = 1;
	boards[ "SC1ROM"   ] = 1;
	boards[ "SEROM"    ] = 1;
	boards[ "SFROM"    ] = 1;
	boards[ "SGROM"    ] = 1;
	boards[ "SHROM"    ] = 1;
	boards[ "SJROM"    ] = 1;
	boards[ "SLROM"    ] = 1;
	boards[ "SLRROM"   ] = 1;
	boards[ "SN1-ROM"  ] = 1;
	boards[ "SOROM"    ] = 1;
	boards[ "SVROM"    ] = 1;
	boards[ "SUROM"    ] = 1;
	boards[ "PNROM"    ] = 9;  // MMC2	
	boards[ "51555"    ] = 4;  // MMC3/MMC6
	boards[ "DRROM"    ] = 4;
	boards[ "EKROM"    ] = 4;
	boards[ "SL1ROM"   ] = 4;
	boards[ "SL2ROM"   ] = 4;
	boards[ "SL3ROM"   ] = 4;
	boards[ "TEROM"    ] = 4;
	boards[ "TFROM"    ] = 4;
	boards[ "TGROM"    ] = 4;
	boards[ "TKROM"    ] = 4;
	boards[ "TLROM"    ] = 4;
	boards[ "TQROM"    ] = 4;
	boards[ "TSROM"    ] = 4;
	boards[ "TVROM"    ] = 4;
	boards[ "TL1ROM"   ] = 4;
	boards[ "TLSROM"   ] = 4;
	boards[ "B4"       ] = 4;
	boards[ "HKROM"    ] = 4;	
	boards[ "ELROM"    ] = 5;  // MMC5
	boards[ "ETROM"    ] = 5;
	boards[ "EWROM"    ] = 5;	
	boards[ "351258"   ] = 2;  // UNROM
	boards[ "351298"   ] = 2;
	boards[ "351908"   ] = 2;
	boards[ "UNROM"    ] = 2;	
	boards[ "NROM"     ] = 0;  // NROM
	boards[ "NROM-128" ] = 0;
	boards[ "NROM-256" ] = 0;
	boards[ "RROM"     ] = 0;
	boards[ "RROM-128" ] = 0;	
	boards[ "GNROM"    ] = 66; // GNROM	
	boards[ "AOROM"    ] = 7;  // AOROM	
	boards[ "BNROM"    ] = 34; // BNROM	
	boards[ "CNROM"    ] = 3;  // CNROM	
	boards[ "CPROM"    ] = 13; // CPROM

	PDXSTRING number;

	// shhh... don't tell anyone

	for (UINT i=0; i < 255; ++i)
		boards[number = i] = i;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT UNIF::Import(CARTRIDGE* const cartridge,PDXFILE& file,const IO::GENERAL::CONTEXT& context)
{
	PDX_ASSERT( cartridge );

	PDXMemZero( pRomCrcs, 16 );
	PDXMemZero( cRomCrcs, 16 );

	cartridge->pRom.Clear();
	cartridge->cRom.Clear();
	cartridge->wRam.Clear();

	PDXSTRING log("UNIF: ");

	{
		HEADER header;

		if (!file.Read(header) || memcmp(header.signature,"UNIF",4))
			return MsgWarning("Not a valid rom image format!");

		log.Resize( 6 ); 
		log += "revision ";
		log += header.revision;        
		LogOutput( log.String() );
	}

	cartridge->info.name       = "unknown";
	cartridge->info.copyright  = "";
	cartridge->info.condition  = IO::CARTRIDGE::GOOD;
	cartridge->info.translated = IO::CARTRIDGE::UNKNOWN;
	cartridge->info.hacked     = IO::CARTRIDGE::UNKNOWN;
	cartridge->info.licensed   = IO::CARTRIDGE::UNKNOWN;
	cartridge->info.bootleg    = IO::CARTRIDGE::UNKNOWN;
	cartridge->info.system     = SYSTEM_NTSC;
	cartridge->info.mirroring  = MIRROR_HORIZONTAL;
	cartridge->info.mapper     = UINT_MAX;

	CHUNK chunk;

	static const CHAR DataIds[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

	BOOL DisableWarning = context.DisableWarnings;
	BOOL success = TRUE;

	while (!file.Eof())
	{
		if (!file.Read( chunk ))
			break;

		UINT length = 0;

		if (!memcmp(chunk.id,"NAME",4))
		{
			if (file.Text().Read( cartridge->info.name ).IsEmpty())
			{
				success = FALSE;
				break;
			}

			length = cartridge->info.name.Length() + 1;

			log.Resize( 6 ); 
			log += "Name: ";
			log += cartridge->info.name;
			LogOutput( log.String() );
		}
		else if ((!memcmp(chunk.id,"READ",4)))
		{
			PDXSTRING comments;

			if (file.Text().Read( comments ).IsEmpty())
			{
				success = FALSE;
				break;
			}

			length = comments.Length() + 1;

			log.Resize( 6 ); 
			log += "Comments: ";
			log += comments;
			LogOutput( log.String() );
		}
		else if ((!memcmp(chunk.id,"DINF",4)))
		{
           #pragma pack(push,1)

			struct DUMP
			{
				CHAR name[100];
				U8   day;
				U8   month;
				U16  year;
				CHAR agent[100];
			};

           #pragma pack(pop)

			DUMP dump;

			if (!(success=file.Read( PDX_CAST(CHAR*,&dump), PDX_CAST(CHAR*,&dump) + sizeof(dump) )))
				break;

			length = sizeof(dump);

			if (*dump.name != '\0')
			{
				log.Resize( 6 ); 
				log += "Dumper Name: ";
				log += dump.name;
				LogOutput( log.String() );
			}

			log.Resize( 6 ); 
			log += "Dump Year: ";
			log += dump.year;
			LogOutput( log.String() );

			log.Resize( 6 ); 
			log += "Dump Month: ";
			log += dump.month;
			LogOutput( log.String() );

			log.Resize( 6 ); 
			log += "Dump Day: ";
			log += dump.day;
			LogOutput( log.String() );

			if (*dump.agent != '\0')
			{
				log.Resize( 6 ); 
				log += "Dumper Agent: ";
				log += dump.agent;
				LogOutput( log.String() );
			}
		}
		else if ((!memcmp(chunk.id,"TVCI",4)))
		{
			if (!(success=file.Readable(sizeof(U8))))
				break;

			length = sizeof(U8);

			log.Resize( 6 ); 
			log += "System: ";

			switch (file.Read<U8>())
			{
       			case 0:  cartridge->info.system = SYSTEM_NTSC; log += "NTSC only";         break;
				case 1:  cartridge->info.system = SYSTEM_PAL;  log += "PAL only";          break;
				default: cartridge->info.system = SYSTEM_NTSC; log += "both PAL and NTSC"; break;
			}			

			LogOutput( log.String() );
		}
		else if (!memcmp(chunk.id,"PCK",3))
		{
			for (UINT i=0; i < 16; ++i)
			{
				if (chunk.id[3] == DataIds[i])
				{
					success = file.Read( pRomCrcs[i] );
					length = sizeof(U32);

					if (success)
					{
						CHAR id[5];
						memcpy( id, chunk.id, 4 );
						id[4] = '\0';

						log.Resize( 6 ); 
						log += id;
						log += " crc - ";
						log.Append( pRomCrcs[i], PDXSTRING::HEX );
						LogOutput( log.String() );
					}
					break;
				}
			}

			if (!success)
				break;
		}
		else if (!memcmp(chunk.id,"PRG",3))
		{
			for (UINT i=0; i < 16; ++i)
			{
				if (chunk.id[3] == DataIds[i])
				{
					CHAR id[5];
					memcpy( id, chunk.id, 4 );
					id[4] = '\0';

					log.Resize( 6 ); 
					log += id;
					log += " data, ";
					log += (chunk.length / n1k);
					log += "k";
					LogOutput( log.String() );

					const UINT pos = cartridge->pRom.Size();
					cartridge->pRom.Grow( chunk.length );
					success = file.Read( cartridge->pRom.At(pos), cartridge->pRom.End() );
					length = chunk.length;

					if (success && pRomCrcs[i])
					{
						log.Resize( 6 ); 
						log += "crc check ";

						if (pRomCrcs[i] == PDXCRC32::Compute( cartridge->pRom.At(pos), length ))
						{
							log += "ok";
						}
						else
						{
							log += "failed";

							if (!DisableWarning)
							{
								DisableWarning = TRUE;
								cartridge->info.condition = IO::CARTRIDGE::BAD;
								MsgWarning( "PRG-ROM crc check failed! Image may not run properly!" );
							}
						}

						LogOutput( log.String() );
					}
					break;
				}
			}

			if (!success)
				break;
		}
		else if (!memcmp(chunk.id,"CCK",3))
		{
			for (UINT i=0; i < 16; ++i)
			{
				if (chunk.id[3] == DataIds[i])
				{
					success = file.Read( cRomCrcs[i] );
					length = sizeof(U32);

					if (success)
					{
						CHAR id[5];
						memcpy( id, chunk.id, 4 );
						id[4] = '\0';

						log.Resize( 6 ); 
						log += id;
						log += " crc - ";
						log.Append( cRomCrcs[i], PDXSTRING::HEX );
						LogOutput( log.String() );
					}
					break;
				}
			}

			if (!success)
				break;
		}
		else if (!memcmp(chunk.id,"CHR",3))
		{
			for (UINT i=0; i < 16; ++i)
			{
				if (chunk.id[3] == DataIds[i])
				{
					CHAR id[5];
					memcpy( id, chunk.id, 4 );
					id[4] = '\0';

					log.Resize( 6 ); 
					log += id;
					log += " data, ";
					log += (chunk.length / n1k);
					log += "k";
					LogOutput( log.String() );

					const UINT pos = cartridge->cRom.Size();
					cartridge->cRom.Grow( chunk.length );
					success = file.Read( cartridge->cRom.At(pos), cartridge->cRom.End() );
					length = chunk.length;
					
					if (success && cRomCrcs[i])
					{
						log.Resize( 6 ); 
						log += "crc check ";

						if (cRomCrcs[i] == PDXCRC32::Compute( cartridge->cRom.At(pos), length ))
						{
							log += "ok";
						}
						else
						{
							log += "failed";

							if (!DisableWarning)
							{
								DisableWarning = TRUE;
								cartridge->info.condition = IO::CARTRIDGE::BAD;
								MsgWarning( "CHR-ROM crc check failed! Image may not run properly!" );
							}
						}

						LogOutput( log.String() );
					}
					break;
				}
			}

			if (!success)
				break;
		}
		else if ((!memcmp(chunk.id,"BATR",4)))
		{
			cartridge->info.battery = TRUE;

			log.Resize( 6 ); 
			log += "Battery present";
			LogOutput( log.String() );
		}
		else if ((!memcmp(chunk.id,"MAPR",4)))
		{
			if (file.Text().Read( cartridge->info.board ).IsEmpty())
			{
				success = FALSE;
				break;
			}

			if 
			(
		     	!memcmp( cartridge->info.board.String(), "NES-", 4 ) || 
				!memcmp( cartridge->info.board.String(), "UNL-", 4 ) || 
				!memcmp( cartridge->info.board.String(), "HVC-", 4 ) || 
				!memcmp( cartridge->info.board.String(), "BTL-", 4 ) || 
				!memcmp( cartridge->info.board.String(), "BMC-", 4 )
			)
			{
				const PDXSTRING tmp(cartridge->info.board);
				cartridge->info.board = tmp.At(4);
			}

			BOARDS::CONSTITERATOR iterator( boards.Find(cartridge->info.board.Begin()) );

			if (iterator != boards.End())
				cartridge->info.mapper = (*iterator).Second();

			log.Resize( 6 ); 			
			log += "Board: ";
			log += cartridge->info.board;			
			LogOutput( log.String() );

			length = cartridge->info.board.Length() + 1;
		}
		else if ((!memcmp(chunk.id,"MIRR",4)))
		{
			if (!(success=file.Readable(sizeof(U8))))
				break;

			length = sizeof(U8);

			log.Resize( 6 ); 
			log += "Mirroring: ";

			switch (file.Read<U8>())
			{
	     		case 0:  cartridge->info.mirroring = MIRROR_HORIZONTAL; log += "horizontal";                    break;
				case 1:  cartridge->info.mirroring = MIRROR_VERTICAL;   log += "vertical";                      break;
				case 2:  cartridge->info.mirroring = MIRROR_ZERO;       log += "zero";                          break;
				case 3:  cartridge->info.mirroring = MIRROR_ONE;        log += "one";                           break;
				case 4:  cartridge->info.mirroring = MIRROR_FOURSCREEN; log += "four-screen";                   break;
				default: cartridge->info.mirroring = MIRROR_HORIZONTAL; log += "controlled by mapper hardware"; break;
			}

			LogOutput( log.String() );
		}
		else if ((!memcmp(chunk.id,"CTRL",4)))
		{
			if (!(success=file.Readable(sizeof(U8))))
				break;

			length = sizeof(U8);

			log.Resize( 6 ); 
			log += "Controller(s): ";

			const UINT controller = file.Read<U8>();

			if (controller & 0x01) 
			{
				cartridge->info.controllers[0] = CONTROLLER_PAD1;
				cartridge->info.controllers[1] = CONTROLLER_PAD2;
				log += "standard joypad";
			}

			if (controller & 0x02) 
			{
				cartridge->info.controllers[1] = CONTROLLER_ZAPPER;

				if (log.Back() == ' ') log += "Zapper";
				else				   log += ", Zapper";
			}

			if (controller & 0x04) 
			{
				if (log.Back() == ' ') log += "R.O.B";
				else				   log += ", R.O.B";
			}

			if (controller & 0x08) 
			{
				cartridge->info.controllers[0] = CONTROLLER_PADDLE;

				if (log.Back() == ' ') log += "Paddle";
				else				   log += ", Paddle";
			}

			if (controller & 0x10) 
			{
				cartridge->info.controllers[1] = CONTROLLER_POWERPAD;

				if (log.Back() == ' ') log += "Power Pad";
				else				   log += ", Power Pad";
			}

			if (controller & 0x20) 
			{
				cartridge->info.controllers[2] = CONTROLLER_PAD3;
				cartridge->info.controllers[3] = CONTROLLER_PAD4;

				if (log.Back() == ' ') log += "Four-Score adapter";
				else				   log += ", Four-Score adapter";
			}

			if (log.Back() == ' ') 
				log += "not defined";

			LogOutput( log.String() );
		}
		else if ((!memcmp(chunk.id,"VROR",4)))
		{
			cartridge->info.IsCRam = TRUE;

			log.Resize( 6 ); 			
			log += "VRAM override";
			LogOutput( log.String() );
		}

		file.Seek( PDXFILE::CURRENT, chunk.length - length );
	}

	if (cartridge->pRom.IsEmpty())
	{
		log.Resize( 6 ); 
		log += "PRG-ROM is missing.. aborting";
		LogOutput( log.String() );

		return MsgWarning("PRG-ROM is missing!");
	}

	if (!success)
		return MsgWarning("Corrupt file!");

	cartridge->info.crc     = PDXCRC32::Compute( file.Begin(), file.Size() );
	cartridge->info.pRomCrc = PDXCRC32::Compute( cartridge->pRom.Begin(), cartridge->pRom.Size() );
	cartridge->info.cRomCrc = PDXCRC32::Compute( cartridge->cRom.Begin(), cartridge->cRom.Size() );

   #ifdef NES_USE_ROM_DATABASE

	CheckDatabase( cartridge, file, context );

   #endif

	if (cartridge->info.mapper == UINT_MAX)
	{
		PDXSTRING msg;

		msg << "Couldn't find the corresponding mapper to the board: \"";
		msg << cartridge->info.board;
		msg << "\". You may specify which mapper this cartridge use by entering the appropriate number in the field.";

		PDXSTRING choice;

		if (MsgInput("Unsupported board!",msg.String(),choice) && choice.Length() >= 1 && choice.Length() <= 3)
		{
			const INT id = atoi( choice.String() );

			if (id >= 0 && id <= 255)
				cartridge->info.mapper = id;
		}
	}

	if (cartridge->info.mapper == UINT_MAX)
		return PDX_FAILURE;

	cartridge->wRam.Resize( PDX_MAX(cartridge->info.wRam,n8k) );	
	cartridge->wRam.Fill( cartridge->wRam.At(0x0000), cartridge->wRam.At(0x1000), 0x60 );
	cartridge->wRam.Fill( cartridge->wRam.At(0x1000), cartridge->wRam.At(0x2000), 0x70 );
	cartridge->wRam.Fill( cartridge->wRam.At(0x2000), cartridge->wRam.End(),      0x00 );

	cartridge->info.pRom = cartridge->pRom.Size();
	cartridge->info.cRom = cartridge->cRom.Size();
	cartridge->info.wRam = cartridge->wRam.Size();

	cartridge->pRom.Defrag();
	cartridge->cRom.Defrag();
	cartridge->wRam.Defrag();

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#ifdef NES_USE_ROM_DATABASE

PDXRESULT UNIF::CheckDatabase(CARTRIDGE* const cartridge,PDXFILE& file,const IO::GENERAL::CONTEXT& context)
{
	if (database.IsEmpty())
		ImportDatabase();

	PDXMAP<IMAGE,U32>::CONSTITERATOR iterator( database.Find(cartridge->info.pRomCrc) );

	if (iterator != database.End())
	{
		const IMAGE& image = (*iterator).Second();

		cartridge->info.condition = (image.bad ? IO::CARTRIDGE::BAD : IO::CARTRIDGE::GOOD);
		cartridge->info.mapper = image.mapper;

		if (cartridge->info.board.IsEmpty())
			cartridge->info.board = MAPPER::boards[cartridge->info.mapper];

		if      (image.vs)  cartridge->info.system = SYSTEM_VS;
		else if (image.p10) cartridge->info.system = SYSTEM_PC10;

		if (cartridge->wRam.Size() < image.wRamSize)
		{
			cartridge->info.wRam = image.wRamSize;
			cartridge->wRam.Resize( image.wRamSize );
		}
	}

	return PDX_OK;
}

#endif

NES_NAMESPACE_END
