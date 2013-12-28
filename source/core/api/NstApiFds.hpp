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

#ifndef NST_API_FDS_H
#define NST_API_FDS_H

#include <iosfwd>
#include <vector>
#include "NstApi.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#if NST_ICC >= 810
#pragma warning( push )
#pragma warning( disable : 304 444 )
#elif NST_MSVC >= 1200
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

			template<typename T>
			Fds(T& e)
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

			bool IsAnyDiskInserted() const throw();

			Result InsertDisk(uint,uint) throw();
			Result ChangeSide() throw();
			Result EjectDisk() throw();

			Result SetBIOS(std::istream*) throw();
			Result GetBIOS(std::ostream&) const throw();
			bool HasBIOS() const throw();

			uint GetNumDisks() const throw();
			uint GetNumSides() const throw();
			int GetCurrentDisk() const throw();
			int GetCurrentDiskSide() const throw();
			bool CanChangeDiskSide() const throw();
			bool HasHeader() const throw();

			struct DiskData
			{
				DiskData() throw();
				~DiskData() throw();

				typedef std::vector<uchar> Data;

				struct File
				{
					File() throw();

					enum Type
					{
						TYPE_UNKNOWN,
						TYPE_PRG,
						TYPE_CHR,
						TYPE_NMT
					};

					uchar id;
					uchar index;
					ushort address;
					Type type;
					Data data;
					char name[12];
				};

				typedef std::vector<File> Files;

				Files files;
				Data raw;
			};

			Result GetDiskData(uint,DiskData&) const throw();

			enum Motor
			{
				MOTOR_OFF,
				MOTOR_READ,
				MOTOR_WRITE
			};

			typedef void (NST_CALLBACK *DiskChangeCallback)(UserData,Event,uint,uint);
			typedef void (NST_CALLBACK *DiskAccessLampCallback)(UserData,Motor);

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
			void operator () (Motor motor) const
			{
				if (function)
					function( userdata, motor );
			}
		};
	}
}

#if NST_MSVC >= 1200 || NST_ICC >= 810
#pragma warning( pop )
#endif

#endif
