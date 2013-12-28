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

#ifndef NST_MACHINE_H
#define NST_MACHINE_H

#include "NstCpu.hpp"
#include "NstPpu.hpp"
#include "NstTracker.hpp"
#include "NstVideoRenderer.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			class Output;
		}

		namespace Sound
		{
			class Output;
		}

		namespace Input
		{
			class Device;
			class Adapter;
			class Controllers;
		}

		class Image;
		class Cheats;
		class ImageDatabase;

		class Machine
		{
		public:

			Machine();
			~Machine();

			void ExecuteFrame
			(
				Video::Output*,
				Sound::Output*,
				Input::Controllers*
			);

			enum ColorMode
			{
				COLORMODE_YUV,
				COLORMODE_RGB,
				COLORMODE_CUSTOM
			};

			Result Load(StdStream,uint);
			bool   Unload();
			bool   PowerOff();
			void   Reset(bool);
			void   SetMode(Mode);
			bool   LoadState(State::Loader&);
			void   SaveState(State::Saver&) const;
			void   InitializeInputDevices() const;
			Result UpdateColorMode();
			Result UpdateColorMode(ColorMode);

		private:

			enum
			{
				OPEN_BUS = 0x40
			};

			NES_DECL_POKE( 4016 );
			NES_DECL_PEEK( 4016 );
			NES_DECL_POKE( 4017 );
			NES_DECL_PEEK( 4017 );

			uint state;

		public:

			dword frame;
			Input::Adapter* extPort;
			Input::Device* expPort;
			Image* image;
			Cheats* cheats;
			ImageDatabase* imageDatabase;
			const void* const padding;
			Tracker tracker;
			Cpu cpu;
			Ppu ppu;
			Video::Renderer renderer;

			uint Is(uint a) const
			{
				return state & a;
			}

			bool Is(uint a,uint b) const
			{
				return (state & a) && (state & b);
			}
		};
	}
}

#endif
