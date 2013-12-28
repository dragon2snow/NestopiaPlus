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
#include "NstWindowParam.hpp"
#include "NstWindowMenu.hpp"

namespace Nestopia
{
	using namespace Window;

	Menu::Instances::Translator Menu::Instances::translator = Menu::Instances::TranslateNone;
	Menu::Instances::Menus Menu::Instances::menus;
	ibool Menu::Instances::acceleratorsEnabled = TRUE;

	void Menu::Instances::Update()
	{
		if (acceleratorsEnabled && menus.Size())
			translator = menus.Size() == 1 ? TranslateSingle : TranslateMulti;
		else
			translator = TranslateNone;
	}

	void Menu::Instances::Update(Menu* const menu)
	{
		const ibool useful = menu->acceleratorEnabled && menu->accelerator.IsEnabled();

		if (Instances::Menus::Iterator const instance = menus.Find( menu ))
		{
			if (!useful)
			{
				menus.Erase( instance );
				Update();
			}
		}
		else
		{
			if (useful)
			{
				menus.PushBack( menu );
				Update();
			}
		}
	}

	void Menu::Instances::Remove(Menu* const menu)
	{
		if (Instances::Menus::Iterator const instance = menus.Find( menu ))
		{
			menus.Erase( instance );
			Update();
		}
	}

	void Menu::Instances::EnableAccelerators(const ibool enable)
	{
		acceleratorsEnabled = enable;
		Update();
	}

