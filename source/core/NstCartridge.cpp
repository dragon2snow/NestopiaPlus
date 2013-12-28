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

#include "../paradox/PdxFile.h"
#include "NstTypes.h"
#include "NstCartridge.h"
#include "NstImageFile.h"
#include "NstInes.h"
#include "NstUnif.h"
#include "mapper/NstMappers.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
// constructor
////////////////////////////////////////////////////////////////////////////////////////

CARTRIDGE::CARTRIDGE()
: mapper(NULL) 
{
	info.Reset();
}

////////////////////////////////////////////////////////////////////////////////////////
// destructor
////////////////////////////////////////////////////////////////////////////////////////

CARTRIDGE::~CARTRIDGE()
{
	Unload();
}

////////////////////////////////////////////////////////////////////////////////////////
// load rom image
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT CARTRIDGE::Load(PDXFILE& file,CPU* const cpu,PPU* const ppu,const IO::GENERAL::CONTEXT& c)
{
	context = c;

	Unload();

	info.Reset();
	info.file = file.Name();

	if (file.Peek<U32>() == 0x1A53454EUL)
	{
		INES ines;
		PDX_TRY(ines.Import( this, file, context ));
	}
	else
	{
		UNIF unif;
		PDX_TRY(unif.Import( this, file, context ));
	}

	if (info.system != SYSTEM_VS)
		DetectVS();

	DetectControllers();
	DetectMirroring();

	if (!info.battery)
		DetectBattery();

	if (info.battery)
		LoadBatteryRam();

	delete mapper;

	MAPPER::CONTEXT context;

	context.id           = info.mapper;
	context.cpu          = cpu;
	context.ppu          = ppu;
	context.pRom         = pRom.Begin();
	context.cRom         = cRom.Begin();
	context.wRam         = wRam.Begin();
	context.pRomSize     = pRom.Size();
	context.cRomSize     = cRom.Size();
	context.wRamSize     = wRam.Size();
	context.IsCRam       = info.IsCRam;
	context.wRamInitSize = info.wRam;
	context.mirroring    = info.mirroring;

	if (!(mapper = MAPPER::New(context)))
		return MsgWarning("Unsupported mapper!");

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID CARTRIDGE::DetectMirroring()
{
	if (info.mirroring == MIRROR_FOURSCREEN)
	{
		switch (info.mapper)
		{
     		case  24:
     		case  26:
       		case 118:

		    	info.mirroring = MIRROR_HORIZONTAL;
				LogOutput( "CARTRIDGE: horizontal mirroring forced" );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID CARTRIDGE::DetectBattery()
{
	switch (info.pRomCrc)
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

			info.battery = TRUE;
			LogOutput( "CARTRIDGE: battery present forced" );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID CARTRIDGE::DetectControllers()
{
	switch (info.pRomCrc)
	{
		case 0xFBFC6A6CUL: // Adventures of Bayou Billy, The (E)
		case 0xCB275051UL: // Adventures of Bayou Billy, The (U)
		case 0xFB69C131UL: // Baby Boomer (Unl) (U)
		case 0xF2641AD0UL: // Barker Bill's Trick Shooting (U)
		case 0xBC1DCE96UL: // Chiller (Unl) (U)
		case 0x90CA616DUL: // Duck Hunt (JUE)
		case 0x59E3343FUL: // Freedom Force (U)
		case 0x242A270CUL: // Gotcha! (U)
		case 0x7B5BD2DEUL: // Gumshoe (UE)
		case 0x255B129CUL: // Gun Sight (J)
		case 0x8963AE6EUL: // Hogan's Alley (JU)
		case 0x51D2112FUL: // Laser Invasion (U)
		case 0x0A866C94UL: // Lone Ranger, The (U)
		case 0xE4C04EEAUL: // Mad City (J)
		case 0x9EEF47AAUL: // Mechanized Attack (U)
		case 0xC2DB7551UL: // Shooting Range (U)
		case 0x163E86C0UL: // To The Earth (U)
		case 0x389960DBUL: // Wild Gunman (JUE)
		case 0xED588f00UL: // VS Duck Hunt
		case 0x17AE56BEUL: // VS Freedom Force
		case 0xFF5135A3UL: // VS Hogan's Alley

			info.controllers[0] = CONTROLLER_PAD1;
			info.controllers[1] = CONTROLLER_ZAPPER;
			info.controllers[2] = CONTROLLER_UNCONNECTED;
			info.controllers[3] = CONTROLLER_UNCONNECTED;
			info.controllers[4] = CONTROLLER_UNCONNECTED;
			break;

		case 0x35893B67UL: // Arkanoid (J)
		case 0x6267FBD1UL: // Arkanoid 2 (J)

			info.controllers[0] = CONTROLLER_PADDLE;
			info.controllers[1] = CONTROLLER_PAD2;
			info.controllers[2] = CONTROLLER_UNCONNECTED;
			info.controllers[3] = CONTROLLER_UNCONNECTED;
			info.controllers[4] = CONTROLLER_UNCONNECTED;
			break;

		case 0xBC5F6C94UL: // Athletic World (U)
		case 0xD836A90BUL: // Dance Aerobics (U)
		case 0x96C4CE38UL: // Short Order - Eggsplode (U)
		case 0x987DCDA3UL: // Street Cop (U)

			info.controllers[0] = CONTROLLER_PAD1;
			info.controllers[1] = CONTROLLER_POWERPAD;
			info.controllers[2] = CONTROLLER_UNCONNECTED;
			info.controllers[3] = CONTROLLER_UNCONNECTED;
			info.controllers[4] = CONTROLLER_UNCONNECTED;
			break;

		case 0xF9DEF527UL: // Family BASIC (Ver 2.0)
		case 0xDE34526EUL: // Family BASIC (Ver 2.1a)
		case 0xF050B611UL: // Family BASIC (Ver 3)
		case 0x3AAEED3FUL: // Family BASIC (Ver 3) (Alt)

			info.controllers[0] = CONTROLLER_PAD1;
			info.controllers[1] = CONTROLLER_PAD2;
			info.controllers[2] = CONTROLLER_UNCONNECTED;
			info.controllers[3] = CONTROLLER_UNCONNECTED;
			info.controllers[4] = CONTROLLER_KEYBOARD;
			break;

		case 0x3B997543UL: // Gauntlet 2 (E)
		case 0x2C609B52UL: // Gauntlet 2 (U)
		case 0x1352F1B9UL: // Greg Norman's Golf Power (U) 
		case 0xAB8371F6UL: // Kings of the Beach (U)
		case 0x0939852FUL: // M.U.L.E. (U)
		case 0xDA2CB59AUL: // Nightmare on Elm Street, A (U)
		case 0x9EDD2159UL: // R.C. Pro-Am 2 (U)
		case 0xD3428E2EUL: // Super Jeopardy! (U)
		case 0x15E98A14UL: // Super Spike V'Ball (U)
		case 0xEBB9DF3DUL: // Super Spike V'Ball (E)
		case 0x35190A3FUL: // Super Spike V'Ball - Nintendo World Cup (U)
		case 0x3417EC46UL: // Swords and Serpents (U)
		case 0xD153CAF6UL: // Swords and Serpents (E)

			info.controllers[0] = CONTROLLER_PAD1;
			info.controllers[1] = CONTROLLER_PAD2;
			info.controllers[2] = CONTROLLER_PAD3;
			info.controllers[3] = CONTROLLER_PAD4;
			info.controllers[4] = CONTROLLER_UNCONNECTED;
			break;
	}	
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID CARTRIDGE::DetectVS()
{
	switch (info.pRomCrc)
	{
		// Uni System
		case 0xEB2DBA63UL: // TKO Boxing
		case 0x9818F656UL: // -||-
		case 0x135ADF7CUL: // RBI Baseball
		case 0xED588F00UL: // Duck Hunt
		case 0x16D3F469UL: // Ninja Jajamaru Kun (J)
		case 0x8850924BUL: // Tetris
		case 0x8C0C2DF5UL: // Top Gun
		case 0x70901B25UL: // Slalom
		case 0xCF36261EUL: // Sky Kid
		case 0xE1AA8214UL: // Star Luster
		case 0xD5D7EAC4UL: // Dr. Mario
		case 0xFFBEF374UL: // Castlevania
		case 0xE2C0A2BEUL: // Platoon
		case 0xCBE85490UL: // Excitebike
		case 0x29155E0CUL: // Excitebike (alt)
		case 0x07138C06UL: // Clu Clu Land
		case 0x43A357EFUL: // Ice Climber
		case 0x737DD1BFUL: // Super Mario Bros	
		case 0x4BF3972DUL: // -||-
		case 0x8B60CC58UL: // -||-
		case 0x8192C804UL: // -||-
		case 0xEC461DB9UL: // Pinball
		case 0xE528F651UL: // Pinball (alt)
		case 0x0B65A917UL: // Mach Rider
		case 0x8A6A9848UL: // -||-
		case 0x46914E3EUL: // Soccer
		case 0x70433F2CUL: // Battle City
		case 0x8D15A6E6UL: // bad .nes
		case 0xD99A2087UL: // Gradius
		case 0x1E438D52UL: // Goonies
		case 0xFF5135A3UL: // Hogan's Alley
		case 0x17AE56BEUL: // Freedom Force
		case 0xF9D3B0A3UL: // Super Xevious
		case 0x66BB838FUL: // Super Xevious
		case 0xCC2C4B5DUL: // Golf
		case 0x86167220UL: // Lady Golf
//		case 0xB90497AAUL: // Tennis

		// Dual System
		case 0xB90497AAUL: // Tennis
		case 0x008A9C16UL: // Wrecking Crew 
		case 0xAD407F52UL: // Balloon Fight
		case 0x18A93B7BUL: // Mahjong (J)
		case 0x13A91937UL: // Baseball 
		case 0xF5DEBF88UL: // Baseball 
		case 0xF64D7252UL: // Baseball 
		case 0x968A6E9DUL: // Baseball
		case 0xF42DAB14UL: // Ice Climber
		case 0x7D6B764FUL: // Ice Climber

			info.system = SYSTEM_VS;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// unload rom image
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT CARTRIDGE::Unload()
{
	if (info.battery && !context.DisableSaveRamWrite)
		SaveBatteryRam();

	delete mapper;
	mapper = NULL;

	pRom.Destroy();
	cRom.Destroy();
	wRam.Destroy();

	info.Reset();

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// reset cartridge
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT CARTRIDGE::Reset(const BOOL hard)
{
	if (info.battery && !context.DisableSaveRamWrite)
		SaveBatteryRam();

	if (mapper)
		mapper->Reset( hard );

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// load the battery ram file
////////////////////////////////////////////////////////////////////////////////////////

VOID CARTRIDGE::LoadBatteryRam()
{
	PDXFILE file( PDXFILE::INPUT );

	PDXSTRING SaveName( info.file );
	SaveName.ReplaceFileExtension( "sav" );

	if ((context.LookInRomPathForSaveRam || context.SaveRamPath.IsEmpty()) && PDX_SUCCEEDED(file.Open( SaveName )))
	{
		LoadBatteryRam( file );
	}
	else if (context.SaveRamPath.Size())
	{
		SaveName.ReplaceFilePath( context.SaveRamPath );

		if (PDX_SUCCEEDED(file.Open( SaveName )))
			LoadBatteryRam( file );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// load the battery ram file
////////////////////////////////////////////////////////////////////////////////////////

VOID CARTRIDGE::LoadBatteryRam(PDXFILE& file)
{
	PDX_ASSERT(file.IsOpen());

	BOOL yep = FALSE;

	if (file.Size())
	{
		const UINT size = PDX_MIN( wRam.Size(), file.Size() - file.Position() );
	
		yep = 
		(
        	size >= n8k ||
			MsgQuestion("Spooky Save File","Save file may be invalid! Sure you want to load it in?")
		);

		if (yep)
		{
			if (size < n8k)
				LogOutput("CARTRIDGE: warning, battery-backup ram file is less than 8k in size!");

			file.Read( wRam.At(0), wRam.At(size) );
		}
	}

	PDXSTRING log;

	if (yep)
	{
		log  = "CARTRIDGE: battery-backup ram was read from \"";
		log += file.Name();
		log += "\"";
	}
	else
	{
		log = "CARTRIDGE: battery-backup ram was not read from any file";
	}

	LogOutput( log );
}

////////////////////////////////////////////////////////////////////////////////////////
// save the battery-backup RAM to a file
////////////////////////////////////////////////////////////////////////////////////////

VOID CARTRIDGE::SaveBatteryRam() const
{
	PDXFILE file( PDXFILE::OUTPUT );

	PDXSTRING SaveName( info.file );
	SaveName.ReplaceFileExtension( "sav" );

	BOOL groovie = FALSE;

	if ((context.LookInRomPathForSaveRam || context.SaveRamPath.IsEmpty()) && PDX_SUCCEEDED(file.Open( SaveName )))
	{
		SaveBatteryRam( file );
		groovie = TRUE;
	}
	else if (context.SaveRamPath.Size())
	{
		SaveName.ReplaceFilePath( context.SaveRamPath );

		if (PDX_SUCCEEDED(file.Open( SaveName )))
		{
			SaveBatteryRam( file );
			groovie = TRUE;
		}
	}

	PDXSTRING log;

	if (groovie)
	{
		log  = "CARTRIDGE: battery-backup ram was written to \"";
		log += file.Name();
		log += "\"";
	}
	else
	{
		log = "CARTRIDGE: warning, battery-backup ram was not written to any file";
	}

	LogOutput( log );
}

////////////////////////////////////////////////////////////////////////////////////////
// save the battery-backup RAM to a file
////////////////////////////////////////////////////////////////////////////////////////

VOID CARTRIDGE::SaveBatteryRam(PDXFILE& file) const
{
	PDX_ASSERT( file.IsOpen() );
	file.Write( wRam.Begin(), wRam.End() );
}

NES_NAMESPACE_END
