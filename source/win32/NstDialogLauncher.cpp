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

#include "NstApplicationInstance.hpp"
#include "NstApplicationException.hpp"
#include "NstSystemKeyboard.hpp"
#include "NstDialogLauncher.hpp"

namespace Nestopia
{
	using namespace Window;

	struct Launcher::Handlers
	{
		static const MsgHandler::Entry<Launcher> messages[];
		static const Menu::CmdHandler::Entry<Launcher> commands[];
		static const Control::NotificationHandler::Entry<Launcher> listNotifications[];
		static const Control::NotificationHandler::Entry<Launcher> treeNotifications[];
	};

	const MsgHandler::Entry<Launcher> Launcher::Handlers::messages[] =
	{
		{ WM_INITDIALOG,  &Launcher::OnInitDialog  },
		{ WM_DESTROY,     &Launcher::OnDestroy     },
		{ WM_SIZE,        &Launcher::OnSize        },
		{ WM_DROPFILES,   &Launcher::OnDropFiles   }
	};

	const Menu::CmdHandler::Entry<Launcher> Launcher::Handlers::commands[] =
	{
		{ IDM_LAUNCHER_FILE_RUN,	               &Launcher::OnCmdFileRun                   },
		{ IDM_LAUNCHER_FILE_REFRESH,               &Launcher::OnCmdFileRefresh               },
		{ IDM_LAUNCHER_VIEW_SHOWGRIDS,			   &Launcher::OnCmdViewShowGrids             },
		{ IDM_LAUNCHER_VIEW_SHOWDATABASECORRECTED, &Launcher::OnCmdViewShowDatabaseCorrected },
		{ IDM_LAUNCHER_OPTIONS_COLORS,             &Launcher::OnCmdOptionsColors             },
		{ IDM_LAUNCHER_OPTIONS_PATHS,              &Launcher::OnCmdOptionsPaths              }
	};

	const Control::NotificationHandler::Entry<Launcher> Launcher::Handlers::listNotifications[] =
	{
		{ LVN_GETDISPINFO,	  &Launcher::OnListGetDisplayInfo    },
		{ LVN_KEYDOWN,	      &Launcher::OnListKeyDown           },
		{ LVN_COLUMNCLICK,	  &Launcher::OnListColumnClick       },
		{ LVN_ITEMACTIVATE,	  &Launcher::OnListItemActivate      },
		{ LVN_ITEMCHANGED,	  &Launcher::OnListItemChanged       },
		{ LVN_INSERTITEM,	  &Launcher::OnListInsertItem        },
		{ LVN_DELETEALLITEMS, &Launcher::OnListDeleteAllItems    },
		{ LVN_DELETEITEM,	  &Launcher::OnListDeleteItem        },
	};

	const Control::NotificationHandler::Entry<Launcher> Launcher::Handlers::treeNotifications[] =
	{ 
		{ TVN_SELCHANGING, &Launcher::OnTreeSelectionChanging }
	};

	Launcher::Launcher(const Nes::Cartridge::Database& database,const Managers::Paths& p,const Configuration& cfg)
	: 
	dialog            ( IDD_LAUNCHER, this, Handlers::messages ),
	menu              ( IDR_MENU_LAUNCHER ),
	listNotifications ( IDC_LAUNCHER_LIST, dialog.Messages() ),
	treeNotifications ( IDC_LAUNCHER_TREE, dialog.Messages() ),
	statusBar         ( dialog, STATUSBAR_SECOND_FIELD_WIDTH ),
	list              ( dialog, menu.Commands(), p, cfg, database ),
	colors            ( cfg )
	{
		menu.Commands().Add( this, Handlers::commands );
		dialog.Commands().Add( CMD_ENTER, this, &Launcher::OnCmdEnter );
		listNotifications.Add( this, Handlers::listNotifications );
		treeNotifications.Add( this, Handlers::treeNotifications );

		HeapString name;

		for (uint i=0; i < 5; ++i)
		{
			static const u16 keys[5][2] =
			{
				{ IDM_LAUNCHER_FILE_RUN,     VK_RETURN },
				{ IDM_LAUNCHER_FILE_REFRESH, VK_F5     },
				{ IDM_LAUNCHER_EDIT_FIND,    VK_F3     },
				{ IDM_LAUNCHER_EDIT_INSERT,  VK_INSERT },
				{ IDM_LAUNCHER_EDIT_REMOVE,  VK_DELETE }
			};

			menu[keys[i][0]].Text() >> name;
			menu[keys[i][0]].Text() << (name << '\t' << System::Keyboard::GetName( keys[i][1] ));
		}
	}

