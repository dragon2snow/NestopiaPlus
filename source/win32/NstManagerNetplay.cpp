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

#include "NstIoLog.hpp"
#include "NstIoScreen.hpp"
#include "NstSystemDll.hpp"
#include "NstResourceString.hpp"
#include "NstWindowMenu.hpp"
#include "NstWindowUser.hpp"
#include "NstWindowParam.hpp"
#include "NstSystemThread.hpp"
#include "NstManagerPaths.hpp"
#include "NstManagerEmulator.hpp"
#include "NstDialogNetplay.hpp"
#include "NstManagerNetplay.hpp"
#include "../kaillera/kailleraclient.h"
#include "../core/api/NstApiFds.hpp"
#include <Shlwapi.h>

#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED WM_NULL
#endif

namespace Nestopia
{
	namespace Managers
	{
		class Netplay::Dll : System::Dll
		{
			typedef int (WINAPI *GetVersionFunc)(char*);
			typedef int (WINAPI *InitFunc)();
			typedef int (WINAPI *ShutdownFunc)();
			typedef int (WINAPI *SetInfosFunc)(kailleraInfos*);
			typedef int (WINAPI *SelectServerDialogFunc)(HWND);
			typedef int (WINAPI *ModifyPlayValuesFunc)(void*,int);
			typedef int (WINAPI *ChatSendFunc)(char*);
			typedef int (WINAPI *EndGameFunc)();

			InitFunc const Init;
			ShutdownFunc const Shutdown;

		public:

			using System::Dll::Loaded;

			GetVersionFunc         const GetVersion;
			SetInfosFunc           const SetInfos;
			SelectServerDialogFunc const SelectServerDialog;
			ModifyPlayValuesFunc   const ModifyPlayValues;
			ChatSendFunc           const ChatSend;
			EndGameFunc            const EndGame;

			Dll()
			:
			System::Dll        (_T("kailleraclient.dll")),
			GetVersion         (Fetch< GetVersionFunc         >( "_kailleraGetVersion@4"         )),
			Init               (Fetch< InitFunc               >( "_kailleraInit@0"               )),
			Shutdown           (Fetch< ShutdownFunc           >( "_kailleraShutdown@0"           )),
			SetInfos           (Fetch< SetInfosFunc           >( "_kailleraSetInfos@4"           )),
			SelectServerDialog (Fetch< SelectServerDialogFunc >( "_kailleraSelectServerDialog@4" )),
			ModifyPlayValues   (Fetch< ModifyPlayValuesFunc   >( "_kailleraModifyPlayValues@8"   )),
			ChatSend           (Fetch< ChatSendFunc           >( "_kailleraChatSend@4"           )),
			EndGame            (Fetch< EndGameFunc            >( "_kailleraEndGame@0"            ))
			{
				if
				(
					GetVersion &&
					Init &&
					Shutdown &&
					SetInfos &&
					SelectServerDialog &&
					ModifyPlayValues &&
					ChatSend &&
					EndGame
				)
					Init();
				else
					Unload();
			}

			~Dll()
			{
				if (Loaded())
					Shutdown();
			}
		};

		class Netplay::Kaillera
		{
		public:

			Kaillera(Emulator&,Window::Menu&,const Paths&,Window::Custom&,ibool);
			~Kaillera();

			enum Exception
			{
				ERR_LOAD
			};

			void Start();
			void Disconnect();
			void Chat();
			ibool Close() const;

		private:

			enum
			{
				MAX_PLAYERS         = 8,
				MASTER              = 1,
				WM_NST_OPEN_CLIENT  = WM_APP + 57,
				WM_NST_CLOSE_CLIENT = WM_APP + 58,
				WM_NST_START_GAME   = WM_APP + 59
			};

			class Command
			{
			public:

				void Capture();
				void Release() const;
				void Begin();
				void End();
				uint GetCode();
				void Dispatch(const uint,Nes::Input::Controllers&) const;

			private:

				struct Handler;
				typedef Window::Menu::CmdHandler CmdHandler;
				typedef CmdHandler::Callback MenuCallback;

				enum
				{
					CALLBACK_RESET            = 0,
					CALLBACK_INSERT_DISK      = CALLBACK_RESET + 2,
					CALLBACK_EJECT_DISK       = CALLBACK_INSERT_DISK + 8*2,
					CALLBACK_CHANGE_DISK_SIDE = CALLBACK_EJECT_DISK + 1,
					NUM_CALLBACKS             = CALLBACK_CHANGE_DISK_SIDE + 1
				};

