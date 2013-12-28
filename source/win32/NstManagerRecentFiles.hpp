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

#ifndef NST_MANAGER_RECENTFILES_H
#define NST_MANAGER_RECENTFILES_H

#pragma once

namespace Nestopia
{
	namespace Managers
	{
		class RecentFiles
		{
		public:

			typedef String::Path<true> Path;

			RecentFiles(Emulator&,const Configuration&,Window::Menu&);
			~RecentFiles();

			void Save(Configuration&) const;

		private:

			enum
			{
				MAX_FILES = 9,
				MAX_NAME = 3 + _MAX_PATH
			};

			typedef String::Smart<MAX_NAME> Name;

			void OnMenu(uint);
			void OnLock(uint);
			void OnClear(uint);
			void OnLoad(Emulator::Event);
			void Add(uint,const Name&) const;

			Emulator& emulator;
			const Window::Menu& menu;
		};
	}
}

#endif
