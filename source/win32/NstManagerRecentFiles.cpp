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

#include "NstIoFile.hpp"
#include "NstWindowMenu.hpp"
#include "NstManagerEmulator.hpp"
#include "NstManagerRecentFiles.hpp"

namespace Nestopia
{
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

		uint count = 0;

		String::Stack<16,char> index( "files recent x" );
		Name name( _T("&x ") );

		for (uint i=0; i < MAX_FILES; ++i)
		{
			index.Back() = (char) ('1' + i);
			const GenericString file( cfg[index] );

			if (file.Length())
			{
				name[1] = (tchar) ('1' + count);
				name(3) = file;
				menu[IDM_FILE_RECENT_1 + count++].Text() << name;
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
		String::Stack<16,char> index( "files recent x" );
		Name file;

		for (uint i=0; i < MAX_FILES && menu[IDM_FILE_RECENT_1 + i].Text() >> file; ++i)
		{
			index.Back() = (char) ('1' + i);
			cfg[index].Quote() = file(3);
		}
	}

	void RecentFiles::OnMenu(uint cmd)
	{
		Name file;

		if ((menu[cmd].Text() >> file) > 3)
			Application::Instance::Launch( file(3).Ptr() );
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

	void RecentFiles::Add(const uint idm,const Name& name) const
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
					Name items[MAX_FILES];
		
					for (uint i=IDM_FILE_RECENT_1, j=0; i <= IDM_FILE_RECENT_9 && menu[i].Text() >> items[j]; ++i)
					{
						if (items[j](3) != emulator.GetStartPath())
						{
							items[j][1] = (char) ('2' + j);
							++j;
						}
					}
		
					Add( IDM_FILE_RECENT_1, Name("&1 ") << emulator.GetStartPath() );
		
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
