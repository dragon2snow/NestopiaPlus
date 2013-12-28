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

#include "resource/resource.h"
#include "NstResourceFile.hpp"
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

	struct Sound::Callbacks
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("t", on)
        #endif

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

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif

		static void NST_CALLBACK Load(Nes::Sound::UserData,Nes::Sound::Loader::Type type,Nes::Sound::Loader& loader)
		{
			typedef Nes::Sound::Loader Loader;

			switch (type)
			{
				case Loader::MOERO_PRO_YAKYUU:
				{
					NST_COMPILE_ASSERT( Loader::MOERO_PRO_YAKYUU_SAMPLES == 16 );
					Collection::Buffer sounds[Loader::MOERO_PRO_YAKYUU_SAMPLES];
				
					if (Resource::File( IDR_MOEPROSAMPLES, _T("MoeProSamples") ).Uncompress( sounds, Loader::MOERO_PRO_YAKYUU_SAMPLES ))
					{
						for (uint i=0; i < Loader::MOERO_PRO_YAKYUU_SAMPLES; ++i)
							loader.Load( i, reinterpret_cast<const u8*>(sounds[i].Ptr()), sounds[i].Length(), 22050U );
					}
					break;
				}
			}
		}
	};

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
	dialog      ( new Window::Sound(e,directSound.GetAdapters(),paths,cfg) ),
	recorder    ( new Recorder(m,dialog->GetRecorder(),e) )
	{
		m.Commands().Add( IDM_OPTIONS_SOUND, this, &Sound::OnMenuOptionsSound );
		emulator.Events().Add( this, &Sound::OnEmuEvent );

		Nes::Sound::Output::lockCallback.Set( &Callbacks::Lock, this );
		Nes::Sound::Output::unlockCallback.Set( &Callbacks::Unlock, this );
		Nes::Sound::Loader::loadCallback.Set( &Callbacks::Load, this );

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
    		case Emulator::EVENT_POWER_ON:
    		case Emulator::EVENT_SPEED:

				if (emuOutput)
				{
					const uint speed = emulator.GetSpeed();
					tstring const errMsg = directSound.UpdateSpeed( speed, GetLatency() );

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
				
				Nes::Sound(emulator).SetVolume
				( 
			     	Nes::Sound::CHANNEL_DPCM, 
					event == Emulator::EVENT_REWINDING_PREPARE ? 0 : dialog->GetVolume(Nes::Sound::CHANNEL_DPCM) 
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
		tstring errMsg = NULL;

		if (dialog->IsSoundEnabled())
		{
			Nes::Sound nesSound( emulator );

			errMsg = directSound.Update
			(
   				dialog->GetAdapter(),
				nesSound.GetSampleRate(),
				nesSound.GetSampleBits(),
				nesSound.GetSpeaker() == Nes::Sound::SPEAKER_STEREO ? DirectX::DirectSound::STEREO : DirectX::DirectSound::MONO,
				emulator.GetSpeed(),
				GetLatency()
			);

			if (errMsg == NULL)
			{
				emuOutput = &output;
				nesSound.SetSpeed( emulator.GetSpeed() );
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

	void Sound::Disable(tstring const errMsg)
	{
		emuOutput = NULL;
		Nes::Sound( emulator ).SetVolume( Nes::Sound::ALL_CHANNELS, 0 );
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
