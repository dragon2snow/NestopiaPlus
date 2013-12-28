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

#include "resource/resource.h"
#include "NstIoNsp.hpp"
#include "NstResourceString.hpp"
#include "NstResourceIcon.hpp"
#include "NstApplicationInstance.hpp"
#include "NstManagerEmulator.hpp"
#include "NstManagerPreferences.hpp"
#include "NstWindowUser.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowDialog.hpp"
#include "NstWindowMain.hpp"
#include "../core/api/NstApiCartridge.hpp"
#include <Pbt.h>

namespace Nestopia
{
	namespace Window
	{
		const tchar Main::windowName[] = _T("Nestopia");

		inline Main::State::State()
		:
		menu      ( false ),
		maximized ( false )
		{}

		Main::MainWindow::MainWindow(const Configuration& cfg,const Menu& menu)
		{
			Context context;

			context.className   = Application::Instance::GetClassName();
			context.classStyle  = CLASS_STYLE;
			context.hBackground = (HBRUSH) ::GetStockObject( NULL_BRUSH );
			context.hIcon       = Resource::Icon( Application::Instance::GetIconStyle() == Application::Instance::ICONSTYLE_NES ? IDI_PAD : IDI_PAD_J );
			context.windowName  = windowName;
			context.winStyle    = WIN_STYLE;
			context.exStyle     = WIN_EXSTYLE;

			if (cfg["view show on top"] == Configuration::YES)
			{
				context.exStyle |= WS_EX_TOPMOST;
				menu[IDM_VIEW_ON_TOP].Check();
			}

			if (cfg["view show window menu"] != Configuration::NO)
				context.hMenu = menu.GetHandle();

			Create( context );
		}

		Main::Main
		(
			Managers::Emulator& e,
			const Configuration& cfg,
			Menu& m,
			const Managers::Paths& paths,
			const Managers::Preferences& p,
			const int cmdShow
		)
		:
		preferences ( p ),
		window      ( cfg, m ),
		menu        ( m ),
		emulator    ( e ),
		video       ( window, m, e, paths, cfg ),
		sound       ( window, m, e, paths, p, cfg ),
		input       ( window, m, e, cfg, Managers::Input::Screening(this,&Main::OnReturnInputScreen), Managers::Input::Screening(this,&Main::OnReturnOutputScreen) ),
		frameClock  ( m, e, cfg )
		{
			static const MsgHandler::Entry<Main> messages[] =
			{
				{ WM_SYSKEYDOWN,                                &Main::OnSysKeyDown        },
				{ WM_SYSCOMMAND,                                &Main::OnSysCommand        },
				{ WM_ENTERSIZEMOVE,                             &Main::OnEnterSizeMoveMenu },
				{ WM_ENTERMENULOOP,                             &Main::OnEnterSizeMoveMenu },
				{ WM_EXITSIZEMOVE,                              &Main::OnExitSizeMoveMenu  },
				{ WM_EXITMENULOOP,                              &Main::OnExitSizeMoveMenu  },
				{ WM_ENABLE,                                    &Main::OnEnable            },
				{ WM_ACTIVATE,                                  &Main::OnActivate          },
				{ WM_NCLBUTTONDOWN,                             &Main::OnNclButton         },
				{ WM_POWERBROADCAST,                            &Main::OnPowerBroadCast    },
				{ Application::Instance::WM_NST_COMMAND_RESUME, &Main::OnCommandResume     }
			};

			static const Menu::CmdHandler::Entry<Main> commands[] =
			{
				{ IDM_VIEW_SWITCH_SCREEN, &Main::OnCmdViewSwitchScreen },
				{ IDM_VIEW_MENU,          &Main::OnCmdViewShowMenu     },
				{ IDM_VIEW_ON_TOP,        &Main::OnCmdViewShowOnTop    }
			};

			m.Commands().Add( this, commands );
			m.Hook( window );

			emulator.Events().Add( this, &Main::OnEmuEvent );
			emulator.Hook( this, &Main::OnStartEmulation, &Main::OnStopEmulation );

			Application::Instance::Events::Add( this, &Main::OnAppEvent );

			window.Messages().Add( this, messages );

			menu[IDM_VIEW_MENU].Check();
			menu[IDM_VIEW_SWITCH_SCREEN].Text() << Resource::String(IDS_MENU_FULLSCREEN);

			if (preferences[Managers::Preferences::SAVE_WINDOWPOS])
			{
				const Rect rect
				(
					cfg[ "view window left"   ],
					cfg[ "view window top"    ],
					cfg[ "view window right"  ],
					cfg[ "view window bottom" ]
				);

				const Point mode( video.GetDisplayMode() );

				const bool winpos =
				(
					rect.left < rect.right && rect.top < rect.bottom &&
					rect.left < mode.x && rect.top < mode.y &&
					rect.Width() < mode.x * 2 && rect.Height() < mode.y * 2
				);

				if (winpos)
					window.SetPlacement( rect );
			}

			if (preferences[Managers::Preferences::START_IN_FULLSCREEN])
				window.PostCommand( IDM_VIEW_SWITCH_SCREEN );
			else
				::ShowWindow( window, cmdShow );
		}

