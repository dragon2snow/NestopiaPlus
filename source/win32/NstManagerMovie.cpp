////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2005 Martin Freij
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
#include "NstIoScreen.hpp"
#include "NstIoNsp.hpp"
#include "NstResourceString.hpp"
#include "NstWindowUser.hpp"
#include "NstDialogMovie.hpp"
#include "NstManagerEmulator.hpp"
#include "NstManagerPaths.hpp"
#include "NstManagerMovie.hpp"
#include "../core/api/NstApiMovie.hpp"
#include "../core/api/NstApiRewinder.hpp"

namespace Nestopia
{
	using namespace Managers;

	struct Movie::Callbacks
	{
		static void NST_CALLBACK OnState(Nes::Movie::UserData data,Nes::Movie::State state)
		{
			NST_ASSERT( data );

			uint msg;
			Movie& movie = *static_cast<Movie*>(data);

			if (NES_SUCCEEDED(state))
			{
				switch (state)
				{
					case Nes::Movie::PLAYING:	
						
						msg = IDS_SCREEN_MOVIE_PLAY_STARTED; 
						break;

					case Nes::Movie::RECORDING: 
						
						msg = IDS_SCREEN_MOVIE_REC_STARTED; 
						break;

					case Nes::Movie::STOPPED_PLAYING:	

						movie.Close( REWINDED );
						msg = IDS_SCREEN_MOVIE_PLAY_STOPPED;						
						break;

					case Nes::Movie::STOPPED_RECORDING:	

						movie.Close( FORWARDED );
						msg = IDS_SCREEN_MOVIE_REC_STOPPED;
						break;

					default: 
						
						NST_DEBUG_MSG("Movie::Callbacks::OnState() unknown state!");
						return;
				}

				Io::Screen() << Resource::String( msg );
			}
			else 
			{
				movie.Close( REWINDED, FALSE );

				switch (state)
				{
					case Nes::Movie::ERR_CORRUPT_FILE:		msg = IDS_FILE_ERR_CORRUPT;			break;
					case Nes::Movie::ERR_OUT_OF_MEMORY:		msg = IDS_ERR_OUT_OF_MEMORY;		break;
					case Nes::Movie::ERR_UNSUPPORTED_IMAGE:	msg = IDS_EMU_ERR_UNSUPPORTED_GAME;	break;
					case Nes::Movie::ERR_GENERIC:
					default:                    			msg = IDS_ERR_GENERIC;				break;
				}

				Io::Screen() << Resource::String( Nes::Movie(movie.emulator).IsPlaying() ? IDS_EMU_ERR_MOVIE_PLAY : IDS_EMU_ERR_MOVIE_REC )
						     << ' ' 
						     << Resource::String( msg );	
			}
		}
	};

	Movie::Movie(Emulator& e,Window::Menu& m,const Paths& paths)
	: 
	emulator (e), 
	pos      (REWINDED),
	menu     (m), 
	dialog   (new Window::Movie(paths))
	{
		static const Window::Menu::CmdHandler::Entry<Movie> commands[] =
		{
			{ IDM_FILE_MOVIE_FILE,    &Movie::OnCmdFile    },
			{ IDM_FILE_MOVIE_RECORD,  &Movie::OnCmdRecord  },
			{ IDM_FILE_MOVIE_PLAY,    &Movie::OnCmdPlay    },
			{ IDM_FILE_MOVIE_STOP,    &Movie::OnCmdStop    },
			{ IDM_FILE_MOVIE_REWIND,  &Movie::OnCmdRewind  },
			{ IDM_FILE_MOVIE_FORWARD, &Movie::OnCmdForward }
		};

		static const Window::Menu::PopupHandler::Entry<Movie> popups[] =
		{	  
			{ Window::Menu::PopupHandler::Pos<IDM_POS_FILE,IDM_POS_FILE_MOVIE>::ID, &Movie::OnMenuView }
		};

		m.Commands().Add( this, commands );
		m.PopupRouter().Add( this, popups );
		emulator.Events().Add( this, &Movie::OnEmuEvent );
		Nes::Movie::stateCallback.Set( &Callbacks::OnState, this );
	}

