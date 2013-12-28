////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2005 Martin Freij
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

#ifndef NST_API_MACHINE_H
#define NST_API_MACHINE_H

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
		class Machine : public Base
		{
		public:
	
			Machine(Emulator& e)
			: Base(e) {}
	
			enum 
			{
				ON        = 0x001,
				VS        = 0x010,
				CARTRIDGE = 0x040,
				DISK      = 0x080,
				SOUND     = 0x100,
				GAME      = CARTRIDGE|DISK|VS,
				IMAGE     = GAME|SOUND
			};
	
			enum Mode
			{
				NTSC = 0x04,
				PAL  = 0x08
			};
	
			Result Load          (std::istream&);
			Result LoadCartridge (std::istream&);
			Result LoadDisk      (std::istream&);
			Result LoadSound     (std::istream&);
			Result Unload        ();
	
			Result Power (bool);
			Result Reset (bool);
	
			Mode GetMode() const;
			Mode GetDesiredMode() const;
			Result SetMode (Mode);
			ulong GetFrame() const;
	
			enum Compression
			{
				NO_COMPRESSION,
				USE_COMPRESSION
			};

			Result LoadState (std::istream&);
			Result SaveState (std::ostream&,Compression=USE_COMPRESSION) const;
	
			uint Is (uint) const;
			uint Is (uint,uint) const;
	
		private:
	
			Result Load(std::istream&,uint);
		};
	}
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
