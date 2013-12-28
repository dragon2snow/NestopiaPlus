////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2006 Martin Freij
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

#include <new>
#include "../NstCore.hpp"
#include "NstApiEmulator.hpp"
#include "NstApiMachine.hpp"
#include "NstApiCartridge.hpp"
#include "../NstCartridge.hpp"
#include "../NstImageDatabase.hpp"

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("s", on)
#endif

namespace Nes
{
	namespace Api
	{
		void Cartridge::Info::Clear()
		{
			name.clear();
			maker.clear();
			board.clear();
	
			mapper         = 0;
			pRom           = 0;
			cRom           = 0;
			wRam           = 0;
			isCRam         = false;
			crc            = 0;
			pRomCrc        = 0;
			cRomCrc        = 0;
			system         = SYSTEM_NTSC;
			mirroring      = MIRROR_HORIZONTAL;
			battery        = false;
			trained        = false;
			controllers[0] = Input::PAD1;
			controllers[1] = Input::PAD2;
			controllers[2] = Input::UNCONNECTED;
			controllers[3] = Input::UNCONNECTED;
			controllers[4] = Input::UNCONNECTED;
			condition      = UNKNOWN;
		}
	
		Cartridge::Database Cartridge::GetDatabase()
		{
			return Database(emulator.imageDatabase);
		}
	
		bool Cartridge::Database::IsLoaded() const
		{
			return imageDatabase != NULL;
		}
	
		bool Cartridge::Database::IsEnabled() const
		{
			return imageDatabase && imageDatabase->Enabled();
		}
	
		bool Cartridge::Database::Create()
		{
			if (imageDatabase == NULL)
				imageDatabase = new (std::nothrow) Core::ImageDatabase;
	
			return imageDatabase != NULL;
		}
	
		Result Cartridge::Database::Load(std::istream& stream)
		{
			return Create() ? imageDatabase->Load( &stream ) : RESULT_ERR_OUT_OF_MEMORY;
		}
	
		void Cartridge::Database::Unload()
		{
			if (imageDatabase)
				imageDatabase->Unload();
		}
	
		Result Cartridge::Database::Enable(bool state)
		{
			if (Create())
			{
				if (bool(imageDatabase->Enabled()) != state)
				{
					imageDatabase->Enable( state );
					return RESULT_OK;
				}
	
				return RESULT_NOP;
			}
	
			return RESULT_ERR_OUT_OF_MEMORY;
		}
	
		const Cartridge::Info* Cartridge::GetInfo() const
		{
			if (emulator.Is(Machine::CARTRIDGE))
				return &static_cast<const Core::Cartridge*>(emulator.image)->GetInfo();
	
			return NULL;
		}
	
		Cartridge::Database::Entry Cartridge::Database::FindEntry(ulong crc) const
		{
			return imageDatabase ? imageDatabase->GetHandle( crc ) : NULL;
		}

		Cartridge::Database::Entry Cartridge::Database::FindEntry(const void* data,ulong size,ulong altsize) const
		{
			return imageDatabase ? Core::Cartridge::SearchDatabase( *imageDatabase, data, size, altsize ) : NULL;
		}

		Cartridge::System Cartridge::Database::GetSystem(Entry entry) const 
		{
			return imageDatabase && entry ? imageDatabase->GetSystem( entry ) : SYSTEM_NTSC;
		}
	
		Cartridge::Mirroring Cartridge::Database::GetMirroring(Entry entry) const 
		{ 
			return imageDatabase && entry ? imageDatabase->GetMirroring( entry ) : MIRROR_HORIZONTAL;
		}
	
		ulong Cartridge::Database::GetCrc(Entry entry) const
		{
			return imageDatabase && entry ? imageDatabase->Crc( entry ) : 0;
		}
	
		ulong Cartridge::Database::GetPRomCrc(Entry entry) const
		{
			return imageDatabase && entry ? imageDatabase->pRomCrc( entry ) : 0;
		}
	
		ulong Cartridge::Database::GetPRomSize(Entry entry) const
		{
			return imageDatabase && entry ? imageDatabase->pRomSize( entry ) : 0;
		}
	
		ulong Cartridge::Database::GetCRomSize(Entry entry) const
		{
			return imageDatabase && entry ? imageDatabase->cRomSize( entry ) : 0;
		}
	
		ulong Cartridge::Database::GetWRamSize(Entry entry) const
		{
			return imageDatabase && entry ? imageDatabase->wRamSize( entry ) : 0;
		}
	
		uint Cartridge::Database::GetMapper(Entry entry) const
		{
			return imageDatabase && entry ? imageDatabase->Mapper( entry ) : 0;
		}
	
		bool Cartridge::Database::HasBattery(Entry entry) const
		{
			return imageDatabase && entry && imageDatabase->HasBattery( entry );
		}
	
		bool Cartridge::Database::HasTrainer(Entry entry) const
		{
			return imageDatabase && entry && imageDatabase->HasTrainer( entry );
		}
	
		bool Cartridge::Database::IsBad(Entry entry) const
		{
			return imageDatabase && entry && imageDatabase->IsBad( entry );
		}
	}
}

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif
