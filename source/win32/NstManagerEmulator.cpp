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
#include "NstIoNsp.hpp"
#include "NstIoStream.hpp"
#include "NstIoScreen.hpp"
#include "NstIoIps.hpp"
#include "NstResourceString.hpp"
#include "NstWindowUser.hpp"
#include "NstManagerEmulator.hpp"
#include "NstApplicationInstance.hpp"
#include "../core/api/NstApiCartridge.hpp"
#include "../core/api/NstApiFds.hpp"
#include "../core/api/NstApiNsf.hpp"
#include "../core/api/NstApiMovie.hpp"
#include "../core/api/NstApiRewinder.hpp"

namespace Nestopia
{
	namespace Managers
	{
		struct Emulator::CallbackData
		{
			Nes::User::FileData& data;
			const Nes::User::File type;

			CallbackData(Nes::User::FileData& d,Nes::User::File t)
			: data(d), type(t) {}
		};

		struct Emulator::Callbacks
		{
			static Nes::User::Answer NST_CALLBACK Confirm(Nes::User::UserData,Nes::User::Question question)
			{
				switch (question)
				{
					case Nes::User::QUESTION_NST_PRG_CRC_FAIL_CONTINUE:
					case Nes::User::QUESTION_NSV_PRG_CRC_FAIL_CONTINUE:

						return Window::User::Confirm( IDS_EMU_CRC_MISSMATCH_CONTINUE, IDS_EMU_CRC_MISSMATCH ) ? Nes::User::ANSWER_YES : Nes::User::ANSWER_NO;

					default:

						return Nes::User::ANSWER_DEFAULT;
				}
			}

			static void NST_CALLBACK DoInput(Nes::User::UserData,Nes::User::Input type,cstring info,Nes::User::String& response)
			{
				NST_ASSERT( info && *info && response.empty() );
				NST_VERIFY( type == Nes::User::INPUT_CHOOSE_MAPPER );

				if (type == Nes::User::INPUT_CHOOSE_MAPPER)
				{
					HeapString output;

					if (Window::User::Input( output, Resource::String(IDS_EMU_CHOOSE_MAPPER).Invoke(info), Resource::String(IDS_EMU_TITLE_CHOOSE_MAPPER) ))
					{
						const String::Heap<char> tmp( output );
						response.assign( tmp.Ptr(), tmp.Length() );
					}
				}
			}

			static void NST_CALLBACK DoFileIO(Nes::User::UserData user,Nes::User::File type,Nes::User::FileData& data)
			{
				NST_ASSERT( user );

				Emulator& emulator = *static_cast<Emulator*>(user);
				CallbackData callbackData( data, type );

				switch (type)
				{
					case Nes::User::FILE_LOAD_BATTERY:
					case Nes::User::FILE_LOAD_EEPROM:
					case Nes::User::FILE_LOAD_TAPE:
					case Nes::User::FILE_LOAD_TURBOFILE:

						emulator.LoadImageData( callbackData );
						break;

					case Nes::User::FILE_SAVE_BATTERY:
					case Nes::User::FILE_SAVE_EEPROM:
					case Nes::User::FILE_SAVE_TAPE:
					case Nes::User::FILE_SAVE_TURBOFILE:

						emulator.SaveImageData( callbackData );
						break;

					case Nes::User::FILE_SAVE_FDS:

						emulator.SaveDiskData( callbackData );
						break;
				}
			}

			static void NST_CALLBACK DoEvent(Nes::User::UserData,Nes::User::Event type,const void* data)
			{
				switch (type)
				{
					case Nes::User::EVENT_CPU_JAM:

						Io::Screen() << Resource::String(IDS_SCREEN_CPU_JAM);
						break;

					case Nes::User::EVENT_DISPLAY_TIMER:

						Io::Screen() << static_cast<cstring>( data );
						break;

					case Nes::User::EVENT_TAPE_RECORDING:

						Io::Screen() << Resource::String(IDS_SCREEN_TAPE_RECORDING);
						break;

					case Nes::User::EVENT_TAPE_PLAYING:

						Io::Screen() << Resource::String(IDS_SCREEN_TAPE_PLAYING);
						break;

					case Nes::User::EVENT_TAPE_STOPPED:

						Io::Screen() << Resource::String(IDS_SCREEN_TAPE_STOPPED);
						break;

					case Nes::User::EVENT_NONSTANDARD_DISK:

						Window::User::Warn( IDS_FDS_NONSTANDARD );
						break;
				}
			}

