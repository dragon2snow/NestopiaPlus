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

#include "NstResourceCursor.hpp"
#include "NstObjectHeap.hpp"
#include "NstIoLog.hpp"
#include "NstIoScreen.hpp"
#include "NstSystemKeyboard.hpp"
#include "NstResourceString.hpp"
#include "NstWindowMenu.hpp"
#include "NstWindowCustom.hpp"
#include "NstWindowParam.hpp"
#include "NstManagerEmulator.hpp"
#include "NstDialogInput.hpp"
#include "NstManagerInput.hpp"

namespace Nestopia
{
	using namespace Managers;

	struct Input::Callbacks
	{
		typedef Nes::Input::UserData UserData;
		typedef Nes::Input::Controllers Controllers;
		typedef Window::Input::Settings Settings;
		typedef Settings::Key Key;

		static bool NST_CALLBACK PollPad               (UserData,Controllers::Pad&,uint);
		static bool NST_CALLBACK PollZapper            (UserData,Controllers::Zapper&);
		static bool NST_CALLBACK PollPaddle            (UserData,Controllers::Paddle&);
		static bool NST_CALLBACK PollPowerPad          (UserData,Controllers::PowerPad&);
		static bool NST_CALLBACK PollOekaKidsTablet    (UserData,Controllers::OekaKidsTablet&);
		static bool NST_CALLBACK PollHyperShot         (UserData,Controllers::HyperShot&);
		static bool NST_CALLBACK PollCrazyClimber      (UserData,Controllers::CrazyClimber&);
		static bool NST_CALLBACK PollFamilyTrainer     (UserData,Controllers::FamilyTrainer&);
		static bool NST_CALLBACK PollFamilyKeyboard    (UserData,Controllers::FamilyKeyboard&,uint,uint);
		static bool NST_CALLBACK PollSuborKeyboard     (UserData,Controllers::SuborKeyboard&,uint,uint);
		static bool NST_CALLBACK PollDoremikkoKeyboard (UserData,Controllers::DoremikkoKeyboard&,uint,uint);
		static bool NST_CALLBACK PollMahjong           (UserData,Controllers::Mahjong&,uint);
		static bool NST_CALLBACK PollExcitingBoxing    (UserData,Controllers::ExcitingBoxing&,uint);
		static bool NST_CALLBACK PollTopRider          (UserData,Controllers::TopRider&);
		static bool NST_CALLBACK PollPokkunMoguraa     (UserData,Controllers::PokkunMoguraa&,uint);
		static bool NST_CALLBACK PollPartyTap          (UserData,Controllers::PartyTap&);
		static bool NST_CALLBACK PollVsSystem          (UserData,Controllers::VsSystem&);
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

	Input::Rects::Rects(const Screening& i,const Screening& o)
	: getInput(i), getOutput(o) {}

	Input::ClipBoard::ClipBoard()
	{
		Clear();
	}

	inline Input::AutoFire::AutoFire()
	: step(0), signal(3) {}

	void Input::AutoFire::operator = (uint speed)
	{
		static const uchar speeds[Window::Input::Settings::AUTOFIRE_NUM_SPEEDS] =
		{
			7,6,5,4,3,2
		};

		signal = speeds[speed];
	}

	void Input::ClipBoard::Clear()
	{
		paste = false;
		Heap::Destroy();
	}

	ibool Input::ClipBoard::CanPaste() const
	{
		return Empty() && ::IsClipboardFormatAvailable( CF_TEXT );
	}

	void Input::ClipBoard::Paste()
	{
		paste = CanPaste();
	}

