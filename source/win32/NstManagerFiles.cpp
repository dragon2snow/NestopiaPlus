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

#include "NstIoScreen.hpp"
#include "NstIoLog.hpp"
#include "NstIoIps.hpp"
#include "NstIoNsp.hpp"
#include "NstResourceString.hpp"
#include "NstWindowUser.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowDropFiles.hpp"
#include "NstManagerPaths.hpp"
#include "NstManagerPreferences.hpp"
#include "NstManagerMovie.hpp"
#include "NstManagerTapeRecorder.hpp"
#include "NstManagerSaveStates.hpp"
#include "NstManagerCheats.hpp"
#include "NstManagerFiles.hpp"
#include "NstWindowMain.hpp"
#include "../core/api/NstApiMachine.hpp"
#include "../core/api/NstApiCartridge.hpp"

namespace Nestopia
{
	namespace Managers
	{
		Files::Files
		(
			Emulator& e,
			Window::Menu& m,
			const Paths& p,
			const Preferences& r,
			Movie& o,
			const TapeRecorder& t,
			const Cheats& c,
			const SaveStates& s,
			Window::Main& w
		)
		:
		Manager      ( e, m, this, &Files::OnEmuEvent ),
		paths        ( p ),
		preferences  ( r ),
		movie        ( o ),
		tapeRecorder ( t ),
		cheats       ( c ),
		saveStates   ( s ),
		window       ( w )
		{
			static const Window::MsgHandler::Entry<Files> messages[] =
			{
				{ WM_DROPFILES,                         &Files::OnMsgDropFiles },
				{ WM_COPYDATA,                          &Files::OnMsgCopyData  },
				{ Application::Instance::WM_NST_LAUNCH, &Files::OnMsgLaunch    }
			};

			window.Get().Messages().Add( this, messages );

			static const Window::Menu::CmdHandler::Entry<Files> commands[] =
			{
				{ IDM_FILE_OPEN,     &Files::OnCmdOpen       },
				{ IDM_FILE_CLOSE,    &Files::OnCmdClose      },
				{ IDM_FILE_LOAD_NSP, &Files::OnCmdLoadScript },
				{ IDM_FILE_SAVE_NSP, &Files::OnCmdSaveScript }
			};

			menu.Commands().Add( this, commands );

			UpdateMenu();
		}

		Files::~Files()
		{
			window.Get().Messages().Remove( this );
		}

		void Files::UpdateMenu() const
		{
			const bool available = (emulator.NetPlayers() == 0 && emulator.IsGame());

			menu[IDM_FILE_CLOSE].Enable( emulator.IsImage() );
			menu[IDM_POS_FILE][IDM_POS_FILE_SAVE].Enable( available );
			menu[IDM_FILE_SAVE_NSP].Enable( available );
		}

