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
#include "../NstMachine.hpp"
#include "../NstCartridge.hpp"
#include "../NstLog.hpp"
#include "../NstImageDatabase.hpp"
#include "../NstCartridgeInes.hpp"
#include "NstApiMachine.hpp"

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("s", on)
#endif

namespace Nes
{
	namespace Api
	{
		Cartridge::Setup::Setup() throw()
		{
			Clear();
		}

		void Cartridge::Setup::Clear() throw()
		{
			system = SYSTEM_HOME;
			region = REGION_NTSC;
			prgRom = 0;
			wrkRam = 0;
			wrkRamBacked = 0;
			chrRom = 0;
			chrRam = 0;
			chrRamBacked = 0;
			ppu = PPU_RP2C02;
			mirroring = MIRROR_HORIZONTAL;
			mapper = 0;
			subMapper = 0;
			security = 0;
			version = 0;
			trainer = false;
			wrkRamAuto = false;
		}

		Cartridge::Info::Info() throw()
		{
			Clear();
		}

		void Cartridge::Info::Clear() throw()
		{
			name.clear();
			maker.clear();
			board.clear();

			crc            = 0;
			prgCrc         = 0;
			chrCrc         = 0;
			controllers[0] = Input::PAD1;
			controllers[1] = Input::PAD2;
			controllers[2] = Input::UNCONNECTED;
			controllers[3] = Input::UNCONNECTED;
			controllers[4] = Input::UNCONNECTED;
			adapter        = Input::ADAPTER_NES;
			condition      = DUMP_UNKNOWN;

			setup.Clear();
		}

		Result NST_CALL Cartridge::ReadNesHeader(Setup& setup,const void* const data,const ulong length) throw()
		{
			return Core::Cartridge::Ines::ReadHeader( setup, data, length );
		}

		Result NST_CALL Cartridge::WriteNesHeader(const Setup& setup,void* data,ulong length) throw()
		{
			return Core::Cartridge::Ines::WriteHeader( setup, data, length );
		}

		Cartridge::Database Cartridge::GetDatabase() throw()
		{
			return Database(emulator.imageDatabase);
		}

		bool Cartridge::Database::IsLoaded() const throw()
		{
			return imageDatabase != NULL;
		}

		bool Cartridge::Database::IsEnabled() const throw()
		{
			return imageDatabase && imageDatabase->Enabled();
		}

		bool Cartridge::Database::Create()
		{
			if (imageDatabase == NULL)
				imageDatabase = new (std::nothrow) Core::ImageDatabase;

			return imageDatabase != NULL;
		}

		Result Cartridge::Database::Load(std::istream& stream) throw()
		{
			return Create() ? imageDatabase->Load( &stream ) : RESULT_ERR_OUT_OF_MEMORY;
		}

		void Cartridge::Database::Unload() throw()
		{
			if (imageDatabase)
				imageDatabase->Unload();
		}

		Result Cartridge::Database::Enable(bool state) throw()
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

		const Cartridge::Info* Cartridge::GetInfo() const throw()
		{
			if (emulator.Is(Machine::CARTRIDGE))
				return &static_cast<const Core::Cartridge*>(emulator.image)->GetInfo();

			return NULL;
		}

		Cartridge::Database::Entry Cartridge::Database::FindEntry(ulong crc) const throw()
		{
			return imageDatabase ? imageDatabase->Search( crc ) : NULL;
		}

		Cartridge::Database::Entry Cartridge::Database::FindEntry(const void* file,ulong length) const throw()
		{
			return imageDatabase ? Core::Cartridge::SearchDatabase( *imageDatabase, file, length ) : NULL;
		}

		System Cartridge::Database::GetSystem(Entry entry) const throw()
		{
			return imageDatabase && entry ? imageDatabase->GetSystem( entry ) : SYSTEM_HOME;
		}

		Region Cartridge::Database::GetRegion(Entry entry) const throw()
		{
			return imageDatabase && entry ? imageDatabase->GetRegion( entry ) : REGION_NTSC;
		}

		Cartridge::Mirroring Cartridge::Database::GetMirroring(Entry entry) const throw()
		{
			return imageDatabase && entry ? imageDatabase->GetMirroring( entry ) : MIRROR_HORIZONTAL;
		}

		ulong Cartridge::Database::GetCrc(Entry entry) const throw()
		{
			return imageDatabase && entry ? imageDatabase->Crc( entry ) : 0;
		}

		ulong Cartridge::Database::GetPrgRom(Entry entry) const throw()
		{
			return imageDatabase && entry ? imageDatabase->PrgRom( entry ) : 0;
		}

		ulong Cartridge::Database::GetWrkRam(Entry entry) const throw()
		{
			return imageDatabase && entry ? imageDatabase->WrkRam( entry ) : 0;
		}

		ulong Cartridge::Database::GetWrkRamBacked(Entry entry) const throw()
		{
			return imageDatabase && entry ? imageDatabase->WrkRamBacked( entry ) : 0;
		}

		ulong Cartridge::Database::GetChrRom(Entry entry) const throw()
		{
			return imageDatabase && entry ? imageDatabase->ChrRom( entry ) : 0;
		}

		ulong Cartridge::Database::GetChrRam(Entry entry) const throw()
		{
			return imageDatabase && entry ? imageDatabase->ChrRam( entry ) : 0;
		}

		ulong Cartridge::Database::GetChrRamBacked(Entry entry) const throw()
		{
			return imageDatabase && entry ? imageDatabase->ChrRamBacked( entry ) : 0;
		}

		uint Cartridge::Database::GetMapper(Entry entry) const throw()
		{
			return imageDatabase && entry ? imageDatabase->Mapper( entry ) : 0;
		}

		bool Cartridge::Database::HasTrainer(Entry entry) const throw()
		{
			return imageDatabase && entry && imageDatabase->Trainer( entry );
		}

		Cartridge::Condition Cartridge::Database::GetCondition(Entry entry) const throw()
		{
			return imageDatabase && entry ? imageDatabase->Condition( entry ) : DUMP_UNKNOWN;
		}
	}
}

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif
