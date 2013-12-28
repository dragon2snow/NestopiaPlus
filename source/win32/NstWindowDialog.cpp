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

#include "NstApplicationException.hpp"
#include "NstApplicationLanguage.hpp"
#include "NstResourceIcon.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowDialog.hpp"

namespace Nestopia
{
	namespace Window
	{
		Dialog::Instances Dialog::instances;
		Dialog::ModelessDialogs::Instances Dialog::ModelessDialogs::instances;
		Dialog::ModelessDialogs::Processor Dialog::ModelessDialogs::processor = Dialog::ModelessDialogs::ProcessNone;

		void Dialog::ModelessDialogs::Update()
		{
			switch (instances.Size())
			{
				case 0:  processor = ProcessNone; break;
				case 1:  processor = ProcessSingle; break;
				default: processor = ProcessMultiple; break;
			}
		}

		void Dialog::ModelessDialogs::Add(HWND const hWnd)
		{
			instances.PushBack( hWnd );
			Update();
		}

		ibool Dialog::ModelessDialogs::Remove(HWND const hWnd)
		{
			if (Instances::Iterator const instance = instances.Find( hWnd ))
			{
				instances.Erase( instance );
				Update();
				return true;
			}

			return false;
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		ibool Dialog::ModelessDialogs::ProcessNone(MSG&)
		{
			NST_ASSERT( instances.Size() == 0 );

			return false;
		}

		ibool Dialog::ModelessDialogs::ProcessSingle(MSG& msg)
		{
			NST_ASSERT( instances.Size() == 1 );

			return ::IsDialogMessage( instances.Front(), &msg );
		}

		ibool Dialog::ModelessDialogs::ProcessMultiple(MSG& msg)
		{
			NST_ASSERT( instances.Size() >= 2 );

			Instances::ConstIterator dialog = instances.Begin();
			Instances::ConstIterator const end = instances.End();

			do
			{
				if (::IsDialogMessage( *dialog, &msg ))
					return true;
			}
			while (++dialog != end);

			return false;
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		Dialog::Dialog(const uint i)
		: id(i) {}

		INT_PTR Dialog::Open(const Type type)
		{
			if (hWnd)
			{
				Show();
				Parent().Redraw();
				Activate();
				return 0;
			}

			if (type == MODAL)
			{
				for (uint i=0; i < 3; ++i)
				{
					static const u8 idc[3] = {IDOK,IDCANCEL,IDABORT};

					if (!cmdHandler( idc[i] ))
						cmdHandler.Add( idc[i], this, &Dialog::OnClose );
				}
			}

			if (cmdHandler.Size() && !msgHandler( WM_COMMAND ))
				msgHandler.Add( WM_COMMAND, this, &Dialog::OnCommand );

			if (!msgHandler( WM_CLOSE ))
				msgHandler.Add( WM_CLOSE, this, &Dialog::OnClose );

			if (type == MODAL)
			{
				return DialogBoxParam
				(
					Application::Instance::GetLanguage().GetResourceHandle(),
					MAKEINTRESOURCE(id),
					Application::Instance::GetActiveWindow(),
					DlgProc,
					reinterpret_cast<LPARAM>(this)
				);
			}

			Application::Instance::Waiter wait;

			CreateDialogParam
			(
				Application::Instance::GetLanguage().GetResourceHandle(),
				MAKEINTRESOURCE(id),
				Application::Instance::GetActiveWindow(),
				DlgProc,
				reinterpret_cast<LPARAM>(this)
			);

			if (hWnd == NULL)
				throw Application::Exception(_T("CreateDialogParam() failed!"));

			ModelessDialogs::Add( hWnd );

			return 0;
		}

		void Dialog::Close(const int ret)
		{
			if (hWnd)
			{
				if (ModelessDialogs::Remove( hWnd ))
				{
					::DestroyWindow( hWnd );
					NST_VERIFY( hWnd == NULL );
				}
				else
				{
					::EndDialog( hWnd, ret );
				}
			}
		}

		Dialog::~Dialog()
		{
			Close();
		}

		void Dialog::SetItemIcon(uint item,uint id) const
		{
			::SendDlgItemMessage( hWnd, item, STM_SETIMAGE, IMAGE_ICON, (LPARAM) (HICON) Resource::Icon(id) );
		}

		ibool Dialog::OnClose(Param&)
		{
			Close();
			return true;
		}

		void Dialog::Fetch(HWND const handle)
		{
			NST_ASSERT( handle && !hWnd );

			hWnd = handle;
			instances.PushBack( this );
		}

		void Dialog::Ditch(Instances::Iterator const instance)
		{
			NST_ASSERT( hWnd );

			ModelessDialogs::Remove( hWnd );
			instances.Erase( instance );
			hWnd = NULL;
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		ibool Dialog::OnCommand(Param& param)
		{
			if (const MsgHandler::Item* const item = cmdHandler( LOWORD(param.wParam) ))
				return item->value( param );

			return false;
		}

		BOOL CALLBACK Dialog::DlgProc(HWND hWnd,uint uMsg,WPARAM wParam,LPARAM lParam)
		{
			if (uMsg == WM_INITDIALOG && lParam)
				reinterpret_cast<Dialog*>(lParam)->Fetch( hWnd );

			ibool ret = false;

			for (Instances::Iterator it=instances.Begin(), end=instances.End(); it != end; ++it)
			{
				Dialog& dialog = **it;

				if (dialog.hWnd == hWnd)
				{
					if (const MsgHandler::Item* const item = dialog.msgHandler( uMsg ))
					{
						Param param = {wParam,lParam,0,hWnd};
						ret = item->value( param );
					}

					if (uMsg == WM_DESTROY)
						dialog.Ditch( it );

					break;
				}
			}

			return ret;
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