		void Files::Open(tstring const name,uint types) const
		{
			Application::Instance::Events::Signal( Application::Instance::EVENT_SYSTEM_BUSY );

			if (!window.Get().Activate())
				return;

			if (!types)
			{
				types =
				(
					Paths::File::IMAGE |
					Paths::File::STATE |
					Paths::File::MOVIE |
					Paths::File::SCRIPT |
					Paths::File::IPS |
					Paths::File::BATTERY |
					Paths::File::ARCHIVE
				);
			}

			Path path;

			if (name)
			{
				path = name;

				if (path.File().Empty())
					path = paths.BrowseLoad( types, name );
			}
			else
			{
				path = paths.BrowseLoad( types );
			}

			if (path.Empty())
				return;

			if (path == emulator.GetStartPath())
			{
				AutoStart();
				return;
			}

			Paths::File file;
			Io::Ips ips;
			Io::Nsp::Context context;

			switch (paths.Load( file, types, path ))
			{
				case Paths::File::NONE:
					return;

				case Paths::File::STATE:

					if (emulator.IsGame())
					{
						saveStates.Load( file.data, file.name );
						return;
					}

					types = Paths::File::GAME|Paths::File::ARCHIVE;
					context.state = file.name;
					break;

				case Paths::File::MOVIE:

					if (emulator.IsGame())
					{
						if (movie.Load( file.name, Movie::NOISY ) && preferences[Preferences::AUTOSTART_EMULATION])
						{
							AutoStart();
							window.Get().PostCommand( IDM_FILE_MOVIE_PLAY );
						}

						return;
					}

					types = Paths::File::GAME|Paths::File::ARCHIVE;
					context.movie = file.name;
					break;

				case Paths::File::BATTERY:

					if (emulator.IsCart() && Nes::Cartridge(emulator).GetInfo()->setup.wrkRamBacked)
					{
						if (Window::User::Confirm( IDS_LOAD_APPLY_CURRENT_GAME ))
							context.image = emulator.GetImagePath();
					}

					types = Paths::File::CARTRIDGE|Paths::File::ARCHIVE;
					context.save = file.name;
					break;

				case Paths::File::IPS:

					try
					{
						ips.Parse( file.data.Ptr(), file.data.Size() );
					}
					catch (Io::Ips::Exception id)
					{
						Window::User::Fail( id );
						return;
					}

					if (emulator.IsGame())
					{
						if (Window::User::Confirm( IDS_LOAD_APPLY_CURRENT_GAME ))
							context.image = emulator.GetImagePath();
					}

					types = Paths::File::GAME|Paths::File::ARCHIVE;
					context.ips = file.name;
					break;

				case Paths::File::SCRIPT:

					try
					{
						Io::Nsp::File().Load( file.data, context );
					}
					catch (Io::Nsp::File::Exception ids)
					{
						Window::User::Fail( ids );
						return;
					}

					if (context.image.Empty() && emulator.IsGame())
					{
						if (Window::User::Confirm( IDS_LOAD_APPLY_CURRENT_GAME ))
							context.image = emulator.GetImagePath();
					}

					types = Paths::File::GAME|Paths::File::ARCHIVE;
					break;

				default:

					types = Paths::File::NONE;
					context.image = file.name;
					break;
			}

			if (types)
			{
				if (context.image.Empty())
				{
					context.image = file.name;

					if (!paths.LocateFile( context.image, types ))
					{
						if (Window::User::Confirm( IDS_LOAD_SPECIFY_FILE ))
							context.image = paths.BrowseLoad( types );
						else
							context.image.Clear();
					}
				}

				if (context.image.Empty())
					return;

				path = file.name;

				if (!paths.Load( file, types, context.image ))
					return;

				file.name = path;
			}

			NST_ASSERT( file.type & Paths::File::IMAGE );

			if (!ips.Loaded())
			{
				if (context.ips.Empty())
					context.ips = paths.GetIpsPath( context.image, file.type );

				if (context.ips.FileExists())
				{
					Paths::File input;

					if (paths.Load( input, Paths::File::IPS|Paths::File::ARCHIVE, context.ips, Paths::QUIETLY ))
					{
						try
						{
							ips.Parse( input.data.Ptr(), input.data.Size() );
						}
						catch (...)
						{
							ips.Reset();
							Window::User::Warn( IDS_EMU_WARN_IPS_FAILED );
						}
					}
					else
					{
						Window::User::Warn( IDS_EMU_WARN_IPS_FAILED );
					}
				}
			}

			if (ips.Loaded())
			{
				try
				{
					ips.Patch( file.data.Ptr(), file.data.Size() );
					Io::Log() << "Emulator: patched \"" << context.image << "\" with \"" << context.ips << "\"\r\n";
				}
				catch (...)
				{
					Window::User::Warn( IDS_EMU_WARN_IPS_FAILED );
				}
			}

			if (context.save.Empty())
				context.save = paths.GetSavePath( context.image, file.type );

			if (context.tape.Empty())
				context.tape = tapeRecorder.GetFile( context.save );

			window.Load( context );

			if (!emulator.Load( file.data, file.name, context, !preferences[Preferences::SUPPRESS_WARNINGS] ))
				return;

			if (context.mode == Io::Nsp::Context::UNKNOWN && menu[IDM_MACHINE_SYSTEM_AUTO].Checked())
				Nes::Machine(emulator).SetMode( Nes::Machine(emulator).GetDesiredMode() );

			if (context.state.Length())
			{
				if (paths.Load( file, Paths::File::STATE|Paths::File::ARCHIVE, context.state ))
					saveStates.Load( file.data, file.name );
			}

			if (context.movie.Length())
				movie.Load( context.movie, Movie::QUIET );

			cheats.Load( context );

			AutoStart();
		}

