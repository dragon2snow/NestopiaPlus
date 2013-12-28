////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
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

#include "NstWindowParam.hpp"
#include "NstDialogLogfile.hpp"

namespace Nestopia
{
	using Window::Logfile;

	struct Logfile::Handlers
	{
		static const MsgHandler::Entry<Logfile> messages[];
		static const MsgHandler::Entry<Logfile> commands[];
	};

	const Window::MsgHandler::Entry<Logfile> Logfile::Handlers::messages[] =
	{
		{ WM_INITDIALOG, &Logfile::OnInitDialog }
	};

	const Window::MsgHandler::Entry<Logfile> Logfile::Handlers::commands[] =
	{
		{ IDC_LOGFILE_CLEAR, &Logfile::OnCmdClear },
		{ IDC_LOGFILE_OK,    &Logfile::OnCmdOk    }
	};

	Logfile::Logfile()
	: dialog(IDD_LOGFILE,this,Handlers::messages,Handlers::commands) {}

	ibool Logfile::Open(tstring const string)
	{
		clear = FALSE;

		if (*string)
		{
			text = string;
			dialog.Open();
		}

		return clear;
	}

	ibool Logfile::OnInitDialog(Window::Param&)
	{
		dialog.Edit( IDC_LOGFILE_EDIT ) << text;
		return TRUE;
	}

	ibool Logfile::OnCmdClear(Window::Param& param)
	{
		if (param.Button().IsClicked())
		{
			dialog.Edit( IDC_LOGFILE_EDIT ).Clear();
			clear = TRUE;
		}

		return TRUE;
	}

	ibool Logfile::OnCmdOk(Window::Param& param)
	{
		if (param.Button().IsClicked())
			dialog.Close();

		return TRUE;
	}
}
