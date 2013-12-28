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
#include "NstManagerEmulator.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowUser.hpp"
#include "NstWindowMenu.hpp"
#include "NstDialogSound.hpp"
#include "NstManagerSound.hpp"
#include "NstManagerSoundRecorder.hpp"
#include "NstResourceString.hpp"
#include "NstIoScreen.hpp"

namespace Nestopia
{
	using namespace Managers;

	Sound::Recorder::Recorder
	(
		Window::Menu& m,
		Window::Sound::Recorder& d,
		Emulator& e
	)
	: 
	recording ( FALSE ), 
	dialog    ( d ),
	menu      ( m ),
	emulator  ( e )
	{
		static const Window::Menu::CmdHandler::Entry<Sound::Recorder> commands[] =
		{
			{ IDM_FILE_SOUND_RECORDER_FILE,   &Recorder::OnMenuFile   },
			{ IDM_FILE_SOUND_RECORDER_START,  &Recorder::OnMenuRecord },
			{ IDM_FILE_SOUND_RECORDER_STOP,   &Recorder::OnMenuStop   },
			{ IDM_FILE_SOUND_RECORDER_REWIND, &Recorder::OnMenuRewind }
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
			dialog.WaveFile().Size() &&
			waveFormat.nSamplesPerSec && 
			waveFormat.wBitsPerSample &&
			emulator.Is(Nes::Machine::ON)
		);
	}

	void Sound::Recorder::Close()
	{
		recording = FALSE;

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
		if (dialog.WaveFile().Size())
		{
			switch (event)
			{
				case Emulator::EVENT_POWER_OFF:
		
					recording = FALSE;
		
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
		menu[ IDM_FILE_SOUND_RECORDER_REWIND ].Enable( canRecord );
	}

	void Sound::Recorder::OnMenuFile(uint)
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

	void Sound::Recorder::OnMenuRecord(uint)
	{
		NST_ASSERT( !recording && dialog.WaveFile().Size() );

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

		recording = TRUE;
		UpdateMenu();
	}

	void Sound::Recorder::OnMenuStop(uint)
	{
		recording = FALSE;
		UpdateMenu();
		Io::Screen() << Resource::String(IDS_SCREEN_SOUND_RECORDER_STOP);
	}

	void Sound::Recorder::OnMenuRewind(uint)
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
	}

	void Sound::Recorder::Flush(const Nes::Sound::Output& output)
	{
		if (recording)
		{
			NST_ASSERT( file.IsOpen() );

			try
			{
				file.Write( output.samples, output.length * waveFormat.nBlockAlign );
			}
			catch (Io::Wave::Exception ids)
			{
				recording = FALSE;
				
				Window::User::Fail( ids );
				UpdateMenu();
				
				return;
			}

			size += output.length;

			if (size >= nextBigSizeNotification)
			{
				nextBigSizeNotification += BIG_SIZE;
				Window::User::Inform( IDS_WAVE_WARN_FILE_BIG );
			}
			else if (size >= nextSmallSizeNotification)
			{
				Io::Screen() << (nextSmallSizeNotification / ONE_MB) << Resource::String(IDS_SCREEN_SOUND_RECORDER_WRITTEN);
				nextSmallSizeNotification += SMALL_SIZE;
			}
		}
	}
}
