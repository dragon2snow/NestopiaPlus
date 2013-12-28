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

#include "NstResourceString.hpp"
#include "NstIoScreen.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowUser.hpp"
#include "NstWindowMenu.hpp"
#include "NstManagerEmulator.hpp"
#include "NstDialogSound.hpp"
#include "NstManagerSound.hpp"
#include "NstManagerSoundRecorder.hpp"

namespace Nestopia
{
	namespace Managers
	{
		Sound::Recorder::Recorder
		(
			Window::Menu& m,
			Window::Sound::Recorder& d,
			Emulator& e
		)
		:
		recording ( false ),
		file      ( Io::Wave::MODE_WRITE ),
		dialog    ( d ),
		menu      ( m ),
		emulator  ( e )
		{
			static const Window::Menu::CmdHandler::Entry<Sound::Recorder> commands[] =
			{
				{ IDM_FILE_SOUND_RECORDER_FILE,   &Recorder::OnCmdFile   },
				{ IDM_FILE_SOUND_RECORDER_START,  &Recorder::OnCmdRecord },
				{ IDM_FILE_SOUND_RECORDER_STOP,   &Recorder::OnCmdStop   },
				{ IDM_FILE_SOUND_RECORDER_REWIND, &Recorder::OnCmdRewind }
			};

			m.Commands().Add( this, commands );
			emulator.Events().Add( this, &Recorder::OnEmuEvent );
		}

		Sound::Recorder::~Recorder()
		{
			emulator.Events().Remove( this );
		}

		ibool Sound::Recorder::CanRecord() const
		{
			return
			(
				dialog.WaveFile().Length() &&
				waveFormat.nSamplesPerSec &&
				waveFormat.wBitsPerSample &&
				emulator.Is(Nes::Machine::ON)
			);
		}

		void Sound::Recorder::Close()
		{
			recording = false;

			try
			{
				file.Close();
			}
			catch (Io::Wave::Exception id)
			{
				Window::User::Fail( id );
			}

			UpdateMenu();
		}

		void Sound::Recorder::Enable(const WAVEFORMATEX& newWaveFormat)
		{
			if (waveFormat != newWaveFormat)
			{
				waveFormat = newWaveFormat;
				Close();
			}
		}

		void Sound::Recorder::Disable()
		{
			waveFormat.Clear();
			Close();
		}

		void Sound::Recorder::OnEmuEvent(Emulator::Event event)
		{
			if (dialog.WaveFile().Length())
			{
				switch (event)
				{
					case Emulator::EVENT_POWER_OFF:

						recording = false;

					case Emulator::EVENT_POWER_ON:

						UpdateMenu();
						return;
				}
			}

			switch (event)
			{
				case Emulator::EVENT_NETPLAY_MODE_ON:
				case Emulator::EVENT_NETPLAY_MODE_OFF:

					menu[IDM_POS_FILE][IDM_POS_FILE_SOUNDRECORDER].Enable( event == Emulator::EVENT_NETPLAY_MODE_OFF );
					break;
			}
		}

		void Sound::Recorder::UpdateMenu() const
		{
			const ibool canRecord = CanRecord();
			NST_ASSERT( bool(recording) <= bool(canRecord) );

			menu[ IDM_FILE_SOUND_RECORDER_START  ].Enable( canRecord && !recording );
			menu[ IDM_FILE_SOUND_RECORDER_STOP   ].Enable( recording );
			menu[ IDM_FILE_SOUND_RECORDER_REWIND ].Enable( canRecord && !recording );
		}

		void Sound::Recorder::OnCmdFile(uint)
		{
			dialog.Open();

			if (file.GetName() != dialog.WaveFile())
			{
				if (file.IsOpen())
					Close();
				else
					UpdateMenu();
			}
		}

		void Sound::Recorder::OnCmdRecord(uint)
		{
			NST_ASSERT( !recording && dialog.WaveFile().Length() );

			Application::Instance::GetMainWindow().Post( Application::Instance::WM_NST_COMMAND_RESUME );

			if (!file.IsOpen())
			{
				try
				{
					file.Open( dialog.WaveFile(), waveFormat );
				}
				catch (Io::Wave::Exception id)
				{
					Window::User::Fail( id );
					return;
				}

				size = 0;
				nextSmallSizeNotification = SMALL_SIZE;
				nextBigSizeNotification = BIG_SIZE;
			}

			Io::Screen() << Resource::String(size ? IDS_SCREEN_SOUND_RECORDER_RESUME : IDS_SCREEN_SOUND_RECORDER_START);

			recording = true;
			UpdateMenu();
		}

		void Sound::Recorder::OnCmdStop(uint)
		{
			recording = false;
			UpdateMenu();
			Io::Screen() << Resource::String(IDS_SCREEN_SOUND_RECORDER_STOP);
			Application::Instance::GetMainWindow().Post( Application::Instance::WM_NST_COMMAND_RESUME );
		}

		void Sound::Recorder::OnCmdRewind(uint)
		{
			try
			{
				file.Close();
			}
			catch (Io::Wave::Exception id)
			{
				Window::User::Fail( id );
			}

			UpdateMenu();
			Application::Instance::GetMainWindow().Post( Application::Instance::WM_NST_COMMAND_RESUME );
		}

		void Sound::Recorder::Flush(const Nes::Sound::Output& output)
		{
			if (recording)
			{
				NST_ASSERT( file.IsOpen() );

				try
				{
					for (uint i=0; i < 2; ++i)
						file.Write( output.samples[i], output.length[i] * waveFormat.nBlockAlign );
				}
				catch (Io::Wave::Exception ids)
				{
					recording = false;

					Window::User::Fail( ids );
					UpdateMenu();

					return;
				}

				size += output.length[0] + output.length[1];

				if (size >= nextBigSizeNotification)
				{
					nextBigSizeNotification += BIG_SIZE;
					Window::User::Inform( IDS_WAVE_WARN_FILE_BIG );
				}
				else if (size >= nextSmallSizeNotification)
				{
					Io::Screen() << Resource::String(IDS_SCREEN_SOUND_RECORDER_WRITTEN).Invoke( HeapString() << (nextSmallSizeNotification / ONE_MB) );
					nextSmallSizeNotification += SMALL_SIZE;
				}
			}
		}
	}
}