	Launcher::~Launcher()
	{
		dialog.Close();
	}

	void Launcher::Save(Configuration& cfg,ibool saveFiles)
	{
		list.Save( cfg, saveFiles );
		colors.Save( cfg );
	}

	void Launcher::Open()
	{
		dialog.Open( Dialog::MODELESS );
	}

	void Launcher::Close()
	{
		dialog.Close();
	}

	ibool Launcher::OnInitDialog(Param&)
	{
		menu.Hook( dialog );
		menu.Show();
		statusBar.Enable();

		list = dialog.ListView( IDC_LAUNCHER_LIST );
		tree = dialog.TreeView( IDC_LAUNCHER_TREE );

		dialog.SetIcon( IDI_WINDOW );

		margin = dialog.GetRectangle().Corner() - list.GetWindow().GetRectangle().Corner();

		list.SetColors( colors.GetBackgroundColor(), colors.GetForegroundColor() );
		tree.SetColors( colors.GetBackgroundColor(), colors.GetForegroundColor() );

		menu[ IDM_LAUNCHER_VIEW_SHOWGRIDS ].Check( list.GetStyle() & LVS_EX_GRIDLINES );
		menu[ IDM_LAUNCHER_VIEW_SHOWDATABASECORRECTED ].Check( list.DatabaseCorrectionEnabled() );
		menu[ IDM_LAUNCHER_FILE_REFRESH ].Enable( list.CanRefresh() );

		return TRUE;
	}

	ibool Launcher::OnDestroy(Param&)
	{
		tree.Close();
		list.Close();
		
		return TRUE;
	}

	ibool Launcher::OnSize(Param& param)
	{
		if (param.wParam != SIZE_MINIMIZED)
		{
			const Point edge( dialog.GetRectangle().Corner() );

			list.GetWindow().Resize
			( 
				edge - list.GetWindow().GetPosition() - margin 
			);

			const Rect rect( tree.GetWindow().GetRectangle() );

			tree.GetWindow().Resize
			( 
				Point( rect.right - rect.left, edge.y - rect.top - margin.y ) 
			);
		}

		return TRUE;
	}

	ibool Launcher::OnCmdEnter(Param&)
	{
		OnCmdFileRun(); 
		return TRUE;
	}

	ibool Launcher::OnDropFiles(Param& param)
	{
		list.Insert( param );
		return TRUE;
	}

	void Launcher::UpdateItemCount(const u32 count) const
	{
		menu[IDM_LAUNCHER_EDIT_FIND].Enable( count != 0 );

		statusBar.Text(StatusBar::SECOND_FIELD) << (String::Stack<20,tchar>(_T(" Files: ")) << count).Ptr();
	}

	void Launcher::OnListGetDisplayInfo(const NMHDR& nmhdr)
	{
		list.OnGetDisplayInfo( reinterpret_cast<LPARAM>(&nmhdr) ); 
	}

	void Launcher::OnListKeyDown(const NMHDR& nmhdr)
	{
		switch (reinterpret_cast<const NMLVKEYDOWN&>(nmhdr).wVKey)
		{
			case VK_INSERT: if (menu[ IDM_LAUNCHER_EDIT_INSERT  ].IsEnabled()) dialog.PostCommand( IDM_LAUNCHER_EDIT_INSERT  ); break;	
			case VK_DELETE: if (menu[ IDM_LAUNCHER_EDIT_REMOVE  ].IsEnabled()) dialog.PostCommand( IDM_LAUNCHER_EDIT_REMOVE  ); break;
			case VK_F3:     if (menu[ IDM_LAUNCHER_EDIT_FIND    ].IsEnabled()) dialog.PostCommand( IDM_LAUNCHER_EDIT_FIND    ); break;
			case VK_F5:     if (menu[ IDM_LAUNCHER_FILE_REFRESH ].IsEnabled()) dialog.PostCommand( IDM_LAUNCHER_FILE_REFRESH ); break;
		}
	}

