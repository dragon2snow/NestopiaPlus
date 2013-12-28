////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
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

#ifndef NST_API_CARTRIDGE_H
#define NST_API_CARTRIDGE_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include <iosfwd>
#include <string>
#include "NstApiInput.hpp"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4512 )
#endif

namespace Nes
{
	namespace Core
	{
		class ImageDatabase;
	}

	namespace Api
	{
		class Cartridge : public Base
		{
		public:
	
			Cartridge(Emulator& e)
			: Base(e) {}
	
			enum Mirroring
			{
				MIRROR_HORIZONTAL,
				MIRROR_VERTICAL,
				MIRROR_FOURSCREEN,
				MIRROR_ZERO,
				MIRROR_ONE,
				MIRROR_CONTROLLED
			};
	
			enum System
			{
				SYSTEM_NTSC,
				SYSTEM_PAL,
				SYSTEM_NTSC_PAL,
				SYSTEM_VS,
				SYSTEM_PC10
			};
	
			enum State
			{
				YES     = +1,
				NO      =  0,
				UNKNOWN = -1
			};
	
			class Database
			{
				friend class Cartridge;
	
				Core::ImageDatabase*& imageDatabase;
	
				Database(Core::ImageDatabase*& idb)
				: imageDatabase(idb) {}
	
				bool Create();
	
			public:
	
				typedef const void* Entry;
	
				Result Load(std::istream&);
				void   Unload();
				Result Enable(bool=true);
				bool   IsEnabled() const;
				bool   IsLoaded() const;
				Entry  FindEntry(ulong) const;
				Entry  FindEntry(const void*,ulong,ulong=0) const;
	
				const char* GetName      (Entry) const;
				System      GetSystem    (Entry) const; 
				Mirroring   GetMirroring (Entry) const; 
				ulong       GetCrc       (Entry) const; 
				ulong       GetPRomCrc   (Entry) const; 
				ulong       GetPRomSize  (Entry) const; 
				ulong       GetCRomSize  (Entry) const; 
				ulong       GetWRamSize  (Entry) const; 
				uint        GetMapper    (Entry) const; 
				bool        HasBattery   (Entry) const; 
				bool        HasTrainer   (Entry) const; 
				bool        IsBad        (Entry) const; 
			};
	
			Database GetDatabase();
			
			struct Info
			{
				void Clear();
	
				std::string name;
				std::string maker;
				std::string board;
				uint        mapper;
				ulong       pRom;
				ulong       cRom;
				ulong       wRam;
				bool        isCRam;
				ulong       crc;
				ulong       pRomCrc;
				ulong       cRomCrc;
				System      system;
				Mirroring   mirroring;
				bool        battery;
				bool        trained;		
				Input::Type controllers[5];
				State       condition;
			};
	
			const Info* GetInfo() const;
		};
	}
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
