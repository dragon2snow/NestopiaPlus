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

#include "resource/resource.h"
#include "NstObjectHeap.hpp"
#include "NstApplicationException.hpp"
#include "NstManagerEmulator.hpp"
#include "NstWindowMenu.hpp"
#include "NstDialogSound.hpp"
#include "NstManagerSound.hpp"
#include "NstManagerSoundRecorder.hpp"

namespace Nestopia
{
	using namespace Managers;

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("t", on)
    #endif

	struct Sound::Callbacks
	{
		static bool NST_CALLBACK Lock(Nes::Sound::UserData data,Nes::Sound::Output&)
		{
			Sound& sound = *static_cast<Sound*>(data);

			if (sound.directSound.IsStreaming())
			{
				if (Nes::Sound(sound.emulator).GetLatency())
					return sound.directSound.LockStream( sound.output.samples, sound.output.length );
			}
			else
			{
				sound.directSound.StartStream();
			}

			return false;
		}

		static void NST_CALLBACK Unlock(Nes::Sound::UserData data,Nes::Sound::Output&)
		{
			Sound& sound = *static_cast<Sound*>(data);

			if (sound.recorder->IsRecording())
				sound.recorder->Flush( sound.output );

			sound.directSound.UnlockStream( sound.output.samples );

			Nes::Sound nes( sound.emulator );

			if (nes.GetLatency() >= sound.directSound.NumSamples() * 2)
				nes.EmptyBuffer();
		}
	};

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("", on)
    #endif

	Sound::Sound
	(
		Window::Custom& window,
		Window::Menu& m,
		Emulator& e,
		const Paths& paths,
		const Configuration& cfg
	)
	: 
	emulator    ( e ),
	menu        ( m ),
	directSound ( window ),
	dialog      ( new Window::Sound(directSound.GetAdapters(),paths,cfg) ),
	recorder    ( new Recorder(m,dialog->GetRecorder(),e) )
	{
		m.Commands().Add( IDM_OPTIONS_SOUND, this, &Sound::OnMenuOptionsSound );
		emulator.Events().Add( this, &Sound::OnEmuEvent );

		Nes::Sound::Output::lockCallback.Set( &Callbacks::Lock, this );
		Nes::Sound::Output::unlockCallback.Set( &Callbacks::Unlock, this );

		UpdateSettings();
	}

	Sound::~Sound()
	{
		emulator.Events().Remove( this );
	}

	uint Sound::GetLatency() const
	{
		return emulator.Is(Nes::Machine::SOUND) ? DirectX::DirectSound::LATENCY_MAX + 1 : dialog->GetLatency() + 1;
	}

	void Sound::OnEmuEvent(Emulator::Event event)
	{
		switch (event)
		{
    		case Emulator::EVENT_SPEED:

				if (emuOutput)
				{
					const uint speed = emulator.GetSpeed();
					cstring const errMsg = directSound.UpdateSpeed( speed, GetLatency() );

					if (errMsg == NULL)
					{
						Nes::Sound( emulator ).SetSpeed( speed );
						recorder->Enable( directSound.GetWaveFormat() );
					}
					else					
					{
						Disable( errMsg );
					}
				}
				break;

			case Emulator::EVENT_REWINDING_PREPARE:
			case Emulator::EVENT_REWINDING_STOP:

				directSound.StopStream();

				Nes::Sound(emulator).EnableChannels
				( 
					event == Emulator::EVENT_REWINDING_PREPARE ? dialog->GetChannels() & ~uint(Nes::Sound::CHANNEL_DPCM) :
					                                             dialog->GetChannels()
				);
				break;

			case Emulator::EVENT_NETPLAY_MODE_ON:
			case Emulator::EVENT_NETPLAY_MODE_OFF:

				menu[IDM_OPTIONS_SOUND].Enable( event == Emulator::EVENT_NETPLAY_MODE_OFF );
				break;
		}
	}

	void Sound::OnMenuOptionsSound(uint)
	{
		dialog->Open();
		UpdateSettings();
	}

	void Sound::Save(Configuration& cfg) const
	{
		dialog->Save( cfg );
	}

	void Sound::UpdateSettings()
	{
		cstring errMsg = NULL;

		if (dialog->IsSoundEnabled())
		{
			errMsg = directSound.Update
			(
   				dialog->GetAdapter(),
				dialog->GetRate(),
				dialog->GetBits(),
				dialog->IsStereo() ? DirectX::DirectSound::STEREO : DirectX::DirectSound::MONO,
				emulator.GetSpeed(),
				GetLatency(),
				dialog->GetVolume()
			);

			if (errMsg == NULL)
			{
				emuOutput = &output;

				Nes::Sound nes( emulator );

				nes.SetSampleBits( dialog->GetBits() );
				nes.SetSampleRate( dialog->GetRate() );
				nes.SetSpeed( emulator.GetSpeed() );
				nes.SetAutoTranspose( dialog->IsPitchAdjust() );
				nes.EnableChannels( dialog->GetChannels() );
				nes.SetSpeaker( dialog->IsStereo() ? Nes::Sound::SPEAKER_STEREO : Nes::Sound::SPEAKER_MONO );

				recorder->Enable( directSound.GetWaveFormat() );
				return;
			}
		}
		else
		{
			directSound.Destroy();
		}

		Disable( errMsg );
	}

	void Sound::Disable(cstring const errMsg)
	{
		emuOutput = NULL;
		Nes::Sound( emulator ).EnableChannels( Nes::Sound::NO_CHANNELS );
		recorder->Disable();

		if (errMsg)
			Application::Exception( errMsg ).Issue();
	}

	void Sound::StartEmulation() const
	{
		Nes::Sound( emulator ).EmptyBuffer();
	}

	void Sound::StopEmulation()
	{
		directSound.StopStream();
	}
}