	Movie::~Movie()
	{
		emulator.Events().Remove( this );
		Nes::Movie::stateCallback.Set( NULL, NULL );
	}

    ibool Movie::CanPlay() const
	{
		return
		(
    		emulator.Is( Nes::Machine::GAME ) &&
			Nes::Movie( emulator ).IsStopped() &&
			dialog->GetMovieFile().Size() &&
			Io::File::FileExist( dialog->GetMovieFile() )
		);
	}

	ibool Movie::CanRecord() const
	{
		return
		(
			emulator.Is( Nes::Machine::GAME ) &&
			Nes::Movie( emulator ).IsStopped() &&
			dialog->GetMovieFile().Size() &&
			!Io::File::FileProtected( dialog->GetMovieFile() )
		);
	}

	ibool Movie::CanStop() const
	{
		return !Nes::Movie( emulator ).IsStopped();
	}

	ibool Movie::CanRewind() const
	{
		return pos != REWINDED && CanPlay();
	}

	ibool Movie::CanForward() const
	{
		return 
		(
	     	pos != FORWARDED && 
			emulator.Is( Nes::Machine::GAME ) &&
			Nes::Movie( emulator ).IsStopped() &&
			dialog->GetMovieFile().Size() &&
			Io::File::FileExist( dialog->GetMovieFile() ) &&
			!Io::File::FileProtected( dialog->GetMovieFile() )
		);
	}

    void Movie::OnMenuView(Window::Menu::PopupHandler::Param& param)
	{
		param.menu[ IDM_FILE_MOVIE_PLAY    ].Enable( CanPlay()    );
		param.menu[ IDM_FILE_MOVIE_RECORD  ].Enable( CanRecord()  );
		param.menu[ IDM_FILE_MOVIE_STOP    ].Enable( CanStop()    );
		param.menu[ IDM_FILE_MOVIE_REWIND  ].Enable( CanRewind()  );
		param.menu[ IDM_FILE_MOVIE_FORWARD ].Enable( CanForward() );
	}

	ibool Movie::Open(const std::fstream::open_mode mode)
	{
		NST_ASSERT( dialog->GetMovieFile().Size() && !stream.is_open() );

		stream.clear();
		stream.open( dialog->GetMovieFile(), mode | std::fstream::binary );

		if (stream.is_open())
			return TRUE;
		
		Io::Screen() << Resource::String( (mode & std::fstream::out) ? IDS_EMU_ERR_MOVIE_REC : IDS_EMU_ERR_MOVIE_PLAY )
			         << ' ' 
			         << Resource::String( IDS_FILE_ERR_OPEN );		

		return FALSE;
	}

	void Movie::Close(Pos p,ibool ok)
	{
		pos = p;

		if (stream.is_open())
		{
			stream.seekp( 0, ok ? std::fstream::end : std::fstream::beg );
			stream.close();
		}
	}

	ibool Movie::Load(const Paths::Path& fileName,const Alert alert)
	{
		if (fileName.Archive().Size())
		{
			Nes::Movie(emulator).Eject();

			dialog->ClearMovieFile();

			if (alert == NOISY)
				Window::User::Fail( IDS_FILE_ERR_CANT_USE_IN_ARCHIVE );
			else
				Io::Log() << "Movie: ignoring file, can't use it while it's archived..\r\n";

			return FALSE;
		}
		else if (dialog->SetMovieFile( fileName ))
		{
			Nes::Movie(emulator).Eject();
		}

		return TRUE;
	}
  
	void Movie::Save(Io::Nsp::Context& context) const
	{
		context.movie = dialog->GetMovieFile();
	}

	void Movie::OnCmdFile(uint)
	{
		const Paths::Path old( dialog->GetMovieFile() );
		dialog->Open();
		
		if (old != dialog->GetMovieFile())
			Nes::Movie(emulator).Eject();
	}

