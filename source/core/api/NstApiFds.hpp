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

#ifndef NST_API_FDS_H
#define NST_API_FDS_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include <iosfwd>
#include "NstApi.hpp"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4512 )
#endif

namespace Nes
{
	namespace Api
	{
		class Fds : public Base
		{
			struct DiskChange;
			struct Lamp;

		public:
	
			Fds(Emulator& e)
			: Base(e) {}
			
			enum 
			{
				NO_DISK = -1
			};

			enum Event
			{
				DISK_INSERT,
				DISK_EJECT
			};
	
			bool IsAnyDiskInserted() const;
	
			Result InsertDisk(uint,uint);
			Result ChangeSide();
			Result EjectDisk();
	
			Result SetBIOS(std::istream*);
			Result GetBIOS(std::ostream&) const;
	
			uint GetNumDisks() const;
			uint GetNumSides() const;
			int GetCurrentDisk() const;
			int GetCurrentDiskSide() const;
			bool HasHeader() const;
	
			typedef void (NST_CALLBACK *DiskChangeCallback)(UserData,Event,uint,uint);
			typedef void (NST_CALLBACK *DiskAccessLampCallback)(UserData,bool);
	
			static Lamp diskAccessLampCallback;
			static DiskChange diskChangeCallback;
		};

		struct Fds::DiskChange : Core::UserCallback<Fds::DiskChangeCallback>
		{
			void operator () (Event event,uint disk,uint side) const
			{
				if (function)
					function( userdata, event, disk, side );
			}
		};

		struct Fds::Lamp : Core::UserCallback<Fds::DiskAccessLampCallback>
		{
			void operator () (bool on) const
			{
				if (function)
					function( userdata, on );
			}
		};
	}
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
