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

#include "NstResourceCursor.hpp"
#include "NstWindowParam.hpp"
#include "NstDialogAbout.hpp"
#include "NstApplicationInstance.hpp"
#include <ShellAPI.h>

namespace Nestopia
{
	using namespace Window;

	struct About::Handlers
	{
		static const MsgHandler::Entry<About> messages[];
		static const MsgHandler::Entry<About> commands[];
	};

	const MsgHandler::Entry<About> About::Handlers::messages[] =
	{
		{ WM_INITDIALOG, &About::OnInitDialog },
		{ WM_SETCURSOR,  &About::OnSetCursor  }
	};

	const MsgHandler::Entry<About> About::Handlers::commands[] =
	{
		{ IDC_ABOUT_GNU,  &About::OnCmdClick },
		{ IDC_ABOUT_URL1, &About::OnCmdClick },
		{ IDC_ABOUT_URL2, &About::OnCmdClick },
		{ IDC_ABOUT_OK,   &About::OnCmdOk    }
	};

	About::About()
	: dialog(IDD_ABOUT,this,Handlers::messages,Handlers::commands) {}

	ibool About::OnInitDialog(Param&)
	{
		dialog.SetItemIcon( IDC_ABOUT_ICON, Application::Instance::GetIconStyle() == Application::Instance::ICONSTYLE_NES ? IDI_APP : IDI_APP_J );
		dialog.Control( IDC_ABOUT_NAMEVERSION ).Text() << (String::Heap<char>() << "Nestopia " << Application::Instance::GetVersion()).Ptr();
		return TRUE;
	}

	ibool About::OnSetCursor(Param& param)
	{
		HCURSOR const hCursor =
		(
			param.Cursor().IsInside( IDC_ABOUT_GNU  ) ||
			param.Cursor().IsInside( IDC_ABOUT_URL1 ) ||
			param.Cursor().IsInside( IDC_ABOUT_URL2 )
		) 
			? Resource::Cursor::GetUpArrow() : Resource::Cursor::GetArrow();

		::SetCursor( hCursor );
		::SetWindowLongPtr( dialog, DWLP_MSGRESULT, TRUE );

		return TRUE;
	}

	ibool About::OnCmdClick(Param& param)
	{
		if (param.Button().IsClicked())
		{
			HeapString url;

			if (dialog.Control( param.Button().GetId() ).Text() >> url)
				::ShellExecute( NULL, _T("open"), url.Ptr(), NULL, NULL, SW_SHOWNORMAL );
		}

		return TRUE;
	}

	ibool About::OnCmdOk(Param& param)
	{
		if (param.Button().IsClicked())
			dialog.Close();

		return TRUE;
	}
}