		bool Files::Close() const
		{
			if
			(
				!emulator.IsOn() ||
				!preferences[Preferences::CONFIRM_RESET] ||
				Window::User::Confirm( IDS_ARE_YOU_SURE, IDS_MACHINE_POWER_OFF_TITLE )
			)
			{
				emulator.Unload();
				return true;
			}

			return false;
		}

		void Files::AutoStart() const
		{
			if
			(
				preferences[Preferences::AUTOSTART_EMULATION] &&
				emulator.IsImage() && !emulator.IsOn()
			)
				emulator.Power( true );
		}

		void Files::DisplayLoadMessage(const bool loaded) const
		{
			const uint length = window.GetMaxMessageLength();
			const uint threshold = (loaded ? 10 : 12);

			if (length > threshold)
				Io::Screen() << Resource::String( loaded ? IDS_SCREEN_LOADED : IDS_SCREEN_UNLOADED ).Invoke( Path::Compact( emulator.GetImagePath().Target(), length - (threshold - 1) ) );
		}

		ibool Files::OnMsgDropFiles(Window::Param& param)
		{
			Window::DropFiles dropFiles( param );

			if (dropFiles.Size() && menu[IDM_FILE_OPEN].Enabled())
				Open( dropFiles[0].Ptr() );

			return true;
		}

		ibool Files::OnMsgCopyData(Window::Param& param)
		{
			NST_VERIFY( param.lParam );

			if (menu[IDM_FILE_OPEN].Enabled() && param.lParam)
			{
				const COPYDATASTRUCT& copyData = *reinterpret_cast<COPYDATASTRUCT*>(param.lParam);
				NST_VERIFY( copyData.dwData == Application::Instance::COPYDATA_OPENFILE_ID );

				if (copyData.dwData == Application::Instance::COPYDATA_OPENFILE_ID)
				{
					NST_VERIFY( copyData.lpData && copyData.cbData >= sizeof(tchar) && static_cast<tstring>(copyData.lpData)[copyData.cbData/sizeof(tchar)-1] == '\0' );

					if (copyData.lpData && copyData.cbData >= sizeof(tchar) && static_cast<tstring>(copyData.lpData)[copyData.cbData/sizeof(tchar)-1] == '\0')
						Open( static_cast<tstring>(copyData.lpData) );
				}
			}

			param.lResult = true;
			return true;
		}

		ibool Files::OnMsgLaunch(Window::Param& param)
		{
			NST_ASSERT( param.lParam );
			Open( reinterpret_cast<tstring>(param.lParam), param.wParam );
			return true;
		}

		void Files::OnCmdOpen(uint)
		{
			Open();
		}

		void Files::OnCmdClose(uint)
		{
			Close();
		}

		void Files::OnCmdLoadScript(uint)
		{
			Open( NULL, Paths::File::SCRIPT|Paths::File::ARCHIVE );
		}

		void Files::OnCmdSaveScript(uint)
		{
			if (emulator.IsGame())
			{
				Io::Nsp::Context context;

				emulator.Save( context );
				movie.Save( context );
				cheats.Save( context );
				window.Save( context );

				Io::Nsp::File::Output output;
				Io::Nsp::File().Save( output, context );

				const Path path( paths.BrowseSave( Paths::File::SCRIPT, Paths::SUGGEST ) );

				if (path.Length())
					paths.Save( output.Ptr(), output.Length(), Paths::File::SCRIPT, path );
			}
		}

		void Files::OnEmuEvent(const Emulator::Event event,const Emulator::Data data)
		{
			switch (event)
			{
				case Emulator::EVENT_LOAD:
				case Emulator::EVENT_UNLOAD:

					DisplayLoadMessage( event == Emulator::EVENT_LOAD );
					UpdateMenu();
					break;

				case Emulator::EVENT_NETPLAY_MODE:

					menu[IDM_FILE_OPEN].Enable( !data );
					menu[IDM_FILE_LOAD_NSP].Enable( !data );
					menu[IDM_POS_FILE][IDM_POS_FILE_LOAD].Enable( !data );
					break;
			}
		}
	}
}
