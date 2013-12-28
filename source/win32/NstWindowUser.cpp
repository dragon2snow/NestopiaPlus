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
				cstring const text;
				cstring const title;
				String::Heap& response;
				Dialog dialog;

				ibool OnInitDialog(Param&)
				{
					dialog.Text() << title;

					if (text)
						dialog.Edit(IDC_USERINPUT_TEXT) << text;

					if (response.Size())
						dialog.Edit(IDC_USERINPUT_EDIT) << response;

					return TRUE;
				}

				ibool OnCmdOk(Param&)
				{
					dialog.Edit(IDC_USERINPUT_EDIT) >> response;
					dialog.Close(TRUE);
					return TRUE;
				}

				ibool OnCmdAbort(Param&)
				{
					dialog.Close(FALSE);
					return TRUE;
				}

			public:

				InputDialog(cstring const t,cstring const i,String::Heap& r)
				: text(t), title(i), response(r), dialog(IDD_USERINPUT) 
				{
					dialog.Messages().Add( WM_INITDIALOG, this, &InputDialog::OnInitDialog );
					dialog.Commands().Add( IDC_USERINPUT_OK, this, &InputDialog::OnCmdOk );
					dialog.Commands().Add( IDC_USERINPUT_ABORT, this, &InputDialog::OnCmdAbort );
				}

				INT_PTR Open()
				{
					return dialog.Open();
				}
			};

			enum
			{
				FLAGS = MB_TOPMOST|MB_SETFOREGROUND
			};

			void Fail (const uint textId,uint titleId)
			{
				if (titleId == 0)
					titleId = IDS_TITLE_ERROR;

				::MessageBox
				( 
					Application::Instance::GetActiveWindow(), 
					Resource::String( textId ),
			    	Resource::String( titleId ),
					FLAGS|MB_OK|MB_ICONERROR
				);
			}

			void Warn (const uint textId,uint titleId)
			{
				if (titleId == 0)
					titleId = IDS_TITLE_WARNING;

				::MessageBox
				( 
					Application::Instance::GetActiveWindow(), 
					Resource::String( textId ),
			    	Resource::String( titleId ),
					FLAGS|MB_OK|MB_ICONWARNING
				);
			}

			void Inform (const uint textId,uint titleId)
			{
				if (titleId == 0)
					titleId = IDS_TITLE_NESTOPIA;

				::MessageBox
				( 
					Application::Instance::GetActiveWindow(), 
					Resource::String( textId ),
			    	Resource::String( titleId ),
					FLAGS|MB_OK|MB_ICONINFORMATION
				);
			}

			ibool Confirm (const uint textId,uint titleId)
			{
				if (titleId == 0)
					titleId = IDS_TITLE_NESTOPIA;

				const int ret = ::MessageBox
				( 
					Application::Instance::GetActiveWindow(), 
					Resource::String( textId ),
			    	Resource::String( titleId ),
					FLAGS|MB_YESNO|MB_ICONQUESTION
				);

				return ret == IDYES;
			}

			ibool Issue (const Type type,const uint textId,const uint titleId)
			{
				NST_ASSERT( type == FAIL || type == WARN || type == INFORM || type == CONFIRM );

				switch (type)
				{
					case FAIL:   Fail   ( textId, titleId ); return TRUE;
					case WARN:   Warn   ( textId, titleId ); return TRUE;
					case INFORM: Inform ( textId, titleId ); return TRUE;
				}

				return Confirm( textId, titleId );
			}

			ibool Input (String::Heap& response,cstring const text,cstring const title)
			{
				return InputDialog( text, title ? title : Resource::String(IDS_TITLE_NESTOPIA), response ).Open();
			}

			ibool Input (String::Heap& response,const uint textId,const uint titleId)
			{
				return Input( response, Resource::String(textId), Resource::String(titleId ? titleId : IDS_TITLE_NESTOPIA) );
			}
		}
	}
}
