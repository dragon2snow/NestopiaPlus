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

#include <iostream>
#include "NstIoLog.hpp"
#include "NstIoScreen.hpp"
#include "NstIoNsp.hpp"
#include "NstResourceString.hpp"
#include "NstWindowUser.hpp"
#include "NstManagerPaths.hpp"
#include "NstDialogMovie.hpp"
#include "NstManagerAviConverter.hpp"
#include "NstManagerMovie.hpp"
#include "../core/api/NstApiRewinder.hpp"

namespace Nestopia
{
	namespace Managers
	{
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
					movie.Close( REWINDED, false );

					switch (state)
					{
						case Nes::Movie::ERR_CORRUPT_FILE:      msg = IDS_FILE_ERR_CORRUPT;         break;
						case Nes::Movie::ERR_OUT_OF_MEMORY:     msg = IDS_ERR_OUT_OF_MEMORY;        break;
						case Nes::Movie::ERR_UNSUPPORTED_IMAGE: msg = IDS_EMU_ERR_UNSUPPORTED_GAME; break;
						case Nes::Movie::ERR_GENERIC:
						default:                                msg = IDS_ERR_GENERIC;              break;
					}

					Io::Screen() << Resource::String( Nes::Movie(movie.emulator).IsPlaying() ? IDS_EMU_ERR_MOVIE_PLAY : IDS_EMU_ERR_MOVIE_REC )
                                 << ' '
                                 << Resource::String( msg );
				}
			}
		};

		Movie::Movie(Emulator& e,Window::Menu& m,const Paths& p)
		:
		emulator (e),
		pos      (REWINDED),
		menu     (m),
		dialog   (new Window::Movie(p)),
		paths    (p)
		{
			static const Window::Menu::CmdHandler::Entry<Movie> commands[] =
			{
				{ IDM_FILE_MOVIE_FILE,       &Movie::OnCmdFile      },
				{ IDM_FILE_MOVIE_RECORD,     &Movie::OnCmdRecord    },
				{ IDM_FILE_MOVIE_PLAY,       &Movie::OnCmdPlay      },
				{ IDM_FILE_MOVIE_STOP,       &Movie::OnCmdStop      },
				{ IDM_FILE_MOVIE_REWIND,     &Movie::OnCmdRewind    },
				{ IDM_FILE_MOVIE_FORWARD,    &Movie::OnCmdForward   },
				{ IDM_FILE_MOVIE_EXPORT_AVI, &Movie::OnCmdExportAvi }
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
				dialog->GetMovieFile().Length() &&
				dialog->GetMovieFile().FileExists()
			);
		}

		ibool Movie::CanRecord() const
		{
			return
			(
				emulator.Is( Nes::Machine::GAME ) &&
				Nes::Movie( emulator ).IsStopped() &&
				dialog->GetMovieFile().Length() &&
				!dialog->GetMovieFile().FileProtected()
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
				dialog->GetMovieFile().Length() &&
				dialog->GetMovieFile().FileExists() &&
				!dialog->GetMovieFile().FileProtected()
			);
		}

		void Movie::OnMenuView(Window::Menu::PopupHandler::Param& param)
		{
			param.menu[ IDM_FILE_MOVIE_PLAY       ].Enable( CanPlay()    );
			param.menu[ IDM_FILE_MOVIE_RECORD     ].Enable( CanRecord()  );
			param.menu[ IDM_FILE_MOVIE_STOP       ].Enable( CanStop()    );
			param.menu[ IDM_FILE_MOVIE_REWIND     ].Enable( CanRewind()  );
			param.menu[ IDM_FILE_MOVIE_FORWARD    ].Enable( CanForward() );
			param.menu[ IDM_FILE_MOVIE_EXPORT_AVI ].Enable( CanPlay()    );
		}

		ibool Movie::Open(const std::fstream::openmode mode)
		{
			NST_ASSERT( dialog->GetMovieFile().Length() && !stream.is_open() );

			stream.clear();
			stream.open( dialog->GetMovieFile().Ptr(), mode | std::fstream::binary );

			if (stream.is_open())
				return true;

			Io::Screen() << Resource::String( (mode & std::fstream::out) ? IDS_EMU_ERR_MOVIE_REC : IDS_EMU_ERR_MOVIE_PLAY )
                         << ' '
                         << Resource::String( IDS_FILE_ERR_OPEN );

			return false;
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

		ibool Movie::Load(const Path& fileName,const Alert alert)
		{
			if (fileName.Archive().Length())
			{
				Nes::Movie(emulator).Eject();

				dialog->ClearMovieFile();

				if (alert == NOISY)
					Window::User::Fail( IDS_FILE_ERR_CANT_USE_IN_ARCHIVE );
				else
					Io::Log() << "Movie: ignoring file, can't use it while it's archived..\r\n";

				return false;
			}
			else if (dialog->SetMovieFile( fileName ))
			{
				Nes::Movie(emulator).Eject();
			}

			return true;
		}

		void Movie::Save(Io::Nsp::Context& context) const
		{
			context.movie = dialog->GetMovieFile();
		}

		void Movie::OnCmdFile(uint)
		{
			const Path old( dialog->GetMovieFile() );
			dialog->Open();

			if (old != dialog->GetMovieFile())
				Nes::Movie(emulator).Eject();
		}

		void Movie::OnCmdRecord(uint)
		{
			const ibool on = emulator.Is( Nes::Machine::ON );

			if (CanRecord() && (on || emulator.Power( true )))
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
						Close( REWINDED, false );

						if (!on)
							emulator.Power( false );

						uint msg;

						switch (result)
						{
							case Nes::RESULT_ERR_CORRUPT_FILE:  msg = IDS_FILE_ERR_CORRUPT;  break;
							case Nes::RESULT_ERR_INVALID_FILE:  msg = IDS_FILE_ERR_INVALID;  break;
							case Nes::RESULT_ERR_OUT_OF_MEMORY: msg = IDS_ERR_OUT_OF_MEMORY; break;
							default:                            msg = IDS_EMU_ERR_GENERIC;   break;
						}

						Io::Screen() << Resource::String( IDS_EMU_ERR_MOVIE_REC )
                                     << ' '
                                     << Resource::String( msg );
					}
				}
			}

			Application::Instance::GetMainWindow().Post( Application::Instance::WM_NST_COMMAND_RESUME );
		}

		void Movie::OnCmdPlay(uint)
		{
			const ibool on = emulator.Is( Nes::Machine::ON );

			if (CanPlay() && (on || emulator.Power( true )))
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
						Close( REWINDED, false );

						if (!on)
							emulator.Power( false );

						if (result != Nes::RESULT_ERR_INVALID_CRC)
						{
							uint msg;

							switch (result)
							{
								case Nes::RESULT_ERR_CORRUPT_FILE:             msg = IDS_FILE_ERR_CORRUPT;                 break;
								case Nes::RESULT_ERR_INVALID_FILE:             msg = IDS_FILE_ERR_INVALID;                 break;
								case Nes::RESULT_ERR_OUT_OF_MEMORY:            msg = IDS_ERR_OUT_OF_MEMORY;                break;
								case Nes::RESULT_ERR_UNSUPPORTED_FILE_VERSION: msg = IDS_EMU_ERR_UNSUPPORTED_FILE_VERSION; break;
								default:                                       msg = IDS_EMU_ERR_GENERIC;                  break;
							}

							Io::Screen() << Resource::String( IDS_EMU_ERR_MOVIE_PLAY )
                                         << ' '
                                         << Resource::String( msg );
						}
					}
				}
			}

			Application::Instance::GetMainWindow().Post( Application::Instance::WM_NST_COMMAND_RESUME );
		}

		void Movie::OnCmdExportAvi(uint)
		{
			if (CanPlay())
			{
				if (Nes::Rewinder(emulator).IsEnabled())
				{
					Io::Screen() << Resource::String( IDS_EMU_ERR_MOVIE_REWINDER );
				}
				else if (Open( std::fstream::in ))
				{
					if (const uint ids = AviConverter( emulator, stream ).Record( paths.BrowseSave( Paths::File::AVI, Paths::SUGGEST, dialog->GetMovieFile() ) ))
					{
						if (ids == IDS_AVI_WRITE_ABORT || ids == IDS_AVI_WRITE_FINISHED)
							Window::User::Inform( ids );
						else
							Window::User::Fail( ids );
					}

					Close( REWINDED );
				}
			}
		}

		void Movie::OnCmdStop(uint)
		{
			Nes::Movie(emulator).Eject();
			Application::Instance::GetMainWindow().Post( Application::Instance::WM_NST_COMMAND_RESUME );
		}

		void Movie::OnCmdRewind(uint)
		{
			if (CanRewind())
			{
				pos = REWINDED;
				Io::Screen() << Resource::String( IDS_SCREEN_MOVIE_REWINDED );
			}

			Application::Instance::GetMainWindow().Post( Application::Instance::WM_NST_COMMAND_RESUME );
		}

		void Movie::OnCmdForward(uint)
		{
			if (CanForward())
			{
				pos = FORWARDED;
				Io::Screen() << Resource::String( IDS_SCREEN_MOVIE_FORWARDED );
			}

			Application::Instance::GetMainWindow().Post( Application::Instance::WM_NST_COMMAND_RESUME );
		}

		void Movie::OnEmuEvent(Emulator::Event event)
		{
			switch (event)
			{
				case Emulator::EVENT_NETPLAY_MODE_ON:
				case Emulator::EVENT_NETPLAY_MODE_OFF:

					menu[IDM_POS_FILE][IDM_POS_FILE_MOVIE].Enable( event == Emulator::EVENT_NETPLAY_MODE_OFF );
					break;

				case Emulator::EVENT_POWER_OFF:

					if (Nes::Movie(emulator).IsPlaying())
						Nes::Movie(emulator).Eject();

					break;
			}
		}
	}
}