			static void NST_CALLBACK Rewind(Nes::Rewinder::UserData user,Nes::Rewinder::State state)
			{
				static_cast<Emulator*>(user)->events
				(
					state == Nes::Rewinder::PREPARING ? EVENT_REWINDING_PREPARE :
					state == Nes::Rewinder::REWINDING ? EVENT_REWINDING_START :
														EVENT_REWINDING_STOP
				);
			}
		};

		void Emulator::EventHandler::Add(const Callback& callback)
		{
			NST_ASSERT( bool(callback) && !callbacks.Find( callback ) );
			callbacks.PushBack( callback );
		}

		void Emulator::EventHandler::Remove(const void* const instance)
		{
			for (Callbacks::Iterator it(callbacks.Begin()); it != callbacks.End(); ++it)
			{
				if (it->VoidPtr() == instance)
				{
					callbacks.Erase( it );
					break;
				}
			}
		}

		void Emulator::EventHandler::operator () (const Event event) const
		{
			for (Callbacks::ConstIterator it=callbacks.Begin(), end=callbacks.End(); it != end; ++it)
				(*it)( event );
		}

		inline Emulator::Settings::Cartridge::Cartridge()
		: writeProtect(false) {}

		inline Emulator::Settings::Fds::Fds()
		: save(DISKIMAGE_SAVE_TO_IMAGE) {}

		inline Emulator::Settings::Timing::Timing()
		:
		speed           (DEFAULT_SPEED),
		baseSpeed       (DEFAULT_SPEED),
		speeding        (false),
		sync            (false),
		tripleBuffering (false),
		rewinding       (false)
		{}

		inline Emulator::Settings::Settings()
		: askSave(false) {}

		void Emulator::Settings::Reset()
		{
			paths.start.Clear();
			paths.image.Clear();
			paths.save.Clear();
			paths.tape.Clear();
			fds.original.Destroy();
			askSave = false;
		}

		inline Emulator::State::State()
		:
		running     (false),
		paused      (false),
		frame       (0),
		activator   (this,&State::NoActivator),
		inactivator (this,&State::NoInactivator)
		{}

		ibool Emulator::State::NoActivator()
		{
			return false;
		}

		void Emulator::State::NoInactivator()
		{
		}

		Emulator::Netplay::Netplay()
		: player(0), players(0) {}

		Emulator::Emulator()
		{
			Nes::User::eventCallback.Set( &Callbacks::DoEvent, NULL );
			Nes::User::inputCallback.Set( &Callbacks::DoInput, NULL );
			Nes::User::questionCallback.Set( &Callbacks::Confirm, NULL );
			Nes::User::fileIoCallback.Set( &Callbacks::DoFileIO, this );
			Nes::Rewinder::stateCallback.Set( &Callbacks::Rewind, this );
		}

		Emulator::~Emulator()
		{
			Unload();
		}

		void Emulator::Initialize() const
		{
			events( EVENT_INIT );
		}

		void Emulator::Unhook()
		{
			state.activator.Set( &state, &State::NoActivator );
			state.inactivator.Set( &state, &State::NoInactivator );
		}

		void Emulator::Resume()
		{
			if (!state.running && !state.paused && Is(Nes::Machine::ON) && (!Is(Nes::Machine::SOUND) || Nes::Nsf(*this).IsPlaying()))
				state.running = state.activator();
		}

		void Emulator::Stop()
		{
			if (state.running)
			{
				state.running = false;
				state.inactivator();
			}
		}

		void Emulator::Wait()
		{
			Stop();
			Resume();
		}

		ibool Emulator::Pause(const ibool pause)
		{
			if (bool(state.paused) != bool(pause))
			{
				Stop();

				state.paused = pause && Is(Nes::Machine::ON);
				events( state.paused ? EVENT_PAUSE : EVENT_RESUME );

				if (!state.paused)
					Resume();

				return true;
			}

			return false;
		}

