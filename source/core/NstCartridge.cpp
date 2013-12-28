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
#include "NstInes.h"
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

	{
		INES ines;
		PDX_TRY(ines.Import( this, file, context ));
	}

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
	context.wRamInitSize = info.wRam;
	context.mirroring    = info.mirroring;

	if (!(mapper = MAPPER::New(context)))
		return MsgWarning("Unsupported mapper!");

	return PDX_OK;
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
