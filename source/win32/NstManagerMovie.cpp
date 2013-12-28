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

#include "NstObjectHeap.hpp"
#include "NstIoFile.hpp"
#include "NstIoLog.hpp"
#include "NstIoScreen.hpp"
#include "NstIoStream.hpp"
#include "NstIoNsp.hpp"
#include "NstResourceString.hpp"
#include "NstWindowUser.hpp"
#include "NstWindowMenu.hpp"
#include "NstDialogMovie.hpp"
#include "NstManagerEmulator.hpp"
#include "NstManagerPaths.hpp"
#include "NstManagerMovie.hpp"
#include "../core/api/NstApiMovie.hpp"

namespace Nestopia
{
	using namespace Managers;

	class Movie::Data
	{
		typedef Window::Movie::MovieFile MovieFile;

	public:

		ibool SetPlayingMode();
		ibool SetRecordingMode();
		ibool Update(const MovieFile&);
		void  Flush();

	private:

		void ClearOutput();

		ibool dirty;
		const Paths& paths;
		Io::Stream::Input input;
		Io::Stream::Output output;
		MovieFile filename;

	public:

		Data(const Paths& p,const MovieFile& m)
		: paths(p), dirty(FALSE)
		{
			Update( m );
		}

		ibool IsRecordingMode() const
		{
			return output.GetData().Size() > 0;
		}

		ibool IsPlayingMode() const
		{
			return input.GetData().Size() > 0;
		}

		ibool CanRecord() const
		{
			return IsRecordingMode() || filename.Size();
		}

		ibool CanPlay() const
		{
			return IsPlayingMode() || IsRecordingMode() || (filename.Size() && Io::File::FileExist( filename ));
		}

		void MarkDirty()
		{
			dirty = TRUE;
		}

		void Validate()
		{
			input.clear();
			output.clear();
		}

		void Invalidate()
		{
			dirty = FALSE;
			input.setstate( Io::Stream::Input::badbit );
			output.setstate( Io::Stream::Output::badbit );
			ClearOutput();
		}

		std::istream& GetInput()
		{
			return input;
		}

		std::ostream& GetOutput(ibool clear)
		{
			if (clear)
				ClearOutput();

			return output;
		}
	};

	struct Movie::Callbacks
	{
		static void NST_CALLBACK OnState(Nes::Movie::UserData data,Nes::Movie::State state)
		{
			NST_ASSERT( data );

			Movie& movie = *static_cast<Movie*>(data);

			if (state == Nes::Movie::STOPPED)
			{
				uint msg;

				if (movie.data->IsPlayingMode()) 
				{
					movie.pos = REWINDED;
					msg = IDS_SCREEN_MOVIE_PLAY_STOPPED;
				}
				else
				{
					movie.pos = FORWARDED;
					msg = IDS_SCREEN_MOVIE_REC_STOPPED;
				}

				Io::Screen() << Resource::String( msg );
			}
			else 
			{
				uint err;

				switch (state)
				{
					case Nes::Movie::ERR_CORRUPT_FILE:		err = IDS_FILE_ERR_CORRUPT;			break;
					case Nes::Movie::ERR_OUT_OF_MEMORY:		err = IDS_ERR_OUT_OF_MEMORY;		break;
					case Nes::Movie::ERR_UNSUPPORTED_IMAGE:	err = IDS_EMU_ERR_UNSUPPORTED_GAME;	break;
					case Nes::Movie::ERR_GENERIC:			err = IDS_ERR_GENERIC;				break;
					default: return;
				}

				movie.pos = REWINDED;
				movie.data->Invalidate();

				Io::Screen() << Resource::String( movie.data->IsPlayingMode() ? IDS_EMU_ERR_MOVIE_PLAY : IDS_EMU_ERR_MOVIE_REC )
							 << ' ' 
							 << Resource::String( err );	
			}

			movie.data->Validate();
			movie.UpdateMenu();
		}
	};

	Movie::Movie(Emulator& e,Window::Menu& m,const Paths& paths)
	: 
	emulator (e), 
	menu     (m), 
	dialog   (new Window::Movie(paths)), 
	data     (new Data(paths,dialog->GetMovieFile())),
	pos      (REWINDED)
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

