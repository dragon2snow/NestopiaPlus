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

#ifndef NST_MANAGER_FDS_H
#define NST_MANAGER_FDS_H

#pragma once

#include "NstWindowMenu.hpp"
#include "../core/api/NstApiFds.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Fds;
	}

	namespace Managers
	{
		class Fds
		{
		public:

			Fds(Emulator&,const Configuration&,Window::Menu&,const Paths&);
			~Fds();

			void Save(Configuration&) const;

		private:

			struct Callbacks;

			void UpdateSettings() const;

			void OnEmuEvent (Emulator::Event);
			void OnDiskChange (Nes::Fds::Event,uint,uint) const;
			void OnMenuInsert (Window::Menu::PopupHandler::Param&);

			void OnCmdInsertDisk (uint);
			void OnCmdChangeSide (uint);
			void OnCmdEjectDisk  (uint);
			void OnCmdOptions    (uint);

			Emulator& emulator;
			ibool master;
			const Window::Menu& menu;
			Object::Heap<Window::Fds> dialog;
		};
	}
}

#endif