				enum
				{
					PACKET_TYPE                 = Nes::b00001111,
					PACKET_DATA                 = Nes::b11110000,
					PACKET_DATA_SYSTEM          = Nes::b00000011,
					PACKET_DATA_SYSTEM_AUTO     = Nes::b00000000,
					PACKET_DATA_SYSTEM_NTSC     = Nes::b00000001,
					PACKET_DATA_SYSTEM_PAL      = Nes::b00000010,
					PACKET_DATA_NO_SPRITE_LIMIT = Nes::b00000100,
					PACKET_DATA_SHIFT           = 4,
					PACKET_STARTUP              = 1,
					PACKET_RESET                = 2,
					PACKET_INSERT_DISK_SIDE_A   = 3,
					PACKET_INSERT_DISK_SIDE_B   = 4,
					PACKET_EJECT_DISK           = 5,
					PACKET_INSERT_COIN_1        = 6,
					PACKET_INSERT_COIN_2        = 7
				};

				struct CoinCallback
				{
					Nes::Input::UserData data;
					Nes::Input::Controllers::VsSystem::PollCallback code;

					CoinCallback()
					: code(NULL) {}
				};

				struct Settings
				{
					uint system;
					ibool noSpriteLimit;
				};

				void OnReset      (uint);
				void OnInsertDisk (uint);
				void OnEjectDisk  (uint);
				void OnDiskSide   (uint);

				uint command;
				MenuCallback menuCallbacks[NUM_CALLBACKS];
				CoinCallback coinCallback;
				Settings settings;
			};

			class Input
			{
			public:

				void Capture();
				void Release() const;
				void Begin();
				void UpdatePorts(uint);

			private:

				struct PollCallback
				{
					Nes::Input::UserData data;
					Nes::Input::Controllers::Pad::PollCallback code;
				};

				uint pads[4];
				PollCallback pollCallback;
				Nes::Input::Type normalSetup[5];

			public:

				void Dispatch(const uint port,const uint packet,Nes::Input::Controllers& controllers) const
				{
					NST_ASSERT( port < 4 );
					controllers.pad[pads[port]].buttons = packet;
				}

				uint GetCode() const
				{
					if (instance->network.player <= 4)
					{
						Nes::Input::Controllers::Pad pad;
						pollCallback.code( pollCallback.data, pad, pads[instance->network.player-1] );
						return pad.buttons;
					}
					else
					{
						return 0;
					}
				}
			};

			struct Network
			{
				ibool connected;
				Command command;
				Input input;
				uint player;
				uint players;
				Path game;
			};

			struct Callbacks;
			class Client;

			ibool ResetWindow() const;
			void  Initialize();
			void  DisableVisualStyles();
			void  RestoreVisualStyles();
			void  StartNetwork(System::Thread::Terminator);

			ibool OnOpenClient  (Window::Param&);
			ibool OnCloseClient (Window::Param&);
			ibool OnStartGame   (Window::Param&);
			ibool OnEnable      (Window::Param&);

			void OnEmuFrame (Nes::Input::Controllers*);
			void OnEmuEvent (Emulator::Event);

			const Dll dll;
			Emulator& emulator;
			Window::Menu& menu;
			Window::Custom& window;
			Window::Netplay::Chat chat;
			Window::Netplay dialog;
			System::Thread thread;
			Window::MsgHandler::Callback enableCallback;
			Network network;
			DWORD visualStyles;

			static Kaillera* instance;

		public:

			ibool ShouldGoFullscreen() const
			{
				return dialog.ShouldGoFullscreen();
			}

			void SaveFile() const
			{
				dialog.SaveFile();
			}

			ibool Idle() const
			{
				return thread.Idle();
			}
		};

		struct Netplay::Kaillera::Callbacks
		{
			static int WINAPI Start(char* game,int player,int players)
			{
				NST_VERIFY( game && *game && players );

				uchar dummy[MAX_PLAYERS * 2];

				if (game && *game && instance->dll.ModifyPlayValues( dummy, 2 ) != -1)
				{
					instance->network.game = game;
					instance->network.player = player;
					instance->network.players = players;
					instance->window.Post( WM_NST_START_GAME );
				}
				else
				{
					instance->dll.EndGame();
				}

				return 0;
			}

			static void WINAPI ClientDrop(char* nick,int playerNum)
			{
				static const HeapString player( HeapString() << Resource::String(IDS_TEXT_PLAYER) << ' ' );
				static const HeapString droppedOut( HeapString() << ") " << Resource::String(IDS_TEXT_DROPPEDOUT) );

				if (nick && *nick)
					Io::Screen() << player << playerNum << " (" << nick << droppedOut;
			}

			static void WINAPI ChatRecieve(char* nick,char* text)
			{
				static const HeapString says( HeapString() << Resource::String(IDS_TEXT_SAYS) << ": " );

				if (nick && *nick && text && *text)
					Io::Screen() << nick << says << text;
			}
		};