		ibool Emulator::UsesBaseSpeed() const
		{
			return
			(
				(settings.timing.speeding|settings.timing.rewinding) == 0 ||
				(settings.timing.speed == DEFAULT_SPEED)
			);
		}

		uint Emulator::GetBaseSpeed()
		{
			if (settings.timing.baseSpeed != DEFAULT_SPEED)
			{
				return settings.timing.baseSpeed;
			}
			else if (Nes::Machine(*this).GetMode() == Nes::Machine::NTSC)
			{
				return Nes::FPS_NTSC;
			}
			else
			{
				return Nes::FPS_PAL;
			}
		}

		uint Emulator::GetSpeed()
		{
			return UsesBaseSpeed() ? GetBaseSpeed() : settings.timing.speed;
		}

		void Emulator::ToggleSpeed(const ibool speeding)
		{
			if (bool(settings.timing.speeding) != bool(speeding) && !netplay)
			{
				settings.timing.speeding = bool(speeding);
				events( speeding ? EVENT_SPEEDING_ON : EVENT_SPEEDING_OFF );
			}
		}

		void Emulator::ToggleRewind(const ibool rewinding)
		{
			if (bool(settings.timing.rewinding) != bool(rewinding) && !netplay)
			{
				settings.timing.rewinding = bool(rewinding);
				events( rewinding ? EVENT_REWINDING_ON : EVENT_REWINDING_OFF );
			}
		}

		void Emulator::ResetSpeed(const uint baseSpeed,const ibool sync,const ibool tripleBuffering)
		{
			settings.timing.speed = DEFAULT_SPEED;
			settings.timing.baseSpeed = baseSpeed;
			settings.timing.speeding = false;
			settings.timing.sync = sync;
			settings.timing.tripleBuffering = tripleBuffering;
			settings.timing.rewinding = false;

			events( EVENT_BASE_SPEED );
			events( EVENT_SPEED );
		}

		void Emulator::SetSpeed(const uint speed)
		{
			if (settings.timing.speed != speed)
			{
				settings.timing.speed = speed;
				events( EVENT_SPEED );
			}
		}

		ibool Emulator::IsDiskImage(const Collection::Buffer& buffer) const
		{
			return buffer.Size() >= 4 &&
			(
				*reinterpret_cast<const u32*>(buffer.Begin()) == 0x1A534446U ||
				*reinterpret_cast<const u32*>(buffer.Begin()) == 0x494E2A01U
			);
		}

		void Emulator::LoadImageData(CallbackData& callbackData)
		{
			NST_ASSERT( callbackData.data.empty() );

			cstring desc = "image save";
			Path path( settings.paths.save );

			switch (callbackData.type)
			{
				case Nes::User::FILE_LOAD_BATTERY:

					desc = "battery";
					break;

				case Nes::User::FILE_LOAD_EEPROM:

					desc = "EEPROM";
					break;

				case Nes::User::FILE_LOAD_TAPE:

					desc = "tape";
					path = settings.paths.tape;
					break;

				case Nes::User::FILE_LOAD_TURBOFILE:

					desc = "turbo file";

					if (path.Length())
						path.Extension() = _T("tf");

					break;
			}

			if (path.FileExists())
			{
				try
				{
					Io::File file( path, Io::File::COLLECT );

					if (const uint size = file.Size())
					{
						callbackData.data.resize( size );
						file.Read( &callbackData.data.front(), size );
						Io::Log() << "Emulator: loaded " << desc << " data from \"" << path << "\"\r\n";
					}
					else
					{
						Io::Log() << "Emulator: warning, " << desc << " data file \"" << path << "\" is empty!\r\n";
					}
				}
				catch (Io::File::Exception)
				{
					callbackData.data.clear();
					Window::User::Warn( IDS_CARTRIDGE_LOAD_FAILED );
				}
			}
			else if (callbackData.type != Nes::User::FILE_LOAD_TAPE)
			{
				if (path.Length())
				{
					Io::Log() << "Emulator: " << desc << " data file \"" << path << "\" not found\r\n";
				}
				else
				{
					Io::Log() << "Emulator: " << desc << " data was not loaded!\r\n";
				}
			}
		}

