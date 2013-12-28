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

#pragma comment(lib,"kailleraclient")

#include "NstIoLog.hpp"
#include "NstIoScreen.hpp"
#include "NstSystemDll.hpp"
#include "NstResourceString.hpp"
#include "NstResourceVersion.hpp"
#include "NstWindowMenu.hpp"
#include "NstWindowCustom.hpp"
#include "NstWindowUser.hpp"
#include "NstWindowParam.hpp"
#include "NstSystemThread.hpp"
#include "NstManagerPaths.hpp"
#include "NstManagerEmulator.hpp"
#include "NstDialogNetplay.hpp"
#include "NstManagerNetplay.hpp"
#include "../kaillera/kailleraclient.h"

namespace Nestopia
{
	using namespace Managers;

	class Netplay::Kaillera : public System::Dll
	{
		enum 
		{
			MAX_PLAYERS         = 8,
			MASTER              = 1,
			WM_NST_OPEN_CLIENT	= WM_APP + 57,
			WM_NST_CLOSE_CLIENT = WM_APP + 58,
			WM_NST_START_GAME   = WM_APP + 59,
			WM_NST_GAME_COMMAND = WM_APP + 60
		};

		class Command
		{
		public:

			void Capture();
			void Release() const;
			void Begin();
			void End();
			uint GetCode();

			static void Dispatch(const uint,Nes::Input::Controllers&);

		private:

			struct Handler;
			typedef Window::Menu::CmdHandler CmdHandler;
			typedef CmdHandler::Callback MenuCallback;

			enum
			{
				CALLBACK_RESET       = 0,
				CALLBACK_INSERT_DISK = CALLBACK_RESET + 2,
				CALLBACK_EJECT_DISK  = CALLBACK_INSERT_DISK + 8*2,
				NUM_CALLBACKS        = CALLBACK_EJECT_DISK + 1
			};

			enum
			{
				PACKET_TYPE                 = Nes::b00001111,
				PACKET_DATA                 = Nes::b11110000,
				PACKET_DATA_SYSTEM			= Nes::b00000011,
				PACKET_DATA_SYSTEM_AUTO     = Nes::b00000000,
				PACKET_DATA_SYSTEM_NTSC     = Nes::b00000001,
				PACKET_DATA_SYSTEM_PAL      = Nes::b00000010,
				PACKET_DATA_NO_SPRITE_LIMIT	= Nes::b00000100,
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

			ibool OnMessage(Window::Param&);

			void OnReset      (uint);
			void OnInsertDisk (uint);
			void OnEjectDisk  (uint);
			
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
			uint GetCode() const;

		private:

			struct Handler;
			typedef Window::Menu::CmdHandler CmdHandler;
			typedef CmdHandler::Callback MenuCallback;

			enum
			{
				NUM_MENU_CALLBACKS = 4 * 5
			};

			void SelectPort(const uint,const uint);

			void OnPort1 (uint);
			void OnPort2 (uint);
			void OnPort3 (uint);
			void OnPort4 (uint);

			struct PollCallback
			{
				Nes::Input::UserData data;
				Nes::Input::Controllers::Pad::PollCallback code;
			};

			uint padIndex;
			PollCallback pollCallback;
			MenuCallback menuCallbacks[NUM_MENU_CALLBACKS];
		};

		struct Network
		{
			ibool connected;
			Command command;
			Input input;
			uint player;
			uint players;
			String::Path<false> game;
		};

		struct Callbacks;
		class Client;

		ibool ResetWindow() const;
		void  Initialize();
		void  Start();
		void  StartNetwork(System::Thread::Interrupt);

		void OnMenuStart      (uint);
		void OnMenuDisconnect (uint);
		void OnMenuChat       (uint);

		ibool OnOpenClient  (Window::Param&);
		ibool OnCloseClient (Window::Param&);
		ibool OnStartGame   (Window::Param&);
		ibool OnEnable      (Window::Param&);

