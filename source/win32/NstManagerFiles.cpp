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

#include "NstObjectHeap.hpp"
#include "NstIoFile.hpp"
#include "NstIoScreen.hpp"
#include "NstIoLog.hpp"
#include "NstIoIps.hpp"
#include "NstIoNsp.hpp"
#include "NstResourceString.hpp"
#include "NstWindowUser.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowDropFiles.hpp"
#include "NstWindowMenu.hpp"
#include "NstWindowMain.hpp"
#include "NstApplicationInstance.hpp"
#include "NstManagerEmulator.hpp"
#include "NstManagerPreferences.hpp"
#include "NstManagerPaths.hpp"
#include "NstManagerMovie.hpp"
#include "NstManagerTapeRecorder.hpp"
#include "NstManagerSaveStates.hpp"
#include "NstManagerCheats.hpp"
#include "NstManagerFiles.hpp"
#include "../core/api/NstApiCartridge.hpp"

namespace Nestopia
{
	using namespace Managers;

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
	emulator     ( e ),
	menu         ( m ),
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

		static const Window::Menu::CmdHandler::Entry<Files> commands[] =
		{
			{ IDM_FILE_OPEN,	 &Files::OnCmdOpen       },
			{ IDM_FILE_CLOSE,	 &Files::OnCmdClose      },
			{ IDM_FILE_LOAD_NSP, &Files::OnCmdLoadScript },
			{ IDM_FILE_SAVE_NSP, &Files::OnCmdSaveScript }
		};

		w.Get().Messages().Add( this, messages );
		m.Commands().Add( this, commands );
		emulator.Events().Add( this, &Files::OnEmuEvent );
	}

	Files::~Files()
	{
		emulator.Events().Remove( this );
	}

	void Files::Open(tstring const name,uint types) const
	{
		emulator.Wait();

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

				if (emulator.Is( Nes::Machine::GAME ))
				{
					saveStates.Load( file.data, file.name );
					return;
				}

				types = Paths::File::GAME|Paths::File::ARCHIVE;
				context.state = file.name;
				break;

			case Paths::File::MOVIE:

				if (emulator.Is( Nes::Machine::GAME ))
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

				if (emulator.Is( Nes::Machine::CARTRIDGE ) && Nes::Cartridge(emulator).GetInfo()->battery)
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

				if (emulator.Is( Nes::Machine::GAME ))
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

				if (context.image.Empty() && emulator.Is( Nes::Machine::GAME ))
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

			if (Io::File::FileExist( context.ips.Ptr() ))
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
		
		if (context.mode == Io::Nsp::Context::UNKNOWN && menu[IDM_MACHINE_SYSTEM_AUTO].IsChecked())
			emulator.AutoSetMode();

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

	ibool Files::Close() const
	{
		if 
		(
			!emulator.Is( Nes::Machine::ON ) ||
			!preferences[Preferences::CONFIRM_RESET] ||
			Window::User::Confirm( IDS_ARE_YOU_SURE, IDS_MACHINE_POWER_OFF_TITLE )
		)
		{
			emulator.Unload();
			return TRUE;
		}

		return FALSE;
	}

	void Files::AutoStart() const
	{
		if 
		(
			preferences[Preferences::AUTOSTART_EMULATION] &&
			emulator.Is(Nes::Machine::IMAGE) && !emulator.Is(Nes::Machine::ON)
		)
		{
			emulator.Power( TRUE );

			if (emulator.Is( Nes::Machine::SOUND ))
				emulator.PlaySong();
		}
	}

	ibool Files::OnMsgDropFiles(Window::Param& param)
	{
		Window::DropFiles dropFiles( param );

		if (dropFiles.Size() && menu[IDM_FILE_OPEN].IsEnabled())
			Open( dropFiles[0].Ptr() );

		return TRUE;
	}

	ibool Files::OnMsgCopyData(Window::Param& param)
	{
		NST_VERIFY( param.lParam );

		if (menu[IDM_FILE_OPEN].IsEnabled() && param.lParam)
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

		param.lResult = TRUE;
		return TRUE;
	}

	ibool Files::OnMsgLaunch(Window::Param& param)
	{
		NST_ASSERT( param.lParam );
		Open( reinterpret_cast<tstring>(param.lParam), param.wParam );
		return TRUE;
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
		if (emulator.Is( Nes::Machine::GAME ))
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

	void Files::OnEmuEvent(Emulator::Event event)
	{
		switch (event)
		{
			case Emulator::EVENT_LOAD:
			{
				menu[ IDM_FILE_CLOSE ].Enable();
				menu[ IDM_POS_FILE ][ IDM_POS_FILE_SAVE ].Enable( emulator.Is(Nes::Machine::GAME) );
				menu[ IDM_FILE_SAVE_NSP ].Enable( emulator.Is(Nes::Machine::GAME) );

				const uint length = window.GetMaxMessageLength();

				if (length > 10)
				{
					Io::Screen() << Resource::String( IDS_SCREEN_LOADED ) 
						         << " \""
								 << Path::Compact( emulator.GetImagePath().Target(), length - 9 ) 
								 << '\"';
				}
				break;
			}
		
			case Emulator::EVENT_UNLOAD:
			{
				menu[ IDM_FILE_CLOSE ].Disable();
				menu[ IDM_POS_FILE ][ IDM_POS_FILE_SAVE ].Disable();
				menu[ IDM_FILE_SAVE_NSP ].Disable();

				const uint length = window.GetMaxMessageLength();

				if (length > 12)
				{
					Io::Screen() << Resource::String( IDS_SCREEN_UNLOADED ) 
						         << " \""
								 << Path::Compact( emulator.GetImagePath().Target(), length - 11 ) 
								 << '\"';
				}				
				break;
			}		

			case Emulator::EVENT_NETPLAY_MODE_ON:
			case Emulator::EVENT_NETPLAY_MODE_OFF:
			{
				const ibool state = (event == Emulator::EVENT_NETPLAY_MODE_OFF);

				menu[ IDM_FILE_OPEN ].Enable( state );
				menu[ IDM_FILE_LOAD_NSP ].Enable( state );
				menu[ IDM_POS_FILE ][ IDM_POS_FILE_LOAD ].Enable( state );
				break;
			}
		}
	}
}
