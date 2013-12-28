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

#ifndef NST_MANAGER_EMULATOR_H
#define NST_MANAGER_EMULATOR_H

#pragma once

#include "NstCollectionVector.hpp"
#include "NstObjectDelegate.hpp"
#include "NstString.hpp"
#include "../core/api/NstApiEmulator.hpp"
#include "../core/api/NstApiMachine.hpp"
#include "../core/api/NstApiVideo.hpp"
#include "../core/api/NstApiSound.hpp"
#include "../core/api/NstApiInput.hpp"

namespace Nes
{
	using namespace Api;
}

namespace Nestopia
{
	namespace Io
	{
		namespace Nsp
		{
			struct Context;
		}

		namespace Stream
		{
			class Input;
			class Output;
		}
	}

	namespace Managers
	{
		class Emulator : public Nes::Emulator
		{
		public:

			Emulator();
			~Emulator();

		private:

			struct Netplay
			{
				Netplay();

				typedef Object::Delegate<void,Nes::Input::Controllers*> Callback;
				
				Callback callback;
				uint player, players;

				operator bool () const
				{
					return callback;
				}
			};

		public:

			enum
			{
				DEFAULT_SPEED = 0
			};

			enum Alert
			{
				NOISY,
				QUIETLY,
				STICKY
			};

			enum DiskImageSaveMethod
			{
				DISKIMAGE_SAVE_DISABLED,
				DISKIMAGE_SAVE_TO_IMAGE,
				DISKIMAGE_SAVE_TO_IPS
			};

			enum Event
			{
				EVENT_LOAD = 1,
				EVENT_UNLOAD,
				EVENT_POWER_ON,
				EVENT_POWER_OFF,
				EVENT_RESET_SOFT,
				EVENT_RESET_HARD,
				EVENT_MODE_NTSC,
				EVENT_MODE_PAL,
				EVENT_PAUSE,
				EVENT_RESUME,
				EVENT_NETPLAY_MODE_ON,
				EVENT_NETPLAY_MODE_OFF,
				EVENT_NETPLAY_LOAD,
				EVENT_NETPLAY_UNLOAD,
				EVENT_NETPLAY_POWER_ON,
				EVENT_NETPLAY_POWER_OFF,
				EVENT_SPEED,
				EVENT_BASE_SPEED,
				EVENT_SPEEDING_ON,
				EVENT_SPEEDING_OFF,
				EVENT_REWINDING_ON,
				EVENT_REWINDING_OFF,
				EVENT_REWINDING_PREPARE,
				EVENT_REWINDING_START,
				EVENT_REWINDING_STOP,
				EVENT_NSF_PLAY,
				EVENT_NSF_STOP,
				EVENT_NSF_NEXT,
				EVENT_NSF_PREV,
				EVENT_PORT1_CONTROLLER,
				EVENT_PORT2_CONTROLLER,
				EVENT_PORT3_CONTROLLER,
				EVENT_PORT4_CONTROLLER,
				EVENT_PORT5_CONTROLLER
			};

			typedef String::Path<false> Path;

			void  Stop();
			void  Resume();
			void  Wait();
			ibool Pause(ibool);
			void  ResetSpeed(uint,ibool);
			void  SetSpeed(uint);
			uint  GetBaseSpeed();
			uint  GetSpeed();
			ibool SetMode(Nes::Machine::Mode);
			void  ToggleSpeed(ibool);
			void  ToggleRewind(ibool);
			ibool AutoSetMode();
			void  AutoSelectController(uint);
			void  AutoSelectControllers();
			void  ConnectController(uint,Nes::Input::Type);
			ibool IsControllerConnected(Nes::Input::Type);
			void  PlaySong();
			void  StopSong();
			void  SelectNextSong();
			void  SelectPrevSong();
			void  Save(Io::Nsp::Context&);
			ibool Load(Collection::Buffer&,const Path&,const Io::Nsp::Context&,ibool);
			ibool Unload();
			ibool SaveState(Collection::Buffer&,ibool,Alert=NOISY);
			ibool LoadState(Collection::Buffer&,Alert=NOISY);
			ibool Power(ibool);
			ibool Reset(ibool);
			void  Execute(Nes::Video::Output*,Nes::Sound::Output*,Nes::Input::Controllers*);
			void  BeginNetplayMode();
			void  EndNetplayMode();
			void  DisableNetplay();
			void  Unhook();

