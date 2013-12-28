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

#ifndef NST_MANAGER_LAUNCHER_H
#define NST_MANAGER_LAUNCHER_H

#pragma once

#include "NstWindowCustom.hpp"
#include "NstCollectionBitSet.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Launcher;
	}

	namespace Managers
	{
		class Launcher
		{
		public:

			Launcher(Emulator&,const Configuration&,Window::Menu&,const Paths&,Window::Custom&);
			~Launcher();

			void Save(Configuration&,ibool) const;

		private:

			enum
			{
				MIN_DIALOG_WIDTH = 640,
				MIN_DIALOG_HEIGHT = 480,
				AVAILABLE = 0,
				FITS
			};

			void Update();
			void OnMenu(uint);
			void OnDisplayChange(Window::Param&);
			void OnEmuEvent(Emulator::Event);
			void OnAppEvent(Application::Instance::Event,const void*);

			static Nes::Emulator& ImportDatabase(Nes::Emulator&);

			Emulator& emulator;
			const Window::Menu& menu;
			Collection::BitSet state;
			Object::Heap<Window::Launcher> dialog;
		};
	}
}

#endif