		class Netplay::Kaillera::Client
		{
			// Uses a hook for monitoring the Kaillera windows activity.
			// The bug seems to be located in the Kaillera code so I have
			// to resolve to some dirty hacks to prevent the message queue
			// from entering an infinite loop. This will happen if the user
			// closes the main server list window while others are open.

			struct Instance
			{
				HHOOK hHook;
				DWORD threadId;

				Instance()
				: hHook(NULL) {}
			};

			static Instance instance;

			class Callbacks
			{
				static NST_NO_INLINE ibool IsKaillera(const Window::Generic window)
				{
					HeapString name;
					window.Text() >> name;
					return name.Length() >= 8 && name(0,8) == _T("Kaillera");
				}

			public:

				static BOOL CALLBACK Destroy(HWND hWnd,LPARAM)
				{
					if (IsKaillera( hWnd ))
						::SendMessage( hWnd, WM_SYSCOMMAND, SC_CLOSE, 0 );

					return true;
				}

				static BOOL CALLBACK Find(HWND hWnd,LPARAM lParam)
				{
					if (::GetParent( hWnd ) && IsKaillera( hWnd ))
					{
						*reinterpret_cast<HWND*>(lParam) = hWnd;
						return false;
					}

					return true;
				}

				static BOOL CALLBACK Show(HWND hWnd,LPARAM lParam)
				{
					if (IsKaillera( hWnd ))
						::ShowWindow( hWnd, lParam );

					return true;
				}
			};

			template<typename T>
			static void Enumerate(BOOL (CALLBACK* callback)(HWND,LPARAM),T t)
			{
				::EnumThreadWindows( instance.threadId, callback, LPARAM(t) );
			}

			static ibool IsZombie(HWND hWnd)
			{
				if (!::GetParent( hWnd ))
				{
					hWnd = NULL;
					Enumerate( Callbacks::Find, &hWnd );

					if (hWnd)
						return true;
				}

				return false;
			}

			static LRESULT CALLBACK MessageSpy(int iCode,WPARAM wParam,LPARAM lParam)
			{
				if (iCode == HC_ACTION)
				{
					MSG& msg = *reinterpret_cast<MSG*>(lParam);

					if (msg.message == WM_CLOSE && IsZombie( msg.hwnd ))
						msg.message = WM_NULL;
				}

				return ::CallNextHookEx( instance.hHook, iCode, wParam, lParam );
			}

		public:

			NST_NO_INLINE Client()
			{
				NST_ASSERT( !instance.hHook );

				instance.threadId = ::GetCurrentThreadId();

				instance.hHook = ::SetWindowsHookEx
				(
					WH_GETMESSAGE,
					MessageSpy,
					::GetModuleHandle(NULL),
					instance.threadId
				);

				if (!instance.hHook)
					throw "SetWindowsHookEx() failed!";

				Kaillera::instance->window.Post( WM_NST_OPEN_CLIENT );
			}

			static void Run()
			{
				Kaillera::instance->dll.SelectServerDialog( NULL );
			}

			static void Show()
			{
				if (instance.hHook)
					Enumerate( Callbacks::Show, SW_SHOW );
			}

			static void Hide()
			{
				if (instance.hHook)
					Enumerate( Callbacks::Show, SW_HIDE );
			}

			static void Close()
			{
				if (instance.hHook)
					Enumerate( Callbacks::Destroy, 0 );
			}

			static ibool IsOpen()
			{
				return instance.hHook != NULL;
			}

			NST_NO_INLINE ~Client()
			{
				NST_ASSERT( instance.hHook );

				::UnhookWindowsHookEx( instance.hHook );
				instance.hHook = NULL;

				Kaillera::instance->window.Post( WM_NST_CLOSE_CLIENT );
			}
		};

		struct Netplay::Kaillera::Command::Handler
		{
			static const Window::Menu::CmdHandler::Entry<Command> messages[];
		};

