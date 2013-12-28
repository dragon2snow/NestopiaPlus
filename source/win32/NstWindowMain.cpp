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
#include "NstResourceString.hpp"
#include "NstResourceIcon.hpp"
#include "NstApplicationException.hpp"
#include "NstApplicationInstance.hpp"
#include "NstManagerEmulator.hpp"
#include "NstManagerPreferences.hpp"
#include "NstWindowUser.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowDialog.hpp"
#include "NstWindowMain.hpp"
#include "NstIoLog.hpp"
#include "../core/api/NstApiCartridge.hpp"

namespace Nestopia
{
	NST_COMPILE_ASSERT
	(
		IDM_VIEW_WINDOWSIZE_2X  - IDM_VIEW_WINDOWSIZE_1X == 1 &&
		IDM_VIEW_WINDOWSIZE_3X  - IDM_VIEW_WINDOWSIZE_1X == 2 &&
		IDM_VIEW_WINDOWSIZE_4X  - IDM_VIEW_WINDOWSIZE_1X == 3 &&
		IDM_VIEW_WINDOWSIZE_5X  - IDM_VIEW_WINDOWSIZE_1X == 4 &&
		IDM_VIEW_WINDOWSIZE_6X  - IDM_VIEW_WINDOWSIZE_1X == 5 &&
		IDM_VIEW_WINDOWSIZE_7X  - IDM_VIEW_WINDOWSIZE_1X == 6 &&
		IDM_VIEW_WINDOWSIZE_8X  - IDM_VIEW_WINDOWSIZE_1X == 7 &&
		IDM_VIEW_WINDOWSIZE_9X  - IDM_VIEW_WINDOWSIZE_1X == 8 &&
		IDM_VIEW_WINDOWSIZE_MAX - IDM_VIEW_WINDOWSIZE_1X == 9 
	);

	NST_COMPILE_ASSERT
	(
		IDS_MENU_2X - IDS_MENU_1X == 1 &&
		IDS_MENU_3X - IDS_MENU_1X == 2 &&
		IDS_MENU_4X - IDS_MENU_1X == 3 &&
		IDS_MENU_5X - IDS_MENU_1X == 4 &&
		IDS_MENU_6X - IDS_MENU_1X == 5 &&
		IDS_MENU_7X - IDS_MENU_1X == 6 &&
		IDS_MENU_8X - IDS_MENU_1X == 7 &&
		IDS_MENU_9X - IDS_MENU_1X == 8
	);

	using namespace Window;

	const char Main::windowName[] = "Nestopia";