	void Launcher::OnListColumnClick(const NMHDR& nmhdr)
	{
		Application::Instance::Waiter wait;
		list.Sort( reinterpret_cast<const NMLISTVIEW&>(nmhdr).iSubItem );
	}

	void Launcher::OnListItemActivate(const NMHDR&)
	{
		OnCmdFileRun();
	}

	void Launcher::OnListItemChanged(const NMHDR& nmhdr)
	{
		const NMLISTVIEW& nmlv = reinterpret_cast<const NMLISTVIEW&>(nmhdr);

		if ((nmlv.uOldState ^ nmlv.uNewState) & LVIS_SELECTED)
		{
			if (nmlv.uNewState & LVIS_SELECTED)
			{
				if (const List::Files::Entry* const entry = list[nmlv.iItem])
				{
					{
						Path path( entry->GetPath(list.GetStrings()), entry->GetFile(list.GetStrings()) );

						if (path.Length() > _MAX_PATH)
						{
							path.ShrinkTo( _MAX_PATH-3 );
							path << "...";
						}

						statusBar.Text(StatusBar::FIRST_FIELD) << path.Ptr();
					}

					menu[IDM_LAUNCHER_FILE_RUN].Enable();
					menu[IDM_LAUNCHER_EDIT_REMOVE].Enable();
				}
			}
			else
			{
				OnNoSelection();
			}
		}
	}

	void Launcher::OnListInsertItem(const NMHDR&)
	{
		UpdateItemCount( list.Size() );
	}

	void Launcher::OnListDeleteItem(const NMHDR&)
	{
		const uint size = list.Size();
		UpdateItemCount( size - (size != 0) );
	}

	void Launcher::OnListDeleteAllItems(const NMHDR&)
	{
		OnNoSelection();
		const uint size = list.Size();
		UpdateItemCount( size - (size != 0) );
	}

	void Launcher::OnTreeSelectionChanging(const NMHDR& nmhdr)
	{
		list.Draw( tree.GetType(reinterpret_cast<const NMTREEVIEW&>(nmhdr).itemNew.hItem) );
	}

	void Launcher::OnNoSelection() const
	{
		statusBar.Text(StatusBar::FIRST_FIELD).Clear(); 

		menu[IDM_LAUNCHER_FILE_RUN].Disable();
		menu[IDM_LAUNCHER_EDIT_REMOVE].Disable();
	}

	void Launcher::OnCmdFileRun(uint) 
	{
		if (const List::Files::Entry* const entry = list.GetSelection())
			Application::Instance::Launch( Path(entry->GetPath(list.GetStrings()),entry->GetFile(list.GetStrings())).Ptr() );
	}

	void Launcher::OnCmdFileRefresh(uint) 
	{
		list.Refresh();
	}

	void Launcher::OnCmdViewShowGrids(uint) 
	{
		menu[IDM_LAUNCHER_VIEW_SHOWGRIDS].Check( list.ToggleGrids() );
	}

	void Launcher::OnCmdViewShowDatabaseCorrected(uint) 
	{
		menu[IDM_LAUNCHER_VIEW_SHOWDATABASECORRECTED].Check( list.ToggleDatabase() );
	}

	void Launcher::OnCmdOptionsPaths(uint) 
	{
		list.OpenPathDialog();
		menu[IDM_LAUNCHER_FILE_REFRESH].Enable( list.CanRefresh() );
	}

	void Launcher::OnCmdOptionsColors(uint) 
	{
		colors.Open();

		list.SetColors( colors.GetBackgroundColor(), colors.GetForegroundColor(), List::REPAINT );
		tree.SetColors( colors.GetBackgroundColor(), colors.GetForegroundColor(), Tree::REPAINT );
	}
}
