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

#include "NstWindowCustom.hpp"
#include "NstWindowParam.hpp"
#include "NstCtrlStandard.hpp"

namespace Nestopia
{
	using Window::Control::NotificationHandler;

	NotificationHandler::NotificationHandler(const uint id,Window::MsgHandler& m)
	: control(id), msgHandler(m)
	{
		Initialize();
	}

	NotificationHandler::~NotificationHandler()
	{
		msgHandler.Hooks().Remove( this );
	}

	void NotificationHandler::Initialize()
	{
		msgHandler.Hooks().Add( WM_NOTIFY, this, &NotificationHandler::OnNotify );
	}

	void NotificationHandler::OnNotify(Window::Param& param)
	{
		NST_ASSERT( param.lParam );

		const NMHDR& nmhdr = *reinterpret_cast<const NMHDR*>(param.lParam);

		if (control == nmhdr.idFrom)
		{
			if (const Items::Entry* const entry = items.Find( nmhdr.code ))
				entry->value( nmhdr );
		}
	}
}