	inline Main::State::State()
	: 
	menu      ( FALSE ),
	maximized ( FALSE )
	{}

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
	emulator    ( e ),
	menu        ( m )
	{
		static const MsgHandler::Entry<Main> messages[] =
		{
			{ WM_SYSCOMMAND,     &Main::OnSysCommand        },
			{ WM_ENTERSIZEMOVE,  &Main::OnEnterSizeMoveMenu },
			{ WM_ENTERMENULOOP,  &Main::OnEnterSizeMoveMenu },
			{ WM_EXITSIZEMOVE,   &Main::OnExitSizeMoveMenu  },
			{ WM_EXITMENULOOP,   &Main::OnExitSizeMoveMenu  },
			{ WM_ENABLE,		 &Main::OnEnable            },
			{ WM_ACTIVATE,		 &Main::OnActivate          },
			{ WM_NCLBUTTONDOWN,	 &Main::OnNclButton         },
			{ WM_DISPLAYCHANGE,  &Main::OnDisplayChange     },
			{ WM_POWERBROADCAST, &Main::OnPowerBroadCast    }
		};

		static const Menu::CmdHandler::Entry<Main> commands[] =
		{
			{ IDM_VIEW_WINDOWSIZE_1X,  &Main::OnCmdViewScreenSize   },
			{ IDM_VIEW_WINDOWSIZE_2X,  &Main::OnCmdViewScreenSize   },
			{ IDM_VIEW_WINDOWSIZE_3X,  &Main::OnCmdViewScreenSize   },
			{ IDM_VIEW_WINDOWSIZE_4X,  &Main::OnCmdViewScreenSize   },
			{ IDM_VIEW_WINDOWSIZE_5X,  &Main::OnCmdViewScreenSize   },
			{ IDM_VIEW_WINDOWSIZE_6X,  &Main::OnCmdViewScreenSize   },
			{ IDM_VIEW_WINDOWSIZE_7X,  &Main::OnCmdViewScreenSize   },
			{ IDM_VIEW_WINDOWSIZE_8X,  &Main::OnCmdViewScreenSize   },
			{ IDM_VIEW_WINDOWSIZE_9X,  &Main::OnCmdViewScreenSize   },
			{ IDM_VIEW_WINDOWSIZE_MAX, &Main::OnCmdViewScreenSize   },
			{ IDM_VIEW_SWITCH_SCREEN,  &Main::OnCmdViewSwitchScreen },
			{ IDM_VIEW_MENU,     	   &Main::OnCmdViewShowMenu     },
			{ IDM_VIEW_ON_TOP,		   &Main::OnCmdViewShowOnTop    }
		};

		static const Menu::PopupHandler::Entry<Main> popups[] =
		{	  
			{ Menu::PopupHandler::Pos<IDM_POS_VIEW,IDM_POS_VIEW_SCREENSIZE>::ID, &Main::OnMenuViewScreenSize }
		};

		m.Commands().Add( this, commands );
		m.PopupRouter().Add( this, popups );
		m.Hook( window );

		emulator.Events().Add( this, &Main::OnEmuEvent );
		emulator.Hook( this, &Main::OnStartEmulation, &Main::OnStopEmulation );

		{
			Dynamic::Context context;

			context.className   = Application::Instance::GetClassName();
			context.classStyle  = CLASS_STYLE;
			context.hBackground = (HBRUSH) ::GetStockObject( NULL_BRUSH );
			context.hIcon       = Resource::Icon( IDI_WINDOW );
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

			window.Create( context );
		}

		video.Construct( window, m, emulator, paths, cfg );
		sound.Construct( window, m, emulator, paths, cfg );

		{
			Managers::Input::Screening in( this, &Main::OnReturnInputScreen ); 
			Managers::Input::Screening out( this, &Main::OnReturnOutputScreen );
			input.Construct( window, m, emulator, cfg, in, out );
		}

		frameClock.Construct( m, emulator, cfg );

		Application::Instance::Events::Add( this, &Main::OnAppEvent );

		window.Messages().Add( this, messages );

		menu[IDM_VIEW_MENU].Check();
		menu[IDM_VIEW_SWITCH_SCREEN].Text() << Resource::String(IDS_MENU_FULLSCREEN);

		{
			const String::Heap& type = cfg["view size fullscreen"];

			video->SetFullscreenScale
			(
				( type == "1"         ) ? 0 :
     			( type == "2"         ) ? 1 :
     			( type == "3"         ) ? 2 :
				( type == "4"         ) ? 3 :
				( type == "5"         ) ? 4 :
				( type == "6"         ) ? 5 :
				( type == "7"         ) ? 6 :
				( type == "8"         ) ? 7 :
				( type == "9"         ) ? 8 :
    			( type == "stretched" ) ? Managers::Video::STRETCHED : 8
			);
		}

		{
			const String::Heap& type = cfg["view size window"];

			Resize
			(
				( type == "2" ) ? 1 :
     			( type == "3" ) ? 2 :
    			( type == "4" ) ? 3 :
				( type == "5" ) ? 4 :
				( type == "6" ) ? 5 :
				( type == "7" ) ? 6 :
				( type == "8" ) ? 7 :
				( type == "9" ) ? 8 : 0
			);
		}

		UpdateScreenSize( Managers::Video::GetDisplayMode() );

		if (preferences[Managers::Preferences::START_IN_FULLSCREEN])
			window.PostCommand( IDM_VIEW_SWITCH_SCREEN );
		else
			window.Show( cmdShow );
	}