			Nes::Input::Type GetController(uint);
				   
		private:

			struct Callbacks;

			class EventHandler
			{
			public:

				void Remove(const void*);

			private:

				friend class Emulator;
				friend struct Callbacks;

				typedef Object::Delegate<void,Event> Callback;
				typedef Collection::Vector<Callback> Callbacks;

				Callbacks callbacks;

				EventHandler() {}

				void operator () (Event) const;

			public:

				void Add(const Callback&);

				template<typename Data,typename Code>
				void Add(Data* data,Code code)
				{
					Add( Callback(data,code) );
				}
			};

			struct Settings
			{
				inline Settings();

				void Reset();

				struct Timing
				{
					inline Timing();

					uint  speed;
					uchar baseSpeed;
					bool  speeding;
					bool  sync;
					bool  rewinding;
				};

				struct Paths
				{
					Path start;
					Path image;
					Path save;
				};

				struct Fds
				{
					inline Fds();

					DiskImageSaveMethod save;
					Collection::Buffer original;
				};

				struct Cartridge
				{
					inline Cartridge();

					ibool writeProtect;
				};

				Timing timing;
				Paths paths;
				Cartridge cartridge;
				Fds fds;
				ibool askSave;
			};

			struct State
			{
				typedef Object::Delegate<ibool> Activator;
				typedef Object::Delegate<void> Inactivator;

				inline State();

				ibool NoActivator();
				void NoInactivator();

				ibool running;
				ibool paused;
				uint frame;
				Activator activator;
				Inactivator inactivator;
			};

			ibool IsDiskImage(const Collection::Buffer&) const;
			ibool UsesSaveData();
			ibool UsesBaseSpeed() const;
			void  EnableNetplay(const Netplay::Callback&,uint,uint);
			void  Unpause();

			template<typename T> void LoadCartridgeData(T&) const;
			void SaveCartridgeData(const void*,uint);
			void LoadDiskData(Collection::Buffer&);
			void SaveDiskData(const void*,uint);

			State state;
			EventHandler events;
			Netplay netplay;
			Settings settings;

		public:

			ibool IsRunning() const
			{
				return state.running;
			}

			ibool IsIdle() const
			{
				return !state.running;
			}

			ibool IsPaused() const
			{
				return state.paused;
			}

			ibool IsSpeeding() const
			{
				return settings.timing.speeding;
			}

			ibool IsRewinding() const
			{
				return settings.timing.rewinding;
			}

			uint GetFrame()
			{
				return state.frame;
			}

			uint Is(uint what)
			{
				return Nes::Machine(*this).Is( what );
			}

			uint Is(uint what,uint that)
			{
				return Nes::Machine(*this).Is( what, that );
			}

			void AskBeforeSaving()
			{
				settings.askSave = TRUE;
			}

			DiskImageSaveMethod GetDiskImageSaveMethod() const
			{
				return settings.fds.save;
			}

			void SetDiskImageSaveMethod(DiskImageSaveMethod method)
			{
				settings.fds.save = method;
			}

			void WriteProtectCartridge(ibool state)
			{
				settings.cartridge.writeProtect = state;
			}

			ibool IsCartridgeWriteProtected() const
			{
				return settings.cartridge.writeProtect;
			}

			const Path& GetStartPath() const
			{
				return settings.paths.start;
			}

			const Path& GetImagePath() const
			{
				return settings.paths.image;
			}

			const Path& GetSavePath() const
			{
				return settings.paths.save;
			}

			ibool SyncFrameRate() const
			{
				return settings.timing.sync;
			}

			uint GetPlayer() const
			{
				return netplay.player;
			}

			uint NumPlayers() const
			{
				return netplay.players;
			}

			EventHandler& Events()
			{
				return events;
			}

			template<typename Data,typename Activator,typename Inactivator>
			void Hook(Data data,Activator activator,Inactivator inactivator)
			{
				state.activator.Set( data, activator );
				state.inactivator.Set( data, inactivator );
			}

			template<typename Data,typename Code>
			void EnableNetplay(Data* data,Code code,uint player,uint players)
			{
				EnableNetplay( Netplay::Callback(data,code), player, players );
			}
		};
	}
}

#endif