		void Emulator::SaveImageData(CallbackData& callbackData) const
		{
			NST_ASSERT( !callbackData.data.empty() );

			cstring desc = "image save";
			uint ids = IDS_EMU_MOVIE_SAVE_IMAGEDATA;
			Path path( settings.paths.save );

			switch (callbackData.type)
			{
				case Nes::User::FILE_SAVE_BATTERY:

					desc = "battery";
					break;

				case Nes::User::FILE_SAVE_EEPROM:

					desc = "EEPROM";
					ids = IDS_EMU_MOVIE_SAVE_EEPROM;
					break;

				case Nes::User::FILE_SAVE_TAPE:

					desc = "cassette tape";
					ids = IDS_EMU_MOVIE_SAVE_TAPE;
					path = settings.paths.tape;
					break;

				case Nes::User::FILE_SAVE_TURBOFILE:

					desc = "turbo file";
					ids = IDS_EMU_MOVIE_SAVE_TURBOFILE;

					if (path.Length())
						path.Extension() = _T("tf");

					break;
			}

			if (settings.cartridge.writeProtect)
			{
				Io::Log() << "Emulator: write-protection enabled, discarding " << desc << " data..\r\n";
			}
			else if (path.Length())
			{
				if (!settings.askSave || Window::User::Confirm( ids ))
				{
					try
					{
						Io::File( path, Io::File::DUMP ).Write( &callbackData.data.front(), callbackData.data.size() );
					}
					catch (Io::File::Exception)
					{
						Window::User::Warn( IDS_CARTRIDGE_SAVE_FAILED );
						return;
					}

					Io::Log() << "Emulator: " << desc << " data was saved to \"" << path << "\"\r\n";
				}
			}
			else
			{
				Io::Log() << "Emulator: warning, " << desc << " data was not saved!\r\n";
			}
		}

		void Emulator::LoadDiskData(Collection::Buffer& data)
		{
			NST_ASSERT( !data.Empty() );

			settings.fds.original = data;

			switch (settings.fds.save)
			{
				case DISKIMAGE_SAVE_TO_IPS:

					if (settings.paths.save.FileExists())
					{
						Io::Ips ips;
						Io::Ips::PatchData patchData;

						try
						{
							Io::File( settings.paths.save, Io::File::COLLECT ).Stream() >> patchData;
							ips.Parse( patchData.Ptr(), patchData.Size() );
							ips.Patch( data.Ptr(), data.Size() );
						}
						catch (...)
						{
							Window::User::Warn( IDS_FDS_IPSDATALOAD_FAILED );
							return;
						}

						Io::Log() << "Emulator: patched image with IPS disk data file \"" << settings.paths.save << "\"\r\n";
					}
					else if (settings.paths.save.Length())
					{
						Io::Log() << "Emulator: IPS disk data file \"" << settings.paths.save << "\" not found\r\n";
					}
					break;

				case DISKIMAGE_SAVE_TO_IMAGE:

					if (!settings.paths.image.FileExists())
						Window::User::Warn( IDS_EMU_NOFDSFILE );

					break;
			}
		}

		void Emulator::SaveDiskData(CallbackData& callbackData) const
		{
			NST_ASSERT( !callbackData.data.empty() );

			if (settings.askSave && !Window::User::Confirm( IDS_EMU_MOVIE_SAVE_FDS ))
				return;

			switch (settings.fds.save)
			{
				case DISKIMAGE_SAVE_TO_IMAGE:

					if (settings.paths.image.FileExists())
					{
						try
						{
							Io::File( settings.paths.image, Io::File::DUMP ).Write( &callbackData.data.front(), callbackData.data.size() );
						}
						catch (Io::File::Exception)
						{
							Window::User::Warn( IDS_FDS_SAVE_FAILED );
							break;
						}

						Io::Log() << "Emulator: updated disk image file \"" << settings.paths.image << "\"\r\n";
					}
					else
					{
						Io::Log() << "Emulator: warning, disk image file \"" << settings.paths.image << "\" not found, discarding changes!\r\n";
					}
					break;

				case DISKIMAGE_SAVE_TO_IPS:

					if (settings.paths.save.Length())
					{
						NST_ASSERT( settings.fds.original.Size() == callbackData.data.size() );

						if (std::memcmp( settings.fds.original.Ptr(), &callbackData.data.front(), callbackData.data.size() ))
						{
							Io::Ips::PatchData patchData;

							try
							{
								Io::Ips::Create( settings.fds.original.Ptr(), &callbackData.data.front(), callbackData.data.size(), patchData );
								Io::File( settings.paths.save, Io::File::DUMP ).Stream() << patchData;
							}
							catch (...)
							{
								Window::User::Warn( IDS_FDS_IPSDATASAVE_FAILED );
								break;
							}

							Io::Log() << "Emulator: saved IPS disk data to \"" << settings.paths.save << "\"\r\n";
						}

						break;
					}

				case DISKIMAGE_SAVE_DISABLED:

					Io::Log() << "Emulator: changes made to the disk image was not saved..\r\n";
					break;
			}
		}

