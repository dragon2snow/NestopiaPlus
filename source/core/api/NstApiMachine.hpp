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

#ifndef NST_API_MACHINE_H
#define NST_API_MACHINE_H

#include <iosfwd>
#include "NstApi.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#if NST_ICC >= 810
#pragma warning( push )
#pragma warning( disable : 444 )
#elif NST_MSVC >= 1200
#pragma warning( push )
#pragma warning( disable : 4512 )
#endif

namespace Nes
{
	namespace Api
	{
		class Machine : public Base
		{
			struct EventCaller;

		public:

			template<typename T>
			Machine(T& e)
			: Base(e) {}

			enum
			{
				ON        = 0x001,
				VS        = 0x010,
				PC10      = 0x020,
				CARTRIDGE = 0x040,
				DISK      = 0x080,
				SOUND     = 0x100,
				GAME      = CARTRIDGE|DISK,
				IMAGE     = GAME|SOUND
			};

			enum Mode
			{
				NTSC = 0x04,
				PAL  = 0x08
			};

			enum
			{
				CLK_NTSC_DOT   = Clocks::NTSC_CLK,
				CLK_NTSC_DIV   = Clocks::NTSC_DIV,
				CLK_NTSC_VSYNC = Clocks::RP2C02_HVSYNC * ulong(Clocks::NTSC_DIV),
				CLK_PAL_DOT    = Clocks::PAL_CLK,
				CLK_PAL_DIV    = Clocks::PAL_DIV,
				CLK_PAL_VSYNC  = Clocks::RP2C07_HVSYNC * ulong(Clocks::PAL_DIV)
			};

			Result Load          (std::istream&) throw();
			Result LoadCartridge (std::istream&) throw();
			Result LoadDisk      (std::istream&) throw();
			Result LoadSound     (std::istream&) throw();
			Result Unload        () throw();

			Result Power (bool) throw();
			Result Reset (bool) throw();

			Mode   GetMode() const throw();
			Mode   GetDesiredMode() const throw();
			Result SetMode (Mode) throw();

			enum Compression
			{
				NO_COMPRESSION,
				USE_COMPRESSION
			};

			Result LoadState (std::istream&) throw();
			Result SaveState (std::ostream&,Compression=USE_COMPRESSION) const throw();

			uint Is (uint) const throw();
			bool Is (uint,uint) const throw();

			bool IsLocked() const;

			enum Event
			{
				EVENT_LOAD,
				EVENT_UNLOAD,
				EVENT_POWER_ON,
				EVENT_POWER_OFF,
				EVENT_RESET_SOFT,
				EVENT_RESET_HARD,
				EVENT_MODE_NTSC,
				EVENT_MODE_PAL
			};

			enum
			{
				NUM_EVENT_CALLBACKS = 8
			};

			typedef void (NST_CALLBACK *EventCallback) (UserData,Event,Result);

			static EventCaller eventCallback;

		private:

			Result Load(std::istream&,uint);
		};

		struct Machine::EventCaller : Core::UserCallback<Machine::EventCallback>
		{
			void operator () (Event event,Result result=RESULT_OK) const
			{
				if (function)
					function( userdata, event, result );
			}
		};
	}
}

#if NST_MSVC >= 1200 || NST_ICC >= 810
#pragma warning( pop )
#endif

#endif
