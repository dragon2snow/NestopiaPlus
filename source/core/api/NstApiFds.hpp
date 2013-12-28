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
			struct DiskCaller;
			struct DriveCaller;

		public:

			template<typename T>
			Fds(T& e)
			: Base(e) {}

			enum
			{
				NO_DISK = -1
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

			enum Event
			{
				DISK_INSERT,
				DISK_EJECT,
				DISK_NONSTANDARD
			};

			enum Motor
			{
				MOTOR_OFF,
				MOTOR_READ,
				MOTOR_WRITE
			};

			enum
			{
				NUM_DISK_CALLBACKS = 3,
				NUM_DRIVE_CALLBACKS = 3
			};

			typedef void (NST_CALLBACK *DiskCallback)(UserData,Event,uint,uint);
			typedef void (NST_CALLBACK *DriveCallback)(UserData,Motor);

			static DiskCaller diskCallback;
			static DriveCaller driveCallback;
		};

		struct Fds::DiskCaller : Core::UserCallback<Fds::DiskCallback>
		{
			void operator () (Event event,uint disk,uint side) const
			{
				if (function)
					function( userdata, event, disk, side );
			}
		};

		struct Fds::DriveCaller : Core::UserCallback<Fds::DriveCallback>
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