		const Window::Menu::CmdHandler::Entry<Netplay::Kaillera::Command> Netplay::Kaillera::Command::Handler::messages[] =
		{
			{ IDM_MACHINE_RESET_SOFT,                   &Command::OnReset      },
			{ IDM_MACHINE_RESET_HARD,                   &Command::OnReset      },
			{ IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A, &Command::OnInsertDisk },
			{ IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_B, &Command::OnInsertDisk },
			{ IDM_MACHINE_EXT_FDS_INSERT_DISK_2_SIDE_A, &Command::OnInsertDisk },
			{ IDM_MACHINE_EXT_FDS_INSERT_DISK_2_SIDE_B, &Command::OnInsertDisk },
			{ IDM_MACHINE_EXT_FDS_INSERT_DISK_3_SIDE_A, &Command::OnInsertDisk },
			{ IDM_MACHINE_EXT_FDS_INSERT_DISK_3_SIDE_B, &Command::OnInsertDisk },
			{ IDM_MACHINE_EXT_FDS_INSERT_DISK_4_SIDE_A, &Command::OnInsertDisk },
			{ IDM_MACHINE_EXT_FDS_INSERT_DISK_4_SIDE_B, &Command::OnInsertDisk },
			{ IDM_MACHINE_EXT_FDS_INSERT_DISK_5_SIDE_A, &Command::OnInsertDisk },
			{ IDM_MACHINE_EXT_FDS_INSERT_DISK_5_SIDE_B, &Command::OnInsertDisk },
			{ IDM_MACHINE_EXT_FDS_INSERT_DISK_6_SIDE_A, &Command::OnInsertDisk },
			{ IDM_MACHINE_EXT_FDS_INSERT_DISK_6_SIDE_B, &Command::OnInsertDisk },
			{ IDM_MACHINE_EXT_FDS_INSERT_DISK_7_SIDE_A, &Command::OnInsertDisk },
			{ IDM_MACHINE_EXT_FDS_INSERT_DISK_7_SIDE_B, &Command::OnInsertDisk },
			{ IDM_MACHINE_EXT_FDS_INSERT_DISK_8_SIDE_A, &Command::OnInsertDisk },
			{ IDM_MACHINE_EXT_FDS_INSERT_DISK_8_SIDE_B, &Command::OnInsertDisk },
			{ IDM_MACHINE_EXT_FDS_EJECT_DISK,           &Command::OnEjectDisk  },
			{ IDM_MACHINE_EXT_FDS_CHANGE_SIDE,          &Command::OnDiskSide   }
		};

		void Netplay::Kaillera::Command::Capture()
		{
			CmdHandler& menu = instance->menu.Commands();

			for (uint i=0; i < NST_COUNT(Handler::messages); ++i)
				menuCallbacks[i] = menu[Handler::messages[i].key].Replace( this, Handler::messages[i].function );
		}

		void Netplay::Kaillera::Command::Release() const
		{
			CmdHandler& menu = instance->menu.Commands();

			for (uint i=0; i < NST_COUNT(Handler::messages); ++i)
				menu[Handler::messages[i].key] = menuCallbacks[i];
		}

		void Netplay::Kaillera::Command::Begin()
		{
			if (instance->emulator.Is( Nes::Machine::VS ))
			{
				Nes::Input::Controllers::VsSystem::callback.Get( coinCallback.code, coinCallback.data );
				Nes::Input::Controllers::VsSystem::callback.Set( NULL, NULL );
			}

			if (instance->menu[IDM_MACHINE_SYSTEM_NTSC].Checked())
			{
				settings.system = IDM_MACHINE_SYSTEM_NTSC;
			}
			else if (instance->menu[IDM_MACHINE_SYSTEM_PAL].Checked())
			{
				settings.system = IDM_MACHINE_SYSTEM_PAL;
			}
			else
			{
				settings.system = IDM_MACHINE_SYSTEM_AUTO;
			}

			settings.noSpriteLimit = Nes::Video(instance->emulator).AreUnlimSpritesEnabled();

			if (instance->network.player == MASTER)
			{
				command = PACKET_STARTUP;

				if (settings.noSpriteLimit)
					command |= (PACKET_DATA_NO_SPRITE_LIMIT << PACKET_DATA_SHIFT);

				switch (settings.system)
				{
					case IDM_MACHINE_SYSTEM_NTSC: command |= (PACKET_DATA_SYSTEM_NTSC << PACKET_DATA_SHIFT); break;
					case IDM_MACHINE_SYSTEM_PAL:  command |= (PACKET_DATA_SYSTEM_PAL  << PACKET_DATA_SHIFT); break;
				}
			}
			else
			{
				command = 0;
			}
		}

		void Netplay::Kaillera::Command::End()
		{
			if (coinCallback.code)
			{
				Nes::Input::Controllers::VsSystem::callback.Set( coinCallback.code, coinCallback.data );
				coinCallback.code = NULL;
			}

			if (instance->network.player != MASTER)
			{
				instance->window.PostCommand( settings.system );

				if (bool(settings.noSpriteLimit) != bool(Nes::Video(instance->emulator).AreUnlimSpritesEnabled()))
					Nes::Video(instance->emulator).EnableUnlimSprites( settings.noSpriteLimit );
			}
		}

