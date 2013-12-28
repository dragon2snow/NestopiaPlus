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

#include "../NstCore.hpp"
#include "NstApiEmulator.hpp"
#include "NstApiMachine.hpp"
#include "NstApiFds.hpp"
#include "../NstFds.hpp"

namespace Nes
{
	namespace Api
	{
		Fds::DiskChange Fds::diskChangeCallback;
		Fds::Lamp Fds::diskAccessLampCallback;

		bool Fds::IsAnyDiskInserted() const
		{
			if (emulator.Is(Machine::DISK))
				return static_cast<const Core::Fds*>(emulator.image)->IsAnyDiskInserted();
	
			return false;
		}
	
		Result Fds::InsertDisk(uint disk,uint side)
		{
			if (emulator.Is(Machine::DISK) && !emulator.tracker.IsLocked())
			{
				const Result result = static_cast<Core::Fds*>(emulator.image)->InsertDisk( disk, side );

				if (result == RESULT_OK)
					emulator.tracker.Flush();

				return result;
			}

			return RESULT_ERR_NOT_READY;
		}
	
		Result Fds::ChangeSide()
		{
			const int disk = GetCurrentDisk();

			if (disk != NO_DISK)
				return InsertDisk( disk, GetCurrentDiskSide() == 0 ? 1 : 0 );

			return RESULT_ERR_NOT_READY;
		}

		Result Fds::EjectDisk()
		{
			if (emulator.Is(Machine::DISK) && !emulator.tracker.IsLocked())
			{
				const Result result = static_cast<Core::Fds*>(emulator.image)->EjectDisk();

				if (result == RESULT_OK)
					emulator.tracker.Flush();

				return result;
			}

			return RESULT_ERR_NOT_READY;
		}
	
		Result Fds::SetBIOS(std::istream* const stream)
		{
			return Core::Fds::Bios::Set( stream );
		}
	
		Result Fds::GetBIOS(std::ostream& stream) const
		{
			return Core::Fds::Bios::Get( &stream );
		}
	
		uint Fds::GetNumDisks() const
		{
			if (emulator.Is(Machine::DISK))
				return static_cast<const Core::Fds*>(emulator.image)->NumDisks();
	
			return 0;
		}
	
		uint Fds::GetNumSides() const
		{
			if (emulator.Is(Machine::DISK))
				return static_cast<const Core::Fds*>(emulator.image)->NumSides();
	
			return 0;
		}
	
		int Fds::GetCurrentDisk() const
		{
			if (emulator.Is(Machine::DISK))
				return static_cast<const Core::Fds*>(emulator.image)->CurrentDisk();
	
			return NO_DISK;
		}
	
		int Fds::GetCurrentDiskSide() const
		{
			if (emulator.Is(Machine::DISK))
				return static_cast<const Core::Fds*>(emulator.image)->CurrentDiskSide();
	
			return NO_DISK;
		}
	
		bool Fds::HasHeader() const
		{
			return emulator.Is(Machine::DISK) && static_cast<const Core::Fds*>(emulator.image)->HasHeader();
		}
	}
}