	uint Input::ClipBoard::Query(const u8* NST_RESTRICT const keyboard,const Type type)
	{
		const ibool p = paste;
		paste = false;

		if (Empty())
		{
			if (p || (keyboard[DIK_F12] & 0x80))
			{
				if (::IsClipboardFormatAvailable( CF_UNICODETEXT ) && ::OpenClipboard( Application::Instance::GetMainWindow() ))
				{
					if (HANDLE const handle = ::GetClipboardData( CF_UNICODETEXT ))
					{
						if (wstring string = (wstring) ::GlobalLock( handle ))
						{
							pos = 0;
							releasing = 0;
							hold = 0;
							shifted = false;

							bool kana = false;

							for (wchar_t p; '\0' != (p = *string); ++string)
							{
								bool mode = false;

								if (p < 32)
								{
									if (p != '\r')
										continue;
								}
								else if (p >= 'A' && p <= 'Z')
								{
									p = p - 'A' + 'a';
								}
								else if (p > 122)
								{
									if (p == '¥')
									{
										p = '\\';
									}
									else if (type == SUBOR)
									{
										if (p > 125)
											continue;
									}
									else if (p > 0xFF00)
									{
										switch (p)
										{
											case 0xFF71: p = '1';  break;
											case 0xFF72: p = '2';  break;
											case 0xFF73: p = '3';  break;
											case 0xFF74: p = '4';  break;
											case 0xFF75: p = '5';  break;

											case 0xFF67: p = '!';  break;
											case 0xFF68: p = '\"'; break;
											case 0xFF69: p = '#';  break;
											case 0xFF6A: p = '$';  break;
											case 0xFF6B: p = '%';  break;

											case 0xFF6F: p = 'c' | 0x80; break;

											case 0xFF76: p = 'q';  break;
											case 0xFF77: p = 'w';  break;
											case 0xFF78: p = 'e';  break;
											case 0xFF79: p = 'r';  break;
											case 0xFF7A: p = 't';  break;

											case 0xFF9E: 
												
												if (Length())
													Back() |= 0x100;

												continue;

											case 0xFF9F: 

												if (Length())
													Back() |= 0x80;

												continue;

											case 0xFF7B: p = 'a';  break;
											case 0xFF7C: p = 's';  break;
											case 0xFF7D: p = 'd';  break;
											case 0xFF7E: p = 'f';  break;
											case 0xFF7F: p = 'g';  break;

											case 0xFF80: p = 'z';  break;
											case 0xFF81: p = 'x';  break;
											case 0xFF82: p = 'c';  break;
											case 0xFF83: p = 'v';  break;
											case 0xFF84: p = 'b';  break;

											case 0xFF85: p = '6';  break;
											case 0xFF86: p = '7';  break;
											case 0xFF87: p = '8';  break;
											case 0xFF88: p = '9';  break;
											case 0xFF89: p = '0';  break;

											case 0xFF8A: p = 'y';  break;
											case 0xFF8B: p = 'u';  break;
											case 0xFF8C: p = 'i';  break;
											case 0xFF8D: p = 'o';  break;
											case 0xFF8E: p = 'p';  break;

											case 0xFF8F: p = 'h';  break;
											case 0xFF90: p = 'j';  break;
											case 0xFF91: p = 'k';  break;
											case 0xFF92: p = 'l';  break;
											case 0xFF93: p = ';';  break;

											case 0xFF94: p = 'n';  break;
											case 0xFF95: p = 'm';  break;
											case 0xFF96: p = ',';  break;

											case 0xFF6C: p = 'n' | 0x80; break;
											case 0xFF6D: p = 'm' | 0x80; break;
											case 0xFF6E: p = '<';        break;

											case 0xFF97: p = '-';  break;
											case 0xFF98: p = '^';  break;
											case 0xFF99: p = '\\'; break;
											case 0xFF9A: p = '@';  break;
											case 0xFF9B: p = '[';  break;

											case 0xFF9C: p = '.';  break;
											case 0xFF66: p = '-';  break;
											case 0xFF9D: p = '\t'; break;

											case 0xFF61: p = ']';        break;
											case 0xFF62: p = '[' | 0x80; break;
											case 0xFF63: p = ']' | 0x80; break;
											
											default: continue;										
										}

										mode = true;
									}
									else
									{
										continue;										
									}
								}

								if (kana != mode)
								{
									kana = mode;
									Heap::operator << (char(0xFF));
								}

								Heap::operator << (p);
							}

							if (kana)
								Heap::operator << (char(0xFF));

							::GlobalUnlock( handle );
						}
					}

					::CloseClipboard();

					if (Length())
					{
						Io::Screen() << Resource::String(IDS_SCREEN_TEXT_PASTE_1) 
							         << ' '
							         << Length()
									 << ' '
									 << Resource::String(IDS_SCREEN_TEXT_PASTE_2);
					}
				}
			}
		}
		else
		{
			if (keyboard[DIK_ESCAPE] & 0x80)
				Clear();
		}

		return Length();
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("t", on)
    #endif

	inline bool Input::AutoFire::Signaled() const
	{
		return step >= signal;
	}

	inline bool Input::ClipBoard::Shifted() const
	{
		return shifted;
	}

	inline void Input::ClipBoard::Shift()
	{
		shifted = true;
	}

	inline uint Input::ClipBoard::operator * ()
	{
		return releasing ? --releasing, UINT_MAX : (*this)[pos];
	}

	void Input::ClipBoard::operator ++ ()
	{
		hold = (hold + 1) & 7;

		if (!hold)
		{
			shifted = false;
			releasing = 32;

			if (++pos == Length())
				Clear();
		}
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("", on)
    #endif

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
			{ IDM_MACHINE_INPUT_AUTOSELECT,	          	 &Input::OnCmdMachineAutoSelectController },
			{ IDM_MACHINE_INPUT_PORT1_UNCONNECTED,		 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT1_PAD1,				 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT1_PAD2,				 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT1_PAD3,				 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT1_PAD4,				 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT1_ZAPPER,			 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT1_PADDLE,			 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT1_POWERPAD,			 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT2_UNCONNECTED,		 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT2_PAD1,				 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT2_PAD2,				 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT2_PAD3,				 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT2_PAD4,				 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT2_ZAPPER,			 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT2_PADDLE,			 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT2_POWERPAD,			 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT3_UNCONNECTED,		 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT3_PAD1,				 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT3_PAD2,				 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT3_PAD3,				 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT3_PAD4,				 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT4_UNCONNECTED,		 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT4_PAD1,				 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT4_PAD2,				 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT4_PAD3,				 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_PORT4_PAD4,				 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_EXP_UNCONNECTED,		 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_EXP_FAMILYTRAINER,       &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_EXP_FAMILYBASICKEYBOARD, &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_EXP_SUBORKEYBOARD,       &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_EXP_DOREMIKKOKEYBOARD,   &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_EXP_PADDLE,				 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_EXP_OEKAKIDSTABLET,		 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_EXP_HYPERSHOT,			 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_EXP_CRAZYCLIMBER,		 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_EXP_MAHJONG,			 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_EXP_EXCITINGBOXING,		 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_EXP_TOPRIDER,			 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_EXP_POKKUNMOGURAA,		 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_INPUT_EXP_PARTYTAP,	     	 &Input::OnCmdMachinePort				  },
			{ IDM_MACHINE_EXT_KEYBOARD_PASTE,		     &Input::OnCmdMachineKeyboardPaste	      },
			{ IDM_OPTIONS_INPUT,                         &Input::OnCmdOptionsInput                }
		};

		static const Window::Menu::PopupHandler::Entry<Input> popups[] =
		{	  
			{ Window::Menu::PopupHandler::Pos<IDM_POS_MACHINE,IDM_POS_MACHINE_EXT,IDM_POS_MACHINE_EXT_KEYBOARD>::ID, &Input::OnMenuKeyboard }
		};

		menu.Commands().Add( this, commands );
		menu.PopupRouter().Add( this, popups );
		emulator.Events().Add( this, &Input::OnEmuEvent );

		{
			HeapString name;
			menu[IDM_MACHINE_EXT_KEYBOARD_PASTE].Text() >> name;
			menu[IDM_MACHINE_EXT_KEYBOARD_PASTE].Text() << (name << '\t' << System::Keyboard::GetName( VK_F12 ));
		}

		typedef Nes::Input::Controllers Controllers;

		Controllers::Pad::callback.Set               ( &Callbacks::PollPad,               this   );
		Controllers::Zapper::callback.Set            ( &Callbacks::PollZapper,            &rects );
		Controllers::Paddle::callback.Set            ( &Callbacks::PollPaddle,            &rects );
		Controllers::PowerPad::callback.Set          ( &Callbacks::PollPowerPad,          this   );
		Controllers::OekaKidsTablet::callback.Set    ( &Callbacks::PollOekaKidsTablet,    &rects );
		Controllers::HyperShot::callback.Set         ( &Callbacks::PollHyperShot,         this   );
		Controllers::FamilyTrainer::callback.Set     ( &Callbacks::PollFamilyTrainer,     this   );
		Controllers::FamilyKeyboard::callback.Set    ( &Callbacks::PollFamilyKeyboard,    this   );
		Controllers::SuborKeyboard::callback.Set     ( &Callbacks::PollSuborKeyboard,     this   );
		Controllers::DoremikkoKeyboard::callback.Set ( &Callbacks::PollDoremikkoKeyboard, this   );
		Controllers::CrazyClimber::callback.Set      ( &Callbacks::PollCrazyClimber,      this   );
		Controllers::Mahjong::callback.Set           ( &Callbacks::PollMahjong,           this   );
		Controllers::ExcitingBoxing::callback.Set    ( &Callbacks::PollExcitingBoxing,    this   );
		Controllers::TopRider::callback.Set          ( &Callbacks::PollTopRider,          this   );
		Controllers::PokkunMoguraa::callback.Set     ( &Callbacks::PollPokkunMoguraa,     this   );
		Controllers::PartyTap::callback.Set          ( &Callbacks::PollPartyTap,          this   );
		Controllers::VsSystem::callback.Set          ( &Callbacks::PollVsSystem,          this   );

		menu[IDM_MACHINE_INPUT_AUTOSELECT].Check( cfg["machine autoselect controllers"] != Application::Configuration::NO );	

		{
			NST_COMPILE_ASSERT
			(
     			Emulator::EVENT_PORT2_CONTROLLER - Emulator::EVENT_PORT1_CONTROLLER == 1 &&
				Emulator::EVENT_PORT3_CONTROLLER - Emulator::EVENT_PORT1_CONTROLLER == 2 &&
				Emulator::EVENT_PORT4_CONTROLLER - Emulator::EVENT_PORT1_CONTROLLER == 3
			);

			String::Stack<16,char> string("machine port #");

			for (uint i=0; i < 5; ++i)
			{
				string[13] = (char) ('1' + i);
				const GenericString type( cfg[string] );

				Nes::Input::Type controller = Nes::Input::UNCONNECTED;

				switch (i)
				{
					case 0:
					case 1:

						     if (type == _T( "zapper"   )) { controller = Nes::Input::ZAPPER;   break; }
						else if (type == _T( "paddle"   )) { controller = Nes::Input::PADDLE;   break; }
						else if (type == _T( "powerpad" )) { controller = Nes::Input::POWERPAD; break; }

					case 2:
					case 3:

     						 if (type == _T( "pad1"        )) controller = Nes::Input::PAD1;
						else if (type == _T( "pad2"        )) controller = Nes::Input::PAD2;
						else if (type == _T( "pad3"        )) controller = Nes::Input::PAD3;
						else if (type == _T( "pad4"        )) controller = Nes::Input::PAD4;
						else if (type == _T( "unconnected" )) controller = Nes::Input::UNCONNECTED;
						else if (i == 0                     ) controller = Nes::Input::PAD1;
						else if (i == 1                     ) controller = Nes::Input::PAD2;

						break;

					case 4:

       						 if (type == _T( "paddle"            )) controller = Nes::Input::PADDLE;
       					else if (type == _T( "familytrainer"     )) controller = Nes::Input::FAMILYTRAINER;
       					else if (type == _T( "familykeyboard"    )) controller = Nes::Input::FAMILYKEYBOARD;
						else if (type == _T( "suborkeyboard"     )) controller = Nes::Input::SUBORKEYBOARD;
						else if (type == _T( "doremikkokeyboard" )) controller = Nes::Input::DOREMIKKOKEYBOARD;
						else if (type == _T( "oekakidstablet"    )) controller = Nes::Input::OEKAKIDSTABLET;
						else if (type == _T( "hypershot"         )) controller = Nes::Input::HYPERSHOT;
						else if (type == _T( "crazyclimber"      )) controller = Nes::Input::CRAZYCLIMBER;
						else if (type == _T( "mahjong"           )) controller = Nes::Input::MAHJONG;
						else if (type == _T( "excitingboxing"    )) controller = Nes::Input::EXCITINGBOXING;
						else if (type == _T( "toprider"          )) controller = Nes::Input::TOPRIDER;
						else if (type == _T( "pokkunmoguraa"     )) controller = Nes::Input::POKKUNMOGURAA;
						else if (type == _T( "partytap"          )) controller = Nes::Input::PARTYTAP;
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
			String::Stack<16,char> string("machine port #");

			for (uint i=0; i < 5; ++i)
			{
				tstring type;

				switch (Nes::Input(emulator).GetConnectedController( i ))
				{
    				case Nes::Input::PAD1:              type = _T( "pad1"              ); break;
					case Nes::Input::PAD2:              type = _T( "pad2"              ); break;
					case Nes::Input::PAD3:              type = _T( "pad3"              ); break;
					case Nes::Input::PAD4:              type = _T( "pad4"              ); break;
					case Nes::Input::ZAPPER:            type = _T( "zapper"            ); break;
					case Nes::Input::PADDLE:            type = _T( "paddle"            ); break;
					case Nes::Input::POWERPAD:          type = _T( "powerpad"          ); break;
					case Nes::Input::FAMILYTRAINER:     type = _T( "familytrainer"     ); break;
					case Nes::Input::FAMILYKEYBOARD:    type = _T( "familykeyboard"    ); break;
					case Nes::Input::SUBORKEYBOARD:     type = _T( "suborkeyboard"     ); break;
					case Nes::Input::DOREMIKKOKEYBOARD: type = _T( "doremikkokeyboard" ); break;
					case Nes::Input::OEKAKIDSTABLET:    type = _T( "oekakidstablet"    ); break;
					case Nes::Input::HYPERSHOT:         type = _T( "hypershot"         ); break;
					case Nes::Input::CRAZYCLIMBER:      type = _T( "crazyclimber"      ); break;
					case Nes::Input::MAHJONG:           type = _T( "mahjong"           ); break;
					case Nes::Input::EXCITINGBOXING:    type = _T( "excitingboxing"    ); break;
					case Nes::Input::TOPRIDER:          type = _T( "toprider"          ); break;
					case Nes::Input::POKKUNMOGURAA:     type = _T( "pokkunmoguraa"     ); break;
					case Nes::Input::PARTYTAP:          type = _T( "partytap"          ); break;
					default:                            type = _T( "unconnected"       ); break;
				}

				string[13] = (char) ('1' + i);
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
		clipBoard.Clear();
		directInput.Unacquire();
		cursor.Unacquire();
		Window::Menu::EnableAccelerators( TRUE );
	}

	void Input::UpdateDevices()
	{
		if (emulator.Is(Nes::Machine::ON,Nes::Machine::GAME))
		{
			cursor.Acquire( emulator );

			Window::Menu::EnableAccelerators
			( 
		     	!Nes::Input(emulator).IsAnyControllerConnected
				(
			    	Nes::Input::FAMILYKEYBOARD,
					Nes::Input::SUBORKEYBOARD,
					Nes::Input::DOREMIKKOKEYBOARD
				) 
			);
		}
	}

	void Input::UpdateSettings()
	{
		autoFire = dialog->GetSettings().AutoFireSpeed();

		for (uint i=0; i < 4; ++i)
			nesControllers.pad[i].allowSimulAxes = dialog->GetSettings().AllowSimulAxes();

		typedef Window::Input::Settings Settings;

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
			{ Settings::MACHINE_KEYS + Settings::MACHINE_KEY_CHANGE_DISK_SIDE,	 IDM_MACHINE_EXT_FDS_CHANGE_SIDE          }, 
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
			dialog->GetSettings().GetKey( lut[i][0] ).GetVirtKey( accel[i] );
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
				uint id;

				switch (emulator.GetController(0))
				{
					case Nes::Input::PAD1:	   id = IDM_MACHINE_INPUT_PORT1_PAD1;        break;
					case Nes::Input::PAD2:	   id = IDM_MACHINE_INPUT_PORT1_PAD2;        break;
					case Nes::Input::PAD3:	   id = IDM_MACHINE_INPUT_PORT1_PAD3;        break;
					case Nes::Input::PAD4:	   id = IDM_MACHINE_INPUT_PORT1_PAD4;        break;
					case Nes::Input::ZAPPER:   id = IDM_MACHINE_INPUT_PORT1_ZAPPER;      break;
					case Nes::Input::PADDLE:   id = IDM_MACHINE_INPUT_PORT1_PADDLE;      break;
					case Nes::Input::POWERPAD: id = IDM_MACHINE_INPUT_PORT1_POWERPAD;    break;
					default:				   id = IDM_MACHINE_INPUT_PORT1_UNCONNECTED; break;
				}

				menu[id].Check( IDM_MACHINE_INPUT_PORT1_UNCONNECTED, IDM_MACHINE_INPUT_PORT1_POWERPAD );		
				UpdateDevices();
				break;
			}
		
			case Emulator::EVENT_PORT2_CONTROLLER:
			{
				uint id;

				switch (emulator.GetController(1))
				{
					case Nes::Input::PAD1:	   id = IDM_MACHINE_INPUT_PORT2_PAD1;        break;         
					case Nes::Input::PAD2:	   id = IDM_MACHINE_INPUT_PORT2_PAD2;        break;         
					case Nes::Input::PAD3:	   id = IDM_MACHINE_INPUT_PORT2_PAD3;        break;         
					case Nes::Input::PAD4:	   id = IDM_MACHINE_INPUT_PORT2_PAD4;        break;         
					case Nes::Input::ZAPPER:   id = IDM_MACHINE_INPUT_PORT2_ZAPPER;      break;     
					case Nes::Input::PADDLE:   id = IDM_MACHINE_INPUT_PORT2_PADDLE;      break;     
					case Nes::Input::POWERPAD: id = IDM_MACHINE_INPUT_PORT2_POWERPAD;    break;     
					default:	               id = IDM_MACHINE_INPUT_PORT2_UNCONNECTED; break;
				}
		
				menu[id].Check( IDM_MACHINE_INPUT_PORT2_UNCONNECTED, IDM_MACHINE_INPUT_PORT2_POWERPAD );		
				UpdateDevices();
				break;
			}
		
			case Emulator::EVENT_PORT3_CONTROLLER:
			{
				uint id;

				switch (emulator.GetController(2))
				{
					case Nes::Input::PAD1: id = IDM_MACHINE_INPUT_PORT3_PAD1;        break;         
					case Nes::Input::PAD2: id = IDM_MACHINE_INPUT_PORT3_PAD2;        break;         
					case Nes::Input::PAD3: id = IDM_MACHINE_INPUT_PORT3_PAD3;        break;         
					case Nes::Input::PAD4: id = IDM_MACHINE_INPUT_PORT3_PAD4;        break;         
					default:	           id = IDM_MACHINE_INPUT_PORT3_UNCONNECTED; break;
				}
		
				menu[id].Check( IDM_MACHINE_INPUT_PORT3_UNCONNECTED, IDM_MACHINE_INPUT_PORT3_PAD4 );		
				UpdateDevices();
				break;
			}
		
			case Emulator::EVENT_PORT4_CONTROLLER:
			{
				uint id;

				switch (emulator.GetController(3))
				{
					case Nes::Input::PAD1: id = IDM_MACHINE_INPUT_PORT4_PAD1;        break;         
					case Nes::Input::PAD2: id = IDM_MACHINE_INPUT_PORT4_PAD2;        break;         
					case Nes::Input::PAD3: id = IDM_MACHINE_INPUT_PORT4_PAD3;        break;         
					case Nes::Input::PAD4: id = IDM_MACHINE_INPUT_PORT4_PAD4;        break;         
					default:	           id = IDM_MACHINE_INPUT_PORT4_UNCONNECTED; break;
				}
		
				menu[id].Check( IDM_MACHINE_INPUT_PORT4_UNCONNECTED, IDM_MACHINE_INPUT_PORT4_PAD4 );		
				UpdateDevices();
				break;
			}
		
			case Emulator::EVENT_PORT5_CONTROLLER:
			{
				uint id;

				switch (emulator.GetController(4))
				{
					case Nes::Input::FAMILYTRAINER:	    id = IDM_MACHINE_INPUT_EXP_FAMILYTRAINER;       break;      
					case Nes::Input::FAMILYKEYBOARD:	id = IDM_MACHINE_INPUT_EXP_FAMILYBASICKEYBOARD; break;
					case Nes::Input::SUBORKEYBOARD:	    id = IDM_MACHINE_INPUT_EXP_SUBORKEYBOARD;       break;      
					case Nes::Input::DOREMIKKOKEYBOARD: id = IDM_MACHINE_INPUT_EXP_DOREMIKKOKEYBOARD;   break;  
					case Nes::Input::PADDLE:			id = IDM_MACHINE_INPUT_EXP_PADDLE;              break;             
					case Nes::Input::OEKAKIDSTABLET:	id = IDM_MACHINE_INPUT_EXP_OEKAKIDSTABLET;      break;     
					case Nes::Input::HYPERSHOT:		    id = IDM_MACHINE_INPUT_EXP_HYPERSHOT;           break;          
					case Nes::Input::CRAZYCLIMBER:	    id = IDM_MACHINE_INPUT_EXP_CRAZYCLIMBER;        break;       
					case Nes::Input::MAHJONG:			id = IDM_MACHINE_INPUT_EXP_MAHJONG;             break;            
					case Nes::Input::EXCITINGBOXING:	id = IDM_MACHINE_INPUT_EXP_EXCITINGBOXING;      break;     
					case Nes::Input::TOPRIDER:		    id = IDM_MACHINE_INPUT_EXP_TOPRIDER;            break;           
					case Nes::Input::POKKUNMOGURAA:	    id = IDM_MACHINE_INPUT_EXP_POKKUNMOGURAA;       break;      
					case Nes::Input::PARTYTAP:		    id = IDM_MACHINE_INPUT_EXP_PARTYTAP;            break;           
					default:		                    id = IDM_MACHINE_INPUT_EXP_UNCONNECTED;         break;        
				}
		
				menu[id].Check( IDM_MACHINE_INPUT_EXP_UNCONNECTED, IDM_MACHINE_INPUT_EXP_PARTYTAP );		
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
				menu[IDM_POS_MACHINE][IDM_POS_MACHINE_INPUT][IDM_POS_MACHINE_INPUT_EXP].Enable( state );
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
			{ 0, Nes::Input::UNCONNECTED       },
			{ 0, Nes::Input::PAD1		       },
			{ 0, Nes::Input::PAD2		       },
			{ 0, Nes::Input::PAD3		       },
			{ 0, Nes::Input::PAD4		       },
			{ 0, Nes::Input::ZAPPER		       },
			{ 0, Nes::Input::PADDLE		       },
			{ 0, Nes::Input::POWERPAD	       },
			{ 1, Nes::Input::UNCONNECTED       },
			{ 1, Nes::Input::PAD1		       },
			{ 1, Nes::Input::PAD2		       },
			{ 1, Nes::Input::PAD3		       },
			{ 1, Nes::Input::PAD4		       },
			{ 1, Nes::Input::ZAPPER		       },
			{ 1, Nes::Input::PADDLE		       },
			{ 1, Nes::Input::POWERPAD	       },
			{ 2, Nes::Input::UNCONNECTED       },
			{ 2, Nes::Input::PAD1		       },
			{ 2, Nes::Input::PAD2		       },
			{ 2, Nes::Input::PAD3		       },
			{ 2, Nes::Input::PAD4		       },
			{ 3, Nes::Input::UNCONNECTED       },
			{ 3, Nes::Input::PAD1		       },
			{ 3, Nes::Input::PAD2		       },
			{ 3, Nes::Input::PAD3		       },
			{ 3, Nes::Input::PAD4		       },
			{ 4, Nes::Input::UNCONNECTED       },
			{ 4, Nes::Input::FAMILYTRAINER     },
			{ 4, Nes::Input::FAMILYKEYBOARD    },
			{ 4, Nes::Input::SUBORKEYBOARD     },
			{ 4, Nes::Input::DOREMIKKOKEYBOARD },
			{ 4, Nes::Input::PADDLE            },
			{ 4, Nes::Input::OEKAKIDSTABLET    },
			{ 4, Nes::Input::HYPERSHOT         },
			{ 4, Nes::Input::CRAZYCLIMBER      },
			{ 4, Nes::Input::MAHJONG           },
			{ 4, Nes::Input::EXCITINGBOXING    },
			{ 4, Nes::Input::TOPRIDER          },
			{ 4, Nes::Input::POKKUNMOGURAA     },
			{ 4, Nes::Input::PARTYTAP          }
		};

		const uchar* const offset = table[id - IDM_MACHINE_INPUT_PORT1_UNCONNECTED];
		emulator.ConnectController( offset[0], (Nes::Input::Type) offset[1] );
		Application::Instance::Post( Application::Instance::WM_NST_COMMAND_RESUME );
	}

	void Input::OnCmdMachineKeyboardPaste(uint)
	{		
		clipBoard.Paste();
		Application::Instance::Post( Application::Instance::WM_NST_COMMAND_RESUME );
	}

	void Input::OnMenuKeyboard(Window::Menu::PopupHandler::Param& param)
	{
		param.menu[IDM_MACHINE_EXT_KEYBOARD_PASTE].Enable
		( 
			Nes::Input(emulator).IsAnyControllerConnected( Nes::Input::FAMILYKEYBOARD, Nes::Input::SUBORKEYBOARD ) &&
			emulator.Is(Nes::Machine::GAME,Nes::Machine::ON) && 
			clipBoard.CanPaste() 
		);
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
		
		buttons = 0;		
		keys[ Settings::PAD_KEY_MIC ].GetState( buttons, Controllers::Pad::MIC );
		
		pad.mic = buttons;

		return true;
	}

	bool NST_CALLBACK Input::Callbacks::PollZapper(UserData data,Controllers::Zapper& zapper)
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

	bool NST_CALLBACK Input::Callbacks::PollOekaKidsTablet(UserData data,Controllers::OekaKidsTablet& tablet)
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

	bool NST_CALLBACK Input::Callbacks::PollFamilyTrainer(UserData data,Controllers::FamilyTrainer& familyTrainer)
	{
		static_cast<Input*>(data)->AutoPoll();

		const Key* NST_RESTRICT keys = static_cast<const Input*>(data)->dialog->GetSettings().GetKeys(Settings::POWERPAD_KEYS);

		for (uint i=0; i < Settings::POWERPAD_NUM_SIDE_A_KEYS; ++i)
			familyTrainer.sideA[i] = keys[i].GetState();

		keys += Settings::POWERPAD_NUM_SIDE_A_KEYS;

		for (uint i=0; i < Settings::POWERPAD_NUM_SIDE_B_KEYS; ++i)
			familyTrainer.sideB[i] = keys[i].GetState();

		return true;
	}

	bool NST_CALLBACK Input::Callbacks::PollFamilyKeyboard(UserData data,Controllers::FamilyKeyboard& keyboard,uint part,uint mode)
	{
		Input& input = *static_cast<Input*>(data);

		input.AutoPoll();
		const u8* const NST_RESTRICT buffer = input.directInput.GetKeyboardBuffer();

		uint key = 0;

        #define NST_2(a,b) (a | (u16(b) << 8))

		if (input.clipBoard.Query( buffer, ClipBoard::FAMILY ))
		{
			static const u16 asciiMap[Controllers::FamilyKeyboard::NUM_PARTS][Controllers::FamilyKeyboard::NUM_MODES][4] =
			{
				{		
					{ 0, '\r', NST_2('[','['|0x80), NST_2(']',']'|0x80) },
					{ 0xFF, 0, '\\', 0 }
				},
				{
					{ 0, '@', NST_2(':','*'), NST_2(';','+') },
					{ NST_2('\t','_'), NST_2('/','?'), NST_2('-','='), '^' }
				},
				{
					{ 0, NST_2('o','o'|0x80), 'l', 'k' },
					{ NST_2('.','>'), NST_2(',','<'), NST_2('p','p'|0x80), '0' }
				},
				{
					{ 0, NST_2('i','i'|0x80), NST_2('u','u'|0x80), 'j' },
					{ NST_2('m','m'|0x80), NST_2('n','n'|0x80), NST_2('9',')'), NST_2('8','(') }
				},
				{
					{ 0, NST_2('y','y'|0x80), 'g', 'h' },
					{ 'b', 'v', NST_2('7','\''), NST_2('6','&') }
				},
				{
					{ 0, 't', 'r', 'd' },
					{ 'f', NST_2('c','c'|0x80), NST_2('5','%'), NST_2('4','$') }
				},
				{
					{ 0, 'w', 's', 'a' },
					{ 'x', 'z', 'e', NST_2('3','#') }
				},
				{
					{ 0, 0, 'q', 0 },
					{ 0, 0xFE, NST_2('1','!'), NST_2('2','\"') }
				},
				{
					{ 0, 0, 0, 0 },
					{ 0, ' ', 0, 0 }
				},
			};

			if (input.clipBoard.Shifted() && part == 7 && mode == 1)
				key = 0x02;

			uint next = *input.clipBoard;

			if (next != UINT_MAX && (next & 0x100))
			{
				next &= 0xFF;

				if (part == 7 && mode == 1)
					key |= 0x04;
			}

			for (uint i=0; i < 4; ++i)
			{
				if (uint(asciiMap[part][mode][i] & 0xFF) == next)
				{
					key |= 1U << (i+1);
					++input.clipBoard;
					break;
				}
				else if (uint(asciiMap[part][mode][i] >> 8) == next)
				{
					if (input.clipBoard.Shifted())
					{
						key |= 1U << (i+1);
						++input.clipBoard;
					}
					else
					{
						input.clipBoard.Shift();
					}
					break;
				}
			}
		}
		else
		{								   
			static const u16 dikMap[Controllers::FamilyKeyboard::NUM_PARTS][Controllers::FamilyKeyboard::NUM_MODES][4] =
			{
				{		
					{ DIK_F8, NST_2(DIK_RETURN,DIK_NUMPADENTER), DIK_LBRACKET, DIK_RBRACKET },
					{ NST_2(DIK_KANA,DIK_F9), DIK_RSHIFT, NST_2(DIK_YEN,DIK_BACKSLASH), DIK_END }
				},
				{												 
					{ DIK_F7, NST_2(DIK_GRAVE,DIK_AT), NST_2(DIK_APOSTROPHE,DIK_COLON), DIK_SEMICOLON },
					{ NST_2(DIK_UNDERLINE,DIK_TAB), NST_2(DIK_SLASH,DIK_NUMPADSLASH), NST_2(DIK_MINUS,DIK_NUMPADMINUS), NST_2(DIK_EQUALS,DIK_CIRCUMFLEX) }
				},							  
				{
					{ DIK_F6, DIK_O, DIK_L, DIK_K },
					{ DIK_PERIOD, NST_2(DIK_COMMA,DIK_NUMPADCOMMA), DIK_P, NST_2(DIK_0,DIK_NUMPAD0) }
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
					{ DIK_F1, DIK_ESCAPE, DIK_Q, NST_2(DIK_LCONTROL,DIK_RCONTROL) },
					{ DIK_LSHIFT, 0, NST_2(DIK_1,DIK_NUMPAD1), NST_2(DIK_2,DIK_NUMPAD2) }
				},
				{
					{ DIK_HOME, DIK_UP, DIK_RIGHT, DIK_LEFT },
					{ DIK_DOWN, DIK_SPACE, NST_2(DIK_BACK,DIK_DELETE), DIK_INSERT }
				},
			};

			if (part == 7 && mode == 1)
				key |= (::GetKeyState(VK_CAPITAL) & 0x0001) << 2;

			for (uint i=0; i < 4; ++i)
			{
				const uint pushed = 0x80 &
				(
					buffer[dikMap[part][mode][i] & 0xFF] |
					buffer[dikMap[part][mode][i] >>   8]
				);

				key |= pushed >> ( 7 - ( i + 1 ) );
			}
		}

        #undef NST_2

		keyboard.parts[part] = (uchar) key;

		return true;
	}

	bool NST_CALLBACK Input::Callbacks::PollSuborKeyboard(UserData data,Controllers::SuborKeyboard& keyboard,uint part,uint mode)
	{
		Input& input = *static_cast<Input*>(data);

		input.AutoPoll();
		const u8* const NST_RESTRICT buffer = input.directInput.GetKeyboardBuffer();

		uint key = 0;

        #define NST_2(a,b) (a | (u16(b) << 8))

		if (input.clipBoard.Query( buffer, ClipBoard::SUBOR ))
		{
			static const u16 asciiMap[Controllers::SuborKeyboard::NUM_PARTS][Controllers::SuborKeyboard::NUM_MODES][4] =
			{
				{
					{ NST_2('4','$'), 'g', 'f', 'c' }, 
					{ 0, 'e', NST_2('5','%'), 'v' }
				}, 
				{
					{ NST_2('2','@'), 'd', 's', 0 }, 
					{ 0, 'w', NST_2('3','#'), 'x' }
				}, 
				{
					{ 0, 0, 0, 0 }, 
					{ 0, 0, 0, 0 }
				}, 
				{
					{ NST_2('9','('), 'i', 'l', NST_2(',','<') }, 
					{ 0, 'o', NST_2('0',')'), NST_2('.','>') }
				}, 
				{
					{ NST_2(']','}'), '\r', 0, 0 }, 
					{ 0, NST_2('[','{'), NST_2('\\','|'), 0 }
				}, 
				{
					{ 'q', 0, 'z', 0 }, 
					{ 0, 'a', NST_2('1','!'), 0 }
				}, 
				{
					{ NST_2('7','&'), 'y', 'k', 'm' }, 
					{ 0, 'u', NST_2('8','*'), 'j' }
				}, 
				{
					{ NST_2('-','_'), NST_2(';',':'), NST_2('\'','\"'), NST_2('/','?') }, 
					{ 0, 'p', NST_2('=','+'), 0 }
				}, 
				{
					{ 't', 'h', 'n', ' ' }, 
					{ 0, 'r', NST_2('6','^'), 'b' }
				}, 
				{	
					{ 0, 0, 0, 0 }, 
					{ 0, 0, 0, 0 }
				}
			};

			if (input.clipBoard.Shifted() && part == 7 && mode == 1)
				key = 0x10;

			const uint next = *input.clipBoard;

			for (uint i=0; i < 4; ++i)
			{
				if (uint(asciiMap[part][mode][i] & 0xFF) == next)
				{
					key |= 1U << (i+1);
					++input.clipBoard;
					break;
				}
				else if (uint(asciiMap[part][mode][i] >> 8) == next)
				{
					if (input.clipBoard.Shifted())
					{
						key |= 1U << (i+1);
						++input.clipBoard;
					}
					else
					{
						input.clipBoard.Shift();
					}
					break;
				}
			}
		}
		else
		{
			static const u16 dikMap[Controllers::SuborKeyboard::NUM_PARTS][Controllers::SuborKeyboard::NUM_MODES][4] =
			{
				{
					{ NST_2(DIK_4,DIK_NUMPAD4), DIK_G, DIK_F, DIK_C }, 
     	 			{ DIK_F2, DIK_E, NST_2(DIK_5,DIK_NUMPAD5), DIK_V }
				}, 
				{
					{ NST_2(DIK_2,DIK_NUMPAD2), DIK_D, DIK_S, DIK_END }, 
					{ DIK_F1, DIK_W, NST_2(DIK_3,DIK_NUMPAD3), DIK_X }
				}, 
				{
					{ DIK_INSERT, DIK_BACK, DIK_NEXT, DIK_RIGHT }, 
					{ DIK_F8, DIK_PRIOR, DIK_DELETE, DIK_HOME }
				}, 
				{
					{ NST_2(DIK_9,DIK_NUMPAD9), DIK_I, DIK_L, NST_2(DIK_COMMA,DIK_NUMPADCOMMA) }, 
					{ DIK_F5, DIK_O, NST_2(DIK_0,DIK_NUMPAD0), DIK_PERIOD }
				}, 
				{
					{ DIK_RBRACKET, NST_2(DIK_RETURN,DIK_NUMPADENTER), DIK_UP, DIK_LEFT }, 
					{ DIK_F7, DIK_LBRACKET, NST_2(DIK_BACKSLASH,DIK_YEN), DIK_DOWN }
				}, 
				{
					{ DIK_Q, DIK_CAPITAL, DIK_Z, DIK_TAB }, 
					{ DIK_ESCAPE, DIK_A, NST_2(DIK_1,DIK_NUMPAD1), NST_2(DIK_LCONTROL,DIK_RCONTROL) }
				}, 
				{
					{ NST_2(DIK_7,DIK_NUMPAD7), DIK_Y, DIK_K, DIK_M }, 
					{ DIK_F4, DIK_U, NST_2(DIK_8,DIK_NUMPAD8), DIK_J }
				}, 
				{
					{ NST_2(DIK_MINUS,DIK_NUMPADMINUS), DIK_SEMICOLON, NST_2(DIK_APOSTROPHE,DIK_COLON), NST_2(DIK_SLASH,DIK_NUMPADSLASH) }, 
					{ DIK_F6, DIK_P, NST_2(DIK_EQUALS,DIK_CIRCUMFLEX), NST_2(DIK_LSHIFT,DIK_RSHIFT) }
				}, 
				{
					{ DIK_T, DIK_H, DIK_N, DIK_SPACE }, 
					{ DIK_F3, DIK_R, NST_2(DIK_6,DIK_NUMPAD6), DIK_B }
				}, 
				{	
					{ NST_2(DIK_GRAVE,DIK_AT), 0, 0, 0 }, 
					{ 0, 0, 0, 0 }
				}
			};

			for (uint i=0; i < 4; ++i)
			{
				const uint pushed = 0x80 &
				(
					buffer[dikMap[part][mode][i] & 0xFF] |
					buffer[dikMap[part][mode][i] >>   8]
				);

				key |= pushed >> ( 7 - ( i + 1 ) );
			}
		}

        #undef NST_2

		keyboard.parts[part] = (uchar) key;

		return true;
	}

	bool NST_CALLBACK Input::Callbacks::PollDoremikkoKeyboard(UserData data,Controllers::DoremikkoKeyboard& doremikko,const uint part,const uint mode)
	{
		typedef Controllers::DoremikkoKeyboard Note;

		static_cast<Input*>(data)->AutoPoll();

		const u8* const NST_RESTRICT keyboard = static_cast<const Input*>(data)->directInput.GetKeyboardBuffer();

		uint keys = 0;

		switch (part)
		{
     		case Note::PART_1:
			
				if (mode == Note::MODE_B)
				{
					if (keyboard[ DIK_Z ] & 0x80) keys |= Note::MODE_B_0;
					if (keyboard[ DIK_S ] & 0x80) keys |= Note::MODE_B_1;
				}			
		     	break;
		
			case Note::PART_2:
			
				if (mode == Note::MODE_A)
				{
					if (keyboard[ DIK_X ] & 0x80) keys |= Note::MODE_A_0;
					if (keyboard[ DIK_D ] & 0x80) keys |= Note::MODE_A_1;
					if (keyboard[ DIK_C ] & 0x80) keys |= Note::MODE_A_2;
					if (keyboard[ DIK_V ] & 0x80) keys |= Note::MODE_A_3;
				}
				else
				{
					if (keyboard[ DIK_G ] & 0x80) keys |= Note::MODE_B_0;
					if (keyboard[ DIK_B ] & 0x80) keys |= Note::MODE_B_1;
				}			
	     		break;
		
			case Note::PART_3:
			
				if (mode == Note::MODE_A)
				{
					if (keyboard[ DIK_H ] & 0x80) keys |= Note::MODE_A_0;
					if (keyboard[ DIK_N ] & 0x80) keys |= Note::MODE_A_1;
					if (keyboard[ DIK_J ] & 0x80) keys |= Note::MODE_A_2;
					if (keyboard[ DIK_M ] & 0x80) keys |= Note::MODE_A_3;
				}
				else
				{
					if (keyboard[ DIK_COMMA ] & 0x80) keys |= Note::MODE_B_0;
					if (keyboard[ DIK_L     ] & 0x80) keys |= Note::MODE_B_1;
				}			
     			break;
		
			case Note::PART_4:
			
				if (mode == Note::MODE_A)
				{
					if (keyboard[ DIK_PERIOD    ] & 0x80) keys |= Note::MODE_A_0;
					if (keyboard[ DIK_SEMICOLON ] & 0x80) keys |= Note::MODE_A_1;
					if (keyboard[ DIK_SLASH     ] & 0x80) keys |= Note::MODE_A_2;
					if (keyboard[ DIK_Q         ] & 0x80) keys |= Note::MODE_A_3;
				}
				else
				{
					if (keyboard[ DIK_2 ] & 0x80) keys |= Note::MODE_B_0;
					if (keyboard[ DIK_W ] & 0x80) keys |= Note::MODE_B_1;
				}			
       			break;
		
			case Note::PART_5:
			
				if (mode == Note::MODE_A)
				{
					if (keyboard[ DIK_3 ] & 0x80) keys |= Note::MODE_A_0;
					if (keyboard[ DIK_E ] & 0x80) keys |= Note::MODE_A_1;
					if (keyboard[ DIK_4 ] & 0x80) keys |= Note::MODE_A_2;
					if (keyboard[ DIK_R ] & 0x80) keys |= Note::MODE_A_3;
				}
				else
				{
					if (keyboard[ DIK_T ] & 0x80) keys |= Note::MODE_B_0;
					if (keyboard[ DIK_6 ] & 0x80) keys |= Note::MODE_B_1;
				}			
     			break;
		
			case Note::PART_6:
	
				if (mode == Note::MODE_A)
				{
					if (keyboard[ DIK_Y ] & 0x80) keys |= Note::MODE_A_0;
					if (keyboard[ DIK_7 ] & 0x80) keys |= Note::MODE_A_1;
					if (keyboard[ DIK_U ] & 0x80) keys |= Note::MODE_A_2;
					if (keyboard[ DIK_I ] & 0x80) keys |= Note::MODE_A_3;
				}
				else
				{
					if (keyboard[ DIK_9 ] & 0x80) keys |= Note::MODE_B_0;
					if (keyboard[ DIK_O ] & 0x80) keys |= Note::MODE_B_1;
				}			
     			break;
		
			case Note::PART_7:

				if (mode == Note::MODE_A)
				{
					if (keyboard[ DIK_0     ] & 0x80) keys |= Note::MODE_A_0;
					if (keyboard[ DIK_P     ] & 0x80) keys |= Note::MODE_A_1;
					if (keyboard[ DIK_MINUS ] & 0x80) keys |= Note::MODE_A_2;
					if (keyboard[ DIK_AT    ] & 0x80) keys |= Note::MODE_A_3;
				}			
     			break;
		}	

		doremikko.keys = keys;

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

	bool NST_CALLBACK Input::Callbacks::PollPartyTap(UserData data,Controllers::PartyTap& partyTap)
	{
		static_cast<Input*>(data)->AutoPoll();

		const Key* const NST_RESTRICT keys = static_cast<const Input*>(data)->dialog->GetSettings().GetKeys(Settings::PARTYTAP_KEYS);

		uint units = 0;

		keys[ Settings::PARTYTAP_UNIT_1 ].GetState( units, Controllers::PartyTap::UNIT_1 );
		keys[ Settings::PARTYTAP_UNIT_2 ].GetState( units, Controllers::PartyTap::UNIT_2 );
		keys[ Settings::PARTYTAP_UNIT_3 ].GetState( units, Controllers::PartyTap::UNIT_3 );
		keys[ Settings::PARTYTAP_UNIT_4 ].GetState( units, Controllers::PartyTap::UNIT_4 );
		keys[ Settings::PARTYTAP_UNIT_5 ].GetState( units, Controllers::PartyTap::UNIT_5 );
		keys[ Settings::PARTYTAP_UNIT_6 ].GetState( units, Controllers::PartyTap::UNIT_6 );

		partyTap.units = units;

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