		ibool Emulator::Load
		(
			Collection::Buffer& image,
			const Path& start,
			const Io::Nsp::Context& context,
			const ibool warn
		)
		{
			Application::Instance::Waiter wait;

			Unload();

			Io::Log() << "Emulator: loading \"" << context.image << "\"\r\n";

			settings.paths.start = start;
			settings.paths.image = context.image;
			settings.paths.save = context.save;
			settings.paths.tape = context.tape;

			if (IsDiskImage( image ))
				LoadDiskData( image );

			Nes::Result result;

			{
				Io::Stream::Input stream( image );

				retry:
				{
					result = Nes::Machine(*this).Load( stream );

					if (result == Nes::RESULT_ERR_MISSING_BIOS && !netplay)
					{
						if (Window::User::Confirm( IDS_EMU_SUPPLY_BIOS_FILE ))
						{
							events( EVENT_QUERY_FDS_BIOS );

							if (Nes::Fds(*this).HasBIOS())
							{
								stream.seekg( 0, std::istream::beg );
								goto retry;
							}
						}

						settings.Reset();
						return false;
					}
				}
			}

			if (NES_SUCCEEDED(result))
			{
				events( netplay ? EVENT_NETPLAY_LOAD : EVENT_LOAD );

				if (context.mode != Io::Nsp::Context::UNKNOWN)
					SetMode( (Nes::Machine::Mode) context.mode );

				if (Is( Nes::Machine::GAME ))
				{
					for (uint port=0; port < Nes::Input::NUM_PORTS; ++port)
					{
						if (context.controllers[port] != Io::Nsp::Context::UNKNOWN)
							ConnectController( port, (Nes::Input::Type) context.controllers[port] );
					}
				}

				uint msg = ~0U;

				switch (result)
				{
					case Nes::RESULT_WARN_BAD_DUMP:              msg = IDS_EMU_WARN_BAD_DUMP;       break;
					case Nes::RESULT_WARN_BAD_PROM:              msg = IDS_EMU_WARN_BAD_PROM;       break;
					case Nes::RESULT_WARN_BAD_CROM:              msg = IDS_EMU_WARN_BAD_CROM;       break;
					case Nes::RESULT_WARN_BAD_FILE_HEADER:       msg = IDS_EMU_WARN_BAD_INES;       break;
					case Nes::RESULT_WARN_INCORRECT_FILE_HEADER: msg = IDS_EMU_WARN_INCORRECT_INES; break;
					case Nes::RESULT_WARN_ENCRYPTED_ROM:         msg = IDS_EMU_WARN_ENCRYPTED;      break;
				}

				if (msg != ~0U)
				{
					if (warn)
						Window::User::Warn( msg );
					else
						Io::Log() << "Emulator: warning, " << Resource::String( msg ) << "\r\n";
				}

				return true;
			}
			else
			{
				uint msg;

				switch (result)
				{
					case Nes::RESULT_ERR_CORRUPT_FILE:           msg = IDS_FILE_ERR_CORRUPT;              break;
					case Nes::RESULT_ERR_INVALID_FILE:           msg = IDS_FILE_ERR_INVALID;              break;
					case Nes::RESULT_ERR_UNSUPPORTED_GAME:       msg = IDS_EMU_ERR_UNSUPPORTED_GAME;      break;
					case Nes::RESULT_ERR_UNSUPPORTED_SOUND_CHIP: msg = IDS_EMU_ERR_UNSUPPORTED_SOUNDCHIP; break;
					case Nes::RESULT_ERR_UNSUPPORTED_MAPPER:     msg = IDS_EMU_ERR_UNSUPPORTED_MAPPER;    break;
					case Nes::RESULT_ERR_UNSUPPORTED_VSSYSTEM:   msg = IDS_EMU_ERR_UNSUPPORTED_VSSYSTEM;  break;
					case Nes::RESULT_ERR_MISSING_BIOS:           msg = IDS_EMU_ERR_BIOS_FILE_MISSING;     break;
					case Nes::RESULT_ERR_INVALID_MAPPER:         msg = IDS_EMU_ERR_INVALID_MAPPER;        break;
					case Nes::RESULT_ERR_OUT_OF_MEMORY:          msg = IDS_ERR_OUT_OF_MEMORY;             break;
					default:                                     msg = IDS_ERR_GENERIC;                   break;
				}

				Window::User::Fail( msg );

				settings.Reset();
			}

			return false;
		}

