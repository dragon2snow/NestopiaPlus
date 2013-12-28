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

#include "NstWindowParam.hpp"
#include "NstManagerPaths.hpp"
#include "NstDialogMovie.hpp"

namespace Nestopia
{
	using namespace Window;

	struct Movie::Handlers
	{
		static const MsgHandler::Entry<Movie> messages[];
		static const MsgHandler::Entry<Movie> commands[];
	};

	const MsgHandler::Entry<Movie> Movie::Handlers::messages[] =
	{
		{ WM_INITDIALOG, &Movie::OnInitDialog }
	};

	const MsgHandler::Entry<Movie> Movie::Handlers::commands[] =
	{
		{ IDC_MOVIE_CLEAR,  &Movie::OnCmdClear  },
		{ IDC_MOVIE_BROWSE, &Movie::OnCmdBrowse },
		{ IDC_MOVIE_OK,     &Movie::OnCmdOk     },
		{ IDC_MOVIE_CANCEL, &Movie::OnCmdCancel }
	};

	Movie::Movie(const Managers::Paths& p)
	: dialog(IDD_MOVIE,this,Handlers::messages,Handlers::commands), paths(p) {}

	Movie::~Movie()
	{
	}

	void Movie::FixFile()
	{
		if (movieFile.File().Size())
		{
			if (movieFile.Directory().Empty())
				movieFile.Directory() = paths.GetDefaultDirectory( Managers::Paths::File::MOVIE );

			if (movieFile.Extension().Empty())
				movieFile.Extension() = "nsv";
		}
		else
		{
			movieFile.Clear();
		}
	}

	ibool Movie::SetMovieFile(const MovieFile& file)
	{
		const MovieFile old( movieFile );
		movieFile = file;
		FixFile();
		return movieFile != old;
	}

	ibool Movie::OnInitDialog(Param&)
	{
		dialog.Edit(IDC_MOVIE_FILE) << movieFile;

		return TRUE;
	}

	ibool Movie::OnCmdClear(Param& param)
	{
		if (param.Button().IsClicked())
			dialog.Edit(IDC_MOVIE_FILE).Clear();

		return TRUE;
	}

	ibool Movie::OnCmdBrowse(Param& param)
	{
		if (param.Button().IsClicked())	
			dialog.Edit(IDC_MOVIE_FILE).Try() << paths.BrowseSave( Managers::Paths::File::MOVIE, movieFile );

		return TRUE;
	}

	ibool Movie::OnCmdOk(Param& param)
	{
		if (param.Button().IsClicked())
		{
			dialog.Edit(IDC_MOVIE_FILE) >> movieFile;
			FixFile();
			dialog.Close();
		}

		return TRUE;
	}

	ibool Movie::OnCmdCancel(Param& param)
	{
		if (param.Button().IsClicked())
			dialog.Close();

		return TRUE;
	}
}
