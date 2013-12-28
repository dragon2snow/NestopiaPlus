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

#include "NstIoFile.hpp"
#include "NstWindowMenu.hpp"
#include "NstManagerEmulator.hpp"
#include "NstManagerRecentFiles.hpp"

namespace Nestopia
{
	NST_COMPILE_ASSERT
	(
		IDM_FILE_RECENT_2 == IDM_FILE_RECENT_1 + 1 &&
		IDM_FILE_RECENT_3 == IDM_FILE_RECENT_1 + 2 &&
		IDM_FILE_RECENT_4 == IDM_FILE_RECENT_1 + 3 &&
		IDM_FILE_RECENT_5 == IDM_FILE_RECENT_1 + 4 &&
		IDM_FILE_RECENT_6 == IDM_FILE_RECENT_1 + 5 &&
		IDM_FILE_RECENT_7 == IDM_FILE_RECENT_1 + 6 &&
		IDM_FILE_RECENT_8 == IDM_FILE_RECENT_1 + 7 &&
		IDM_FILE_RECENT_9 == IDM_FILE_RECENT_1 + 8
	);

	using namespace Managers;

	RecentFiles::RecentFiles(Emulator& e,const Configuration& cfg,Window::Menu& m)
	:
	emulator ( e ),
	menu     ( m )
	{
		static const Window::Menu::CmdHandler::Entry<RecentFiles> commands[] =
		{
			{ IDM_FILE_RECENT_1,     &RecentFiles::OnMenu  },
			{ IDM_FILE_RECENT_2,     &RecentFiles::OnMenu  },
			{ IDM_FILE_RECENT_3,     &RecentFiles::OnMenu  },
			{ IDM_FILE_RECENT_4,     &RecentFiles::OnMenu  },
			{ IDM_FILE_RECENT_5,     &RecentFiles::OnMenu  },
			{ IDM_FILE_RECENT_6,     &RecentFiles::OnMenu  },
			{ IDM_FILE_RECENT_7,     &RecentFiles::OnMenu  },
			{ IDM_FILE_RECENT_8,     &RecentFiles::OnMenu  },
			{ IDM_FILE_RECENT_9,     &RecentFiles::OnMenu  },
			{ IDM_FILE_RECENT_LOCK,  &RecentFiles::OnLock  },
			{ IDM_FILE_RECENT_CLEAR, &RecentFiles::OnClear }
		};

		m.Commands().Add( this, commands );
		emulator.Events().Add( this, &RecentFiles::OnLoad );

		menu[IDM_FILE_RECENT_LOCK].Check( cfg["files recent locked"] == Configuration::YES );

		uint count = 0;

		String::Stack<16,char> index( "files recent x" );
		Path path;

		for (uint i=0; i < MAX_FILES; ++i)
		{
			index.Back() = '1' + i;
			path = cfg[index];

			if (path.Length())
			{
				path.MakePretty();
				path.Insert( 0, "&x ", 3 );
				path[1] = '1' + count;

				menu[IDM_FILE_RECENT_1 + count++].Text() << path;
			}
		}

		menu[IDM_FILE_RECENT_CLEAR].Enable( count );

		for (count += IDM_FILE_RECENT_1; count <= IDM_FILE_RECENT_9; ++count)
			menu[count].Remove();
	}

	RecentFiles::~RecentFiles()
	{
		emulator.Events().Remove( this );
	}

	void RecentFiles::Save(Configuration& cfg) const
	{
		cfg["files recent locked"].YesNo() = menu[IDM_FILE_RECENT_LOCK].IsChecked();

		String::Stack<16,char> index( "files recent x" );
		HeapString path;

		for (uint i=0; i < MAX_FILES && menu[IDM_FILE_RECENT_1 + i].Text() >> path; ++i)
		{
			index.Back() = '1' + i;
			cfg[index].Quote() = path(3);
		}
	}

	void RecentFiles::OnMenu(uint cmd)
	{
		HeapString path;

		if ((menu[cmd].Text() >> path) > 3)
			Application::Instance::Launch( path(3).Ptr() );
	}

	void RecentFiles::OnLock(uint)
	{
		menu[IDM_FILE_RECENT_LOCK].ToggleCheck();
	}

	void RecentFiles::OnClear(uint)
	{
		menu[IDM_FILE_RECENT_CLEAR].Disable();

		for (uint i=IDM_FILE_RECENT_1; i <= IDM_FILE_RECENT_9; ++i)
			menu[i].Remove();
	}

	void RecentFiles::Add(const uint idm,const HeapString& name) const
	{
		if (menu[idm].Exists())
		{
			menu[idm].Text() << name;
		}
		else
		{
			Window::Menu::Item item( menu[IDM_POS_FILE][IDM_POS_FILE_RECENTFILES] );
			menu.Insert( item[item.NumItems() - 3], idm, name );
		}
	}

	void RecentFiles::OnLoad(Emulator::Event event)
	{
		switch (event)
		{
			case Emulator::EVENT_LOAD:

				if (menu[IDM_FILE_RECENT_LOCK].IsUnchecked())
				{
					HeapString items[MAX_FILES];
					HeapString curPath( emulator.GetStartPath() );

					for (uint i=IDM_FILE_RECENT_1, j=0; i <= IDM_FILE_RECENT_9 && menu[i].Text() >> items[j]; ++i)
					{
						if (items[j](3) != curPath)
							items[j][1] = ('2' + j), ++j;
					}

					curPath.Insert( 0, "&1 ", 3 );
					Add( IDM_FILE_RECENT_1, curPath );

					for (uint i=0; i < MAX_FILES-1 && items[i].Length(); ++i)
						Add( IDM_FILE_RECENT_2 + i, items[i] );

					menu[IDM_FILE_RECENT_CLEAR].Enable();
				}
				break;

			case Emulator::EVENT_NETPLAY_MODE_ON:
			case Emulator::EVENT_NETPLAY_MODE_OFF:

				menu[IDM_POS_FILE][IDM_POS_FILE_RECENTFILES].Enable
				(
					event == Emulator::EVENT_NETPLAY_MODE_OFF
				);
				break;
		}
	}
}
