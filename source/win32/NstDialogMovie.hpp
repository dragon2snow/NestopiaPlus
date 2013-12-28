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

#ifndef NST_DIALOG_MOVIE_H
#define NST_DIALOG_MOVIE_H

#pragma once

#include "NstWindowDialog.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Movie
		{
		public:

			explicit Movie(const Managers::Paths&);
			~Movie();

			typedef String::Path<false> MovieFile;

			void SetMovieFile(const MovieFile&);

		private:

			struct Handlers;

			ibool OnInitDialog (Param&);
			ibool OnCmdBrowse  (Param&);
			ibool OnCmdClear   (Param&);
			ibool OnCmdOk      (Param&);
			ibool OnCmdCancel  (Param&);

			Dialog dialog;
			const Managers::Paths& paths;
			MovieFile movieFile;

		public:

			void Open()
			{
				dialog.Open();
			}

			const MovieFile& GetMovieFile() const
			{
				return movieFile;
			}
		};
	}
}

#endif