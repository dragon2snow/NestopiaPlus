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

#include "NstWindowMenu.hpp"
#include "NstManagerEmulator.hpp"
#include "NstManagerRecentDirs.hpp"

namespace Nestopia
{
	using namespace Managers;

	RecentDirs::RecentDirs(Emulator& e,const Configuration& cfg,Window::Menu& m)
	: 
	emulator ( e ),
	menu     ( m )
	{
		static const Window::Menu::CmdHandler::Entry<RecentDirs> commands[] =
		{
			{ IDM_FILE_RECENT_DIR_1,     &RecentDirs::OnMenu  },
			{ IDM_FILE_RECENT_DIR_2,     &RecentDirs::OnMenu  },
			{ IDM_FILE_RECENT_DIR_3,     &RecentDirs::OnMenu  },
			{ IDM_FILE_RECENT_DIR_4,     &RecentDirs::OnMenu  },
			{ IDM_FILE_RECENT_DIR_5,     &RecentDirs::OnMenu  },
			{ IDM_FILE_RECENT_DIR_LOCK,  &RecentDirs::OnLock  },
			{ IDM_FILE_RECENT_DIR_CLEAR, &RecentDirs::OnClear }
		};

		m.Commands().Add( this, commands );
		emulator.Events().Add( this, &RecentDirs::OnLoad );

		uint count = 0;

		String::Stack<24,char> index( "files recent dir x" );
		Name name( _T("&x ") );

		for (uint i=0; i < MAX_DIRS; ++i)
		{
			index.Back() = (char) ('1' + i);
			const GenericString dir( cfg[index] );

			if (dir.Length())
			{
				name[1] = (tchar) ('1' + count);
				name(3) = dir;
				menu[IDM_FILE_RECENT_DIR_1 + count++].Text() << name;
			}
		}

		menu[IDM_FILE_RECENT_DIR_CLEAR].Enable( count );

		for (count += IDM_FILE_RECENT_DIR_1; count <= IDM_FILE_RECENT_DIR_5; ++count)
			menu[count].Remove();

		menu[IDM_FILE_RECENT_DIR_LOCK].Check( cfg["files recent dir locked"] == Configuration::YES );
	}

	RecentDirs::~RecentDirs()
	{
		emulator.Events().Remove( this );
	}

	void RecentDirs::Save(Configuration& cfg) const
	{
		String::Stack<24,char> index( "files recent dir x" );
		Name dir;

		for (uint i=0; i < MAX_DIRS && menu[IDM_FILE_RECENT_DIR_1 + i].Text() >> dir; ++i)
		{
			index.Back() = (char) ('1' + i);
			cfg[index].Quote() = dir(3);
		}

		cfg["files recent dir locked"].YesNo() = menu[IDM_FILE_RECENT_DIR_LOCK].IsChecked();
	}

	void RecentDirs::OnMenu(uint cmd)
	{
		Name dir;

		if ((menu[cmd].Text() >> dir) > 3)
			Application::Instance::Launch( dir(3).Ptr() );
	}

	void RecentDirs::OnLock(uint)
	{
		menu[IDM_FILE_RECENT_DIR_LOCK].ToggleCheck();
	}

	void RecentDirs::OnClear(uint)
	{
		for (uint i=IDM_FILE_RECENT_DIR_1; i <= IDM_FILE_RECENT_DIR_5; ++i)
			menu[i].Remove();

		menu[IDM_FILE_RECENT_DIR_CLEAR].Disable();
	}

	void RecentDirs::Add(const uint idm,const Name& name) const
	{
		if (menu[idm].Exists())
		{
			menu[idm].Text() << name;
		}
		else
		{
			Window::Menu::Item item( menu[IDM_POS_FILE][IDM_POS_FILE_RECENTDIRS] );
			menu.Insert( item[item.NumItems() - 3], idm, name );
		}
	}

	void RecentDirs::OnLoad(Emulator::Event event)
	{
		switch (event)
		{
     		case Emulator::EVENT_LOAD:

				if (menu[IDM_FILE_RECENT_DIR_LOCK].IsUnchecked())
				{
					Name items[MAX_DIRS];
					const GenericString curDir( emulator.GetStartPath().Directory() );
			
					for (uint i=IDM_FILE_RECENT_DIR_1, j=0; i <= IDM_FILE_RECENT_DIR_5 && menu[i].Text() >> items[j]; ++i)
					{
						if (items[j](3) != curDir)
							items[j][1] = (char) ('2' + j), ++j;
					}
			
					Add( IDM_FILE_RECENT_DIR_1, Name("&1 ") << curDir );
			
					for (uint i=0; i < MAX_DIRS-1 && items[i].Length(); ++i)
						Add( IDM_FILE_RECENT_DIR_2 + i, items[i] );
			
					menu[IDM_FILE_RECENT_DIR_CLEAR].Enable();
				}
				break;

			case Emulator::EVENT_NETPLAY_MODE_ON:
			case Emulator::EVENT_NETPLAY_MODE_OFF:
			
				menu[IDM_POS_FILE][IDM_POS_FILE_RECENTDIRS].Enable
				(
					event == Emulator::EVENT_NETPLAY_MODE_OFF
				);
				break;			
		}
	}
}