		m.Commands().Add( this, commands );
		emulator.Events().Add( this, &Movie::OnEmuEvent );
		Nes::Movie::stateCallback.Set( &Callbacks::OnState, this );
	}

	Movie::~Movie()
	{
		emulator.Events().Remove( this );
		Nes::Movie::stateCallback.Set( NULL, NULL );
	}

	void Movie::Data::ClearOutput()
	{
		if (output.GetData().Size())
		{
			Collection::Buffer tmp;
			output.Export( tmp );
		}
	}

	ibool Movie::Data::Update(const MovieFile& newfile)
	{
		if (filename != newfile)
		{
			Flush();

			if (newfile.File().Size())
			{
				filename = newfile;

				if (!filename.Directory().Size())
					filename.Directory() = paths.GetDefaultDirectory( Paths::File::MOVIE );

				return TRUE;
			}
			else
			{
				filename.Clear();
			}
		}

		return FALSE;
	}

	ibool Movie::Data::SetPlayingMode()
	{
		NST_ASSERT( filename.Size() );

		if (input.bad() || output.bad())
			return FALSE;

		if (input.GetData().Empty())
		{
			output.seekp( 0, Io::Stream::Input::end );
			input = output;

			if (input.GetData().Empty())
			{
				Paths::File file;

				if (!paths.Load( file, Paths::File::MOVIE, filename, Paths::STICKY, IDS_EMU_ERR_MOVIE_PLAY ))
					return FALSE;

				input = file.data;
			}
		}

		input.seekg( 0, Io::Stream::Input::beg );
		return TRUE;
	}

	ibool Movie::Data::SetRecordingMode()
	{
		NST_ASSERT( filename.Size() );

		if (input.bad() || output.bad())
			return FALSE;

		if (output.GetData().Empty())
		{
			input.seekg( 0, Io::Stream::Input::end );
			output = input;

			if (output.GetData().Empty() && Io::File::FileExist( filename ))
			{
				Paths::File file;

				if (!paths.Load( file, Paths::File::MOVIE, filename, Paths::STICKY, IDS_EMU_ERR_MOVIE_REC ))
					return FALSE;

				output = file.data;
			}
		}

		output.seekp( 0, Io::Stream::Output::beg );
		return TRUE;
	}

	void Movie::Data::Flush()
	{
		if (filename.Size())
		{
			input.seekg( 0, Io::Stream::Input::end );
			output.seekp( 0, Io::Stream::Input::end );

			Collection::Buffer buffer;
			output.Export( buffer );

			if (buffer.Empty())
				input.Export( buffer );

			if (buffer.Size() && dirty)
				paths.Save( buffer, Paths::File::MOVIE, filename, Paths::STICKY, IDS_EMU_ERR_MOVIE_REC );
		}

		dirty = FALSE;
	}

	void Movie::UpdateMenu() const
	{
		const ibool game = emulator.Is( Nes::Machine::GAME );
		const ibool stopped = Nes::Movie( emulator ).IsStopped();
		const ibool idle = game && stopped;

		menu[ IDM_FILE_MOVIE_RECORD  ].Enable( idle && data->CanRecord() );
		menu[ IDM_FILE_MOVIE_PLAY    ].Enable( idle && data->CanPlay() );
		menu[ IDM_FILE_MOVIE_STOP    ].Enable( game && !stopped );
		menu[ IDM_FILE_MOVIE_REWIND  ].Enable( idle && pos == FORWARDED && data->CanPlay() );
		menu[ IDM_FILE_MOVIE_FORWARD ].Enable( idle && pos == REWINDED && data->CanPlay()  );
	}

	ibool Movie::Load(const Paths::Path& filename,const Alert alert)
	{
		OnCmdStop();

		if (filename.Archive().Size())
		{
			if (alert == NOISY)
				Window::User::Fail( IDS_FILE_ERR_CANT_USE_IN_ARCHIVE );
			else
				Io::Log() << "Movie: ignoring file, can't use it while it's archived..\r\n";

			return FALSE;
		}
		else
		{
			dialog->SetMovieFile( filename );

			if (data->Update( dialog->GetMovieFile() ))		
				pos = REWINDED;

			UpdateMenu();
			
			return TRUE;
		}
	}
  
	void Movie::Save(Io::Nsp::Context& context) const
	{
		context.movie = dialog->GetMovieFile();
	}

	void Movie::OnCmdFile(uint)
	{
		OnCmdStop();
		dialog->Open();
		
		if (data->Update( dialog->GetMovieFile() ))		
			pos = REWINDED;

		UpdateMenu();
	}

	void Movie::OnCmdRecord(uint)
	{
		if (!menu[IDM_FILE_MOVIE_RECORD].IsEnabled())
			return;

		emulator.Wait();

		if (data->SetRecordingMode())
		{
			const ibool on = emulator.Is( Nes::Machine::ON );

			if (on || emulator.Power( TRUE ))
			{
				const Nes::Result result = Nes::Movie(emulator).Record
				( 
			     	data->GetOutput(pos == REWINDED), 
					pos == REWINDED ? Nes::Movie::CLEAN : Nes::Movie::APPEND
				);

				if (NES_SUCCEEDED(result))
				{
					Io::Screen() << Resource::String( IDS_SCREEN_MOVIE_REC_STARTED );

					data->MarkDirty();
					UpdateMenu();

					if (!on)
						emulator.Resume();
				}
				else
				{
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
		if (!menu[IDM_FILE_MOVIE_PLAY].IsEnabled())
			return;

		emulator.Wait();

		if (data->SetPlayingMode())
		{
			const ibool on = emulator.Is( Nes::Machine::ON );

			if (on || emulator.Power( TRUE ))
			{
				const Nes::Result result = Nes::Movie(emulator).Play( data->GetInput() );

				if (NES_SUCCEEDED(result))
				{
					Io::Screen() << Resource::String( IDS_SCREEN_MOVIE_PLAY_STARTED );

					UpdateMenu();

					if (!on)
						emulator.Resume();
				}
				else
				{
					if (!on)
						emulator.Power( FALSE );
				
					if (result != Nes::RESULT_ERR_INVALID_CRC)
					{
						uint msg;
					
						switch (result)
						{
							case Nes::RESULT_ERR_CORRUPT_FILE:             msg = IDS_FILE_ERR_CORRUPT;                  break;  
							case Nes::RESULT_ERR_INVALID_FILE:             msg = IDS_FILE_ERR_INVALID;                  break;  
							case Nes::RESULT_ERR_OUT_OF_MEMORY:	           msg = IDS_ERR_OUT_OF_MEMORY;                 break;
							case Nes::RESULT_ERR_UNSUPPORTED_FILE_VERSION: msg = IDS_EMU_ERR_UNSUPPORTED_FILE_VERSION ; break;
							default:						   	           msg = IDS_EMU_ERR_GENERIC;                   break;
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
		Nes::Movie(emulator).Stop();
	}

	void Movie::OnCmdRewind(uint)
	{
		if (menu[IDM_FILE_MOVIE_REWIND].IsEnabled())
		{
			Io::Screen() << Resource::String( IDS_SCREEN_MOVIE_REWINDED );
			pos = REWINDED;
			UpdateMenu();
		}
	}

	void Movie::OnCmdForward(uint)
	{
		if (menu[IDM_FILE_MOVIE_FORWARD].IsEnabled())
		{
			Io::Screen() << Resource::String( IDS_SCREEN_MOVIE_FORWARDED );		
			pos = FORWARDED;
			UpdateMenu();
		}
	}

	void Movie::OnEmuEvent(Emulator::Event event)
	{
		switch (event)
		{
			case Emulator::EVENT_UNLOAD:

				data->Flush();

			case Emulator::EVENT_LOAD:

				pos = REWINDED;
				UpdateMenu();
				break;

			case Emulator::EVENT_NETPLAY_MODE_ON:
			case Emulator::EVENT_NETPLAY_MODE_OFF:

				menu[IDM_POS_FILE][IDM_POS_FILE_MOVIE].Enable( event == Emulator::EVENT_NETPLAY_MODE_OFF );
				break;
		}
	}
}