		Main::~Main()
		{
			emulator.Unhook();
			emulator.Events().Remove( this );
			window.Messages().RemoveAll( this );
			Application::Instance::Events::Remove( this );
		}

		inline ibool Main::Fullscreen() const
		{
			return video.Fullscreen();
		}

		inline ibool Main::Windowed() const
		{
			return video.Windowed();
		}

		void Main::Save(Configuration& cfg) const
		{
			cfg[ "view show on top"      ].YesNo() = menu[IDM_VIEW_ON_TOP].Checked();
			cfg[ "view show window menu" ].YesNo() = (Windowed() ? menu.Visible() : state.menu);

			Rect rect( video.Fullscreen() ? state.rect : window.GetPlacement() );
			rect.Position() += Point(rect.left < 0 ? -rect.left : 0, rect.top < 0 ? -rect.top : 0 );

			if (preferences[Managers::Preferences::SAVE_WINDOWPOS])
			{
				cfg[ "view window left"   ] = rect.left;
				cfg[ "view window top"    ] = rect.top;
				cfg[ "view window right"  ] = rect.right;
				cfg[ "view window bottom" ] = rect.bottom;
			}

			video.Save( cfg, rect );
			sound.Save( cfg );
			input.Save( cfg );
			frameClock.Save( cfg );
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		int Main::Run()
		{
			MSG msg;

			while (const BOOL ret = ::GetMessage( &msg, NULL, 0, 0 ))
			{
				if (ret == -1)
					return EXIT_FAILURE;

				if (!Dialog::ProcessMessage( msg ) && !Menu::TransAccelerator( msg ))
				{
					::TranslateMessage( &msg );
					::DispatchMessage( &msg );
				}

				if (emulator.Idle())
					continue;

				for (;;)
				{
					if (video.MustClearFrameScreen() && emulator.Is(Nes::Machine::GAME))
						video.ClearScreen();

					while (::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ))
					{
						if (!Dialog::ProcessMessage( msg ) && !Menu::TransAccelerator( msg ))
						{
							::TranslateMessage( &msg );
							::DispatchMessage( &msg );
						}

						if (msg.message == WM_QUIT)
							return msg.wParam;
					}

					if (emulator.Running())
					{
						if (emulator.Is(Nes::Machine::GAME))
						{
							for (uint skips=frameClock.GameSynchronize( video.ThrottleRequired(frameClock.GetRefreshRate()) ); skips; --skips)
								emulator.Execute( NULL, sound.GetOutput(), input.GetOutput() );

							emulator.Execute( video.GetOutput(), sound.GetOutput(), input.GetOutput() );
							frameClock.GameHalt();
							video.PresentScreen( frameClock.UsesAutoFrameSkip() );
							frameClock.GameResume();
							input.Poll();
						}
						else
						{
							NST_ASSERT( emulator.Is(Nes::Machine::SOUND) );
							emulator.Execute( NULL, sound.GetOutput(), NULL );
							frameClock.SoundSynchronize();
						}
					}
					else
					{
						break;
					}
				}
			}

			return msg.wParam;
		}

		void Main::OnReturnInputScreen(Rect& rect)
		{
			rect = video.GetInputRect();
		}

