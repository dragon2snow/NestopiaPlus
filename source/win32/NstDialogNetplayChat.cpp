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
#include "NstWindowMenu.hpp"
#include "NstDialogNetPlay.hpp"
#include "../kaillera/kailleraclient.h"

namespace Nestopia
{
	using namespace Window;

	struct Netplay::Chat::Handlers
	{
		static const MsgHandler::Entry<Chat> messages[];
	};

	const MsgHandler::Entry<Netplay::Chat> Netplay::Chat::Handlers::messages[] =
	{
		{ WM_INITDIALOG, &Chat::OnInit    },
		{ WM_COMMAND,    &Chat::OnCommand }
	};

	Netplay::Chat::Chat()
	: dialog( IDD_CHAT, this, Handlers::messages ) {}
	
	void Netplay::Chat::Open()
	{
		dialog.Open( Dialog::MODELESS );
	}

	void Netplay::Chat::Close()
	{
		dialog.Close();
		text.Destroy();
	}

	ibool Netplay::Chat::OnInit(Param&)
	{
		dialog.Edit(IDC_CHAT_EDIT).Limit(255);
		return TRUE;
	}

	ibool Netplay::Chat::OnCommand(Param& param)
	{
		if (param.wParam == 1 && param.lParam == 0)
		{
			const Control::Edit edit( dialog.Edit(IDC_CHAT_EDIT) );

			edit.Text() >> text;

			if (text.Size())
			{
				text.Trim();

				if (text.Size())
					::kailleraChatSend( text );

				edit.Text().Clear();
			}
		}

		return FALSE;
	}
}