		ibool Emulator::Unload()
		{
			Stop();

			const ibool loaded = Is(Nes::Machine::IMAGE);

			if (loaded)
			{
				if (Is(Nes::Machine::ON))
				{
					Unpause();
					state.frame = 0;
					Nes::Machine(*this).Power( false );
					events( netplay ? EVENT_NETPLAY_POWER_OFF : EVENT_POWER_OFF );
				}

				Nes::Machine(*this).Unload();
				events( netplay ? EVENT_NETPLAY_UNLOAD : EVENT_UNLOAD );

				Io::Log() << "Emulator: unloaded \"" << settings.paths.image << "\"\r\n";

				settings.Reset();
			}

			return loaded;
		}

		void Emulator::Save(Io::Nsp::Context& context)
		{
			context.image = settings.paths.image;
			context.mode = (Is(Nes::Machine::PAL) ? Nes::Machine::PAL : Nes::Machine::NTSC);

			if (Is(Nes::Machine::GAME))
			{
				if (settings.paths.save.FileExists())
					context.save = settings.paths.save;

				if (settings.paths.tape.FileExists())
					context.tape = settings.paths.tape;

				NST_COMPILE_ASSERT( Io::Nsp::Context::NUM_CONTROLLER_PORTS == Nes::Input::NUM_PORTS );

				for (uint i=0; i < Nes::Input::NUM_PORTS; ++i)
					context.controllers[i] = Nes::Input(*this).GetConnectedController(i);
			}
		}

		ibool Emulator::SaveState(Collection::Buffer& buffer,const ibool compress,const Alert alert)
		{
			buffer.Clear();

			Nes::Result result;

			{
				Io::Stream::Output stream( buffer );
				result = Nes::Machine(*this).SaveState( stream, compress ? Nes::Machine::USE_COMPRESSION : Nes::Machine::NO_COMPRESSION );
				stream.Export( buffer );
			}

			if (NES_SUCCEEDED(result))
				return true;

			uint msg;

			switch (result)
			{
				case Nes::RESULT_ERR_CORRUPT_FILE:  msg = IDS_FILE_ERR_CORRUPT;  break;
				case Nes::RESULT_ERR_OUT_OF_MEMORY: msg = IDS_ERR_OUT_OF_MEMORY; break;
				default:                            msg = IDS_EMU_ERR_GENERIC;   break;
			}

			if (alert == NOISY)
			{
				Window::User::Fail( msg, IDS_EMU_ERR_SAVE_STATE );
			}
			else if (alert == STICKY)
			{
				Io::Screen() << Resource::String( IDS_EMU_ERR_SAVE_STATE )
                             << ' '
                             << Resource::String( msg );
			}
			else
			{
				Io::Log() << "Emulator: "
                          << Resource::String( IDS_EMU_ERR_SAVE_STATE )
                          << ' '
                          << Resource::String( msg )
                          << "\r\n";
			}

			return false;
		}

