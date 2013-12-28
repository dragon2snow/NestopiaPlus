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

#include "NstResourceCursor.hpp"
#include "NstObjectHeap.hpp"
#include "NstIoLog.hpp"
#include "NstWindowMenu.hpp"
#include "NstWindowCustom.hpp"
#include "NstWindowParam.hpp"
#include "NstManagerEmulator.hpp"
#include "NstDialogInput.hpp"
#include "NstManagerInput.hpp"

namespace Nestopia
{
	NST_COMPILE_ASSERT
	(
		( IDM_MACHINE_INPUT_PORT1_PAD1		            - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 1  &&  
		( IDM_MACHINE_INPUT_PORT1_PAD2		            - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 2  && 
		( IDM_MACHINE_INPUT_PORT1_PAD3		            - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 3  && 
		( IDM_MACHINE_INPUT_PORT1_PAD4	                - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 4  && 
		( IDM_MACHINE_INPUT_PORT1_ZAPPER	            - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 5  && 
		( IDM_MACHINE_INPUT_PORT1_PADDLE	            - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 6  && 
		( IDM_MACHINE_INPUT_PORT1_POWERPAD              - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 7  && 
		( IDM_MACHINE_INPUT_PORT2_UNCONNECTED           - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 8  && 
		( IDM_MACHINE_INPUT_PORT2_PAD1		            - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 9  &&
		( IDM_MACHINE_INPUT_PORT2_PAD2		            - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 10 && 
		( IDM_MACHINE_INPUT_PORT2_PAD3	  	            - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 11 &&  
		( IDM_MACHINE_INPUT_PORT2_PAD4	                - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 12 && 
		( IDM_MACHINE_INPUT_PORT2_ZAPPER	            - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 13 && 
		( IDM_MACHINE_INPUT_PORT2_PADDLE	            - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 14 && 
		( IDM_MACHINE_INPUT_PORT2_POWERPAD              - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 15 && 
		( IDM_MACHINE_INPUT_PORT3_UNCONNECTED           - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 16 && 
		( IDM_MACHINE_INPUT_PORT3_PAD1		            - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 17 && 
		( IDM_MACHINE_INPUT_PORT3_PAD2		            - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 18 && 
		( IDM_MACHINE_INPUT_PORT3_PAD3		            - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 19 &&
		( IDM_MACHINE_INPUT_PORT3_PAD4	                - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 20 && 
		( IDM_MACHINE_INPUT_PORT4_UNCONNECTED           - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 21 && 
		( IDM_MACHINE_INPUT_PORT4_PAD1		            - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 22 && 
		( IDM_MACHINE_INPUT_PORT4_PAD2		            - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 23 && 
		( IDM_MACHINE_INPUT_PORT4_PAD3		            - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 24 && 
		( IDM_MACHINE_INPUT_PORT4_PAD4	                - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 25 && 
		( IDM_MACHINE_INPUT_SPECIAL_UNCONNECTED         - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 26 &&
		( IDM_MACHINE_INPUT_SPECIAL_FAMILYBASICKEYBOARD - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 27 &&
		( IDM_MACHINE_INPUT_SPECIAL_PADDLE              - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 28 &&
		( IDM_MACHINE_INPUT_SPECIAL_OEKAKIDSTABLET      - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 29 &&
		( IDM_MACHINE_INPUT_SPECIAL_HYPERSHOT           - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 30 &&
		( IDM_MACHINE_INPUT_SPECIAL_CRAZYCLIMBER        - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 31 &&
		( IDM_MACHINE_INPUT_SPECIAL_MAHJONG             - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 32 &&
		( IDM_MACHINE_INPUT_SPECIAL_EXCITINGBOXING      - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 33 &&
		( IDM_MACHINE_INPUT_SPECIAL_TOPRIDER            - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 34 &&
		( IDM_MACHINE_INPUT_SPECIAL_POKKUNMOGURAA       - IDM_MACHINE_INPUT_PORT1_UNCONNECTED ) == 35 
	);

	NST_COMPILE_ASSERT
	(
	    IDM_POS_MACHINE_INPUT_PORT2   == IDM_POS_MACHINE_INPUT_PORT1 + 1 &&
		IDM_POS_MACHINE_INPUT_PORT3   == IDM_POS_MACHINE_INPUT_PORT1 + 2 &&
		IDM_POS_MACHINE_INPUT_PORT4   == IDM_POS_MACHINE_INPUT_PORT1 + 3 &&
		IDM_POS_MACHINE_INPUT_SPECIAL == IDM_POS_MACHINE_INPUT_PORT1 + 4
	);

	using namespace Managers;

	struct Input::Callbacks
	{
		typedef Nes::Input::UserData UserData;
		typedef Nes::Input::Controllers Controllers;
		typedef Window::Input::Settings Settings;
		typedef Settings::Key Key;

		static bool NST_CALLBACK PollPad            (UserData,Controllers::Pad&,uint);
		static bool NST_CALLBACK PollZapper         (UserData,Controllers::Zapper&);
		static bool NST_CALLBACK PollPaddle         (UserData,Controllers::Paddle&);
		static bool NST_CALLBACK PollPowerPad       (UserData,Controllers::PowerPad&);
		static bool NST_CALLBACK PollOekaKidsTablet (UserData,Controllers::OekaKidsTablet&);
		static bool NST_CALLBACK PollHyperShot      (UserData,Controllers::HyperShot&);
		static bool NST_CALLBACK PollCrazyClimber   (UserData,Controllers::CrazyClimber&);
		static bool NST_CALLBACK PollKeyboard       (UserData,Controllers::Keyboard&,uint,uint);
		static bool NST_CALLBACK PollMahjong        (UserData,Controllers::Mahjong&,uint);
		static bool NST_CALLBACK PollExcitingBoxing (UserData,Controllers::ExcitingBoxing&,uint);
		static bool NST_CALLBACK PollTopRider       (UserData,Controllers::TopRider&);
		static bool NST_CALLBACK PollPokkunMoguraa  (UserData,Controllers::PokkunMoguraa&,uint);
		static bool NST_CALLBACK PollVsSystem       (UserData,Controllers::VsSystem&);
	};

	const Resource::Cursor Input::Cursor::gun( IDC_CURSOR_GUN  );

	const uint Input::Cursor::primaryButtonId = ::GetSystemMetrics( SM_SWAPBUTTON ) ? VK_RBUTTON : VK_LBUTTON;
	const uint Input::Cursor::secondaryButtonId = ::GetSystemMetrics( SM_SWAPBUTTON ) ? VK_LBUTTON : VK_RBUTTON;

	Input::Cursor::Cursor(Window::Custom& w)
	: 
	hCurrent    ( Resource::Cursor::GetArrow() ),
	hCursor     ( Resource::Cursor::GetArrow() ),
	pos         ( 0 ),
	autoHide    ( FALSE ),
	deadline    ( ~DWORD(0) ),
	inNonClient ( FALSE ),
	window      ( w )
	{
		static const Window::MsgHandler::Entry<Input::Cursor> messages[] =
		{
			{ WM_SETCURSOR,          &Cursor::OnSetCursor   },
			{ WM_MOUSEMOVE,          &Cursor::OnNop         },
			{ WM_LBUTTONDOWN,        &Cursor::OnNop         },
			{ WM_RBUTTONDOWN,        &Cursor::OnRButtonDown },
			{ WM_LBUTTONUP,          &Cursor::OnNop         },
			{ WM_RBUTTONUP,          &Cursor::OnButtonUp    }
		};

		static const Window::MsgHandler::HookEntry<Input::Cursor> hooks[] =
		{
			{ WM_ENTERSIZEMOVE, &Cursor::OnEnterSizeMoveMenu },
			{ WM_ENTERMENULOOP, &Cursor::OnEnterSizeMoveMenu },
			{ WM_EXITSIZEMOVE,  &Cursor::OnExitSizeMoveMenu  },
			{ WM_EXITMENULOOP,  &Cursor::OnExitSizeMoveMenu  }
		};

		window.Messages().Add( this, messages, hooks );
	}

	void Input::Cursor::Acquire(Emulator& emulator)
	{
		if (emulator.IsControllerConnected( Nes::Input::ZAPPER ))
		{
			hCursor = gun;
			autoHide = FALSE;
		}
		else if 
		(
			emulator.IsControllerConnected( Nes::Input::PADDLE ) || 
			emulator.IsControllerConnected( Nes::Input::OEKAKIDSTABLET )
		)
		{
			hCursor = NULL;
			autoHide = FALSE;
		}
		else
		{
			hCursor = Resource::Cursor::GetArrow();
			autoHide = TRUE;
		}

		hCurrent = hCursor;
		inNonClient = FALSE;

		Window::MsgHandler& router = window.Messages();

		router[ WM_MOUSEMOVE   ].Set( this, autoHide ? &Cursor::OnMouseMove : &Cursor::OnNop );
		router[ WM_LBUTTONDOWN ].Set( this, autoHide ? &Cursor::OnLButtonDown: &Cursor::OnNop );
		router[ WM_LBUTTONUP   ].Set( this, autoHide ? &Cursor::OnButtonUp : &Cursor::OnNop );
	}

	void Input::Cursor::Unacquire()
	{
		inNonClient = autoHide = FALSE;
		hCurrent = hCursor = Resource::Cursor::GetArrow();

		Window::MsgHandler& router = window.Messages();

		router[ WM_MOUSEMOVE   ].Set( this, &Cursor::OnNop );
		router[ WM_LBUTTONDOWN ].Set( this, &Cursor::OnNop );
		router[ WM_LBUTTONUP   ].Set( this, &Cursor::OnNop );
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("t", on)
    #endif

	inline void Input::Cursor::Refresh()
	{
		if (hCurrent != hCursor)
			::SetCursor( hCurrent=hCursor );
	}

	inline DWORD Input::Cursor::NextTime()
	{
		return ::GetTickCount() + TIME_OUT;
	}

	inline void Input::Cursor::UpdateTime()
	{
		if (autoHide)
			deadline = NextTime();
	}

	ibool Input::Cursor::OnNop(Window::Param&)
	{
		return TRUE;
	}

	ibool Input::Cursor::OnSetCursor(Window::Param& param)
	{
		if (LOWORD(param.lParam) == HTCLIENT)
		{			   
			param.lResult = TRUE;
			::SetCursor( hCurrent );
			return TRUE;
		}

		return FALSE;
	}

	ibool Input::Cursor::OnMouseMove(Window::Param& param)
	{
		NST_VERIFY( autoHide );

		if (pos != param.lParam)
		{
			pos = param.lParam;
			hCurrent = hCursor;
			deadline = NextTime();
		}

		return TRUE;
	}

	ibool Input::Cursor::OnLButtonDown(Window::Param&)
	{
		Refresh();
		return TRUE;
	}

	ibool Input::Cursor::OnRButtonDown(Window::Param& param)
	{						
		Refresh();

		if (hCursor != gun)
			param.Window().SendCommand( IDM_VIEW_MENU );

		return TRUE;
	}

	ibool Input::Cursor::OnButtonUp(Window::Param&)
	{
		Refresh();
		UpdateTime();
		return TRUE;
	}

	void Input::Cursor::OnEnterSizeMoveMenu(Window::Param&)
	{
		hCurrent = hCursor;
		inNonClient = TRUE;
	}

	void Input::Cursor::OnExitSizeMoveMenu(Window::Param&)
	{
		inNonClient = FALSE;
	}

	void Input::Cursor::AutoHide()
	{
		if (deadline <= ::GetTickCount())
		{
			deadline = ~DWORD(0);

			if (!inNonClient && autoHide && window.IsCursorInsideClientArea())
				::SetCursor( hCurrent=NULL );
		}
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("", on)
    #endif

	inline Input::AutoFire::AutoFire()
	: step(0), signal(3) {}

	inline ibool Input::AutoFire::Signaled() const
	{
		return step >= signal;
	}

	Input::Rects::Rects(const Screening& i,const Screening& o)
	: getInput(i), getOutput(o) {}

	Input::Input
	(
		Window::Custom& w,
		Window::Menu& m,
		Emulator& e,
		const Configuration& cfg,
		const Screening& i,
		const Screening& o
	)
	: 
	rects        ( i, o ),
	menu         ( m ),
	emulator     ( e ),
	polled       ( FALSE ),
	cursor       ( w ),
	directInput  ( w ),
	dialog       ( new Window::Input(directInput,e,cfg) )
	{
		static const Window::Menu::CmdHandler::Entry<Input> commands[] =
		{
			{ IDM_MACHINE_INPUT_AUTOSELECT,	          		 &Input::OnCmdMachineAutoSelectController },
			{ IDM_MACHINE_INPUT_PORT1_UNCONNECTED,			 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT1_PAD1,					 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT1_PAD2,					 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT1_PAD3,					 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT1_PAD4,					 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT1_ZAPPER,				 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT1_PADDLE,				 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT1_POWERPAD,				 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT2_UNCONNECTED,			 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT2_PAD1,					 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT2_PAD2,					 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT2_PAD3,					 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT2_PAD4,					 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT2_ZAPPER,				 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT2_PADDLE,				 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT2_POWERPAD,				 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT3_UNCONNECTED,			 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT3_PAD1,					 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT3_PAD2,					 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT3_PAD3,					 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT3_PAD4,					 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT4_UNCONNECTED,			 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT4_PAD1,					 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT4_PAD2,					 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT4_PAD3,					 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT4_PAD4,					 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_SPECIAL_UNCONNECTED,		 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_SPECIAL_FAMILYBASICKEYBOARD, &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_SPECIAL_PADDLE,				 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_SPECIAL_OEKAKIDSTABLET,		 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_SPECIAL_HYPERSHOT,			 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_SPECIAL_CRAZYCLIMBER,		 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_SPECIAL_MAHJONG,			 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_SPECIAL_EXCITINGBOXING,		 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_SPECIAL_TOPRIDER,			 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_SPECIAL_POKKUNMOGURAA,		 &Input::OnCmdMachinePort				  },
			{ IDM_OPTIONS_INPUT,                             &Input::OnCmdOptionsInput                }
		};

		menu.Commands().Add( this, commands );
		emulator.Events().Add( this, &Input::OnEmuEvent );

		typedef Nes::Input::Controllers Controllers;

		Controllers::Pad::callback.Set            ( &Callbacks::PollPad,            this   );
		Controllers::Zapper::callback.Set         ( &Callbacks::PollZapper,         &rects );
		Controllers::Paddle::callback.Set         ( &Callbacks::PollPaddle,         &rects );
		Controllers::PowerPad::callback.Set       ( &Callbacks::PollPowerPad,       this   );
		Controllers::OekaKidsTablet::callback.Set ( &Callbacks::PollOekaKidsTablet, &rects );
		Controllers::HyperShot::callback.Set      ( &Callbacks::PollHyperShot,      this   );
		Controllers::Keyboard::callback.Set       ( &Callbacks::PollKeyboard,       this   );
		Controllers::CrazyClimber::callback.Set   ( &Callbacks::PollCrazyClimber,   this   );
		Controllers::Mahjong::callback.Set        ( &Callbacks::PollMahjong,        this   );
		Controllers::ExcitingBoxing::callback.Set ( &Callbacks::PollExcitingBoxing, this   );
		Controllers::TopRider::callback.Set       ( &Callbacks::PollTopRider,       this   );
		Controllers::PokkunMoguraa::callback.Set  ( &Callbacks::PollPokkunMoguraa,  this   );
		Controllers::VsSystem::callback.Set       ( &Callbacks::PollVsSystem,       this   );

		menu[IDM_MACHINE_INPUT_AUTOSELECT].Check( cfg["machine autoselect controllers"] != Application::Configuration::NO );	

		{
			NST_COMPILE_ASSERT
			(
     			Emulator::EVENT_PORT2_CONTROLLER - Emulator::EVENT_PORT1_CONTROLLER == 1 &&
				Emulator::EVENT_PORT3_CONTROLLER - Emulator::EVENT_PORT1_CONTROLLER == 2 &&
				Emulator::EVENT_PORT4_CONTROLLER - Emulator::EVENT_PORT1_CONTROLLER == 3
			);

			String::Stack<16> string("machine port #");

			for (uint i=0; i < 5; ++i)
			{
				string[13] = (char)('1' + i);
				const String::Heap& type = cfg[string];

				Nes::Input::Type controller = Nes::Input::UNCONNECTED;

				switch (i)
				{
					case 0:
					case 1:

						     if ( type == "zapper"   ) { controller = Nes::Input::ZAPPER;   break; }
						else if ( type == "paddle"   ) { controller = Nes::Input::PADDLE;   break; }
						else if ( type == "powerpad" ) { controller = Nes::Input::POWERPAD; break; }

					case 2:
					case 3:

     						 if ( type == "pad1"        ) controller = Nes::Input::PAD1;
						else if ( type == "pad2"        ) controller = Nes::Input::PAD2;
						else if ( type == "pad3"        ) controller = Nes::Input::PAD3;
						else if ( type == "pad4"        ) controller = Nes::Input::PAD4;
						else if ( type == "unconnected" ) controller = Nes::Input::UNCONNECTED;
						else if ( i == 0                ) controller = Nes::Input::PAD1;
						else if ( i == 1                ) controller = Nes::Input::PAD2;

						break;

					case 4:

       						 if ( type == "paddle"         ) controller = Nes::Input::PADDLE;
       					else if ( type == "keyboard"       ) controller = Nes::Input::KEYBOARD;
						else if ( type == "oekakidstablet" ) controller = Nes::Input::OEKAKIDSTABLET;
						else if ( type == "hypershot"      ) controller = Nes::Input::HYPERSHOT;
						else if ( type == "crazyclimber"   ) controller = Nes::Input::CRAZYCLIMBER;
						else if ( type == "mahjong"        ) controller = Nes::Input::MAHJONG;
						else if ( type == "excitingboxing" ) controller = Nes::Input::EXCITINGBOXING;
						else if ( type == "toprider"       ) controller = Nes::Input::TOPRIDER;
						else if ( type == "pokkunmoguraa"  ) controller = Nes::Input::POKKUNMOGURAA;
				}

				emulator.ConnectController( i, controller );
				OnEmuEvent( (Emulator::Event) (Emulator::EVENT_PORT1_CONTROLLER + i) );
			}
		}

		UpdateSettings();
	}

	Input::~Input()
	{
		emulator.Events().Remove( this );
	}

	void Input::Save(Configuration& cfg) const
	{
		cfg["machine autoselect controllers"].YesNo() = menu[IDM_MACHINE_INPUT_AUTOSELECT].IsChecked();

		{
			String::Stack<16> string("machine port #");

			for (uint i=0; i < 5; ++i)
			{
				cstring type;

				switch (Nes::Input(emulator).GetConnectedController( i ))
				{
    				case Nes::Input::PAD1:           type = "pad1";           break;
					case Nes::Input::PAD2:           type = "pad2";           break;
					case Nes::Input::PAD3:           type = "pad3";           break;
					case Nes::Input::PAD4:           type = "pad4";           break;
					case Nes::Input::ZAPPER:         type = "zapper";         break;
					case Nes::Input::PADDLE:         type = "paddle";         break;
					case Nes::Input::POWERPAD:       type = "powerpad";       break;
					case Nes::Input::KEYBOARD:       type = "keyboard";       break;
					case Nes::Input::OEKAKIDSTABLET: type = "oekakidstablet"; break;
					case Nes::Input::HYPERSHOT:      type = "hypershot";      break;
					case Nes::Input::CRAZYCLIMBER:   type = "crazyclimber";   break;
					case Nes::Input::MAHJONG:        type = "mahjong";        break;
					case Nes::Input::EXCITINGBOXING: type = "excitingboxing"; break;
					case Nes::Input::TOPRIDER:       type = "toprider";       break;
					case Nes::Input::POKKUNMOGURAA:  type = "pokkunmoguraa";  break;
					default:                         type = "unconnected";    break;
				}

				string[13] = (char)('1' + i);
				cfg[string] = type;
			}
		}

		dialog->Save( cfg );
	}

	void Input::StartEmulation()
	{
		if (emulator.Is(Nes::Machine::GAME))
		{
			directInput.Acquire();
			UpdateDevices();
		}
	}

	void Input::StopEmulation()
	{
		directInput.Unacquire();
		cursor.Unacquire();
		Window::Menu::EnableAccelerators( TRUE );
	}

	void Input::UpdateDevices()
	{
		if (emulator.Is(Nes::Machine::ON,Nes::Machine::GAME))
		{
			cursor.Acquire( emulator );
			Window::Menu::EnableAccelerators( !Nes::Input(emulator).IsAnyControllerConnected(Nes::Input::KEYBOARD) );
		}
	}

	void Input::UpdateSettings()
	{
		typedef Window::Input::Settings Settings;

		static const uchar speeds[Settings::AUTOFIRE_NUM_SPEEDS] =
		{
			7,6,5,4,3,2
		};

		autoFire.signal = speeds[ dialog->GetSettings().AutoFireSpeed() ];

		directInput.Optimize( dialog->GetSettings().GetKeys(), Settings::NUM_KEYS );

		static const ushort lut[Settings::NUM_COMMAND_KEYS][2] =
		{
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_OPEN,				     IDM_FILE_OPEN                        }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_LOAD_SCRIPT,			 IDM_FILE_LOAD_NSP                    }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_LOAD_STATE,			 IDM_FILE_LOAD_NST                    }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_SAVE_SCRIPT,			 IDM_FILE_SAVE_NSP                    }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_SAVE_STATE,			 IDM_FILE_SAVE_NST                    }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_LOAD_STATE_1,	 IDM_FILE_QUICK_LOAD_STATE_SLOT_1     }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_LOAD_STATE_2,	 IDM_FILE_QUICK_LOAD_STATE_SLOT_2     }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_LOAD_STATE_3,	 IDM_FILE_QUICK_LOAD_STATE_SLOT_3     }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_LOAD_STATE_4,	 IDM_FILE_QUICK_LOAD_STATE_SLOT_4     }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_LOAD_STATE_5,	 IDM_FILE_QUICK_LOAD_STATE_SLOT_5     }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_LOAD_STATE_6,	 IDM_FILE_QUICK_LOAD_STATE_SLOT_6     }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_LOAD_STATE_7,	 IDM_FILE_QUICK_LOAD_STATE_SLOT_7     }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_LOAD_STATE_8,	 IDM_FILE_QUICK_LOAD_STATE_SLOT_8     }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_LOAD_STATE_9,	 IDM_FILE_QUICK_LOAD_STATE_SLOT_9     }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_LOAD_LAST_STATE, IDM_FILE_QUICK_LOAD_STATE_SLOT_LAST  }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_SAVE_STATE_1,	 IDM_FILE_QUICK_SAVE_STATE_SLOT_1     }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_SAVE_STATE_2,	 IDM_FILE_QUICK_SAVE_STATE_SLOT_2     }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_SAVE_STATE_3,	 IDM_FILE_QUICK_SAVE_STATE_SLOT_3     }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_SAVE_STATE_4,	 IDM_FILE_QUICK_SAVE_STATE_SLOT_4     }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_SAVE_STATE_5,	 IDM_FILE_QUICK_SAVE_STATE_SLOT_5     }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_SAVE_STATE_6,	 IDM_FILE_QUICK_SAVE_STATE_SLOT_6     }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_SAVE_STATE_7,	 IDM_FILE_QUICK_SAVE_STATE_SLOT_7     }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_SAVE_STATE_8,	 IDM_FILE_QUICK_SAVE_STATE_SLOT_8     }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_SAVE_STATE_9,	 IDM_FILE_QUICK_SAVE_STATE_SLOT_9     }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_SAVE_NEXT_STATE, IDM_FILE_QUICK_SAVE_STATE_SLOT_NEXT  }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_SAVE_SCREENSHOT,		 IDM_FILE_SAVE_SCREENSHOT		      }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_LAUNCHER,				 IDM_FILE_LAUNCHER          		  }, 
			{ Settings::FILE_KEYS    + Settings::FILE_KEY_EXIT,					 IDM_FILE_QUIT                	      }, 
			{ Settings::MACHINE_KEYS + Settings::MACHINE_KEY_POWER, 			 IDM_MACHINE_POWER  			      }, 
			{ Settings::MACHINE_KEYS + Settings::MACHINE_KEY_RESET_SOFT,		 IDM_MACHINE_RESET_SOFT		          }, 
			{ Settings::MACHINE_KEYS + Settings::MACHINE_KEY_RESET_HARD,		 IDM_MACHINE_RESET_HARD		          }, 
			{ Settings::MACHINE_KEYS + Settings::MACHINE_KEY_PAUSE,				 IDM_MACHINE_PAUSE		              }, 
			{ Settings::MACHINE_KEYS + Settings::MACHINE_KEY_UNLIMITED_SPRITES,	 IDM_MACHINE_OPTIONS_UNLIMITEDSPRITES }, 
			{ Settings::MACHINE_KEYS + Settings::MACHINE_KEY_CHANGE_DISK_SIDE,	 IDM_MACHINE_FDS_CHANGE_SIDE          }, 
			{ Settings::NSF_KEYS     + Settings::NSF_KEY_PLAY,					 IDM_MACHINE_NSF_PLAY			      }, 
			{ Settings::NSF_KEYS     + Settings::NSF_KEY_STOP,					 IDM_MACHINE_NSF_STOP			      }, 
			{ Settings::NSF_KEYS     + Settings::NSF_KEY_NEXT,					 IDM_MACHINE_NSF_NEXT			      }, 
			{ Settings::NSF_KEYS     + Settings::NSF_KEY_PREV,					 IDM_MACHINE_NSF_PREV			      }, 
			{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SCREENSIZE_1X,		 IDM_VIEW_WINDOWSIZE_1X			      }, 
			{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SCREENSIZE_2X,		 IDM_VIEW_WINDOWSIZE_2X 			  }, 
			{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SCREENSIZE_3X,		 IDM_VIEW_WINDOWSIZE_3X 			  }, 
			{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SCREENSIZE_4X,		 IDM_VIEW_WINDOWSIZE_4X 			  }, 
			{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SCREENSIZE_5X,		 IDM_VIEW_WINDOWSIZE_5X			      }, 
			{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SCREENSIZE_6X,		 IDM_VIEW_WINDOWSIZE_6X 			  }, 
			{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SCREENSIZE_7X,		 IDM_VIEW_WINDOWSIZE_7X 			  }, 
			{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SCREENSIZE_8X,		 IDM_VIEW_WINDOWSIZE_8X 			  }, 
			{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SCREENSIZE_9X,		 IDM_VIEW_WINDOWSIZE_9X 			  }, 
			{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SCREENSIZE_MAX,	     IDM_VIEW_WINDOWSIZE_MAX 			  }, 
			{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SHOW_MENU,			 IDM_VIEW_MENU  			          }, 
			{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SHOW_STATUSBAR,	     IDM_VIEW_STATUSBAR       			  }, 
			{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SHOW_ONTOP,		     IDM_VIEW_ON_TOP     				  }, 
			{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SHOW_FPS,			     IDM_VIEW_FPS        			      }, 
			{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_FULLSCREEN,		     IDM_VIEW_SWITCH_SCREEN				  },
			{ Settings::HELP_KEYS    + Settings::HELP_KEY_HELP,	       	         IDM_HELP_HELP      				  }
		};

		ACCEL accel[Settings::NUM_COMMAND_KEYS];

		for (uint i=0; i < Settings::NUM_COMMAND_KEYS; ++i)
		{
			dialog->GetSettings().GetKey( lut[i][0] ).GetVirtualKey( accel[i] );
			accel[i].cmd = lut[i][1];
		}

		menu.SetKeys( accel, Settings::NUM_COMMAND_KEYS );
	}

	void Input::OnEmuEvent(Emulator::Event event)
	{
		switch (event)
		{			
			case Emulator::EVENT_PORT1_CONTROLLER:
			{
				const Nes::Input::Type type = emulator.GetController(0);
		
				menu[ IDM_MACHINE_INPUT_PORT1_UNCONNECTED ].Check( type == Nes::Input::UNCONNECTED );
				menu[ IDM_MACHINE_INPUT_PORT1_PAD1        ].Check( type == Nes::Input::PAD1		   );
				menu[ IDM_MACHINE_INPUT_PORT1_PAD2        ].Check( type == Nes::Input::PAD2		   );
				menu[ IDM_MACHINE_INPUT_PORT1_PAD3        ].Check( type == Nes::Input::PAD3		   );
				menu[ IDM_MACHINE_INPUT_PORT1_PAD4        ].Check( type == Nes::Input::PAD4		   );
				menu[ IDM_MACHINE_INPUT_PORT1_ZAPPER      ].Check( type == Nes::Input::ZAPPER	   );
				menu[ IDM_MACHINE_INPUT_PORT1_PADDLE      ].Check( type == Nes::Input::PADDLE	   );
				menu[ IDM_MACHINE_INPUT_PORT1_POWERPAD    ].Check( type == Nes::Input::POWERPAD    );
		
				UpdateDevices();
				break;
			}
		
			case Emulator::EVENT_PORT2_CONTROLLER:
			{
				const Nes::Input::Type type = emulator.GetController(1);
		
				menu[ IDM_MACHINE_INPUT_PORT2_UNCONNECTED ].Check( type == Nes::Input::UNCONNECTED );
				menu[ IDM_MACHINE_INPUT_PORT2_PAD1        ].Check( type == Nes::Input::PAD1		   );
				menu[ IDM_MACHINE_INPUT_PORT2_PAD2        ].Check( type == Nes::Input::PAD2		   );
				menu[ IDM_MACHINE_INPUT_PORT2_PAD3        ].Check( type == Nes::Input::PAD3		   );
				menu[ IDM_MACHINE_INPUT_PORT2_PAD4        ].Check( type == Nes::Input::PAD4		   );
				menu[ IDM_MACHINE_INPUT_PORT2_ZAPPER      ].Check( type == Nes::Input::ZAPPER	   );
				menu[ IDM_MACHINE_INPUT_PORT2_PADDLE      ].Check( type == Nes::Input::PADDLE	   );
				menu[ IDM_MACHINE_INPUT_PORT2_POWERPAD    ].Check( type == Nes::Input::POWERPAD	   );
		
				UpdateDevices();
				break;
			}
		
			case Emulator::EVENT_PORT3_CONTROLLER:
			{
				const Nes::Input::Type type = emulator.GetController(2);
		
				menu[ IDM_MACHINE_INPUT_PORT3_UNCONNECTED ].Check( type == Nes::Input::UNCONNECTED );
				menu[ IDM_MACHINE_INPUT_PORT3_PAD1        ].Check( type == Nes::Input::PAD1		   );
				menu[ IDM_MACHINE_INPUT_PORT3_PAD2        ].Check( type == Nes::Input::PAD2		   );
				menu[ IDM_MACHINE_INPUT_PORT3_PAD3        ].Check( type == Nes::Input::PAD3		   );
				menu[ IDM_MACHINE_INPUT_PORT3_PAD4        ].Check( type == Nes::Input::PAD4		   );
		
				UpdateDevices();
				break;
			}
		
			case Emulator::EVENT_PORT4_CONTROLLER:
			{
				const Nes::Input::Type type = emulator.GetController(3);
		
				menu[ IDM_MACHINE_INPUT_PORT4_UNCONNECTED ].Check( type == Nes::Input::UNCONNECTED );
				menu[ IDM_MACHINE_INPUT_PORT4_PAD1        ].Check( type == Nes::Input::PAD1		   );
				menu[ IDM_MACHINE_INPUT_PORT4_PAD2        ].Check( type == Nes::Input::PAD2		   );
				menu[ IDM_MACHINE_INPUT_PORT4_PAD3        ].Check( type == Nes::Input::PAD3		   );
				menu[ IDM_MACHINE_INPUT_PORT4_PAD4        ].Check( type == Nes::Input::PAD4		   );
		
				UpdateDevices();
				break;
			}
		
			case Emulator::EVENT_PORT5_CONTROLLER:
			{
				const Nes::Input::Type type = emulator.GetController(4);
		
				menu[ IDM_MACHINE_INPUT_SPECIAL_UNCONNECTED         ].Check( type == Nes::Input::UNCONNECTED    );
				menu[ IDM_MACHINE_INPUT_SPECIAL_FAMILYBASICKEYBOARD ].Check( type == Nes::Input::KEYBOARD       );
				menu[ IDM_MACHINE_INPUT_SPECIAL_PADDLE              ].Check( type == Nes::Input::PADDLE         );
				menu[ IDM_MACHINE_INPUT_SPECIAL_OEKAKIDSTABLET      ].Check( type == Nes::Input::OEKAKIDSTABLET );
				menu[ IDM_MACHINE_INPUT_SPECIAL_HYPERSHOT           ].Check( type == Nes::Input::HYPERSHOT      );
				menu[ IDM_MACHINE_INPUT_SPECIAL_CRAZYCLIMBER        ].Check( type == Nes::Input::CRAZYCLIMBER   );
				menu[ IDM_MACHINE_INPUT_SPECIAL_MAHJONG             ].Check( type == Nes::Input::MAHJONG        );
				menu[ IDM_MACHINE_INPUT_SPECIAL_EXCITINGBOXING      ].Check( type == Nes::Input::EXCITINGBOXING );
				menu[ IDM_MACHINE_INPUT_SPECIAL_TOPRIDER            ].Check( type == Nes::Input::TOPRIDER       );
				menu[ IDM_MACHINE_INPUT_SPECIAL_POKKUNMOGURAA       ].Check( type == Nes::Input::POKKUNMOGURAA  );
		
				UpdateDevices();
				break;
			}

			case Emulator::EVENT_LOAD:

				if (emulator.Is(Nes::Machine::GAME) && menu[IDM_MACHINE_INPUT_AUTOSELECT].IsChecked())
					emulator.AutoSelectControllers();

				break;

    		case Emulator::EVENT_NETPLAY_LOAD:
			
				for (uint i=0; i < 4; ++i)
					menu[IDM_POS_MACHINE][IDM_POS_MACHINE_INPUT][IDM_POS_MACHINE_INPUT_PORT1+i].Enable( emulator.GetPlayer() == (i+1) );

				break;

    		case Emulator::EVENT_NETPLAY_UNLOAD:

				for (uint i=IDM_POS_MACHINE_INPUT_PORT1; i <= IDM_POS_MACHINE_INPUT_PORT4; ++i)
					menu[IDM_POS_MACHINE][IDM_POS_MACHINE_INPUT][i].Enable();

				break;

			case Emulator::EVENT_NETPLAY_MODE_ON:
			case Emulator::EVENT_NETPLAY_MODE_OFF:
			{
				const ibool state = (event == Emulator::EVENT_NETPLAY_MODE_OFF);

				menu[IDM_MACHINE_INPUT_AUTOSELECT].Enable( state );
				menu[IDM_POS_MACHINE][IDM_POS_MACHINE_INPUT][IDM_POS_MACHINE_INPUT_SPECIAL].Enable( state );
				menu[IDM_OPTIONS_INPUT].Enable( state );
				break;
			}
		}
	}

	void Input::OnCmdOptionsInput(uint) 
	{ 
		dialog->Open();
		UpdateSettings();
	}

	void Input::OnCmdMachineAutoSelectController(uint)
	{
		if (!menu[IDM_MACHINE_INPUT_AUTOSELECT].ToggleCheck())
			emulator.AutoSelectControllers();
	}

	void Input::OnCmdMachinePort(uint id) 
	{
		static const uchar table[][2] =
		{
			{ 0, Nes::Input::UNCONNECTED    },
			{ 0, Nes::Input::PAD1		    },
			{ 0, Nes::Input::PAD2		    },
			{ 0, Nes::Input::PAD3		    },
			{ 0, Nes::Input::PAD4		    },
			{ 0, Nes::Input::ZAPPER		    },
			{ 0, Nes::Input::PADDLE		    },
			{ 0, Nes::Input::POWERPAD	    },
			{ 1, Nes::Input::UNCONNECTED    },
			{ 1, Nes::Input::PAD1		    },
			{ 1, Nes::Input::PAD2		    },
			{ 1, Nes::Input::PAD3		    },
			{ 1, Nes::Input::PAD4		    },
			{ 1, Nes::Input::ZAPPER		    },
			{ 1, Nes::Input::PADDLE		    },
			{ 1, Nes::Input::POWERPAD	    },
			{ 2, Nes::Input::UNCONNECTED    },
			{ 2, Nes::Input::PAD1		    },
			{ 2, Nes::Input::PAD2		    },
			{ 2, Nes::Input::PAD3		    },
			{ 2, Nes::Input::PAD4		    },
			{ 3, Nes::Input::UNCONNECTED    },
			{ 3, Nes::Input::PAD1		    },
			{ 3, Nes::Input::PAD2		    },
			{ 3, Nes::Input::PAD3		    },
			{ 3, Nes::Input::PAD4		    },
			{ 4, Nes::Input::UNCONNECTED    },
			{ 4, Nes::Input::KEYBOARD	    },
			{ 4, Nes::Input::PADDLE         },
			{ 4, Nes::Input::OEKAKIDSTABLET },
			{ 4, Nes::Input::HYPERSHOT      },
			{ 4, Nes::Input::CRAZYCLIMBER   },
			{ 4, Nes::Input::MAHJONG        },
			{ 4, Nes::Input::EXCITINGBOXING },
			{ 4, Nes::Input::TOPRIDER       },
			{ 4, Nes::Input::POKKUNMOGURAA  }
		};

		const uchar* const offset = table[id - IDM_MACHINE_INPUT_PORT1_UNCONNECTED];
		emulator.ConnectController( offset[0], (Nes::Input::Type) offset[1] );
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("t", on)
    #endif

	void Input::ForcePoll()
	{
		directInput.Poll();
		emulator.ToggleSpeed( dialog->GetSettings().GetKeys(Window::Input::Settings::EMULATION_KEYS)[Window::Input::Settings::EMULATION_KEY_ALT_SPEED].GetState() );
		emulator.ToggleRewind( dialog->GetSettings().GetKeys(Window::Input::Settings::EMULATION_KEYS)[Window::Input::Settings::EMULATION_KEY_REWIND].GetState() );
	}

	inline void Input::AutoPoll()
	{
		if (!polled)
		{
			polled = TRUE;
			ForcePoll();
		}
	}

	bool NST_CALLBACK Input::Callbacks::PollPad(UserData data,Controllers::Pad& pad,uint index)
	{
		NST_COMPILE_ASSERT
		( 
			Settings::TYPE_PAD1 == 0 && 
			Settings::TYPE_PAD2 == 1 && 
			Settings::TYPE_PAD3 == 2 && 
			Settings::TYPE_PAD4 == 3
		);

		NST_ASSERT(index < 4);

		static_cast<Input*>(data)->AutoPoll();

		const Key* const NST_RESTRICT keys = static_cast<const Input*>(data)->dialog->GetSettings().GetKeys(index,0);

		uint buttons = 0;

		keys[ Settings::PAD_KEY_A      ].GetState( buttons, Controllers::Pad::A      );
		keys[ Settings::PAD_KEY_B      ].GetState( buttons, Controllers::Pad::B      );
		keys[ Settings::PAD_KEY_SELECT ].GetState( buttons, Controllers::Pad::SELECT );
		keys[ Settings::PAD_KEY_START  ].GetState( buttons, Controllers::Pad::START  );
		keys[ Settings::PAD_KEY_UP     ].GetState( buttons, Controllers::Pad::UP     );
		keys[ Settings::PAD_KEY_DOWN   ].GetState( buttons, Controllers::Pad::DOWN   );
		keys[ Settings::PAD_KEY_LEFT   ].GetState( buttons, Controllers::Pad::LEFT   );
		keys[ Settings::PAD_KEY_RIGHT  ].GetState( buttons, Controllers::Pad::RIGHT  );

		if (static_cast<const Input*>(data)->autoFire.Signaled())
		{
			keys[ Settings::PAD_KEY_AUTOFIRE_A ].GetState( buttons, Controllers::Pad::A );
			keys[ Settings::PAD_KEY_AUTOFIRE_B ].GetState( buttons, Controllers::Pad::B );
		}

		pad.buttons = buttons;

		return true;
	}

	bool NST_CALLBACK Input::Callbacks::PollZapper(UserData data,Controllers::Zapper& NST_RESTRICT zapper)
	{
		POINT mouse;
		::GetCursorPos( &mouse );

		Window::Rect output;
		static_cast<const Rects*>(data)->getOutput( output );

		if (output.IsInside( mouse ))
		{
			if (!(::GetAsyncKeyState( Cursor::secondaryButtonId ) & 0x8000U))
			{
				zapper.fire = ::GetAsyncKeyState( Cursor::primaryButtonId ) & 0x8000U;

				if (zapper.x != ~0U)
				{
					Window::Rect input;
					static_cast<const Rects*>(data)->getInput( input );

					zapper.x = uint(input.left) + (uint(mouse.x) - uint(output.left)) * (uint(input.Width())-1) / uint(output.Width());
					zapper.y = uint(input.top) + (uint(mouse.y) - uint(output.top)) * (uint(input.Height())-1) / uint(output.Height());
				}
				else if (zapper.fire) // gun off-screen unlock
				{
					zapper.x = ~1U; 
				}
			}
			else // gun off-screen lock
			{
				zapper.fire = TRUE;
				zapper.y = zapper.x = ~0U;
			}
		}
		else
		{
			zapper.fire = FALSE;
		}

		return true;
	}

	bool NST_CALLBACK Input::Callbacks::PollPaddle(UserData data,Controllers::Paddle& paddle)
	{
		POINT mouse;
		::GetCursorPos( &mouse );

		Window::Rect output;
		static_cast<const Rects*>(data)->getOutput( output );

		if (output.IsInside( mouse ))
		{
			Window::Rect input;
			static_cast<const Rects*>(data)->getInput( input );

			paddle.x = uint(input.left) + (uint(mouse.x) - uint(output.left)) * (uint(input.Width())-1) / uint(output.Width());
			paddle.button = ::GetAsyncKeyState( Cursor::primaryButtonId ) & 0x8000U;
		}
		else
		{
			paddle.button = FALSE;
		}

		return true;
	}

	bool NST_CALLBACK Input::Callbacks::PollPowerPad(UserData data,Controllers::PowerPad& powerPad)
	{
		static_cast<Input*>(data)->AutoPoll();

		const Key* NST_RESTRICT keys = static_cast<const Input*>(data)->dialog->GetSettings().GetKeys(Settings::POWERPAD_KEYS);

		for (uint i=0; i < Settings::POWERPAD_NUM_SIDE_A_KEYS; ++i)
			powerPad.sideA[i] = keys[i].GetState();

		keys += Settings::POWERPAD_NUM_SIDE_A_KEYS;

		for (uint i=0; i < Settings::POWERPAD_NUM_SIDE_B_KEYS; ++i)
			powerPad.sideB[i] = keys[i].GetState();

		return true;
	}

	bool NST_CALLBACK Input::Callbacks::PollOekaKidsTablet(UserData data,Controllers::OekaKidsTablet& NST_RESTRICT tablet)
	{
		POINT mouse;
		::GetCursorPos( &mouse );

		Window::Rect output;
		static_cast<const Rects*>(data)->getOutput( output );

		if (output.IsInside( mouse ))
		{
			Window::Rect input;
			static_cast<const Rects*>(data)->getInput( input );

			tablet.x = uint(input.left) + (uint(mouse.x) - uint(output.left)) * (uint(input.Width())-1) / uint(output.Width());
			tablet.y = uint(input.top) + (uint(mouse.y) - uint(output.top)) * (uint(input.Height())-1) / uint(output.Height());
			tablet.button = ::GetAsyncKeyState( Cursor::primaryButtonId ) & 0x8000U;
		}
		else
		{
			tablet.button = 0;
		}

		return true;
	}

	bool NST_CALLBACK Input::Callbacks::PollHyperShot(UserData data,Controllers::HyperShot& hyperShot)
	{
		static_cast<Input*>(data)->AutoPoll();

		const Key* const NST_RESTRICT keys[2] = 
		{
			static_cast<const Input*>(data)->dialog->GetSettings().GetKeys(Settings::PAD1_KEYS),
			static_cast<const Input*>(data)->dialog->GetSettings().GetKeys(Settings::PAD2_KEYS)
		};

		uint buttons = 0;

		keys[0][ Settings::PAD_KEY_A ].GetState( buttons, Controllers::HyperShot::PLAYER1_BUTTON_1 );
		keys[0][ Settings::PAD_KEY_B ].GetState( buttons, Controllers::HyperShot::PLAYER1_BUTTON_2 );
		keys[1][ Settings::PAD_KEY_A ].GetState( buttons, Controllers::HyperShot::PLAYER2_BUTTON_1 );
		keys[1][ Settings::PAD_KEY_B ].GetState( buttons, Controllers::HyperShot::PLAYER2_BUTTON_2 );

		if (static_cast<const Input*>(data)->autoFire.Signaled())
		{
			keys[0][ Settings::PAD_KEY_AUTOFIRE_A ].GetState( buttons, Controllers::HyperShot::PLAYER1_BUTTON_1 );
			keys[0][ Settings::PAD_KEY_AUTOFIRE_B ].GetState( buttons, Controllers::HyperShot::PLAYER1_BUTTON_2 );
			keys[1][ Settings::PAD_KEY_AUTOFIRE_A ].GetState( buttons, Controllers::HyperShot::PLAYER2_BUTTON_1 );
			keys[1][ Settings::PAD_KEY_AUTOFIRE_B ].GetState( buttons, Controllers::HyperShot::PLAYER2_BUTTON_2 );
		}

		hyperShot.buttons = buttons;

		return true;
	}

	bool NST_CALLBACK Input::Callbacks::PollCrazyClimber(UserData data,Controllers::CrazyClimber& crazyClimber)
	{
		static_cast<Input*>(data)->AutoPoll();

		const Key* const NST_RESTRICT keys = static_cast<const Input*>(data)->dialog->GetSettings().GetKeys(Settings::CRAZYCLIMBER_KEYS);

		uint buttons = 0;

		keys[ Settings::CRAZYCLIMBER_KEY_LEFT_UP    ].GetState( buttons, Controllers::CrazyClimber::UP    );
		keys[ Settings::CRAZYCLIMBER_KEY_LEFT_RIGHT ].GetState( buttons, Controllers::CrazyClimber::RIGHT );
		keys[ Settings::CRAZYCLIMBER_KEY_LEFT_DOWN  ].GetState( buttons, Controllers::CrazyClimber::DOWN  );
		keys[ Settings::CRAZYCLIMBER_KEY_LEFT_LEFT  ].GetState( buttons, Controllers::CrazyClimber::LEFT  );

		crazyClimber.left = buttons;

		buttons = 0;

		keys[ Settings::CRAZYCLIMBER_KEY_RIGHT_UP    ].GetState( buttons, Controllers::CrazyClimber::UP    );
		keys[ Settings::CRAZYCLIMBER_KEY_RIGHT_RIGHT ].GetState( buttons, Controllers::CrazyClimber::RIGHT );
		keys[ Settings::CRAZYCLIMBER_KEY_RIGHT_DOWN  ].GetState( buttons, Controllers::CrazyClimber::DOWN  );
		keys[ Settings::CRAZYCLIMBER_KEY_RIGHT_LEFT  ].GetState( buttons, Controllers::CrazyClimber::LEFT  );

		crazyClimber.right = buttons;

		return true;
	}

	bool NST_CALLBACK Input::Callbacks::PollKeyboard(UserData data,Controllers::Keyboard& keyboard,uint part,uint mode)
	{
		static_cast<Input*>(data)->AutoPoll();

    #define NST_2(a,b) (a | (ushort(b) << 8))

		static const ushort keyMap[9][2][4] =
		{
			{		
				{ DIK_F8, NST_2(DIK_RETURN,DIK_NUMPADENTER), DIK_LBRACKET, DIK_RBRACKET },
				{ NST_2(DIK_F9,DIK_APPS), DIK_RSHIFT, DIK_BACKSLASH, DIK_END }
			},
			{
				{ DIK_F7, DIK_AT, DIK_APOSTROPHE, DIK_SEMICOLON },
				{ DIK_UNDERLINE, NST_2(DIK_SLASH,DIK_NUMPADSLASH), NST_2(DIK_MINUS,DIK_NUMPADMINUS), DIK_EQUALS }
			},
			{
				{ DIK_F6, DIK_O, DIK_L, DIK_K },
				{ DIK_PERIOD, DIK_COMMA, DIK_P, NST_2(DIK_0,DIK_NUMPAD0) }
			},
			{
				{ DIK_F5, DIK_I, DIK_U, DIK_J },
				{ DIK_M,  DIK_N, NST_2(DIK_9,DIK_NUMPAD9), NST_2(DIK_8,DIK_NUMPAD8) }
			},
			{
				{ DIK_F4, DIK_Y, DIK_G, DIK_H },
				{ DIK_B, DIK_V, NST_2(DIK_7,DIK_NUMPAD7), NST_2(DIK_6,DIK_NUMPAD6) }
			},
			{
				{ DIK_F3, DIK_T, DIK_R, DIK_D },
				{ DIK_F, DIK_C, NST_2(DIK_5,DIK_NUMPAD5), NST_2(DIK_4,DIK_NUMPAD4) }
			},
			{
				{ DIK_F2, DIK_W, DIK_S, DIK_A },
				{ DIK_X, DIK_Z, DIK_E, NST_2(DIK_3,DIK_NUMPAD3) }
			},
			{
				{ DIK_F1, DIK_ESCAPE, DIK_Q, DIK_CAPITAL },
				{ DIK_LSHIFT, DIK_F10, NST_2(DIK_1,DIK_NUMPAD1), NST_2(DIK_2,DIK_NUMPAD2) }
			},
			{
				{ DIK_HOME, DIK_UP, DIK_RIGHT, DIK_LEFT },
				{ DIK_DOWN, DIK_SPACE, NST_2(DIK_BACK,DIK_DELETE), DIK_INSERT }
			},
		};

    #undef NST_2

		const u8* const NST_RESTRICT buffer = static_cast<const Input*>(data)->directInput.GetKeyboardBuffer();

		uint key = 0;

		for (uint i=0; i < 4; ++i)
		{
			const uint pushed = 
			(
				(buffer[(keyMap[part][mode][i] & 0x00FF) >> 0] & 0x80) |
				(buffer[(keyMap[part][mode][i] & 0xFF00) >> 8] & 0x80)
			);

			key |= pushed >> ( 7 - ( i + 1 ) );
		}

		keyboard.parts[part] = (uchar) key;

		return true;
	}

	bool NST_CALLBACK Input::Callbacks::PollMahjong(UserData data,Controllers::Mahjong& mahjong,uint part)
	{
		static_cast<Input*>(data)->AutoPoll();

		const Key* const NST_RESTRICT keys = static_cast<const Input*>(data)->dialog->GetSettings().GetKeys(Settings::MAHJONG_KEYS);

		uint buttons = 0;

		switch (part)
		{
			case Controllers::Mahjong::PART_1:
		
				keys[ Settings::MAHJONG_KEY_I ].GetState( buttons, Controllers::Mahjong::PART_1_I );
				keys[ Settings::MAHJONG_KEY_J ].GetState( buttons, Controllers::Mahjong::PART_1_J );
				keys[ Settings::MAHJONG_KEY_K ].GetState( buttons, Controllers::Mahjong::PART_1_K );
				keys[ Settings::MAHJONG_KEY_L ].GetState( buttons, Controllers::Mahjong::PART_1_L );
				keys[ Settings::MAHJONG_KEY_M ].GetState( buttons, Controllers::Mahjong::PART_1_M );
				keys[ Settings::MAHJONG_KEY_N ].GetState( buttons, Controllers::Mahjong::PART_1_N );
				break;
		
			case Controllers::Mahjong::PART_2:
		
				keys[ Settings::MAHJONG_KEY_A ].GetState( buttons, Controllers::Mahjong::PART_2_A );
				keys[ Settings::MAHJONG_KEY_B ].GetState( buttons, Controllers::Mahjong::PART_2_B );
				keys[ Settings::MAHJONG_KEY_C ].GetState( buttons, Controllers::Mahjong::PART_2_C );
				keys[ Settings::MAHJONG_KEY_D ].GetState( buttons, Controllers::Mahjong::PART_2_D );
				keys[ Settings::MAHJONG_KEY_E ].GetState( buttons, Controllers::Mahjong::PART_2_E );
				keys[ Settings::MAHJONG_KEY_F ].GetState( buttons, Controllers::Mahjong::PART_2_F );
				keys[ Settings::MAHJONG_KEY_G ].GetState( buttons, Controllers::Mahjong::PART_2_G );
				keys[ Settings::MAHJONG_KEY_H ].GetState( buttons, Controllers::Mahjong::PART_2_H );
				break;
		
			case Controllers::Mahjong::PART_3:
		
				keys[ Settings::MAHJONG_KEY_SELECT ].GetState( buttons, Controllers::Mahjong::PART_3_SELECT );
				keys[ Settings::MAHJONG_KEY_START  ].GetState( buttons, Controllers::Mahjong::PART_3_START  );
				keys[ Settings::MAHJONG_KEY_KAN    ].GetState( buttons, Controllers::Mahjong::PART_3_KAN    );
				keys[ Settings::MAHJONG_KEY_PON    ].GetState( buttons, Controllers::Mahjong::PART_3_PON    );
				keys[ Settings::MAHJONG_KEY_CHII   ].GetState( buttons, Controllers::Mahjong::PART_3_CHII   );
				keys[ Settings::MAHJONG_KEY_REACH  ].GetState( buttons, Controllers::Mahjong::PART_3_REACH  );
				keys[ Settings::MAHJONG_KEY_RON    ].GetState( buttons, Controllers::Mahjong::PART_3_RON    );
				break;
		}

		mahjong.buttons = buttons;

		return true;
	}

	bool NST_CALLBACK Input::Callbacks::PollExcitingBoxing(UserData data,Controllers::ExcitingBoxing& excitingBoxing,uint part)
	{
		static_cast<Input*>(data)->AutoPoll();

		const Key* const NST_RESTRICT keys = static_cast<const Input*>(data)->dialog->GetSettings().GetKeys(Settings::EXCITINGBOXING_KEYS);

		uint buttons = 0;

		switch (part)
		{		
			case Controllers::ExcitingBoxing::PART_1:
		
				keys[ Settings::EXCITINGBOXING_KEY_RIGHT_HOOK ].GetState( buttons, Controllers::ExcitingBoxing::PART_1_RIGHT_HOOK );
				keys[ Settings::EXCITINGBOXING_KEY_RIGHT_MOVE ].GetState( buttons, Controllers::ExcitingBoxing::PART_1_RIGHT_MOVE );
				keys[ Settings::EXCITINGBOXING_KEY_LEFT_MOVE  ].GetState( buttons, Controllers::ExcitingBoxing::PART_1_LEFT_MOVE  );
				keys[ Settings::EXCITINGBOXING_KEY_LEFT_HOOK  ].GetState( buttons, Controllers::ExcitingBoxing::PART_1_LEFT_HOOK  );
				break;

			case Controllers::ExcitingBoxing::PART_2:

				keys[ Settings::EXCITINGBOXING_KEY_STRAIGHT   ].GetState( buttons, Controllers::ExcitingBoxing::PART_2_STRAIGHT   );
				keys[ Settings::EXCITINGBOXING_KEY_RIGHT_JABB ].GetState( buttons, Controllers::ExcitingBoxing::PART_2_RIGHT_JABB );
				keys[ Settings::EXCITINGBOXING_KEY_BODY       ].GetState( buttons, Controllers::ExcitingBoxing::PART_2_BODY       );
				keys[ Settings::EXCITINGBOXING_KEY_LEFT_JABB  ].GetState( buttons, Controllers::ExcitingBoxing::PART_2_LEFT_JABB  );
				break;																							  
		}

		excitingBoxing.buttons = buttons;

		return true;
	}

	bool NST_CALLBACK Input::Callbacks::PollTopRider(UserData data,Controllers::TopRider& topRider)
	{
		static_cast<Input*>(data)->AutoPoll();

		const Key* const NST_RESTRICT keys = static_cast<const Input*>(data)->dialog->GetSettings().GetKeys(Settings::PAD1_KEYS);

		uint buttons = 0;

		keys[ Settings::PAD_KEY_UP     ].GetState( buttons, Controllers::TopRider::SHIFT_GEAR  );
		keys[ Settings::PAD_KEY_RIGHT  ].GetState( buttons, Controllers::TopRider::STEER_RIGHT );
		keys[ Settings::PAD_KEY_DOWN   ].GetState( buttons, Controllers::TopRider::REAR        );
		keys[ Settings::PAD_KEY_LEFT   ].GetState( buttons, Controllers::TopRider::STEER_LEFT  );
		keys[ Settings::PAD_KEY_SELECT ].GetState( buttons, Controllers::TopRider::SELECT      );
		keys[ Settings::PAD_KEY_START  ].GetState( buttons, Controllers::TopRider::START       );
		keys[ Settings::PAD_KEY_A      ].GetState( buttons, Controllers::TopRider::ACCEL       );
		keys[ Settings::PAD_KEY_B      ].GetState( buttons, Controllers::TopRider::BRAKE       );

		topRider.buttons = buttons;

		return true;
	}

	bool NST_CALLBACK Input::Callbacks::PollPokkunMoguraa(UserData data,Controllers::PokkunMoguraa& pokkunMoguraa,uint row)
	{
		static_cast<Input*>(data)->AutoPoll();

		const Key* const NST_RESTRICT keys = static_cast<const Input*>(data)->dialog->GetSettings().GetKeys(Settings::POKKUNMOGURAA_KEYS);

		uint buttons = 0;

		if (row & Controllers::PokkunMoguraa::ROW_1)
		{
			keys[ Settings::POKKUNMOGURAA_KEY_ROW_1_1 ].GetState( buttons, Controllers::PokkunMoguraa::BUTTON_1 );
			keys[ Settings::POKKUNMOGURAA_KEY_ROW_1_2 ].GetState( buttons, Controllers::PokkunMoguraa::BUTTON_2 );
			keys[ Settings::POKKUNMOGURAA_KEY_ROW_1_3 ].GetState( buttons, Controllers::PokkunMoguraa::BUTTON_3 );
			keys[ Settings::POKKUNMOGURAA_KEY_ROW_1_4 ].GetState( buttons, Controllers::PokkunMoguraa::BUTTON_4 );
		}																														   

		if (row & Controllers::PokkunMoguraa::ROW_2)
		{
			keys[ Settings::POKKUNMOGURAA_KEY_ROW_2_1 ].GetState( buttons, Controllers::PokkunMoguraa::BUTTON_1 );
			keys[ Settings::POKKUNMOGURAA_KEY_ROW_2_2 ].GetState( buttons, Controllers::PokkunMoguraa::BUTTON_2 );
			keys[ Settings::POKKUNMOGURAA_KEY_ROW_2_3 ].GetState( buttons, Controllers::PokkunMoguraa::BUTTON_3 );
			keys[ Settings::POKKUNMOGURAA_KEY_ROW_2_4 ].GetState( buttons, Controllers::PokkunMoguraa::BUTTON_4 );
		}

		if (row & Controllers::PokkunMoguraa::ROW_3)
		{
			keys[ Settings::POKKUNMOGURAA_KEY_ROW_3_1 ].GetState( buttons, Controllers::PokkunMoguraa::BUTTON_1 );
			keys[ Settings::POKKUNMOGURAA_KEY_ROW_3_2 ].GetState( buttons, Controllers::PokkunMoguraa::BUTTON_2 );
			keys[ Settings::POKKUNMOGURAA_KEY_ROW_3_3 ].GetState( buttons, Controllers::PokkunMoguraa::BUTTON_3 );
			keys[ Settings::POKKUNMOGURAA_KEY_ROW_3_4 ].GetState( buttons, Controllers::PokkunMoguraa::BUTTON_4 );
		}

		pokkunMoguraa.buttons = buttons;

		return true;
	}

	bool NST_CALLBACK Input::Callbacks::PollVsSystem(UserData data,Controllers::VsSystem& vsSystem)
	{
		static_cast<Input*>(data)->AutoPoll();

		const Key* const NST_RESTRICT keys = static_cast<const Input*>(data)->dialog->GetSettings().GetKeys(Settings::EMULATION_KEYS);

		uint buttons = 0;

		keys[ Settings::EMULATION_KEY_INSERT_COIN_1 ].GetState( buttons, Controllers::VsSystem::COIN_1 );
		keys[ Settings::EMULATION_KEY_INSERT_COIN_2 ].GetState( buttons, Controllers::VsSystem::COIN_2 );

		vsSystem.insertCoin = buttons;

		return true;
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("", on)
    #endif
}