	void Movie::OnCmdRecord(uint)
	{
		const ibool on = emulator.Is( Nes::Machine::ON );

		if (CanRecord() && (on || emulator.Power( TRUE )))
		{
			if (Nes::Rewinder(emulator).IsEnabled())
			{
				Io::Screen() << Resource::String( IDS_EMU_ERR_MOVIE_REWINDER );
			}
			else if (Open( (pos == REWINDED) ? (std::fstream::trunc|std::fstream::in|::std::fstream::out) : (std::fstream::in|std::fstream::out) ))
			{
				const Nes::Result result = Nes::Movie(emulator).Record( stream, pos == REWINDED ? Nes::Movie::CLEAN : Nes::Movie::APPEND );  
			
				if (NES_FAILED(result))
				{
					Close( REWINDED, FALSE );
			
					if (!on)
						emulator.Power( FALSE );
				
					uint msg;
				
					switch (result)
					{
						case Nes::RESULT_ERR_CORRUPT_FILE:  msg = IDS_FILE_ERR_CORRUPT;  break;  
						case Nes::RESULT_ERR_INVALID_FILE:  msg = IDS_FILE_ERR_INVALID;  break;  
						case Nes::RESULT_ERR_OUT_OF_MEMORY:	msg = IDS_ERR_OUT_OF_MEMORY; break;
						default:							msg = IDS_EMU_ERR_GENERIC;   break;
					}
				
					Io::Screen() << Resource::String( IDS_EMU_ERR_MOVIE_REC )
					         	 << ' ' 
								 << Resource::String( msg );		
				}
			}
		}
	}

	void Movie::OnCmdPlay(uint)
	{
		const ibool on = emulator.Is( Nes::Machine::ON );

		if (CanPlay() && (on || emulator.Power( TRUE )))
		{
			if (Nes::Rewinder(emulator).IsEnabled())
			{
				Io::Screen() << Resource::String( IDS_EMU_ERR_MOVIE_REWINDER );
			}
			else if (Open( std::fstream::in ))
			{
				const Nes::Result result = Nes::Movie(emulator).Play( stream );

				if (NES_SUCCEEDED(result))
				{
					emulator.AskBeforeSaving();
				}
				else
				{
					Close( REWINDED, FALSE );

					if (!on)
						emulator.Power( FALSE );

					if (result != Nes::RESULT_ERR_INVALID_CRC)
					{
						uint msg;

						switch (result)
						{
							case Nes::RESULT_ERR_CORRUPT_FILE:             msg = IDS_FILE_ERR_CORRUPT;                 break;  
							case Nes::RESULT_ERR_INVALID_FILE:             msg = IDS_FILE_ERR_INVALID;                 break;
							case Nes::RESULT_ERR_OUT_OF_MEMORY:	           msg = IDS_ERR_OUT_OF_MEMORY;                break;
							case Nes::RESULT_ERR_UNSUPPORTED_FILE_VERSION: msg = IDS_EMU_ERR_UNSUPPORTED_FILE_VERSION; break;
							default:						   	           msg = IDS_EMU_ERR_GENERIC;                  break;
						}

						Io::Screen() << Resource::String( IDS_EMU_ERR_MOVIE_PLAY )
							         << ' ' 
							         << Resource::String( msg );	
					}
				}			
			}
		}
	}

	void Movie::OnCmdStop(uint)
	{
		Nes::Movie(emulator).Eject();
	}

	void Movie::OnCmdRewind(uint)
	{
		if (CanRewind())
		{
			pos = REWINDED;
			Io::Screen() << Resource::String( IDS_SCREEN_MOVIE_REWINDED );
		}
	}

	void Movie::OnCmdForward(uint)
	{
		if (CanForward())
		{
			pos = FORWARDED;
			Io::Screen() << Resource::String( IDS_SCREEN_MOVIE_FORWARDED );		
		}
	}

	void Movie::OnEmuEvent(Emulator::Event event)
	{
		switch (event)
		{
			case Emulator::EVENT_NETPLAY_MODE_ON:
			case Emulator::EVENT_NETPLAY_MODE_OFF:

				menu[IDM_POS_FILE][IDM_POS_FILE_MOVIE].Enable( event == Emulator::EVENT_NETPLAY_MODE_OFF );
				break;
		}
	}
}