		void Main::OnReturnOutputScreen(Rect& rect)
		{
			rect = video.GetScreenRect();
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		uint Main::GetMaxMessageLength() const
		{
			return video.GetMaxMessageLength();
		}

		void Main::OnStopEmulation()
		{
			::SetThreadPriority( ::GetCurrentThread(), THREAD_PRIORITY_NORMAL );

			sound.StopEmulation();
			video.StopEmulation();
			input.StopEmulation();
			frameClock.StopEmulation();

			menu.ToggleModeless( false );
		}

		ibool Main::OnStartEmulation()
		{
			if (window.Enabled() && (CanRunInBackground() || (window.Active() && !window.Minimized() && (Windowed() || !menu.Visible()))))
			{
				menu.ToggleModeless( CanRunInBackground() );

				int priority;

				switch (preferences.GetPriority())
				{
					case Managers::Preferences::PRIORITY_HIGH:

						if (emulator.Is(Nes::Machine::GAME) && !CanRunInBackground())
						{
							priority = THREAD_PRIORITY_HIGHEST;
							break;
						}

					case Managers::Preferences::PRIORITY_ABOVE_NORMAL:

						priority = THREAD_PRIORITY_ABOVE_NORMAL;
						break;

					default:

						priority = THREAD_PRIORITY_NORMAL;
						break;
				}

				::SetThreadPriority( ::GetCurrentThread(), priority );

				frameClock.StartEmulation();
				video.StartEmulation();
				input.StartEmulation();
				sound.StartEmulation();

				return true;
			}

			return false;
		}

		void Main::Load(const Io::Nsp::Context& context)
		{
			video.LoadPalette( context.palette );
		}

		void Main::Save(Io::Nsp::Context& context) const
		{
			video.SavePalette( context.palette );
		}

		ibool Main::ToggleMenu()
		{
			ibool visible = menu.Visible();

			if (Fullscreen() || !window.Restored())
			{
				if (Fullscreen() && !visible && !CanRunInBackground())
					emulator.Stop();

				visible = menu.Toggle();

				if (Fullscreen() && !visible)
					emulator.Resume();
			}
			else
			{
				Point size;

				if (visible)
				{
					size.y = menu.Height();
					menu.Hide();
					window.Size() -= size;
					visible = false;
				}
				else
				{
					menu.Show();
					size.y = menu.Height();
					window.Size() += size;
					visible = true;
				}
			}

			return visible;
		}

		ibool Main::CanRunInBackground()
		{
			if (emulator.Is(Nes::Machine::SOUND))
				return menu[IDM_MACHINE_NSF_OPTIONS_PLAYINBACKGROUND].Checked();

			return preferences[Managers::Preferences::RUN_IN_BACKGROUND];
		}

		ibool Main::OnSysKeyDown(Param& param)
		{
			return (param.wParam == VK_MENU && !menu.Visible());
		}

		ibool Main::OnSysCommand(Param& param)
		{
			switch (param.wParam & 0xFFF0)
			{
				case SC_MOVE:
				case SC_SIZE:

					return Fullscreen();

				case SC_MONITORPOWER:
				case SC_SCREENSAVE:

					return Fullscreen() || emulator.Is(Nes::Machine::ON);

				case SC_MAXIMIZE:

					if (Fullscreen())
						return true;

				case SC_MINIMIZE:
				case SC_RESTORE:

					if (Windowed())
						emulator.Wait();

					break;
			}

			return false;
		}

		ibool Main::OnEnterSizeMoveMenu(Param&)
		{
			if (!CanRunInBackground())
				emulator.Stop();

			return true;
		}

		ibool Main::OnExitSizeMoveMenu(Param&)
		{
			if (!CanRunInBackground())
				emulator.Resume();

			return true;
		}

		ibool Main::OnEnable(Param& param)
		{
			if (param.wParam)
				emulator.Resume();
			else
				emulator.Stop();

			return true;
		}

		ibool Main::OnActivate(Param& param)
		{
			if (param.Activator().Entering())
			{
				if (Fullscreen() && param.Activator().Minimized())
				{
					emulator.Stop();
				}
				else
				{
					emulator.Resume();
				}
			}
			else
			{
				if (!CanRunInBackground())
				{
					emulator.Stop();
				}
				else if (Fullscreen() && param.Activator().OutsideApplication())
				{
					emulator.Wait();
				}
			}

			return false;
		}

		ibool Main::OnNclButton(Param&)
		{
			emulator.Wait();
			return false;
		}

		ibool Main::OnPowerBroadCast(Param& param)
		{
			switch (param.wParam)
			{
				case PBT_APMQUERYSUSPEND:

					emulator.Stop();
					return true;

				case PBT_APMRESUMESUSPEND:

					emulator.Resume();
					return true;
			}

			return false;
		}

		ibool Main::OnCommandResume(Param& param)
		{
			if (HIWORD(param.wParam) == 0 && Fullscreen() && emulator.Is(Nes::Machine::ON) && menu.Visible())
				window.PostCommand( IDM_VIEW_MENU ); // Hide menu and resume emulation

			return true;
		}

		void Main::OnCmdViewSwitchScreen(uint)
		{
			Application::Instance::Waiter wait;

			emulator.Stop();

			if (Windowed())
			{
				state.menu = menu.Visible();
				state.maximized = window.Maximized();
				state.rect = window.GetPlacement();

				menu.Hide();

				menu[ IDM_VIEW_ON_TOP ].Disable();
				menu[ IDM_VIEW_SWITCH_SCREEN ].Text() << Resource::String(IDS_MENU_DESKTOP);

				window.MakeTopMost( false );
				window.MakeFullscreen();

				Application::Instance::Events::Signal( Application::Instance::EVENT_FULLSCREEN );

				video.SwitchScreen();

				if (!emulator.Is(Nes::Machine::ON))
					menu.Show();
			}
			else
			{
				Application::Instance::Events::Signal( Application::Instance::EVENT_DESKTOP );

				menu.Hide();

				video.SwitchScreen();

				window.Show( false );
				window.MakeTopMost( menu[IDM_VIEW_ON_TOP].Checked() );
				window.MakeWindowed( WIN_STYLE, WIN_EXSTYLE );

				menu[ IDM_VIEW_ON_TOP ].Enable();
				menu[ IDM_VIEW_SWITCH_SCREEN ].Text() << Resource::String(IDS_MENU_FULLSCREEN);

				if (state.menu)
					menu.Show();

				if (state.maximized)
					window.Maximize();

				window.SetPlacement( state.rect );
				window.Show( true );

				Application::Instance::ShowChildWindows();
			}

			::Sleep( 500 );

			emulator.Resume();
		}

		void Main::OnCmdViewShowOnTop(uint)
		{
			NST_ASSERT( Windowed() );

			window.MakeTopMost( menu[IDM_VIEW_ON_TOP].ToggleCheck() );
		}

		void Main::OnCmdViewShowMenu(uint)
		{
			const ibool visible = ToggleMenu();

			if (Fullscreen())
				Application::Instance::ShowChildWindows( visible );
		}

		void Main::OnEmuEvent(Managers::Emulator::Event event)
		{
			switch (event)
			{
				case Managers::Emulator::EVENT_POWER_ON:
				case Managers::Emulator::EVENT_POWER_OFF:

					if (Fullscreen())
					{
						const ibool show = (event == Managers::Emulator::EVENT_POWER_OFF);
						menu.Show( show );
						Application::Instance::ShowChildWindows( show );
					}
					break;

				case Managers::Emulator::EVENT_LOAD:
				case Managers::Emulator::EVENT_NETPLAY_LOAD:
				{
					Path name;

					switch (emulator.Is(Nes::Machine::CARTRIDGE|Nes::Machine::SOUND))
					{
						case Nes::Machine::CARTRIDGE:

							name.Import( Nes::Cartridge(emulator).GetInfo()->name.c_str() );
							break;

						case Nes::Machine::SOUND:

							name.Import( Nes::Nsf(emulator).GetName() );
							break;
					}

					if (name.Empty())
					{
						name = emulator.GetImagePath().Target().File();
						name.Extension().Clear();
					}

					if (name.Length())
						window.Text() << (name << " - " << windowName).Ptr();

					break;
				}

				case Managers::Emulator::EVENT_UNLOAD:
				case Managers::Emulator::EVENT_NETPLAY_UNLOAD:

					window.Text() << windowName;
					break;

				case Managers::Emulator::EVENT_NETPLAY_MODE_ON:
				case Managers::Emulator::EVENT_NETPLAY_MODE_OFF:

					menu[IDM_VIEW_SWITCH_SCREEN].Enable( event == Managers::Emulator::EVENT_NETPLAY_MODE_OFF );
					break;
			}
		}

		void Main::OnAppEvent(Application::Instance::Event event,const void*)
		{
			if (event == Application::Instance::EVENT_SYSTEM_BUSY)
				emulator.Wait();
		}
	}
}
