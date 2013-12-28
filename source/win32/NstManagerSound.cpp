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
#include "NstIoArchive.hpp"
#include "NstApplicationException.hpp"
#include "NstManagerPaths.hpp"
#include "NstManagerEmulator.hpp"
#include "NstWindowMenu.hpp"
#include "NstWindowUser.hpp"
#include "NstDialogSound.hpp"
#include "NstManagerSound.hpp"
#include "NstManagerSoundRecorder.hpp"
#include "NstManagerPreferences.hpp"

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

		#ifdef _MSC_VER
		#pragma warning( push )
		#pragma warning( disable : 4701 )
		#endif

		static void NST_CALLBACK Load(Nes::Sound::UserData user,Nes::Sound::Loader::Type type,Nes::Sound::Loader& loader)
		{
			typedef Nes::Sound::Loader Loader;

			struct Game
			{
				u8 type;
				u8 samples;
				tstring name;
			};

			static const Game games[] =
			{
				{ Loader::MOERO_PRO_YAKYUU,         Loader::MOERO_PRO_YAKYUU_SAMPLES,         _T("moepro")   },
				{ Loader::MOERO_PRO_YAKYUU_88,      Loader::MOERO_PRO_YAKYUU_88_SAMPLES,      _T("moepro88") },
				{ Loader::MOERO_PRO_TENNIS,         Loader::MOERO_PRO_TENNIS_SAMPLES,         _T("mptennis") },
				{ Loader::TERAO_NO_DOSUKOI_OOZUMOU, Loader::TERAO_NO_DOSUKOI_OOZUMOU_SAMPLES, _T("terao")    },
				{ Loader::MOE_PRO_90_KANDOU_HEN,    Loader::MOE_PRO_90_KANDOU_HEN_SAMPLES,    _T("moepro90") },
				{ Loader::MOE_PRO_SAIKYOU_HEN,      Loader::MOE_PRO_SAIKYOU_HEN_SAMPLES,      _T("mpsaikyo") },
				{ Loader::SHIN_MOERO_PRO_YAKYUU,    Loader::SHIN_MOERO_PRO_YAKYUU_SAMPLES,    _T("smoepro")  },
				{ Loader::AEROBICS_STUDIO,          Loader::AEROBICS_STUDIO_SAMPLES,          _T("ftaerobi") }
			};

			const Sound& sound = *static_cast<const Sound*>(user);

			for (uint i=8; i--; )
			{
				if (type != games[i].type)
					continue;

				ibool oneError = false;
				Path path( sound.paths.GetSamplesPath(), games[i].name, _T("") );

				for (uint j=0; ; ++j)
				{
					static const tchar types[][4] =
					{
						_T("zip"),
						_T("rar"),
						_T("7z\0")
					};

					path.Extension() = types[j];

					if (path.FileExists())
					{
						Io::Log() << "Sound: Loading "
                                  << games[i].samples
                                  << " samples from \""
                                  << path
                                  << "\"\r\n";
						break;
					}
					else if (j == NST_COUNT(types)-1)
					{
						if (!sound.preferences[Preferences::SUPPRESS_WARNINGS])
							Window::User::Warn( IDS_EMU_NO_SAMPLES );

						path.Extension().Clear();

						Io::Log() << "Sound: warning, sample package file \""
                                  << path
                                  << ".*\" not found!\r\n";
						return;
					}
				}

				try
				{
					tchar name[] = _T("xx.wav");

					const Io::File file( path, Io::File::READ|Io::File::EXISTING );
					const Io::Archive archive( file );

					for (uint j=0; j < games[i].samples; ++j)
					{
						name[0] = '0' + j / 10;
						name[1] = '0' + j % 10;

						for (uint k=archive.NumFiles(); k--; )
						{
							if (archive[k].GetName().File() != name)
							{
								if (k == 0)
									Io::Log() << "Sound: warning, \"" << name << "\" not found!\r\n";

								continue;
							}

							Collection::Buffer buffer;
							WAVEFORMATEX format;

							try
							{
								Collection::Buffer tmp( archive[k].Size() );
								archive[k].Uncompress( tmp.Ptr() );

								Io::Wave wave( Io::Wave::MODE_READ );

								if (const uint size = wave.Open( tmp.Ptr(), tmp.Size(), format ))
								{
									buffer.Resize( size );
									wave.Read( buffer.Ptr() );
								}
							}
							catch (...)
							{
								buffer.Clear();
							}

							if (buffer.Size() && NES_FAILED(loader.Load( j, buffer.Ptr(), buffer.Size() / format.nBlockAlign, format.nChannels == 2, format.wBitsPerSample, format.nSamplesPerSec )))
								buffer.Clear();

							if (buffer.Empty())
							{
								Io::Log() << "Sound: warning, error loading \"" << name << "\"!\r\n";
								oneError = true;
							}

							break;
						}
					}
				}
				catch (...)
				{
					Io::Log() << "Sound: warning, sample loading error!\r\n";
					oneError = true;
				}

				if (oneError)
					Window::User::Warn( IDS_EMU_ERR_LOAD_SAMPLES );

				break;
			}
		}

		#ifdef _MSC_VER
		#pragma warning( pop )
		#endif
	};

	Sound::Sound
	(
		Window::Custom& window,
		Window::Menu& m,
		Emulator& e,
		const Paths& p,
		const Preferences& r,
		const Configuration& cfg
	)
	:
	emulator    ( e ),
	menu        ( m ),
	paths       ( p ),
	preferences ( r ),
	directSound ( window ),
	dialog      ( new Window::Sound(e,directSound.GetAdapters(),p,cfg) ),
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

			case Emulator::EVENT_REWINDING_START:
			case Emulator::EVENT_REWINDING_STOP:

				Nes::Sound(emulator).SetVolume
				(
					Nes::Sound::CHANNEL_DPCM,
					event == Emulator::EVENT_REWINDING_START ? 0 : dialog->GetVolume(Nes::Sound::CHANNEL_DPCM)
				);

			case Emulator::EVENT_REWINDING_PREPARE:

				directSound.StopStream();
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