		void OnEmuFrame (Nes::Input::Controllers*);	
		void OnEmuEvent (Emulator::Event);
		void OnAppEvent (Application::Instance::Event,const void*);

		Emulator& emulator;
		Window::Menu& menu;
		Window::Custom& window;
		Window::Netplay::Chat chat;
		Window::Netplay* dialog;
		System::Thread thread;
		Window::MsgHandler::Callback enableCallback;
		Network network;
		
		static Kaillera* instance;

	public:

		Kaillera(Emulator&,const Configuration&,Window::Menu&,const Paths&,Window::Custom&);
		~Kaillera();

		void Save(Configuration&,const ibool) const;
		ibool Close() const;
	};

	struct Netplay::Kaillera::Callbacks
	{
		static int WINAPI Start(char* game,int player,int players)
		{
			NST_VERIFY( game && *game && players );

			uchar dummy[MAX_PLAYERS * 2];

			if (game && *game && ::kailleraModifyPlayValues( dummy, 2 ) != -1)
			{
				instance->network.game = game;
				instance->network.player = player;
				instance->network.players = players;
				instance->window.Post( WM_NST_START_GAME );
			}
			else
			{
				::kailleraEndGame();
			}

			return 0;
		}

		static void WINAPI ClientDrop(char* nick,int player)
		{
			if (nick && *nick)
				Io::Screen() << "Player " << player << " (" << nick << ") dropped out...";
		}

		static void WINAPI ChatRecieve(char* nick,char* text)
		{
			if (nick && *nick && text && *text)
				Io::Screen() << nick << " says: " << text;
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
				String::Smart<64> name;
				window.Text() >> name;
				return name.Size() >= 8 && name(0,8) == "Kaillera";
			}

		public:

			static BOOL CALLBACK Destroy(HWND hWnd,LPARAM)
			{
				if (IsKaillera( hWnd ))
					::SendMessage( hWnd, WM_SYSCOMMAND, SC_CLOSE, 0 );

				return TRUE;
			}

			static BOOL CALLBACK Find(HWND hWnd,LPARAM lParam)
			{
				if (::GetParent( hWnd ) && IsKaillera( hWnd ))
				{
					*reinterpret_cast<HWND*>(lParam) = hWnd;
					return FALSE;
				}

				return TRUE;
			}

			static BOOL CALLBACK Show(HWND hWnd,LPARAM lParam)
			{
				if (IsKaillera( hWnd ))
					::ShowWindow( hWnd, lParam );

				return TRUE;
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
					return TRUE;
			}

			return FALSE;
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
			NST_ASSERT( !instance.hHook && Application::Instance::GetHandle() );

			instance.threadId = ::GetCurrentThreadId();

			instance.hHook = ::SetWindowsHookEx
			( 
		     	WH_GETMESSAGE, 
				MessageSpy, 
				Application::Instance::GetHandle(), 
				instance.threadId 
			);

			if (!instance.hHook)
				throw "SetWindowsHookEx() failed!";

			Kaillera::instance->window.Post( WM_NST_OPEN_CLIENT );
		}

