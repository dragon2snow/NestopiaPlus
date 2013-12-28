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
#include "NstResourceFile.hpp"
#include "NstDialogLicence.hpp"

namespace Nestopia
{
	using namespace Window;

	struct Licence::Handlers
	{
		static const MsgHandler::Entry<Licence> messages[];
		static const MsgHandler::Entry<Licence> commands[];
	};

	const MsgHandler::Entry<Licence> Licence::Handlers::messages[] =
	{
		{ WM_INITDIALOG, &Licence::OnInitDialog }
	};

	const MsgHandler::Entry<Licence> Licence::Handlers::commands[] =
	{
		{ IDC_LICENCE_OK, &Licence::OnCmdOk }
	};

	Licence::Licence()
	: dialog(IDD_LICENCE,this,Handlers::messages,Handlers::commands) {}

	ibool Licence::OnInitDialog(Param&)
	{
		const Resource::File file( IDR_LICENCE, "Licence" );

		if (file.GetData().Size())
			dialog.Control( IDC_LICENCE_EDIT ).Text() << static_cast<cstring>( file.GetData() );

		return TRUE;
	}

	ibool Licence::OnCmdOk(Param& param)
	{
		if (param.Button().IsClicked())
			dialog.Close();

		return TRUE;
	}
}