		void Netplay::Kaillera::Command::Dispatch(const uint packet,Nes::Input::Controllers& controllers) const
		{
			if (packet)
			{
				if (packet == PACKET_INSERT_COIN_1)
				{
					controllers.vsSystem.insertCoin = Nes::Input::Controllers::VsSystem::COIN_1;
				}
				else if (packet == PACKET_INSERT_COIN_2)
				{
					controllers.vsSystem.insertCoin = Nes::Input::Controllers::VsSystem::COIN_2;
				}
				else
				{
					uint data = packet >> PACKET_DATA_SHIFT;

					switch (packet & PACKET_TYPE)
					{
						case PACKET_RESET:

							NST_VERIFY( data < 2 );

							if (data < 2)
								menuCallbacks[CALLBACK_RESET + data](IDM_MACHINE_RESET_SOFT + data);

							break;

						case PACKET_INSERT_DISK_SIDE_A:
						case PACKET_INSERT_DISK_SIDE_B:

							data = data * 2 + ((packet & PACKET_TYPE) == PACKET_INSERT_DISK_SIDE_B);

							NST_VERIFY( data < 16 );

							if (data < 16)
								menuCallbacks[CALLBACK_INSERT_DISK + data](IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A + data);

							break;

						case PACKET_EJECT_DISK:

							NST_VERIFY( data == 0 );

							menuCallbacks[CALLBACK_EJECT_DISK](IDM_MACHINE_EXT_FDS_EJECT_DISK);
							break;

						case PACKET_STARTUP:

							if (instance->network.player != MASTER)
							{
								instance->window.SendCommand
								(
									(data & PACKET_DATA_SYSTEM) == PACKET_DATA_SYSTEM_NTSC ? IDM_MACHINE_SYSTEM_NTSC :
									(data & PACKET_DATA_SYSTEM) == PACKET_DATA_SYSTEM_PAL  ? IDM_MACHINE_SYSTEM_PAL  :
                                                                                             IDM_MACHINE_SYSTEM_AUTO
								);

								if (bool(settings.noSpriteLimit) != bool(data & PACKET_DATA_NO_SPRITE_LIMIT))
									Nes::Video(instance->emulator).EnableUnlimSprites( !settings.noSpriteLimit );
							}
							break;
					}
				}
			}
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		uint Netplay::Kaillera::Command::GetCode()
		{
			if (instance->network.player == MASTER)
			{
				if (coinCallback.code)
				{
					Nes::Input::Controllers::VsSystem vs;
					coinCallback.code( coinCallback.data, vs );

					switch (vs.insertCoin)
					{
						case Nes::Input::Controllers::VsSystem::COIN_1:
						case Nes::Input::Controllers::VsSystem::COIN_1|Nes::Input::Controllers::VsSystem::COIN_2:
							return PACKET_INSERT_COIN_1;

						case Nes::Input::Controllers::VsSystem::COIN_2:
							return PACKET_INSERT_COIN_2;
					}
				}

				const uint code = command;
				command = 0;
				return code;
			}

			return 0;
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Netplay::Kaillera::Command::OnReset(uint idm)
		{
			idm -= IDM_MACHINE_RESET_SOFT;
			command = PACKET_RESET | (idm << PACKET_DATA_SHIFT);
		}

		void Netplay::Kaillera::Command::OnInsertDisk(uint idm)
		{
			idm -= IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A;
			command = (PACKET_INSERT_DISK_SIDE_A + (idm % 2)) | ((idm / 2) << PACKET_DATA_SHIFT);
		}

		void Netplay::Kaillera::Command::OnEjectDisk(uint)
		{
			command = PACKET_EJECT_DISK;
		}

		void Netplay::Kaillera::Command::OnDiskSide(uint)
		{
			const int disk = Nes::Fds(instance->emulator).GetCurrentDisk();

			if (disk != Nes::Fds::NO_DISK)
				command = (PACKET_INSERT_DISK_SIDE_A + (Nes::Fds(instance->emulator).GetCurrentDiskSide() ^ 1)) | (uint(disk) << PACKET_DATA_SHIFT);
		}

		void Netplay::Kaillera::Input::Capture()
		{
			for (uint i=0; i < 5; ++i)
				normalSetup[i] = instance->emulator.GetController( i );

			Nes::Input::Controllers::Pad::callback.Get( pollCallback.code, pollCallback.data );
			Nes::Input::Controllers::Pad::callback.Set( NULL, NULL );
		}

		void Netplay::Kaillera::Input::Release() const
		{
			Nes::Input::Controllers::Pad::callback.Set( pollCallback.code, pollCallback.data );

			for (uint i=0; i < 5; ++i)
				instance->emulator.ConnectController( i, normalSetup[i] );
		}

		void Netplay::Kaillera::Input::Begin()
		{
			if (instance->network.player <= 4)
			{
				Nes::Input::Type defaultPad;

				switch (normalSetup[0])
				{
					case Nes::Input::PAD2:
					case Nes::Input::PAD3:
					case Nes::Input::PAD4:

						defaultPad = normalSetup[0];
						break;

					default:

						defaultPad = Nes::Input::PAD1;
						break;
				}

				instance->emulator.ConnectController( instance->network.player-1, defaultPad );
				UpdatePorts( instance->network.player-1 );
			}
			else for (uint i=0; i < 4; ++i)
			{
				pads[i] = i;
				instance->emulator.ConnectController( i, (Nes::Input::Type) (Nes::Input::PAD1+i) );
			}

			for (uint i=instance->network.players; i < 4; ++i)
				instance->emulator.ConnectController( i, Nes::Input::UNCONNECTED );

			instance->emulator.ConnectController( 4, Nes::Input::UNCONNECTED );
		}

		void Netplay::Kaillera::Input::UpdatePorts(const uint port)
		{
			if (port == instance->network.player-1)
			{
				switch (const Nes::Input::Type type = instance->emulator.GetController( port ))
				{
					case Nes::Input::PAD1:
					case Nes::Input::PAD2:
					case Nes::Input::PAD3:
					case Nes::Input::PAD4:

						pads[port] = type - Nes::Input::PAD1;
						break;

					default:

						pads[port] = 0;
						instance->emulator.ConnectController( port, Nes::Input::PAD1 );
						return;
				}

				for (uint i=1; i < 4; ++i)
					pads[(port+i) & 3] = (pads[port] + i) & 3;

				for (uint i=0, n=NST_MIN(4,instance->network.players); i < n; ++i)
				{
					if (i != port)
						instance->emulator.ConnectController( i, (Nes::Input::Type) (Nes::Input::PAD1 + pads[i]) );
				}
			}
		}

		Netplay::Kaillera* Netplay::Kaillera::instance = NULL;
		Netplay::Kaillera::Client::Instance Netplay::Kaillera::Client::instance;

		Netplay::Kaillera::Kaillera
		(
			Emulator& e,
			Window::Menu& m,
			const Paths& paths,
			Window::Custom& w,
			const ibool doFullscreen
		)
		:
		emulator ( e ),
		menu     ( m ),
		window   ( w ),
		dialog   ( emulator, paths, doFullscreen ),
		chat     ( dll.ChatSend )
		{
			if (!dll.Loaded())
				throw ERR_LOAD;

			NST_ASSERT( instance == NULL );

			instance = this;
			emulator.Events().Add( this, &Kaillera::OnEmuEvent );
		}

		Netplay::Kaillera::~Kaillera()
		{
			instance = NULL;
			emulator.Events().Remove( this );
		}

		void Netplay::Kaillera::Initialize()
		{
			static const Window::MsgHandler::Entry<Kaillera> messages[] =
			{
				{ WM_NST_OPEN_CLIENT,  &Kaillera::OnOpenClient  },
				{ WM_NST_CLOSE_CLIENT, &Kaillera::OnCloseClient },
				{ WM_NST_START_GAME,   &Kaillera::OnStartGame   }
			};

			window.Messages().Add( this, messages );

			NST_ASSERT( dialog.GetNumGames() );

			String::Heap<char> strings;

			for (uint i=0, n=dialog.GetNumGames(); i < n; ++i)
				strings << dialog.GetGame(i) << '\0';

			String::Heap<char> name;
			name << "Nestopia " << Application::Instance::GetVersion();

			kailleraInfos info;

			info.appName               = name.Ptr();
			info.gameList              = strings.Ptr();
			info.gameCallback          = Callbacks::Start;
			info.chatReceivedCallback  = Callbacks::ChatRecieve;
			info.clientDroppedCallback = Callbacks::ClientDrop;
			info.moreInfosCallback     = NULL;

			dll.SetInfos( &info );
		}

		void Netplay::Kaillera::DisableVisualStyles()
		{
			// Kaillera doesn't seem to like XP Visual Styles

			struct ComCtl32
			{
				static bool IsVersion6()
				{
					System::Dll comctl32(_T("comctl32.dll"));

					if (comctl32.Loaded())
					{
						if (DLLGETVERSIONPROC getVersion = (DLLGETVERSIONPROC) comctl32("DllGetVersion"))
						{
							DLLVERSIONINFO info;
							info.cbSize = sizeof(info);

							return getVersion( &info ) == NOERROR && info.dwMajorVersion >= 6;
						}
					}

					return false;
				}
			};

			visualStyles = 0;

			static const bool isVersion6 = ComCtl32::IsVersion6();

			if (isVersion6)
			{
				System::Dll uxtheme(_T("uxtheme.dll"));

				if (uxtheme.Loaded())
				{
					typedef DWORD (STDAPICALLTYPE* GetProperty)();
					typedef void (STDAPICALLTYPE* SetProperty)(DWORD);

					if (GetProperty getProperty = (GetProperty) uxtheme("GetThemeAppProperties"))
					{
						visualStyles = getProperty();

						if (visualStyles)
						{
							if (SetProperty setProperty = (SetProperty) uxtheme("SetThemeAppProperties"))
							{
								setProperty( 0 );
								Application::Instance::GetMainWindow().Send( WM_THEMECHANGED );
							}
						}
					}
				}
			}
		}

		void Netplay::Kaillera::RestoreVisualStyles()
		{
			if (visualStyles)
			{
				System::Dll uxtheme(_T("uxtheme.dll"));

				if (uxtheme.Loaded())
				{
					typedef void (STDAPICALLTYPE* SetProperty)(DWORD);

					if (SetProperty setProperty = (SetProperty) uxtheme("SetThemeAppProperties"))
					{
						setProperty( visualStyles );
						visualStyles = 0;
						Application::Instance::GetMainWindow().Send( WM_THEMECHANGED );
					}
				}
			}
		}

		void Netplay::Kaillera::StartNetwork(System::Thread::Terminator)
		{
			Client().Run();
		}

		void Netplay::Kaillera::Start()
		{
			if (ResetWindow() && dialog.Open())
			{
				DisableVisualStyles();
				Initialize();
				thread.Start( System::Thread::Callback(this,&Kaillera::StartNetwork) );
			}
		}

		ibool Netplay::Kaillera::OnOpenClient(Window::Param&)
		{
			menu[IDM_NETPLAY_START].Disable();
			network.command.Capture();
			network.input.Capture();
			emulator.BeginNetplayMode();
			return true;
		}

		ibool Netplay::Kaillera::OnCloseClient(Window::Param&)
		{
			menu[IDM_NETPLAY_START].Enable();
			network.command.Release();
			network.input.Release();
			window.Messages().Remove( this );
			emulator.EndNetplayMode();
			RestoreVisualStyles();

			return true;
		}

		ibool Netplay::Kaillera::Close() const
		{
			if (Client::IsOpen())
			{
				if (!emulator.Unload())
					Client::Close();

				return true;
			}

			return false;
		}

		ibool Netplay::Kaillera::OnStartGame(Window::Param&)
		{
			network.connected = false;

			emulator.EnableNetplay
			(
				this,
				&Kaillera::OnEmuFrame,
				network.player,
				network.players
			);

			Application::Instance::GetMainWindow().Send
			(
				Application::Instance::WM_NST_LAUNCH,
				Paths::File::GAME|Paths::File::ARCHIVE,
				dialog.GetPath(network.game.Ptr())
			);

			if (!emulator.Is(Nes::Machine::ON))
			{
				emulator.DisableNetplay();
				dll.EndGame();
			}

			return true;
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		void Netplay::Kaillera::OnEmuFrame(Nes::Input::Controllers* controllers)
		{
			NST_ASSERT( controllers );

			std::memset( controllers->pad, 0, sizeof(controllers->pad) );
			controllers->vsSystem.insertCoin = 0;

			if (network.connected)
			{
				u8 packets[MAX_PLAYERS][2];

				packets[0][0] = network.input.GetCode();
				packets[0][1] = network.command.GetCode();

				if (dll.ModifyPlayValues( packets, 2 ) != -1)
				{
					for (uint i=0, ports=NST_MIN(4,network.players); i < ports; ++i)
					{
						network.input.Dispatch( i, packets[i][0], *controllers );
						network.command.Dispatch( packets[i][1], *controllers );
					}
				}
				else
				{
					network.connected = false;
					window.PostCommand( IDM_NETPLAY_DISCONNECT );
				}
			}
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		ibool Netplay::Kaillera::ResetWindow() const
		{
			window.SendCommand( IDM_FILE_CLOSE );

			if (emulator.Is( Nes::Machine::IMAGE ))
			{
				// user refused, ok,
				// no netplay then..

				return false;
			}

			while (Application::Instance::NumChildWindows())
			{
				if (!Application::Instance::GetChildWindow().Destroy())
				{
					// some window didn't want to
					// close itself, better abort..

					return false;
				}
			}

			return true;
		}

		void Netplay::Kaillera::Chat()
		{
			chat.Open();
		}

		void Netplay::Kaillera::Disconnect()
		{
			network.connected = false;
			emulator.Unload();
		}

		ibool Netplay::Kaillera::OnEnable(Window::Param& param)
		{
			if (!param.wParam)
				window.Send( WM_ENABLE, true, 0 );

			return true;
		}

		void Netplay::Kaillera::OnEmuEvent(Emulator::Event event)
		{
			static Window::MsgHandler::Callback old;

			switch (event)
			{
				case Emulator::EVENT_NETPLAY_POWER_ON:

					if (emulator.Is( Nes::Machine::GAME ))
					{
						Client::Hide();
						network.connected = true;

						menu[IDM_NETPLAY_DISCONNECT].Enable();
						menu[IDM_NETPLAY_CHAT].Enable();

						if (dialog.ShouldGoFullscreen())
							window.SendCommand( IDM_VIEW_SWITCH_SCREEN );

						network.command.Begin();
						network.input.Begin();
						enableCallback = window.Messages()[WM_ENABLE].Replace( this, &Kaillera::OnEnable );
					}
					break;

				case Emulator::EVENT_PORT1_CONTROLLER:
				case Emulator::EVENT_PORT2_CONTROLLER:
				case Emulator::EVENT_PORT3_CONTROLLER:
				case Emulator::EVENT_PORT4_CONTROLLER:

					if (network.connected)
						network.input.UpdatePorts( event - Emulator::EVENT_PORT1_CONTROLLER );

					break;

				case Emulator::EVENT_NETPLAY_POWER_OFF:

					if (emulator.Is( Nes::Machine::GAME ))
					{
						menu[IDM_NETPLAY_DISCONNECT].Disable();
						menu[IDM_NETPLAY_CHAT].Disable();

						chat.Close();

						if (dialog.ShouldGoFullscreen())
							window.SendCommand( IDM_VIEW_SWITCH_SCREEN );

						network.command.End();
						window.Messages()[WM_ENABLE] = enableCallback;

						Client::Show();
						dll.EndGame();
					}
					break;

				case Emulator::EVENT_NETPLAY_UNLOAD:

					emulator.DisableNetplay();
					break;

				case Emulator::EVENT_INIT:

					menu[IDM_NETPLAY_DISCONNECT].Disable();
					menu[IDM_NETPLAY_CHAT].Disable();
					break;
			}
		}

		Netplay::Netplay
		(
			Emulator& e,
			const Configuration& cfg,
			Window::Menu& m,
			const Paths& p,
			Window::Custom& w
		)
		:
		kaillera     ( NULL ),
		emulator     ( e ),
		menu         ( m ),
		paths        ( p ),
		window       ( w ),
		doFullscreen ( cfg["netplay in fullscreen"] == Configuration::YES )
		{
			const Dll dll;

			menu[ IDM_NETPLAY_START ].Enable( dll.Loaded() );
			menu[ IDM_NETPLAY_DISCONNECT ].Disable();
			menu[ IDM_NETPLAY_CHAT ].Disable();

			Io::Log log;

			if (dll.Loaded())
			{
				Application::Instance::Events::Add( this, &Netplay::OnAppEvent );

				static const Window::Menu::CmdHandler::Entry<Netplay> commands[] =
				{
					{ IDM_NETPLAY_START,      &Netplay::OnCmdStart      },
					{ IDM_NETPLAY_DISCONNECT, &Netplay::OnCmdDisconnect },
					{ IDM_NETPLAY_CHAT,       &Netplay::OnCmdChat       }
				};

				menu.Commands().Add( this, commands );

				char version[16];
				version[0] = '\0';

				dll.GetVersion( version );

				if (*version)
				{
					log << "Kaillera: found \"kailleraclient.dll\" version " << version << "\r\n";

					if (std::strcmp( version, "0.9" ))
						log << "Kaillera: warning, the DLL file may be incompatible with Nestopia!\r\n";
				}
				else
				{
					log << "Kaillera: warning, unknown version of \"kailleraclient.dll\"!\r\n";
				}
			}
			else
			{
				log << "Kaillera: file \"kailleraclient.dll\" not found or initialization failed. "
                       "netplay will be disabled!\r\n";
			}
		}

		Netplay::~Netplay()
		{
			Application::Instance::Events::Remove( this );
			delete kaillera;
		}

		void Netplay::OnCmdStart(uint)
		{
			if (kaillera == NULL)
			{
				try
				{
					kaillera = new Kaillera( emulator, menu, paths, window, doFullscreen );
				}
				catch (Kaillera::Exception)
				{
					return;
				}
			}

			kaillera->Start();
		}

		void Netplay::OnCmdDisconnect(uint)
		{
			if (kaillera)
				kaillera->Disconnect();
		}

		void Netplay::OnCmdChat(uint)
		{
			if (kaillera)
				kaillera->Chat();
		}

		void Netplay::OnAppEvent(Application::Instance::Event event,const void*)
		{
			if (kaillera == NULL || kaillera->Idle())
			{
				switch (event)
				{
					case Application::Instance::EVENT_DESKTOP:
					case Application::Instance::EVENT_FULLSCREEN:

						menu[IDM_NETPLAY_START].Enable( event == Application::Instance::EVENT_DESKTOP );
						break;
				}
			}
		}

		void Netplay::Save(Configuration& cfg,const ibool saveGameList) const
		{
			cfg["netplay in fullscreen"].YesNo() = (kaillera ? kaillera->ShouldGoFullscreen() : doFullscreen);

			if (kaillera && saveGameList)
				kaillera->SaveFile();
		}

		ibool Netplay::Close() const
		{
			return kaillera ? kaillera->Close() : false;
		}
	}
}
