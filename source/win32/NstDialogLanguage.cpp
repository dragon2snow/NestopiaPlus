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

#include "NstWindowParam.hpp"
#include "NstDialogLanguage.hpp"

namespace Nestopia
{
	using namespace Window;

	struct Language::Handlers
	{
		static const MsgHandler::Entry<Language> messages[];
		static const MsgHandler::Entry<Language> commands[];
	};

	const MsgHandler::Entry<Language> Language::Handlers::messages[] =
	{
		{ WM_INITDIALOG, &Language::OnInitDialog }
	};

	const MsgHandler::Entry<Language> Language::Handlers::commands[] =
	{
		{ IDC_LANGUAGE_OK,     &Language::OnCmdOk     },
		{ IDC_LANGUAGE_CANCEL, &Language::OnCmdCancel }
	};

	Language::Language()
	: dialog(IDD_LANGUAGE,this,Handlers::messages,Handlers::commands)
	{
	}

	ibool Language::OnInitDialog(Param&)
	{
		resources.clear();
		Application::Instance::EnumerateResources( resources );

		const Window::Control::ListBox listBox( dialog.ListBox(IDC_LANGUAGE_LIST) );

		Path name;

		for (Application::Instance::ResourcePaths::const_iterator it(resources.begin()), end(resources.end()); it != end; ++it)
		{
			name = it->File();
			name.Extension().Clear();
			::CharUpperBuff( name.Ptr(), 1 );

			const uint index = listBox.Add( name.Ptr() ).GetIndex();
			listBox[index].Data() = (it - resources.begin());

			if (*it == Application::Instance::GetResourcePath())
				listBox[index].Select();
		}

		return true;
	}

	ibool Language::OnCmdOk(Param& param)
	{
		if (param.Button().IsClicked())
		{
			const Window::Control::ListBox listBox( dialog.ListBox(IDC_LANGUAGE_LIST) );

			if (listBox.AnySelection())
			{
				const uint index = listBox.Selection().Data();

				if (resources[index] != Application::Instance::GetResourcePath())
					newResource = resources[index];
			}

			dialog.Close();
		}

		return true;
	}

	ibool Language::OnCmdCancel(Param& param)
	{
		if (param.Button().IsClicked())
			dialog.Close();

		return true;
	}
}