		ibool Emulator::LoadState(Collection::Buffer& buffer,const Alert alert)
		{
			const ibool on = Is(Nes::Machine::ON);

			if (!on && !Power( true ))
				return false;

			Nes::Result result;

			{
				Io::Stream::Input stream( buffer );
				result = Nes::Machine(*this).LoadState( stream );
				stream.Export( buffer );
			}

			if (NES_SUCCEEDED(result))
				return true;

			if (!on)
				Power( false );

			if (result != Nes::RESULT_ERR_INVALID_CRC)
			{
				uint msg;

				switch (result)
				{
					case Nes::RESULT_ERR_CORRUPT_FILE:             msg = IDS_FILE_ERR_CORRUPT;                 break;
					case Nes::RESULT_ERR_INVALID_FILE:             msg = IDS_FILE_ERR_INVALID;                 break;
					case Nes::RESULT_ERR_OUT_OF_MEMORY:            msg = IDS_ERR_OUT_OF_MEMORY;                break;
					case Nes::RESULT_ERR_UNSUPPORTED_FILE_VERSION: msg = IDS_EMU_ERR_UNSUPPORTED_FILE_VERSION; break;
					case Nes::RESULT_ERR_NOT_READY:                msg = IDS_EMU_ERR_LOADSTATE_DISABLED;       break;
					default:                                       msg = IDS_EMU_ERR_GENERIC;                  break;
				}

				if (alert == NOISY)
				{
					Window::User::Fail( msg, IDS_EMU_ERR_LOAD_STATE );
				}
				else if (alert == STICKY)
				{
					Io::Screen() << Resource::String( IDS_EMU_ERR_LOAD_STATE )
                                 << ' '
                                 << Resource::String( msg );
				}
				else
				{
					Io::Log() << "Emulator: "
                              << Resource::String( IDS_EMU_ERR_LOAD_STATE )
                              << ' '
                              << Resource::String( msg )
                              << "\r\n";
				}
			}

			return false;
		}

		ibool Emulator::Power(const ibool turnOn)
		{
			if (bool(turnOn) != bool(Is(Nes::Machine::ON)))
			{
				if (turnOn)
				{
					const Nes::Result result = Nes::Machine(*this).Power( true );

					if (NES_SUCCEEDED(result))
					{
						events( netplay ? EVENT_NETPLAY_POWER_ON : EVENT_POWER_ON );
						Resume();
						return true;
					}
					else
					{
						if (!Is(Nes::Machine::IMAGE))
							events( netplay ? EVENT_NETPLAY_UNLOAD : EVENT_UNLOAD );

						Window::User::Fail
						(
							result == Nes::RESULT_ERR_OUT_OF_MEMORY ?
							IDS_ERR_OUT_OF_MEMORY : IDS_ERR_GENERIC
						);
					}
				}
				else
				{
					Stop();

					if (Is(Nes::Machine::ON))
					{
						Unpause();
						state.frame = 0;
						Nes::Machine(*this).Power( false );
						events( netplay ? EVENT_NETPLAY_POWER_OFF : EVENT_POWER_OFF );
						return true;
					}
				}
			}

			return false;
		}

		ibool Emulator::Reset(const ibool hard)
		{
			Stop();

			if (Is(Nes::Machine::ON) && !Nes::Movie(*this).IsPlaying())
			{
				Unpause();

				state.frame = 0;
				const Nes::Result result = Nes::Machine(*this).Reset( hard );

				if (NES_SUCCEEDED(result))
				{
					events( hard ? EVENT_RESET_HARD : EVENT_RESET_SOFT );
					Resume();
					return true;
				}
				else
				{
					if (!Is(Nes::Machine::ON))
						events( netplay ? EVENT_NETPLAY_POWER_OFF : EVENT_POWER_OFF );

					if (!Is(Nes::Machine::IMAGE))
						events( netplay ? EVENT_NETPLAY_UNLOAD : EVENT_UNLOAD );

					Window::User::Fail
					(
						result == Nes::RESULT_ERR_OUT_OF_MEMORY ? IDS_ERR_OUT_OF_MEMORY :
                                                                  IDS_ERR_GENERIC
					);
				}
			}

			return false;
		}