		static void Run()
		{
			::kailleraSelectServerDialog( NULL );
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
		{ IDM_MACHINE_RESET_SOFT,				&Command::OnReset      },
		{ IDM_MACHINE_RESET_HARD,				&Command::OnReset      },
		{ IDM_MACHINE_FDS_INSERT_DISK_1_SIDE_A,	&Command::OnInsertDisk },
		{ IDM_MACHINE_FDS_INSERT_DISK_1_SIDE_B,	&Command::OnInsertDisk },
		{ IDM_MACHINE_FDS_INSERT_DISK_2_SIDE_A,	&Command::OnInsertDisk },
		{ IDM_MACHINE_FDS_INSERT_DISK_2_SIDE_B,	&Command::OnInsertDisk },
		{ IDM_MACHINE_FDS_INSERT_DISK_3_SIDE_A,	&Command::OnInsertDisk },
		{ IDM_MACHINE_FDS_INSERT_DISK_3_SIDE_B,	&Command::OnInsertDisk },
		{ IDM_MACHINE_FDS_INSERT_DISK_4_SIDE_A,	&Command::OnInsertDisk },
		{ IDM_MACHINE_FDS_INSERT_DISK_4_SIDE_B,	&Command::OnInsertDisk },
		{ IDM_MACHINE_FDS_INSERT_DISK_5_SIDE_A,	&Command::OnInsertDisk },
		{ IDM_MACHINE_FDS_INSERT_DISK_5_SIDE_B,	&Command::OnInsertDisk },
		{ IDM_MACHINE_FDS_INSERT_DISK_6_SIDE_A,	&Command::OnInsertDisk },
		{ IDM_MACHINE_FDS_INSERT_DISK_6_SIDE_B,	&Command::OnInsertDisk },
		{ IDM_MACHINE_FDS_INSERT_DISK_7_SIDE_A,	&Command::OnInsertDisk },
		{ IDM_MACHINE_FDS_INSERT_DISK_7_SIDE_B,	&Command::OnInsertDisk },
		{ IDM_MACHINE_FDS_INSERT_DISK_8_SIDE_A,	&Command::OnInsertDisk },
		{ IDM_MACHINE_FDS_INSERT_DISK_8_SIDE_B,	&Command::OnInsertDisk },
		{ IDM_MACHINE_FDS_EJECT_DISK,			&Command::OnEjectDisk  }
	};

	void Netplay::Kaillera::Command::Capture()
	{
		instance->window.Messages().Add( WM_NST_GAME_COMMAND, this, &Command::OnMessage );

		CmdHandler& menu = instance->menu.Commands();

		for (uint i=0; i < NST_COUNT(Handler::messages); ++i)
			menuCallbacks[i] = menu[Handler::messages[i].key].Replace( this, Handler::messages[i].function );
	}

	void Netplay::Kaillera::Command::Release() const
	{
		instance->window.Messages().Remove( this );

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

		if (instance->menu[IDM_MACHINE_SYSTEM_NTSC].IsChecked())
		{
			settings.system = IDM_MACHINE_SYSTEM_NTSC;
		}
		else if (instance->menu[IDM_MACHINE_SYSTEM_PAL].IsChecked())
		{
			settings.system = IDM_MACHINE_SYSTEM_PAL;
		}
		else
		{
			settings.system = IDM_MACHINE_SYSTEM_AUTO;
		}

		settings.noSpriteLimit = instance->menu[IDM_MACHINE_OPTIONS_UNLIMITEDSPRITES].IsChecked();

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

			if (bool(settings.noSpriteLimit) != bool(instance->menu[IDM_MACHINE_OPTIONS_UNLIMITEDSPRITES].IsChecked()))
				instance->window.PostCommand( IDM_MACHINE_OPTIONS_UNLIMITEDSPRITES );
		}
	}