	Menu::PopupHandler::Key Menu::PopupHandler::GetKey(const uint levels) const
	{
		NST_ASSERT
		( 
			( ( ( levels >>  0 ) & 0xFF ) < IDM_OFFSET ) &&
			( ( ( levels >>  8 ) & 0xFF ) < IDM_OFFSET ) &&
			( ( ( levels >> 16 ) & 0xFF ) < IDM_OFFSET ) &&
			( ( ( levels >> 24 ) & 0xFF ) < IDM_OFFSET )
		);

		HMENU hMenu = menu.handle;
		uint pos = levels & 0xFF;

		if ( levels & 0x0000FF00U ) { hMenu = ::GetSubMenu( hMenu, pos ); pos = (( levels >>  8 ) - 1) & 0xFF; }
		if ( levels & 0x00FF0000U ) { hMenu = ::GetSubMenu( hMenu, pos ); pos = (( levels >> 16 ) - 2) & 0xFF; }
		if ( levels & 0xFF000000U ) { hMenu = ::GetSubMenu( hMenu, pos ); pos = (( levels >> 24 ) - 3) & 0xFF; }

		return Key( hMenu, pos );
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("t", on)
    #endif

	ibool Menu::Instances::TranslateNone(MSG&)
	{
		NST_ASSERT( !acceleratorsEnabled || menus.Empty() );

		return FALSE;
	}

	ibool Menu::Instances::TranslateSingle(MSG& msg)
	{
		NST_ASSERT( acceleratorsEnabled && menus.Size() == 1 );

		return msg.hwnd == *menus.Front()->window ? menus.Front()->accelerator.Translate( msg ) : FALSE;
	}

	ibool Menu::Instances::TranslateMulti(MSG& msg)
	{
		NST_ASSERT( acceleratorsEnabled && menus.Size() >= 2 );

		Menus::ConstIterator menu = menus.Begin();
		Menus::ConstIterator const end = menus.End();

		do
		{
			if (msg.hwnd == *(*menu)->window)
				return (*menu)->accelerator.Translate( msg );
		}
		while (++menu != end);

		return FALSE;
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("", on)
    #endif

	Menu::Menu(const uint id)
	: 
	handle             ( id   ),
	window             ( NULL ),
	acceleratorEnabled ( TRUE )
	{
		if (!handle)
			throw Application::Exception(_T("LoadMenu() failed!"));
	}

	Menu::~Menu()
	{
		Unhook();
	}

	void Menu::Hook(Custom& w)
	{
		static const MsgHandler::Entry<Menu> messages[] =
		{
			{ WM_INITMENUPOPUP,   &Menu::OnInitMenuPopup   },
			{ WM_UNINITMENUPOPUP, &Menu::OnUninitMenuPopup }
		};

		static const MsgHandler::HookEntry<Menu> hooks[] =
		{
			{ WM_CREATE,     &Menu::OnCreate  },
			{ WM_INITDIALOG, &Menu::OnCreate  },
			{ WM_DESTROY,    &Menu::OnDestroy }
		};

		Unhook();

		MsgHandler& router = w.Messages();

		window = &w;
		router.Add( this, messages, hooks );

		if (router( WM_COMMAND ))
			cmdCallback = router[WM_COMMAND].Replace( this, &Menu::OnCommand );
		else
			router.Add( WM_COMMAND, this, &Menu::OnCommand );

		if (w)
			Instances::Update( this );
	}

	void Menu::Unhook()
	{
		if (window)
		{
			if (cmdCallback)
			{
				window->Messages()[WM_COMMAND] = cmdCallback;
				cmdCallback.Reset();
			}

			window->Messages().RemoveAll( this );
			window = NULL;
		}

		Instances::Remove( this );
	}

	ibool Menu::Toggle() const
	{
		const ibool set = !IsVisible();
		Show( set );
		return set;
	}

	void Menu::EnableAccelerator(const ibool enable)
	{
		if (acceleratorEnabled != enable)
		{
			acceleratorEnabled = enable;
			Instances::Update( this );
		}
	}

	void Menu::OnCreate(Param&)
	{
		Instances::Update( this );
	}

	void Menu::OnDestroy(Param&)
	{
		Unhook();
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("t", on)
    #endif

	ibool Menu::OnCommand(Param& param)
	{
		const uint cmd = param.wParam & 0xFFFF;

		if (cmd >= IDM_OFFSET)
		{
			if 
			(
		      	(param.wParam & 0xFFFF0000) != 0x00010000 || 
				(::GetMenuState( handle, cmd, MF_BYCOMMAND ) & (MF_DISABLED|MF_GRAYED)) == 0
			)
			{
				if (const CmdHandler::Item* const item = cmdHandler( cmd ))
					item->value( cmd );
			}
		}
		else if (cmdCallback)
		{
			cmdCallback( param );
		}

		return TRUE;
	}

	ibool Menu::OnInitMenuPopup(Param& param)
	{
		const PopupHandler::Key* key;

		if (const PopupHandler::Handler::Callback* const callback = popupHandler.Find( reinterpret_cast<HMENU>(param.wParam), &key ))
		{
			PopupHandler::Param info( key->item, TRUE );
			(*callback)( info );
		}

		return TRUE;
	}

	ibool Menu::OnUninitMenuPopup(Param& param)
	{
		const PopupHandler::Key* key;

		if (const PopupHandler::Handler::Callback* const callback = popupHandler.Find( reinterpret_cast<HMENU>(param.wParam), &key ))
		{
			PopupHandler::Param info( key->item, TRUE );
			(*callback)( info );
		}

		return TRUE;
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("", on)
    #endif

	void Menu::Clear(HMENU const hMenu)
	{
		NST_ASSERT( hMenu );

		for (int i=::GetMenuItemCount( hMenu ); i > 0; --i)
			::DeleteMenu( hMenu, 0, MF_BYPOSITION );
	}

	uint Menu::NumItems(HMENU const hMenu)
	{
		const int numItems = hMenu ? ::GetMenuItemCount( hMenu ) : 0;
		return NST_MAX(numItems,0);
	}

	ibool Menu::IsVisible() const
	{
		return window && *window && handle == ::GetMenu( *window );
	}

	void Menu::Show(const ibool show) const
	{
		if (window && *window)
			::SetMenu( *window, show ? static_cast<HMENU>(handle) : NULL );
	}

	uint Menu::GetHeight() const
	{
		if (IsVisible())
		{
			MENUBARINFO mbi;
			mbi.cbSize = sizeof(mbi);

			if (::GetMenuBarInfo( *window, OBJID_MENU, 0, &mbi ))
				return NST_MAX((mbi.rcBar.bottom - mbi.rcBar.top) + 1,0);
		}

		return 0;
	}

	void Menu::SetKeys(const ACCEL* const accel,const uint count)
	{
		accelerator.Clear();

		if (count)
		{
			HeapString string;

			for (uint i=0; i < count; ++i)
			{
				if (accel[i].cmd)
				{
					NST_ASSERT( accel[i].cmd >= IDM_OFFSET );

					const Item item( handle, accel[i].cmd );

					if (item.Text() >> string)
					{
						if (accel[i].fVirt && accel[i].key)
							string << '\t' << System::Accelerator::GetKeyName( accel[i] );

						item.Text().SetFullString( string.Ptr() );
					}
				}
			}

			accelerator.Set( accel, count );
		}

		Instances::Update( this );
	}					

	void Menu::Insert(const Item& item,const uint cmd,GenericString name) const
	{
		NST_ASSERT( cmd >= IDM_OFFSET );

		if (item.hMenu)
		{
			HeapString string;

			const ACCEL accel( accelerator[cmd] );

			if (accel.cmd)
			{
				string << name << '\t' << System::Accelerator::GetKeyName( accel );

				if (string.Length() > name.Length() + 1)
					name = string;
			}

			MENUITEMINFO info;

			info.cbSize = sizeof(info);
			info.fMask = MIIM_STRING|MIIM_ID;
			info.wID = cmd;
			info.dwTypeData = const_cast<tchar*>(name.Ptr());

			::InsertMenuItem( item.hMenu, item.pos, item.pos < IDM_OFFSET, &info );
		}
	}

	void Menu::SetColor(const COLORREF color) const
	{
		MENUINFO info;
		HBRUSH const hBrush = ::CreateSolidBrush( color );

		info.cbSize = sizeof(MENUINFO);
		info.fMask = MIM_BACKGROUND|MIM_APPLYTOSUBMENUS;
		info.hbrBack = hBrush;

		::SetMenuInfo( handle, &info );

		if (window)
			::DrawMenuBar( *window );
	}

	void Menu::ResetColor() const
	{
		MENUINFO info;

		info.cbSize = sizeof(MENUINFO);
		info.fMask = MIM_BACKGROUND|MIM_APPLYTOSUBMENUS;
		info.hbrBack = NULL;

		::SetMenuInfo( handle, &info );

		if (window)
			::DrawMenuBar( *window );
	}

	void Menu::ToggleModeless(ibool modeless) const
	{
		MENUINFO info;

		info.cbSize = sizeof(MENUINFO);
		info.fMask = MIM_STYLE;
		info.dwStyle = modeless ? MNS_MODELESS : 0;

		::SetMenuInfo( handle, &info );

		if (window)
			::DrawMenuBar( *window );
	}

	Menu::Item::Type Menu::Item::GetType() const
	{
		Type type = NONE;

		if (hMenu)
		{
			if (pos >= IDM_OFFSET)
			{
				type = COMMAND;
			}
			else
			{
				MENUITEMINFO info;	
				info.cbSize = sizeof(info);
				info.fMask = MIIM_FTYPE|MIIM_SUBMENU|MIIM_ID;

				if (::GetMenuItemInfo( hMenu, pos, TRUE, &info ))
				{
					if (info.hSubMenu)
					{
						type = SUBMENU;
					}
					else if (info.fType & MFT_SEPARATOR)
					{
						type = SEPARATOR;
					}
					else if (info.wID >= IDM_OFFSET)
					{
						type = COMMAND;
					}
				}
			}
		}

		return type;
	}

	uint Menu::Item::NumItems() const
	{
		return hMenu ? Menu::NumItems( pos < IDM_OFFSET ? ::GetSubMenu(hMenu,pos) : hMenu ) : 0;
	}

	inline uint Menu::Item::Flag() const
	{
		return pos >= IDM_OFFSET ? MF_BYCOMMAND : MF_BYPOSITION;
	}

	uint Menu::Item::GetCommand() const
	{
		if (hMenu)
		{
			MENUITEMINFO info;	
			info.cbSize = sizeof(info);
			info.fMask = MIIM_ID;

			if (::GetMenuItemInfo( hMenu, pos, pos < IDM_OFFSET, &info ) && info.wID >= IDM_OFFSET)
				return info.wID;
		}

		return 0;
	}

	void Menu::Item::Remove() const
	{
		if (hMenu)
			::DeleteMenu( hMenu, pos, Flag() );
	}

	void Menu::Item::Clear() const
	{
		NST_ASSERT( pos < IDM_OFFSET );

		if (hMenu)
			Menu::Clear( ::GetSubMenu( hMenu, pos ) );
	}

	ibool Menu::Item::Enable(const ibool enable) const
	{
		return (hMenu ? ::EnableMenuItem( hMenu, pos, (enable ? MF_ENABLED : MF_GRAYED) | Flag() ) == MF_ENABLED : FALSE);
	}

	ibool Menu::Item::IsEnabled() const
	{
		if (hMenu)
		{
			const uint state = ::GetMenuState( hMenu, pos, Flag() );
			return state != uint(-1) && !(state & (MF_DISABLED|MF_GRAYED));
		}

		return FALSE;
	}

	ibool Menu::Item::Check(const ibool check) const
	{
		return (hMenu ? ::CheckMenuItem( hMenu, pos, (check ? MF_CHECKED : MF_UNCHECKED) | Flag() ) == MF_CHECKED : FALSE);
	}

	void Menu::Item::Check(const uint first,const uint last) const
	{
		NST_ASSERT( first <= last && pos >= first && pos <= last );

		if (hMenu)
			::CheckMenuRadioItem( hMenu, first, last, pos, Flag() );
	}

	void Menu::Item::Check(const uint first,const uint last,const ibool check) const
	{
		NST_ASSERT( first <= last && pos >= first && pos <= last );

		if (check)
		{
			Check( first, last );
		}
		else if (hMenu)
		{
			for (uint i = first, state = MF_UNCHECKED | Flag(); i <= last; ++i)
				::CheckMenuItem( hMenu, i, state );
		}
	}

	ibool Menu::Item::IsChecked() const
	{
		const uint state = (hMenu ? ::GetMenuState( hMenu, pos, Flag() ) : 0U);
		return state != uint(-1) && (state & MF_CHECKED);
	}

	ibool Menu::Item::Exists() const
	{
		return hMenu && ::GetMenuState( hMenu, pos, Flag() ) != uint(-1);
	}

	uint Menu::Item::Stream::GetLength() const
	{
		if (item.hMenu)
		{
			MENUITEMINFO info;

			info.cbSize = sizeof(info);
			info.fMask = MIIM_STRING;
			info.dwTypeData = NULL;

			if (::GetMenuItemInfo( item.hMenu, item.pos, item.pos < IDM_OFFSET, &info ))
				return info.cch;
		}

		return 0;
	}

	ibool Menu::Item::Stream::GetFullString(tchar* string,uint length) const
	{
		if (item.hMenu)
		{
			MENUITEMINFO info;

			info.cbSize = sizeof(info);
			info.fMask = MIIM_STRING;
			info.cch = length + 1;
			info.dwTypeData = string;

			return ::GetMenuItemInfo( item.hMenu, item.pos, item.pos < IDM_OFFSET, &info );
		}

		return FALSE;
	}

	void Menu::Item::Stream::SetFullString(tstring string) const
	{
		if (item.hMenu)
		{
			MENUITEMINFO info;

			info.cbSize = sizeof(info);
			info.fMask = MIIM_STRING;
			info.dwTypeData = const_cast<tchar*>(string);

			::SetMenuItemInfo( item.hMenu, item.pos, item.pos < IDM_OFFSET, &info );
		}
	}

	void Menu::Item::Stream::operator << (const GenericString& input) const
	{
		HeapString string;
		GetFullString( string );
		string( 0, string.FindFirstOf('\t') ) = input;
		SetFullString( string.Ptr() );
	}
}
