////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2007 Martin Freij
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
#include "../NstStream.hpp"
#include "../NstCartridge.hpp"
#include "../NstImageDatabase.hpp"
#include "../NstCartridgeInes.hpp"
#include "NstApiMachine.hpp"

namespace Nes
{
	namespace Api
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

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
			cpu = CPU_RP2A03;
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

		Cartridge::Info::~Info() throw()
		{
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
			return Core::Cartridge::Ines::ReadHeader( setup, static_cast<const byte*>(data), length );
		}

		Result NST_CALL Cartridge::WriteNesHeader(const Setup& setup,void* data,ulong length) throw()
		{
			return Core::Cartridge::Ines::WriteHeader( setup, static_cast<byte*>(data), length );
		}

		bool Cartridge::Database::IsLoaded() const throw()
		{
			return emulator.imageDatabase;
		}

		bool Cartridge::Database::IsEnabled() const throw()
		{
			return emulator.imageDatabase && emulator.imageDatabase->Enabled();
		}

		bool Cartridge::Database::Create()
		{
			if (emulator.imageDatabase == NULL)
				emulator.imageDatabase = new (std::nothrow) Core::ImageDatabase;

			return emulator.imageDatabase;
		}

		Result Cartridge::Database::Load(std::istream& stream) throw()
		{
			return Create() ? emulator.imageDatabase->Load( &stream ) : RESULT_ERR_OUT_OF_MEMORY;
		}

		void Cartridge::Database::Unload() throw()
		{
			if (emulator.imageDatabase)
				emulator.imageDatabase->Unload();
		}

		Result Cartridge::Database::Enable(bool state) throw()
		{
			if (Create())
			{
				if (emulator.imageDatabase->Enabled() != state)
				{
					emulator.imageDatabase->Enable( state );
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
			return emulator.imageDatabase ? emulator.imageDatabase->Search( crc ) : NULL;
		}

		Cartridge::Database::Entry Cartridge::Database::FindEntry(const void* file,ulong length) const throw()
		{
			return emulator.imageDatabase ? Core::Cartridge::Ines::SearchDatabase( *emulator.imageDatabase, static_cast<const byte*>(file), length ) : NULL;
		}

		Cartridge::System Cartridge::Database::GetSystem(Entry entry) const throw()
		{
			return emulator.imageDatabase && entry ? emulator.imageDatabase->GetSystem( entry ) : SYSTEM_HOME;
		}

		Cartridge::Region Cartridge::Database::GetRegion(Entry entry) const throw()
		{
			return emulator.imageDatabase && entry ? emulator.imageDatabase->GetRegion( entry ) : REGION_NTSC;
		}

		Cartridge::Mirroring Cartridge::Database::GetMirroring(Entry entry) const throw()
		{
			return emulator.imageDatabase && entry ? emulator.imageDatabase->GetMirroring( entry ) : MIRROR_HORIZONTAL;
		}

		ulong Cartridge::Database::GetCrc(Entry entry) const throw()
		{
			return emulator.imageDatabase && entry ? emulator.imageDatabase->Crc( entry ) : 0;
		}

		ulong Cartridge::Database::GetPrgRom(Entry entry) const throw()
		{
			return emulator.imageDatabase && entry ? emulator.imageDatabase->PrgRom( entry ) : 0;
		}

		ulong Cartridge::Database::GetWrkRam(Entry entry) const throw()
		{
			return emulator.imageDatabase && entry ? emulator.imageDatabase->WrkRam( entry ) : 0;
		}

		ulong Cartridge::Database::GetWrkRamBacked(Entry entry) const throw()
		{
			return emulator.imageDatabase && entry ? emulator.imageDatabase->WrkRamBacked( entry ) : 0;
		}

		ulong Cartridge::Database::GetChrRom(Entry entry) const throw()
		{
			return emulator.imageDatabase && entry ? emulator.imageDatabase->ChrRom( entry ) : 0;
		}

		ulong Cartridge::Database::GetChrRam(Entry entry) const throw()
		{
			return emulator.imageDatabase && entry ? emulator.imageDatabase->ChrRam( entry ) : 0;
		}

		ulong Cartridge::Database::GetChrRamBacked(Entry entry) const throw()
		{
			return emulator.imageDatabase && entry ? emulator.imageDatabase->ChrRamBacked( entry ) : 0;
		}

		uint Cartridge::Database::GetMapper(Entry entry) const throw()
		{
			return emulator.imageDatabase && entry ? emulator.imageDatabase->Mapper( entry ) : 0;
		}

		bool Cartridge::Database::HasTrainer(Entry entry) const throw()
		{
			return emulator.imageDatabase && entry ? emulator.imageDatabase->Trainer( entry ) : false;
		}

		Cartridge::Condition Cartridge::Database::GetCondition(Entry entry) const throw()
		{
			return emulator.imageDatabase && entry ? emulator.imageDatabase->GetCondition( entry ) : DUMP_UNKNOWN;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
