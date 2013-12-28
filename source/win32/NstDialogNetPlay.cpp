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
#include "NstIoLog.hpp"
#include "NstApplicationInstance.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowDropFiles.hpp"
#include "NstManagerPaths.hpp"
#include "NstManagerEmulator.hpp"
#include "NstDialogNetPlay.hpp"
#include "../core/api/NstApiCartridge.hpp"
#include <CommCtrl.h>

namespace Nestopia
{
	namespace Window
	{
		struct Netplay::Handlers
		{
			static const MsgHandler::Entry<Netplay> messages[];
			static const MsgHandler::Entry<Netplay> commands[];
			static const Control::NotificationHandler::Entry<Netplay> notifications[];
		};

		const MsgHandler::Entry<Netplay> Netplay::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &Netplay::OnInitDialog },
			{ WM_DROPFILES,  &Netplay::OnDropFiles  }
		};

		const MsgHandler::Entry<Netplay> Netplay::Handlers::commands[] =
		{
			{ IDC_NETPLAY_ADD,             &Netplay::OnAdd        },
			{ IDC_NETPLAY_REMOVE,          &Netplay::OnRemove     },
			{ IDC_NETPLAY_CLEAR,           &Netplay::OnClear      },
			{ IDC_NETPLAY_DEFAULT,         &Netplay::OnDefault    },
			{ IDC_NETPLAY_LAUNCH,          &Netplay::OnLaunch     },
			{ IDC_NETPLAY_PLAY_FULLSCREEN, &Netplay::OnFullscreen }
		};

		const Control::NotificationHandler::Entry<Netplay> Netplay::Handlers::notifications[] =
		{
			{ LVN_KEYDOWN,     &Netplay::OnKeyDown        },
			{ LVN_ITEMCHANGED, &Netplay::OnItemChanged    },
			{ LVN_INSERTITEM,  &Netplay::OnInsertItem     },
			{ LVN_DELETEITEM,  &Netplay::OnDeleteItem     }
		};

		Netplay::Games::Games()
		: state(UNINITIALIZED) {}

		Netplay::Games::~Games()
		{
			for (Iterator it=Begin(), end=End(); it != end; ++it)
				it->Path::~Path();
		}

		Netplay::Games::Iterator Netplay::Games::Add(const Path& path)
		{
			if (Iterator const it = Insert( path ))
			{
				state = DIRTY;
				return it;
			}

			return NULL;
		}

		void Netplay::Games::Erase(uint index)
		{
			state = DIRTY;
			Iterator const it = At( index );
			it->Path::~Path();
			Array().Erase( it );
		}

		Netplay::Netplay(Managers::Emulator& e,const Managers::Paths& p,ibool fullscreen)
		:
		dialog        ( IDD_NETPLAY, this, Handlers::messages, Handlers::commands ),
		doFullscreen  ( fullscreen ),
		paths         ( p ),
		emulator      ( e ),
		notifications ( IDC_NETPLAY_GAMELIST, dialog.Messages(), this, Handlers::notifications )
		{
		}

		void Netplay::LoadFile()
		{
			HeapString text;

			try
			{
				Io::File( Application::Instance::GetExePath(_T("netplaylist.dat")), Io::File::COLLECT ).ReadText( text );
			}
			catch (Io::File::Exception id)
			{
				if (id == Io::File::ERR_NOT_FOUND)
				{
					Io::Log() << "Netplay: game list file \"netplaylist.dat\" not present..\r\n";
				}
				else
				{
					Io::Log() << "Netplay: warning, couldn't load game list \"netplaylist.dat\"!\r\n";
					games.state = Games::DIRTY;
				}
				return;
			}

			text << '\n';

			Path path;

			for (tstring it=text.Ptr(),offset=text.Ptr(); *it; )
			{
				if (*it == '\r' || *it == '\n')
				{
					if (const uint length = it - offset)
					{
						path.Assign( offset, length );
						path.Trim();
						Add( path );
					}

					do
					{
						++it;
					}
					while (*it == '\r' || *it == '\n');

					offset = it;
				}
				else
				{
					++it;
				}
			}
		}

		void Netplay::SaveFile() const
		{
			if (games.state == Games::DIRTY)
			{
				Io::Log log;
				const Path path( Application::Instance::GetExePath(_T("netplaylist.dat")) );

				if (games.Size())
				{
					HeapString text;

					for (Games::ConstIterator it=games.Begin(), end=games.End(); it != end; ++it)
						text << *it << "\r\n";

					try
					{
						Io::File( path, Io::File::DUMP ).WriteText( text.Ptr(), text.Length() );
						log << "Netplay: saved game list to \"netplaylist.dat\"\r\n";
					}
					catch (Io::File::Exception)
					{
						log << "Netplay: warning, couldn't save game list to \"netplaylist.dat\"!\r\n";
					}
				}
				else if (path.FileExists())
				{
					if (Io::File::Delete( path.Ptr() ))
						log << "Netplay: game list empty, deleted \"netplaylist.dat\"\r\n";
					else
						log << "Netplay: warning, couldn't delete \"netplaylist.dat\"!\r\n";
				}
			}
		}

		void Netplay::Add(Path path)
		{
			enum
			{
				TYPES = Managers::Paths::File::GAME|Managers::Paths::File::ARCHIVE
			};

			if (path.Length() && games.Size() < Games::LIMIT && paths.CheckFile( path, TYPES ) && games.Add( path ) && dialog)
				dialog.ListView( IDC_NETPLAY_GAMELIST ).Add( path.Target().File() );
		}

		ibool Netplay::OnInitDialog(Param&)
		{
			dialog.CheckBox( IDC_NETPLAY_PLAY_FULLSCREEN ).Check( doFullscreen );
			dialog.CheckBox( IDC_NETPLAY_REMOVE ).Disable();

			if (games.state == Games::UNINITIALIZED)
			{
				LoadFile();
			}
			else
			{
				Control::ListView list( dialog.ListView( IDC_NETPLAY_GAMELIST ) );
				list.Reserve( games.Size() );

				for (Games::ConstIterator it=games.Begin(), end=games.End(); it != end; ++it)
					list.Add( it->Target().File().Ptr() );
			}

			dialog.CheckBox( IDC_NETPLAY_CLEAR ).Enable( games.Size() );
			dialog.CheckBox( IDC_NETPLAY_LAUNCH ).Enable( games.Size() );

			return true;
		}

		ibool Netplay::OnAdd(Param& param)
		{
			if (param.Button().Clicked())
				Add( paths.BrowseLoad(Managers::Paths::File::GAME|Managers::Paths::File::ARCHIVE) );

			return true;
		}

		ibool Netplay::OnRemove(Param& param)
		{
			if (param.Button().Clicked())
				dialog.ListView( IDC_NETPLAY_GAMELIST ).Selection().Delete();

			return true;
		}

		ibool Netplay::OnClear(Param& param)
		{
			if (param.Button().Clicked())
				dialog.ListView( IDC_NETPLAY_GAMELIST ).Clear();

			return true;
		}

		ibool Netplay::OnDefault(Param& param)
		{
			if (param.Button().Clicked())
				dialog.CheckBox( IDC_NETPLAY_PLAY_FULLSCREEN ).Check( doFullscreen = false );

			return true;
		}

		ibool Netplay::OnLaunch(Param& param)
		{
			if (param.Button().Clicked())
				dialog.Close( LAUNCH );

			return true;
		}

		ibool Netplay::OnFullscreen(Param& param)
		{
			if (param.Button().Clicked())
				doFullscreen = dialog.CheckBox( IDC_NETPLAY_PLAY_FULLSCREEN ).Checked();

			return true;
		}

		ibool Netplay::OnDropFiles(Param& param)
		{
			DropFiles dropFiles( param );

			if (dropFiles.Inside( dialog.ListView( IDC_NETPLAY_GAMELIST ).GetWindow() ))
			{
				for (uint i=0, n=dropFiles.Size(); i < n; ++i)
					Add( dropFiles[i] );
			}

			return true;
		}

		void Netplay::OnKeyDown(const NMHDR& nmhdr)
		{
			switch (reinterpret_cast<const NMLVKEYDOWN&>(nmhdr).wVKey)
			{
				case VK_INSERT: dialog.PostCommand( IDC_NETPLAY_ADD    ); break;
				case VK_DELETE: dialog.PostCommand( IDC_NETPLAY_REMOVE ); break;
			}
		}

		void Netplay::OnItemChanged(const NMHDR& nmhdr)
		{
			const NMLISTVIEW& nm = reinterpret_cast<const NMLISTVIEW&>(nmhdr);

			if ((nm.uOldState ^ nm.uNewState) & LVIS_SELECTED)
				dialog.CheckBox( IDC_NETPLAY_REMOVE ).Enable( nm.uNewState & LVIS_SELECTED );
		}

		void Netplay::OnInsertItem(const NMHDR&)
		{
			dialog.CheckBox( IDC_NETPLAY_CLEAR ).Enable();
			dialog.CheckBox( IDC_NETPLAY_LAUNCH ).Enable();
		}

		void Netplay::OnDeleteItem(const NMHDR& nmhdr)
		{
			games.Erase( reinterpret_cast<const NMLISTVIEW&>(nmhdr).iItem );

			if (dialog.ListView( IDC_NETPLAY_GAMELIST ).Size() <= 1)
			{
				dialog.CheckBox( IDC_NETPLAY_CLEAR ).Disable();
				dialog.CheckBox( IDC_NETPLAY_LAUNCH ).Disable();
				dialog.CheckBox( IDC_NETPLAY_REMOVE ).Disable();
			}
		}
	}
}
