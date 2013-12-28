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

#ifndef NST_API_EMULATOR_H
#define NST_API_EMULATOR_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#ifdef _MSC_VER
#pragma warning( push )
#endif

#include "../NstCore.hpp"
#include "../NstCpu.hpp"
#include "../NstPpu.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			class Output;
			class Renderer;
			class Palette;
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
		class GameGenie;
		class Cheats;
		class ImageDatabase;
		class Movie;
		class Rewinder;
	}

	namespace Api
	{
		class Emulator
		{
		public:
	
			Emulator();
			~Emulator();
	
			Result Execute
			(
				Core::Video::Output*,
				Core::Sound::Output*,
				Core::Input::Controllers*
			);
	
		private:
	
			friend class Machine;
			friend class Video;
			friend class Sound;
			friend class Input;
			friend class Cartridge;
			friend class Fds;
			friend class Nsf;
			friend class Movie;
			friend class Cheats;
			friend class GameGenie;
			friend class DipSwitches;
			friend class Rewinder;
	
			enum
			{
				OPEN_BUS = 0x40
			};

			Result ExecuteFrame
			(
				Core::Video::Output*,
				Core::Sound::Output*,
				Core::Input::Controllers*
			);

			Result Load (Core::StdStream,uint);
			void   Unload ();
			Result PowerOn ();
			void   PowerOff ();
			Result Reset (bool);
			void   SetMode (Core::Mode);
			Result LoadState (Core::StdStream,bool=true);
			Result SaveState (Core::StdStream,bool);
			void   InitializeInputDevices () const;
			bool   GoodSaveTime() const;
	
			NES_DECL_POKE( 4016 )
			NES_DECL_PEEK( 4016 )
			NES_DECL_POKE( 4017 )				    
			NES_DECL_PEEK( 4017 )
	
			uint state;
			ulong frame;
			Core::Input::Adapter* extPort;
			Core::Input::Device* expPort;
			Core::Image* image;
			Core::Movie* movie;
			Core::Video::Renderer& renderer;
			Core::Video::Palette& palette;
			Core::Cpu cpu;
			Core::Ppu ppu;
			Core::Rewinder* rewinder;
			ibool rewinderSound;
			Core::Cheats* cheats;
			Core::ImageDatabase* imageDatabase;
	
			uint Is(uint what) const
			{
				return state & what;
			}
		};
	}
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
