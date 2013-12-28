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

#ifndef NST_MANAGER_MOVIE_H
#define NST_MANAGER_MOVIE_H

#pragma once

#include <fstream>
#include "NstWindowMenu.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Movie;
	}

	namespace Io
	{
		namespace Nsp
		{
			struct Context;
		}
	}

	namespace Managers
	{
		class Movie
		{
		public:

			Movie(Emulator&,Window::Menu&,const Paths&);
			~Movie();

			enum Alert
			{
				NOISY,
				QUIET
			};

			ibool Load(const Paths::Path&,Alert);
			void Save(Io::Nsp::Context&) const;

		private:

			enum Pos
			{
				REWINDED,
				FORWARDED
			};

			struct Callbacks;

			void  Close (Pos=REWINDED,ibool=TRUE);
			ibool Open  (std::fstream::open_mode);

			ibool CanPlay    () const;
			ibool CanRecord  () const;
			ibool CanStop    () const;
			ibool CanRewind  () const;
			ibool CanForward () const;

			void OnEmuEvent   (Emulator::Event);
			void OnMenuView   (Window::Menu::PopupHandler::Param&);
			void OnCmdFile    (uint);
			void OnCmdRecord  (uint);
			void OnCmdPlay    (uint);
			void OnCmdStop    (uint);
			void OnCmdRewind  (uint);
			void OnCmdForward (uint);

			Emulator& emulator;
			Pos pos;
			std::fstream stream;
			const Window::Menu& menu;
			Object::Heap<Window::Movie> dialog;
		};
	}
}

#endif