	Main::~Main()
	{
		emulator.Unhook();
		emulator.Events().Remove( this );
		window.Messages().RemoveAll( this );
		Application::Instance::Events::Remove( this );
	}

	void Main::Save(Configuration& cfg) const
	{
		cfg[ "view show on top"      ].YesNo() = menu[IDM_VIEW_ON_TOP].IsChecked();
		cfg[ "view show window menu" ].YesNo() = IsWindowMenuEnabled();

		{
			String::Heap& value = cfg["view size window"].GetString();

			// small inaccuracy, status bar and menu row wrapping heights are not added

			Point baseSize( video->GetNesScreen() );
			baseSize += Point::NonClient( WIN_STYLE, WIN_EXSTYLE, IsWindowMenuEnabled() );

			switch (baseSize.ScaleToFit( video->IsFullscreen() ? state.rect : Rect::Window(window), Point::SCALE_NEAREST ))
			{
				case 1:  value = '2'; break;
				case 2:  value = '3'; break;
				case 3:  value = '4'; break;
				case 4:  value = '5'; break;
				case 5:  value = '6'; break;
				case 6:  value = '7'; break;
				case 7:  value = '8'; break;
				case 8:  value = '9'; break;
				default: value = '1'; break;
			}
		}

		{
			String::Heap& value = cfg["view size fullscreen"].GetString();

			switch (video->GetFullscreenScale())
			{								
				case 0:  value = '1';         break;
				case 1:  value = '2';         break;
				case 2:  value = '3';         break;
				case 3:  value = '4';         break;
				case 4:  value = '5';         break;
				case 5:  value = '6';         break;
				case 6:  value = '7';         break;
				case 7:  value = '8';         break;
				case 8:  value = '9';         break;
				default: value = "stretched"; break;
			}
		}

		video->Save( cfg );
		sound->Save( cfg );
		input->Save( cfg );
		frameClock->Save( cfg );
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
			
			if (!Window::Dialog::ProcessMessage( msg ) && !Window::Menu::TranslateAccelerator( msg ))
			{
				::TranslateMessage( &msg );
				::DispatchMessage( &msg );
			}

			if (emulator.IsIdle())
				continue;

			for (;;)
			{
				if (video->MustClearFrameScreen() && emulator.Is(Nes::Machine::GAME))
					video->ClearScreen();

				while (::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE )) 
				{ 
					if (!Window::Dialog::ProcessMessage( msg ) && !Window::Menu::TranslateAccelerator( msg ))
					{
						::TranslateMessage( &msg );
						::DispatchMessage( &msg );
					}

					if (msg.message == WM_QUIT)
						return msg.wParam;
				}

				if (emulator.IsRunning())
				{
					if (emulator.Is(Nes::Machine::GAME))
					{
						for (uint skips=frameClock->NumFrameSkips(); skips; --skips)
							emulator.Execute( NULL, sound->GetOutput(), input->GetOutput() );

						emulator.Execute( video->GetOutput(), sound->GetOutput(), input->GetOutput() );
						input->RefreshCursor();					
						frameClock->SynchronizeGame( !video->IsVSyncEnabled() || emulator.IsSpeedAlternating() );
						video->PresentScreen();
					}
					else
					{
						emulator.Execute( NULL, sound->GetOutput(), NULL );
						frameClock->SynchronizeSound(); 
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
		rect = video->GetNesScreen();
	}

	void Main::OnReturnOutputScreen(Rect& rect)
	{
		rect = video->GetScreenRect();
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("", on)
    #endif

	void Main::OnStopEmulation()
	{
		::SetThreadPriority( ::GetCurrentThread(), THREAD_PRIORITY_NORMAL );

		sound->StopEmulation();
		video->StopEmulation();
		input->StopEmulation();
		frameClock->StopEmulation();

		menu.ToggleModeless( FALSE );
	}

	ibool Main::OnStartEmulation()
	{
		if (window.IsEnabled() && (CanRunInBackground() || (window.IsForeground() && !window.Minimized()))) 
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
		
			frameClock->StartEmulation();
			video->StartEmulation();
			input->StartEmulation();
			sound->StartEmulation();

			return TRUE;
		}

		return FALSE;
	}

	void Main::UpdateScreenSize(const Point screen) const
	{
		for (uint i=0; i < 9; ++i)
			menu[IDM_VIEW_WINDOWSIZE_1X + i].Remove();

		const Point original( Managers::Video::NES_WIDTH, Managers::Video::NES_HEIGHT );
		Point nes( original );

		for (uint i=0; nes.x <= screen.x && nes.y <= screen.y && i < 9; nes = original * (i+2), ++i) 
			menu.Insert( menu[IDM_VIEW_WINDOWSIZE_MAX], IDM_VIEW_WINDOWSIZE_1X + i, Resource::String(IDS_MENU_1X + i) );
	}

	ibool Main::ToggleMenu()
	{
		ibool visible;

		if (IsFullscreen() || !window.Restored())
		{
			visible = menu.Toggle();
		}
		else
		{
			Point size;

			if (menu.IsVisible())
			{
				size.y = menu.GetHeight();
				menu.Hide();
				window.Shrink( size );
				visible = FALSE;
			}
			else
			{
				menu.Show();
				size.y = menu.GetHeight();
				window.Magnify( size );
				visible = TRUE;
			}
		}

		return visible;
	}

	ibool Main::IsWindowMenuEnabled() const
	{
		return IsWindowed() ? menu.IsVisible() : state.menu;
	}

	ibool Main::CanRunInBackground()
	{
		if (emulator.Is(Nes::Machine::SOUND))
			return menu[IDM_MACHINE_NSF_OPTIONS_PLAYINBACKGROUND].IsChecked();

		return preferences[Managers::Preferences::RUN_IN_BACKGROUND];
	}

	ibool Main::IsScreenMatched(const Nes::Machine::Mode mode) const
	{
		if (IsFullscreen())
			return video->GetFullscreenScale() != Managers::Video::STRETCHED;

		const Rect::Picture output( window );
		const Rect& input = video->GetNesScreen( mode );

		return (output.Width() % input.Width()) == 0 && (output.Height() % input.Height()) == 0;
	}

	uint Main::CalculateScreenScale() const
	{
		NST_COMPILE_ASSERT( MAXIMIZE == Managers::Video::STRETCHED );

		if (IsFullscreen())
			return video->GetFullscreenScale();

		if (window.Maximized())
			return MAXIMIZE;

		return static_cast<Point>(video->GetNesScreen().Size()).ScaleToFit
		( 
     		Rect::Picture( window ), 
			Point::SCALE_NEAREST 
		);
	}

	void Main::Resize(const uint scale)
	{
		NST_COMPILE_ASSERT( MAXIMIZE == Managers::Video::STRETCHED );

		if (IsFullscreen())
		{
			video->SetFullscreenScale( scale );
		}
		else if (scale == MAXIMIZE)
		{
			window.Maximize();
		}
		else
		{
			if (!window.Restored())
				window.Restore();

			Point winSize( video->GetNesScreen() );
			winSize.ScaleToFit( Managers::Video::GetDisplayMode(), Point::SCALE_BELOW, scale );

			const ibool menuSet = menu.IsVisible();

			winSize += Point::NonClient( WIN_STYLE, WIN_EXSTYLE, menuSet );
			winSize.y += Rect::Client( window ).bottom - Rect::Picture( window ).bottom;
  
			window.Resize( winSize );

			if (menuSet)
			{
				const int extraHeight = menu.GetHeight() - menu.GetStandardHeight();

				if (extraHeight > 0)
				{
					winSize.y += extraHeight;
					window.Resize( winSize );
				}
			}
		}
	}

	ibool Main::OnSysCommand(Param& param) 
	{
		switch (param.wParam & 0xFFF0)
		{
			case SC_MOVE:
			case SC_SIZE:
		
				return IsFullscreen();
		
			case SC_MONITORPOWER:
			case SC_SCREENSAVE:
		
				return IsFullscreen() || emulator.Is(Nes::Machine::ON);
		
			case SC_MAXIMIZE:
		
				if (IsFullscreen())
					return TRUE;
		
			case SC_MINIMIZE:
			case SC_RESTORE:
		
				if (IsWindowed())
					emulator.Wait();
		
				break;
		}

		return FALSE;
	}

	ibool Main::OnEnterSizeMoveMenu(Param&) 
	{
		if (!CanRunInBackground())
			emulator.Stop();

		return TRUE;
	}

	ibool Main::OnExitSizeMoveMenu(Param&) 
	{
		if (!CanRunInBackground())
			emulator.Resume();

		return TRUE;
	}

	ibool Main::OnEnable(Param& param)
	{
		if (param.wParam)
			emulator.Resume();
		else
			emulator.Stop();

		return TRUE;
	}

	ibool Main::OnActivate(Param& param) 
	{
		if (param.Activator().IsEntering())
		{
			if (IsFullscreen() && param.Activator().Minimized())
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
			else if (IsFullscreen() && param.Activator().IsOutsideApplication())
			{	
				emulator.Wait(); 
			}
		}

		return FALSE;
	}

	ibool Main::OnNclButton(Param&)
	{
		emulator.Wait();
		return FALSE;
	}

	ibool Main::OnDisplayChange(Param& param)
	{
		UpdateScreenSize( Point( LOWORD(param.lParam), HIWORD(param.lParam) ) );
		return TRUE;
	}

	ibool Main::OnPowerBroadCast(Param& param) 
	{
		switch (param.wParam)
		{
			case PBT_APMQUERYSUSPEND:  
		
				emulator.Stop();
				return TRUE;
		
			case PBT_APMRESUMESUSPEND: 
		
				emulator.Resume();
				return TRUE;
		}

		return FALSE;
	}

	void Main::OnCmdViewSwitchScreen(uint) 
	{ 
		Application::Instance::Waiter wait;

		emulator.Stop();

		Application::Instance::Events::Signal
		( 
	     	IsWindowed() ? Application::Instance::EVENT_FULLSCREEN :
                       	   Application::Instance::EVENT_DESKTOP
		);

		if (IsWindowed())
		{
			state.menu = menu.IsVisible();
			state.maximized = window.Maximized();
			state.rect = window.GetNormalWindowRect();

			menu.Hide();

			menu[ IDM_VIEW_ON_TOP ].Disable();
			menu[ IDM_VIEW_SWITCH_SCREEN ].Text() << Resource::String(IDS_MENU_DESKTOP);

			window.SetStyle( FULLSCREEN_STYLE, FULLSCREEN_EXSTYLE );
			window.Set( Managers::Video::GetDisplayMode(), SWP_FRAMECHANGED );

			video->SwitchScreen();

			if (!emulator.Is(Nes::Machine::ON))
				menu.Show();
		}
		else
		{
			menu.Hide();

			video->SwitchScreen();
			window.SetStyle( WIN_STYLE, WIN_EXSTYLE );

			menu[ IDM_VIEW_ON_TOP ].Enable();
			menu[ IDM_VIEW_SWITCH_SCREEN ].Text() << Resource::String(IDS_MENU_FULLSCREEN);

			HWND const zOrder =	menu[IDM_VIEW_ON_TOP].IsChecked() ? HWND_TOPMOST : HWND_NOTOPMOST;

			if (state.menu)
				menu.Show();

			if (state.maximized)
			{
				window.Maximize();
				window.SetNormalWindowRect( state.rect );
				window.Reorder( zOrder, SWP_FRAMECHANGED|SWP_SHOWWINDOW );
			}
			else
			{
				window.Set( state.rect, zOrder, SWP_FRAMECHANGED|SWP_SHOWWINDOW );
			}

			Application::Instance::ShowChildWindows();

			::InvalidateRect( NULL, NULL, FALSE );
		}

		::Sleep( FULLSCREEN_RECOVER_TIME );
		
		emulator.Resume();
	}

	void Main::OnCmdViewShowOnTop(uint) 
	{ 
		NST_ASSERT( IsWindowed() );

		const ibool previous = menu[IDM_VIEW_ON_TOP].ToggleCheck();
		window.Reorder( previous ? HWND_NOTOPMOST : HWND_TOPMOST );
	}

	void Main::OnCmdViewScreenSize(uint cmd)
	{
		Resize( cmd == IDM_VIEW_WINDOWSIZE_MAX ? MAXIMIZE : cmd - IDM_VIEW_WINDOWSIZE_1X );
	}

	void Main::OnCmdViewShowMenu(uint) 
	{
		const ibool visible = ToggleMenu();

		if (IsFullscreen())
			Application::Instance::ShowChildWindows( visible );
	}

	void Main::OnMenuViewScreenSize(Menu::PopupHandler::Param& param)
	{
		uint scale;

		if (IsFullscreen())
		{
			scale = video->GetFullscreenScale();
		}
		else if (window.Maximized())
		{
			scale = MAXIMIZE;
		}
		else if (IsScreenMatched( Nes::Machine(emulator).GetMode() ))
		{
			scale = CalculateScreenScale();
		}
		else
		{
			scale = 0xBEDBABE;
		}

		param.menu[ IDM_VIEW_WINDOWSIZE_1X  ].Check( scale == 0        );
		param.menu[ IDM_VIEW_WINDOWSIZE_2X  ].Check( scale == 1        );
		param.menu[ IDM_VIEW_WINDOWSIZE_3X  ].Check( scale == 2        );
		param.menu[ IDM_VIEW_WINDOWSIZE_4X  ].Check( scale == 3        );
		param.menu[ IDM_VIEW_WINDOWSIZE_5X  ].Check( scale == 4        );
		param.menu[ IDM_VIEW_WINDOWSIZE_6X  ].Check( scale == 5        );
		param.menu[ IDM_VIEW_WINDOWSIZE_7X  ].Check( scale == 6        );
		param.menu[ IDM_VIEW_WINDOWSIZE_8X  ].Check( scale == 7        );
		param.menu[ IDM_VIEW_WINDOWSIZE_9X  ].Check( scale == 8        );
		param.menu[ IDM_VIEW_WINDOWSIZE_MAX ].Check( scale == MAXIMIZE );
	}

	void Main::OnEmuEvent(Managers::Emulator::Event event)
	{
		switch (event)
		{
			case Managers::Emulator::EVENT_MODE_NTSC:
			case Managers::Emulator::EVENT_MODE_PAL:
			
				if (IsWindowed() && IsScreenMatched( event == Managers::Emulator::EVENT_MODE_NTSC ? Nes::Machine::PAL : Nes::Machine::NTSC ))
					Resize( CalculateScreenScale() );
	
				break;

			case Managers::Emulator::EVENT_POWER_ON:
			case Managers::Emulator::EVENT_POWER_OFF:

				if (IsFullscreen())
				{
					const ibool show = (event == Managers::Emulator::EVENT_POWER_OFF);
					menu.Show( show );
					Application::Instance::ShowChildWindows( show );
				}
				break;

	     	case Managers::Emulator::EVENT_LOAD:
			case Managers::Emulator::EVENT_NETPLAY_LOAD:
			{
				String::Smart<512> name;

				switch (emulator.Is(Nes::Machine::CARTRIDGE|Nes::Machine::SOUND))
				{
					case Nes::Machine::CARTRIDGE: 
			
						name = Nes::Cartridge(emulator).GetInfo()->name.c_str(); 
						break;
			
					case Nes::Machine::SOUND:
			
						name = Nes::Nsf(emulator).GetName(); 
						break;
				}

				if (name.Empty())
					name = emulator.GetImagePath().Target().File();

				if (name.Size())
					window.Text() << (name << " - " << windowName);

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