	void Netplay::Kaillera::Command::Dispatch(const uint packet,Nes::Input::Controllers& controllers)
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
				instance->window.Post( WM_NST_GAME_COMMAND, packet, 0 );
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
		idm -= IDM_MACHINE_FDS_INSERT_DISK_1_SIDE_A;
		command = (PACKET_INSERT_DISK_SIDE_A + (idm % 2)) | ((idm / 2) << PACKET_DATA_SHIFT);
	}

	void Netplay::Kaillera::Command::OnEjectDisk(uint)
	{
		command = PACKET_EJECT_DISK;
	}

	ibool Netplay::Kaillera::Command::OnMessage(Window::Param& param)
	{
		uint data = param.wParam >> PACKET_DATA_SHIFT;

		switch (param.wParam & PACKET_TYPE)
		{
			case PACKET_RESET:              
		
				NST_VERIFY( data < 2 );

				if (data < 2)
					menuCallbacks[CALLBACK_RESET + data](IDM_MACHINE_RESET_SOFT + data);
		
				break;
		
			case PACKET_INSERT_DISK_SIDE_A: 
			case PACKET_INSERT_DISK_SIDE_B: 
		
				data = data * 2 + ((param.wParam & PACKET_TYPE) == PACKET_INSERT_DISK_SIDE_B);

				NST_VERIFY( data < 16 );
		
				if (data < 16)
					menuCallbacks[CALLBACK_INSERT_DISK + data](IDM_MACHINE_FDS_INSERT_DISK_1_SIDE_A + data);
		
				break;
		
			case PACKET_EJECT_DISK:         
		
				NST_VERIFY( data == 0 );

				menuCallbacks[CALLBACK_EJECT_DISK](IDM_MACHINE_FDS_EJECT_DISK);
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
						instance->window.SendCommand( IDM_MACHINE_OPTIONS_UNLIMITEDSPRITES );
				}
				break;
		}

		return TRUE;
	}

	struct Netplay::Kaillera::Input::Handler
	{
		struct Entry
		{
			ushort idm;
			ushort type;
			void (Input::*function)(uint);
		};

		static const Entry entries[];
	};

	const Netplay::Kaillera::Input::Handler::Entry Netplay::Kaillera::Input::Handler::entries[] =
	{
		{ IDM_MACHINE_INPUT_PORT1_UNCONNECTED, Nes::Input::UNCONNECTED, &Input::OnPort1 },
		{ IDM_MACHINE_INPUT_PORT1_PAD1,        Nes::Input::PAD1,        &Input::OnPort1 },
		{ IDM_MACHINE_INPUT_PORT1_PAD2,        Nes::Input::PAD2,        &Input::OnPort1 },
		{ IDM_MACHINE_INPUT_PORT1_PAD3,        Nes::Input::PAD3,        &Input::OnPort1 },
		{ IDM_MACHINE_INPUT_PORT1_PAD4,        Nes::Input::PAD4,        &Input::OnPort1 },
		{ IDM_MACHINE_INPUT_PORT2_UNCONNECTED, Nes::Input::UNCONNECTED, &Input::OnPort2 },
		{ IDM_MACHINE_INPUT_PORT2_PAD1,        Nes::Input::PAD1,        &Input::OnPort2 },
		{ IDM_MACHINE_INPUT_PORT2_PAD2,        Nes::Input::PAD2,        &Input::OnPort2 },
		{ IDM_MACHINE_INPUT_PORT2_PAD3,        Nes::Input::PAD3,        &Input::OnPort2 },
		{ IDM_MACHINE_INPUT_PORT2_PAD4,        Nes::Input::PAD4,        &Input::OnPort2 },
		{ IDM_MACHINE_INPUT_PORT3_UNCONNECTED, Nes::Input::UNCONNECTED, &Input::OnPort3 },
		{ IDM_MACHINE_INPUT_PORT3_PAD1,        Nes::Input::PAD1,        &Input::OnPort3 },
		{ IDM_MACHINE_INPUT_PORT3_PAD2,        Nes::Input::PAD2,        &Input::OnPort3 },
		{ IDM_MACHINE_INPUT_PORT3_PAD3,        Nes::Input::PAD3,        &Input::OnPort3 },
		{ IDM_MACHINE_INPUT_PORT3_PAD4,        Nes::Input::PAD4,        &Input::OnPort3 },
		{ IDM_MACHINE_INPUT_PORT4_UNCONNECTED, Nes::Input::UNCONNECTED, &Input::OnPort4 },
		{ IDM_MACHINE_INPUT_PORT4_PAD1,        Nes::Input::PAD1,        &Input::OnPort4 },
		{ IDM_MACHINE_INPUT_PORT4_PAD2,        Nes::Input::PAD2,        &Input::OnPort4 },
		{ IDM_MACHINE_INPUT_PORT4_PAD3,        Nes::Input::PAD3,        &Input::OnPort4 },
		{ IDM_MACHINE_INPUT_PORT4_PAD4,        Nes::Input::PAD4,        &Input::OnPort4 }
	};

	void Netplay::Kaillera::Input::Capture()
	{
		padIndex = 0;

		instance->emulator.ConnectController( 0, Nes::Input::PAD1 );
		instance->emulator.ConnectController( 1, Nes::Input::PAD2 );
		instance->emulator.ConnectController( 2, Nes::Input::PAD3 );
		instance->emulator.ConnectController( 3, Nes::Input::PAD4 );
		instance->emulator.ConnectController( 4, Nes::Input::UNCONNECTED );

		for (uint i=0; i < NST_COUNT(Handler::entries); ++i)
		{
			instance->menu[Handler::entries[i].idm].Check( (i % 5) == 1 );
			menuCallbacks[i] = instance->menu.Commands()[Handler::entries[i].idm].Replace( this, Handler::entries[i].function );
		}

		Nes::Input::Controllers::Pad::callback.Get( pollCallback.code, pollCallback.data );
		Nes::Input::Controllers::Pad::callback.Set( NULL, NULL );
	}

	void Netplay::Kaillera::Input::Release() const
	{
		for (uint i=0; i < NST_COUNT(Handler::entries); ++i)
		{
			instance->menu[Handler::entries[i].idm].Check( instance->emulator.GetController(i % 5) == Handler::entries[i].type );
			instance->menu.Commands()[Handler::entries[i].idm] = menuCallbacks[i];
		}

		Nes::Input::Controllers::Pad::callback.Set( pollCallback.code, pollCallback.data );
	}

	void Netplay::Kaillera::Input::SelectPort(const uint offset,const uint idm)
	{
		instance->menu[offset + padIndex].Uncheck();
		instance->menu[idm].Check();
		padIndex = idm - offset;
	}

	void Netplay::Kaillera::Input::OnPort1(uint idm)
	{
		SelectPort( IDM_MACHINE_INPUT_PORT1_PAD1, idm );
	}

	void Netplay::Kaillera::Input::OnPort2(uint idm)
	{
		SelectPort( IDM_MACHINE_INPUT_PORT2_PAD1, idm );
	}

	void Netplay::Kaillera::Input::OnPort3(uint idm)
	{
		SelectPort( IDM_MACHINE_INPUT_PORT3_PAD1, idm );
	}

	void Netplay::Kaillera::Input::OnPort4(uint idm)
	{
		SelectPort( IDM_MACHINE_INPUT_PORT4_PAD1, idm );
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("t", on)
    #endif

	uint Netplay::Kaillera::Input::GetCode() const
	{
		Nes::Input::Controllers::Pad pad;
		const uint index = padIndex;

		if (index < 4)
			pollCallback.code( pollCallback.data, pad, index );

		return pad.buttons;
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("", on)
    #endif

	Netplay::Kaillera* Netplay::Kaillera::instance = NULL;
	Netplay::Kaillera::Client::Instance Netplay::Kaillera::Client::instance;

	Netplay::Kaillera::Kaillera
	(
     	Emulator& e,
		const Configuration& cfg,
		Window::Menu& m,
		const Paths& paths,
		Window::Custom& w
	)
	: 
	Dll        ( "kailleraclient.dll" ), 
	emulator   ( e ),
	menu       ( m ), 
	window     ( w )
	{
		NST_ASSERT( instance == NULL );

		Io::Log log;

		if (IsSupported())
		{
			instance = this;
			dialog = new Window::Netplay( cfg, emulator, paths );

			emulator.Events().Add( this, &Kaillera::OnEmuEvent );
			Application::Instance::Events::Add( this, &Kaillera::OnAppEvent );

			menu[IDM_NETPLAY_START].Enable();

			static const Window::Menu::CmdHandler::Entry<Kaillera> commands[] =
			{
				{ IDM_NETPLAY_START,      &Kaillera::OnMenuStart      },
				{ IDM_NETPLAY_DISCONNECT, &Kaillera::OnMenuDisconnect },
				{ IDM_NETPLAY_CHAT,       &Kaillera::OnMenuChat       }
			};

			menu.Commands().Add( this, commands );

			::kailleraInit();

			char version[16];
			version[0] = '\0';

			::kailleraGetVersion( version );

			if (*version)
			{
				log << "Kaillera: loaded \"kailleraclient.dll\" version " << cstring(version) << "\r\n";

				if (std::strcmp( version, "0.9" ))
					log << "Kaillera: warning, the loaded DLL file may be incompatible with Nestopia!\r\n";
			}
			else
			{
				log << "Kaillera: warning, DLL file version couldn't be detected!\r\n";
			}
		}
		else
		{
			log << "Kaillera: file \"kailleraclient.dll\" not found or initialization failed. "
			       "netplay will be disabled!\r\n";
		}
	}

	Netplay::Kaillera::~Kaillera()
	{
		if (IsSupported())
		{
			emulator.Events().Remove( this );
			Application::Instance::Events::Remove( this );
			delete dialog;
			::kailleraShutdown();
		}
	}

	void Netplay::Kaillera::Save(Configuration& cfg,const ibool saveGameList) const
	{
		dialog->Save( cfg, saveGameList );
	}

	void Netplay::Kaillera::Initialize()
	{
		static const Window::MsgHandler::Entry<Kaillera> messages[]	=
		{
			{ WM_NST_OPEN_CLIENT,  &Kaillera::OnOpenClient  },
			{ WM_NST_CLOSE_CLIENT, &Kaillera::OnCloseClient },
			{ WM_NST_START_GAME,   &Kaillera::OnStartGame   }
		};

		window.Messages().Add( this, messages );

		NST_ASSERT( dialog->GetNumGames() );

		Collection::Buffer strings;

		for (uint i=0, n=dialog->GetNumGames(); i < n; ++i)
		{
			const Window::Netplay::Game& game = dialog->GetGame( i );
			strings.Append( game, game.Size() );
			strings.PushBack( '\0' );
		}

		strings.PushBack( '\0' );

		String::Smart<16> appName;
		appName << "Nestopia " << Resource::Version();

		kailleraInfos info;

		info.appName               = appName;
		info.gameList              = strings;
		info.gameCallback          = Callbacks::Start;
		info.chatReceivedCallback  = Callbacks::ChatRecieve;
		info.clientDroppedCallback = Callbacks::ClientDrop;
		info.moreInfosCallback     = NULL;

		::kailleraSetInfos( &info );
	}

	void Netplay::Kaillera::StartNetwork(System::Thread::Interrupt interrupt)
	{
		if (interrupt.Demanding())
			interrupt.Acknowledge();

		Client().Run();
	}

	void Netplay::Kaillera::Start()
	{
		Initialize();
		thread.Create( this, &Kaillera::StartNetwork, window, System::Thread::START );
	}

	ibool Netplay::Kaillera::OnOpenClient(Window::Param&)
	{
		menu[IDM_NETPLAY_START].Disable();
		network.command.Capture();
		network.input.Capture();
		emulator.BeginNetplayMode();
		return TRUE;
	}

	ibool Netplay::Kaillera::OnCloseClient(Window::Param&)
	{
		menu[IDM_NETPLAY_START].Enable();
		network.command.Release();
		network.input.Release();
		window.Messages().Remove( this );		
		emulator.EndNetplayMode();
		return TRUE;
	}

	ibool Netplay::Kaillera::Close() const
	{
		if (Client::IsOpen())
		{
			if (!emulator.Unload())
				Client::Close();

			return TRUE;
		}

		return FALSE;
	}

	ibool Netplay::Kaillera::OnStartGame(Window::Param&)
	{
		network.connected = FALSE;

		emulator.EnableNetplay
		(
	     	this, 
			&Kaillera::OnEmuFrame, 
			network.player, 
			network.players 
		);

		Application::Instance::Launch
		( 
			dialog->GetPath(network.game), 
			Paths::File::GAME|Paths::File::ARCHIVE 
		);

		if (!emulator.Is(Nes::Machine::ON))
		{
			emulator.DisableNetplay();
			::kailleraEndGame();
		}

		return TRUE;
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
			uchar packets[MAX_PLAYERS][2];

			packets[0][0] = (u8) network.input.GetCode();
			packets[0][1] = (u8) network.command.GetCode();

			if (::kailleraModifyPlayValues( packets, 2 ) != -1)
			{
				const uint ports = NST_MIN( 4, network.players );

				for (uint i=0; i < ports; ++i)
				{
					controllers->pad[i].buttons = packets[i][0];
					network.command.Dispatch( packets[i][1], *controllers );
				}
			}
			else
			{
				network.connected = FALSE;
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

			return FALSE;
		}
		
		while (Application::Instance::NumChildWindows())
		{
			if (!::DestroyWindow( Application::Instance::GetChildWindow(0) ))
			{
				// some window didn't want to 
				// close itself, better abort..

				return FALSE;
			}
		}

		return TRUE;
	}

	void Netplay::Kaillera::OnMenuStart(uint)
	{
		if (ResetWindow() && dialog->Open())
			Start();
	}

	void Netplay::Kaillera::OnMenuChat(uint)
	{
		chat.Open();
	}

	void Netplay::Kaillera::OnMenuDisconnect(uint)
	{
		network.connected = FALSE;
		emulator.Unload();
	}

	ibool Netplay::Kaillera::OnEnable(Window::Param& param)
	{
		if (!param.wParam)
			window.Send( WM_ENABLE, TRUE, 0 );

		return TRUE;
	}

	void Netplay::Kaillera::OnAppEvent(Application::Instance::Event event,const void*)
	{
		if (thread.IsIdle())
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

	void Netplay::Kaillera::OnEmuEvent(Emulator::Event event)
	{
		static Window::MsgHandler::Callback old;

		switch (event)
		{
         	case Emulator::EVENT_NETPLAY_POWER_ON:

				if (emulator.Is( Nes::Machine::GAME ))
				{
					Client::Hide();
					network.connected = TRUE;

					menu[IDM_NETPLAY_DISCONNECT].Enable();
					menu[IDM_NETPLAY_CHAT].Enable();

					if (dialog->ShouldGoFullscreen())
						window.SendCommand( IDM_VIEW_SWITCH_SCREEN );

					network.command.Begin();
					enableCallback = window.Messages()[WM_ENABLE].Replace( this, &Kaillera::OnEnable );
				}
				break;

    		case Emulator::EVENT_NETPLAY_POWER_OFF:

				if (emulator.Is( Nes::Machine::GAME ))
				{
					menu[IDM_NETPLAY_DISCONNECT].Disable();
					menu[IDM_NETPLAY_CHAT].Disable();

					chat.Close();

					if (dialog->ShouldGoFullscreen())
						window.SendCommand( IDM_VIEW_SWITCH_SCREEN );

					network.command.End();
					window.Messages()[WM_ENABLE] = enableCallback;

					Client::Show();
					::kailleraEndGame();
				}
				break;

			case Emulator::EVENT_NETPLAY_UNLOAD:

				emulator.DisableNetplay();
				break;
		}
	}

	Netplay::Netplay
	(
       	Emulator& emulator,
		const Configuration& cfg,
		Window::Menu& menu,
		const Paths& paths,
		Window::Custom& window
	)
	: kaillera( new Kaillera(emulator,cfg,menu,paths,window) )
	{
		if (!kaillera->IsSupported())
		{
			delete kaillera;
			kaillera = NULL;
		}
	}

	Netplay::~Netplay()
	{
		delete kaillera;
	}

	void Netplay::Save(Configuration& cfg,const ibool saveGameList) const
	{
		if (kaillera)
			kaillera->Save( cfg, saveGameList );
	}

	ibool Netplay::Close() const
	{
		return kaillera ? kaillera->Close() : FALSE;
	}
}