		void Emulator::Unpause()
		{
			if (state.paused)
			{
				state.paused = false;
				events( EVENT_RESUME );
			}
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		void Emulator::Execute
		(
			Nes::Video::Output* const video,
			Nes::Sound::Output* const sound,
			Nes::Input::Controllers* const input
		)
		{
			if (state.running)
			{
				if (netplay)
					netplay.callback( input );

				++state.frame;
				const Nes::Result result = Nes::Emulator::Execute( video, sound, input );

				if (Is(Nes::Machine::ON))
					return;

				Stop();
				events( netplay ? EVENT_NETPLAY_POWER_OFF : EVENT_POWER_OFF );

				Window::User::Fail
				(
					result == Nes::RESULT_ERR_OUT_OF_MEMORY ? IDS_ERR_OUT_OF_MEMORY :
                                                              IDS_EMU_ERR_GENERIC
				);
			}
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		ibool Emulator::SetMode(const Nes::Machine::Mode mode)
		{
			if (Nes::Machine(*this).GetMode() != mode)
			{
				events( mode == Nes::Machine::NTSC ? EVENT_MODE_NTSC : EVENT_MODE_PAL );
				Nes::Machine(*this).SetMode( mode );

				if (settings.timing.baseSpeed == DEFAULT_SPEED)
				{
					events( EVENT_BASE_SPEED );
					events( EVENT_SPEED );
				}

				return true;
			}

			return false;
		}

		ibool Emulator::AutoSetMode()
		{
			return SetMode( Nes::Machine(*this).GetDesiredMode() );
		}

		void Emulator::BeginNetplayMode()
		{
			NST_ASSERT( !netplay && !Is(Nes::Machine::IMAGE) );

			events( EVENT_NETPLAY_MODE_ON );
		}

		void Emulator::EndNetplayMode()
		{
			NST_ASSERT( !netplay && !Is(Nes::Machine::IMAGE) );

			events( EVENT_NETPLAY_MODE_OFF );
		}

		void Emulator::EnableNetplay(const Netplay::Callback& callback,const uint player,const uint players)
		{
			NST_ASSERT( !Is(Nes::Machine::IMAGE) );

			netplay.callback = callback;
			netplay.player = player;
			netplay.players = players;
		}

		void Emulator::DisableNetplay()
		{
			NST_ASSERT( !Is(Nes::Machine::ON) );

			netplay.callback.Unset();
			netplay.player = 0;
			netplay.players = 0;
		}

		void Emulator::ConnectController(const uint port,const Nes::Input::Type type)
		{
			if (Nes::Input(*this).ConnectController( port, type ) == Nes::RESULT_OK)
				events( (Event) (EVENT_PORT1_CONTROLLER + port) );
		}

		void Emulator::AutoSelectController(const uint port)
		{
			NST_ASSERT( port < Nes::Input::NUM_PORTS );

			if (Nes::Input(*this).AutoSelectController( port ) == Nes::RESULT_OK)
				events( (Event) (EVENT_PORT1_CONTROLLER + port) );
		}

		void Emulator::AutoSelectControllers()
		{
			for (uint port=0; port < Nes::Input::NUM_PORTS; ++port)
				AutoSelectController( port );
		}

		void Emulator::PlaySong()
		{
			NST_ASSERT( Is(Nes::Machine::SOUND) );

			if (!state.running && Is(Nes::Machine::ON) && Nes::Nsf(*this).PlaySong() == Nes::RESULT_OK)
			{
				events( EVENT_NSF_PLAY );
				Resume();
			}
		}

		void Emulator::StopSong()
		{
			NST_ASSERT( Is(Nes::Machine::SOUND) );

			Stop();

			if (Is(Nes::Machine::ON) && Nes::Nsf(*this).StopSong() == Nes::RESULT_OK)
				events( EVENT_NSF_STOP );
		}

		void Emulator::SelectNextSong()
		{
			NST_ASSERT( Is(Nes::Machine::SOUND) );

			if (Is(Nes::Machine::ON) && Nes::Nsf(*this).SelectNextSong() == Nes::RESULT_OK)
				events( EVENT_NSF_NEXT );
		}

		void Emulator::SelectPrevSong()
		{
			NST_ASSERT( Is(Nes::Machine::SOUND) );

			if (Is(Nes::Machine::ON) && Nes::Nsf(*this).SelectPrevSong() == Nes::RESULT_OK)
				events( EVENT_NSF_PREV );
		}
	}
}
