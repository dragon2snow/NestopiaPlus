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

#include "NstWindowParam.hpp"
#include "NstDialogBrowse.hpp"
#include "NstDialogLauncher.hpp"

namespace Nestopia
{
	namespace Window
	{
		struct Launcher::List::Paths::Handlers
		{
			static const MsgHandler::Entry<Paths> messages[];
			static const MsgHandler::Entry<Paths> commands[];
			static const Control::NotificationHandler::Entry<Paths> notifications[];
		};

		const MsgHandler::Entry<Launcher::List::Paths> Launcher::List::Paths::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &Paths::OnInitDialog }
		};

		const MsgHandler::Entry<Launcher::List::Paths> Launcher::List::Paths::Handlers::commands[] =
		{
			{ IDC_LAUNCHER_PATHS_ADD,    &Paths::OnCmdAdd    },
			{ IDC_LAUNCHER_PATHS_REMOVE, &Paths::OnCmdRemove },
			{ IDC_LAUNCHER_PATHS_CLEAR,  &Paths::OnCmdClear  },
			{ IDOK,                      &Paths::OnCmdOk     }
		};

		const Control::NotificationHandler::Entry<Launcher::List::Paths> Launcher::List::Paths::Handlers::notifications[] =
		{
			{ LVN_KEYDOWN,     &Paths::OnKeyDown     },
			{ LVN_ITEMCHANGED, &Paths::OnItemChanged },
			{ LVN_INSERTITEM,  &Paths::OnInsertItem  },
			{ LVN_DELETEITEM,  &Paths::OnDeleteItem  }
		};

		Launcher::List::Paths::Paths(const Configuration& cfg)
		:
		dialog ( IDD_LAUNCHER_PATHS, this, Handlers::messages, Handlers::commands ),
		notifications ( IDC_LAUNCHER_PATHS_LIST, dialog.Messages(), this, Handlers::notifications )
		{
			settings.include[ Settings::Include::NES     ] = ( cfg[ "launcher search files nes"          ] != Configuration::NO  );
			settings.include[ Settings::Include::UNF     ] = ( cfg[ "launcher search files unf"          ] != Configuration::NO  );
			settings.include[ Settings::Include::FDS     ] = ( cfg[ "launcher search files fds"          ] != Configuration::NO  );
			settings.include[ Settings::Include::NSF     ] = ( cfg[ "launcher search files nsf"          ] != Configuration::NO  );
			settings.include[ Settings::Include::IPS     ] = ( cfg[ "launcher search files ips"          ] != Configuration::NO  );
			settings.include[ Settings::Include::NSP     ] = ( cfg[ "launcher search files nsp"          ] != Configuration::NO  );
			settings.include[ Settings::Include::ARCHIVE ] = ( cfg[ "launcher search files archive"      ] != Configuration::NO  );
			settings.include[ Settings::Include::ANY     ] = ( cfg[ "launcher search any file extension" ] == Configuration::YES );
			settings.include[ Settings::Include::UNIQUE  ] = ( cfg[ "launcher search no duplicate files" ] != Configuration::NO  );

			NST_COMPILE_ASSERT( LIMIT <= 999 );

			String::Stack<32,char> type("launcher search path ");
			Path path;

			for (uint i=0; i < LIMIT; ++i)
			{
				type(21) = (i+1);
				path = cfg[type];

				if (path.Empty())
					break;

				path.MakePretty( true );

				type << " subs";
				settings.folders.resize( settings.folders.size() + 1 );
				settings.folders.back().path = path;
				settings.folders.back().incSubDir = (cfg[type] == Configuration::YES);
			}
		}

		void Launcher::List::Paths::Save(Configuration& cfg) const
		{
			cfg[ "launcher search files nes"          ].YesNo() = settings.include[ Settings::Include::NES     ];
			cfg[ "launcher search files unf"          ].YesNo() = settings.include[ Settings::Include::UNF     ];
			cfg[ "launcher search files fds"          ].YesNo() = settings.include[ Settings::Include::FDS     ];
			cfg[ "launcher search files nsf"          ].YesNo() = settings.include[ Settings::Include::NSF     ];
			cfg[ "launcher search files ips"          ].YesNo() = settings.include[ Settings::Include::IPS     ];
			cfg[ "launcher search files nsp"          ].YesNo() = settings.include[ Settings::Include::NSP     ];
			cfg[ "launcher search files archive"      ].YesNo() = settings.include[ Settings::Include::ARCHIVE ];
			cfg[ "launcher search any file extension" ].YesNo() = settings.include[ Settings::Include::ANY     ];
			cfg[ "launcher search no duplicate files" ].YesNo() = settings.include[ Settings::Include::UNIQUE  ];

			String::Heap<char> type;

			for (uint i=0, n=settings.folders.size(); i < n; ++i)
			{
				type = "launcher search path ";
				type << (i+1);

				NST_ASSERT( settings.folders[i].path.Length() );
				cfg[type].Quote() = settings.folders[i].path;

				type << " subs";
				cfg[type].YesNo() = settings.folders[i].incSubDir;
			}
		}

		ibool Launcher::List::Paths::OnInitDialog(Param&)
		{
			dialog.CheckBox( IDC_LAUNCHER_PATHS_NES         ).Check( settings.include[ Settings::Include::NES     ] );
			dialog.CheckBox( IDC_LAUNCHER_PATHS_UNF         ).Check( settings.include[ Settings::Include::UNF     ] );
			dialog.CheckBox( IDC_LAUNCHER_PATHS_FDS         ).Check( settings.include[ Settings::Include::FDS     ] );
			dialog.CheckBox( IDC_LAUNCHER_PATHS_NSF         ).Check( settings.include[ Settings::Include::NSF     ] );
			dialog.CheckBox( IDC_LAUNCHER_PATHS_IPS         ).Check( settings.include[ Settings::Include::IPS     ] );
			dialog.CheckBox( IDC_LAUNCHER_PATHS_NSP         ).Check( settings.include[ Settings::Include::NSP     ] );
			dialog.CheckBox( IDC_LAUNCHER_PATHS_ARCHIVE     ).Check( settings.include[ Settings::Include::ARCHIVE ] );
			dialog.CheckBox( IDC_LAUNCHER_PATHS_ALLFILES    ).Check( settings.include[ Settings::Include::ANY     ] );
			dialog.CheckBox( IDC_LAUNCHER_PATHS_UNIQUEFILES ).Check( settings.include[ Settings::Include::UNIQUE  ] );

			dialog.Control( IDC_LAUNCHER_PATHS_REMOVE ).Enable( false );
			dialog.Control( IDC_LAUNCHER_PATHS_CLEAR ).Enable( settings.folders.size() );

			const Control::ListView listView( dialog.ListView(IDC_LAUNCHER_PATHS_LIST) );

			listView.StyleEx() = LVS_EX_CHECKBOXES;
			listView.Reserve( settings.folders.size() );

			for (Settings::Folders::const_iterator it(settings.folders.begin()), end(settings.folders.end()); it != end; ++it)
				listView.Add( it->path, LPARAM(0), it->incSubDir );

			return true;
		}

		ibool Launcher::List::Paths::OnCmdAdd(Param& param)
		{
			if (param.Button().Clicked() && dialog.ListView( IDC_LAUNCHER_PATHS_LIST ).Size() < LIMIT)
			{
				const Path dir( Browser::SelectDirectory() );

				if (dir.Length())
					dialog.ListView( IDC_LAUNCHER_PATHS_LIST ).Add( dir );
			}

			return true;
		}

		ibool Launcher::List::Paths::OnCmdRemove(Param& param)
		{
			if (param.Button().Clicked())
				dialog.ListView( IDC_LAUNCHER_PATHS_LIST ).Selection().Delete();

			return true;
		}

		ibool Launcher::List::Paths::OnCmdClear(Param& param)
		{
			if (param.Button().Clicked())
				dialog.ListView( IDC_LAUNCHER_PATHS_LIST ).Clear();

			return true;
		}

		ibool Launcher::List::Paths::OnCmdOk(Param& param)
		{
			if (param.Button().Clicked())
			{
				settings.include[ Settings::Include::NES     ] = dialog.CheckBox( IDC_LAUNCHER_PATHS_NES         ).Checked();
				settings.include[ Settings::Include::UNF     ] = dialog.CheckBox( IDC_LAUNCHER_PATHS_UNF         ).Checked();
				settings.include[ Settings::Include::FDS     ] = dialog.CheckBox( IDC_LAUNCHER_PATHS_FDS         ).Checked();
				settings.include[ Settings::Include::NSF     ] = dialog.CheckBox( IDC_LAUNCHER_PATHS_NSF         ).Checked();
				settings.include[ Settings::Include::IPS     ] = dialog.CheckBox( IDC_LAUNCHER_PATHS_IPS         ).Checked();
				settings.include[ Settings::Include::NSP     ] = dialog.CheckBox( IDC_LAUNCHER_PATHS_NSP         ).Checked();
				settings.include[ Settings::Include::ARCHIVE ] = dialog.CheckBox( IDC_LAUNCHER_PATHS_ARCHIVE     ).Checked();
				settings.include[ Settings::Include::ANY     ] = dialog.CheckBox( IDC_LAUNCHER_PATHS_ALLFILES    ).Checked();
				settings.include[ Settings::Include::UNIQUE  ] = dialog.CheckBox( IDC_LAUNCHER_PATHS_UNIQUEFILES ).Checked();

				const Control::ListView listView( dialog.ListView(IDC_LAUNCHER_PATHS_LIST) );
				settings.folders.resize( listView.Size() );

				for (uint i=0, n=settings.folders.size(); i < n; ++i)
				{
					listView[i].Text() >> settings.folders[i].path;
					settings.folders[i].incSubDir = listView[i].Checked();
				}

				dialog.Close();
			}

			return true;
		}

		void Launcher::List::Paths::OnKeyDown(const NMHDR& nmhdr)
		{
			switch (reinterpret_cast<const NMLVKEYDOWN&>(nmhdr).wVKey)
			{
				case VK_INSERT: dialog.PostCommand( IDC_LAUNCHER_PATHS_ADD    ); break;
				case VK_DELETE: dialog.PostCommand( IDC_LAUNCHER_PATHS_REMOVE ); break;
			}
		}

		void Launcher::List::Paths::OnItemChanged(const NMHDR& nmhdr)
		{
			const NMLISTVIEW& nm = reinterpret_cast<const NMLISTVIEW&>(nmhdr);

			if ((nm.uOldState ^ nm.uNewState) & LVIS_SELECTED)
				dialog.Control( IDC_LAUNCHER_PATHS_REMOVE ).Enable( nm.uNewState & LVIS_SELECTED );
		}

		void Launcher::List::Paths::OnInsertItem(const NMHDR&)
		{
			dialog.Control( IDC_LAUNCHER_PATHS_CLEAR ).Enable();
		}

		void Launcher::List::Paths::OnDeleteItem(const NMHDR&)
		{
			if (dialog.ListView( IDC_LAUNCHER_PATHS_LIST ).Size() <= 1)
			{
				dialog.Control( IDC_LAUNCHER_PATHS_CLEAR ).Disable();
				dialog.Control( IDC_LAUNCHER_PATHS_REMOVE ).Disable();
			}
		}
	}
}
