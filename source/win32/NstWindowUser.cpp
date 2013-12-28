////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2007 Martin Freij
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

#include "NstResourceString.hpp"
#include "NstWindowDialog.hpp"
#include "NstWindowUser.hpp"
#include "NstApplicationInstance.hpp"
#include <ShellAPI.h>

namespace Nestopia
{
	namespace Window
	{
		namespace User
		{
			class InputDialog
			{
				tstring const text;
				tstring const title;
				HeapString& response;
				Dialog dialog;

				ibool OnInitDialog(Param&)
				{
					dialog.Text() << title;

					if (text)
						dialog.Edit(IDC_USER_INPUT_TEXT) << text;

					if (response.Length())
						dialog.Edit(IDC_USER_INPUT_EDIT) << response.Ptr();

					return true;
				}

				ibool OnCmdOk(Param&)
				{
					dialog.Edit(IDC_USER_INPUT_EDIT) >> response;
					dialog.Close(true);
					return true;
				}

				ibool OnCmdAbort(Param&)
				{
					dialog.Close(false);
					return true;
				}

			public:

				InputDialog(tstring const t,tstring const i,HeapString& r)
				: text(t), title(i), response(r), dialog(IDD_USER_INPUT)
				{
					dialog.Messages().Add( WM_INITDIALOG, this, &InputDialog::OnInitDialog );
					dialog.Commands().Add( IDOK, this, &InputDialog::OnCmdOk );
					dialog.Commands().Add( IDABORT, this, &InputDialog::OnCmdAbort );
				}

				INT_PTR Open()
				{
					return dialog.Open();
				}
			};

			static int Present(tstring const text,tstring const title,const uint flags)
			{
				return ::MessageBox
				(
					Application::Instance::GetActiveWindow(),
					text,
					title,
					MB_TOPMOST|MB_SETFOREGROUND|flags
				);
			}

			void Fail(tstring const text,tstring const title)
			{
				Present( text, title && *title ? title : _T("Nestopia Error"), MB_OK|MB_ICONERROR );
			}

			void Fail(tstring const text,const uint titleId)
			{
				Fail( text, Resource::String(titleId ? titleId : IDS_TITLE_ERROR).Ptr() );
			}

			void Fail(const uint textId,const uint titleId)
			{
				Fail( Resource::String(textId).Ptr(), titleId );
			}

			void Warn(tstring const text,tstring const title)
			{
				Present( text, title && *title ? title : _T("Nestopia Warning"), MB_OK|MB_ICONWARNING );
			}

			void Warn(tstring const text,const uint titleId)
			{
				Warn( text, Resource::String(titleId ? titleId : IDS_TITLE_WARNING).Ptr() );
			}

			void Warn(const uint textId,const uint titleId)
			{
				Warn( Resource::String(textId).Ptr(), titleId );
			}

			void Inform(tstring const text,tstring const title)
			{
				Present( text, title && *title ? title : _T("Nestopia"), MB_OK|MB_ICONINFORMATION );
			}

			void Inform(tstring const text,const uint titleId)
			{
				Inform( text, Resource::String(titleId ? titleId : IDS_TITLE_NESTOPIA).Ptr() );
			}

			void Inform(const uint textId,const uint titleId)
			{
				Inform( Resource::String(textId).Ptr(), titleId );
			}

			bool Confirm(tstring const text,tstring const title)
			{
				return Present( text, title && *title ? title : _T("Nestopia"), MB_YESNO|MB_ICONQUESTION ) == IDYES;
			}

			bool Confirm(tstring const text,const uint titleId)
			{
				return Confirm( text, Resource::String(titleId ? titleId : IDS_TITLE_NESTOPIA).Ptr() );
			}

			bool Confirm(const uint textId,const uint titleId)
			{
				return Confirm( Resource::String(textId), titleId );
			}

			bool Input (HeapString& response,tstring const text,tstring const title)
			{
				return InputDialog( text, title ? title : Resource::String(IDS_TITLE_NESTOPIA), response ).Open();
			}
		}
	}
}
