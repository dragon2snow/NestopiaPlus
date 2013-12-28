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
#include "NstResourceCursor.hpp"
#include "NstIoScreen.hpp"
#include "NstSystemKeyboard.hpp"
#include "NstResourceString.hpp"
#include "NstWindowMenu.hpp"
#include "NstWindowParam.hpp"
#include "NstManagerEmulator.hpp"
#include "NstDialogInput.hpp"
#include "NstManagerInput.hpp"

namespace Nestopia
{
	namespace Managers
	{
		class Input::Callbacks
		{
		public:

			typedef Nes::Input::UserData UserData;
			typedef Window::Input::Settings Settings;
			typedef Settings::Key Key;

			typedef Nes::Input::Controllers::Pad               Pad;
			typedef Nes::Input::Controllers::Zapper            Zapper;
			typedef Nes::Input::Controllers::Paddle            Paddle;
			typedef Nes::Input::Controllers::PowerPad          PowerPad;
			typedef Nes::Input::Controllers::Mouse             Mouse;
			typedef Nes::Input::Controllers::OekaKidsTablet    OekaKidsTablet;
			typedef Nes::Input::Controllers::HyperShot         HyperShot;
			typedef Nes::Input::Controllers::CrazyClimber      CrazyClimber;
			typedef Nes::Input::Controllers::FamilyTrainer     FamilyTrainer;
			typedef Nes::Input::Controllers::FamilyKeyboard    FamilyKeyboard;
			typedef Nes::Input::Controllers::SuborKeyboard     SuborKeyboard;
			typedef Nes::Input::Controllers::DoremikkoKeyboard DoremikkoKeyboard;
			typedef Nes::Input::Controllers::HoriTrack         HoriTrack;
			typedef Nes::Input::Controllers::Pachinko          Pachinko;
			typedef Nes::Input::Controllers::Mahjong           Mahjong;
			typedef Nes::Input::Controllers::ExcitingBoxing    ExcitingBoxing;
			typedef Nes::Input::Controllers::TopRider          TopRider;
			typedef Nes::Input::Controllers::PokkunMoguraa     PokkunMoguraa;
			typedef Nes::Input::Controllers::PartyTap          PartyTap;
			typedef Nes::Input::Controllers::VsSystem          VsSystem;
			typedef Nes::Input::Controllers::KaraokeStudio     KaraokeStudio;

			static bool NST_CALLBACK PollPad               (UserData,Pad&,uint);
			static bool NST_CALLBACK PollZapper            (UserData,Zapper&);
			static bool NST_CALLBACK PollPaddle            (UserData,Paddle&);
			static bool NST_CALLBACK PollPowerPad          (UserData,PowerPad&);
			static bool NST_CALLBACK PollMouse             (UserData,Mouse&);
			static bool NST_CALLBACK PollOekaKidsTablet    (UserData,OekaKidsTablet&);
			static bool NST_CALLBACK PollHyperShot         (UserData,HyperShot&);
			static bool NST_CALLBACK PollCrazyClimber      (UserData,CrazyClimber&);
			static bool NST_CALLBACK PollFamilyTrainer     (UserData,FamilyTrainer&);
			static bool NST_CALLBACK PollFamilyKeyboard    (UserData,FamilyKeyboard&,uint,uint);
			static bool NST_CALLBACK PollSuborKeyboard     (UserData,SuborKeyboard&,uint,uint);
			static bool NST_CALLBACK PollDoremikkoKeyboard (UserData,DoremikkoKeyboard&,uint,uint);
			static bool NST_CALLBACK PollHoriTrack         (UserData,HoriTrack&);
			static bool NST_CALLBACK PollPachinko          (UserData,Pachinko&);
			static bool NST_CALLBACK PollMahjong           (UserData,Mahjong&,uint);
			static bool NST_CALLBACK PollExcitingBoxing    (UserData,ExcitingBoxing&,uint);
			static bool NST_CALLBACK PollTopRider          (UserData,TopRider&);
			static bool NST_CALLBACK PollPokkunMoguraa     (UserData,PokkunMoguraa&,uint);
			static bool NST_CALLBACK PollPartyTap          (UserData,PartyTap&);
			static bool NST_CALLBACK PollVsSystem          (UserData,VsSystem&);
			static bool NST_CALLBACK PollKaraokeStudio     (UserData,KaraokeStudio&);

		private:

			static void PollCursor(UserData,uint&,uint&,uint&);
		};

		const Resource::Cursor Input::Cursor::gun( IDC_CURSOR_GUN  );

		const uint Input::Cursor::primaryButtonId = ::GetSystemMetrics( SM_SWAPBUTTON ) ? VK_RBUTTON : VK_LBUTTON;
		const uint Input::Cursor::secondaryButtonId = ::GetSystemMetrics( SM_SWAPBUTTON ) ? VK_LBUTTON : VK_RBUTTON;

		Input::Cursor::Cursor(Window::Custom& w)
		:
		hCursor     ( Resource::Cursor::GetArrow() ),
		hCurrent    ( Resource::Cursor::GetArrow() ),
		deadline    ( DWORD(~0UL) ),
		window      ( w ),
		wheel       ( WHEEL_MAX )
		{
			static const Window::MsgHandler::Entry<Input::Cursor> messages[] =
			{
				{ WM_SETCURSOR,   &Cursor::OnSetCursor      },
				{ WM_MOUSEMOVE,   &Cursor::OnNop            },
				{ WM_LBUTTONDOWN, &Cursor::OnNop            },
				{ WM_RBUTTONDOWN, &Cursor::OnRButtonDownNop },
				{ WM_LBUTTONUP,   &Cursor::OnNop            },
				{ WM_RBUTTONUP,   &Cursor::OnNop            },
				{ WM_MOUSEWHEEL,  &Cursor::OnWheel          }
			};

			window.Messages().Add( this, messages );
		}

		void Input::Cursor::Acquire(Emulator& emulator)
		{
			ibool autoHide = false;

			if (emulator.ControllerConnected( Nes::Input::ZAPPER ))
			{
				hCursor = gun;
			}
			else if
			(
				emulator.ControllerConnected( Nes::Input::PADDLE ) ||
				emulator.ControllerConnected( Nes::Input::HORITRACK ) ||
				emulator.ControllerConnected( Nes::Input::PACHINKO ) ||
				emulator.ControllerConnected( Nes::Input::OEKAKIDSTABLET ) ||
				emulator.ControllerConnected( Nes::Input::MOUSE )
			)
			{
				hCursor = NULL;
			}
			else
			{
				hCursor = Resource::Cursor::GetArrow();
				autoHide = true;
			}

			Window::MsgHandler& router = window.Messages();

			router[ WM_MOUSEMOVE   ].Set( this, autoHide ? &Cursor::OnMouseMove : &Cursor::OnNop );
			router[ WM_LBUTTONDOWN ].Set( this, autoHide ? &Cursor::OnLButtonDown: &Cursor::OnNop );
			router[ WM_LBUTTONUP   ].Set( this, autoHide ? &Cursor::OnButtonUp : &Cursor::OnNop );
			router[ WM_RBUTTONDOWN ].Set( this, autoHide ? &Cursor::OnRButtonDown : hCursor != gun ? &Cursor::OnRButtonDownNop : &Cursor::OnNop );
			router[ WM_RBUTTONUP   ].Set( this, autoHide ? &Cursor::OnButtonUp : &Cursor::OnNop );

			hCurrent = hCursor;
			deadline = autoHide ? ::GetTickCount() + TIME_OUT : DWORD(~0UL);
		}

		void Input::Cursor::Unacquire()
		{
			deadline = DWORD(~0UL);
			hCurrent = hCursor = Resource::Cursor::GetArrow();

			Window::MsgHandler& router = window.Messages();

			router[ WM_MOUSEMOVE   ].Set( this, &Cursor::OnNop            );
			router[ WM_LBUTTONDOWN ].Set( this, &Cursor::OnNop            );
			router[ WM_LBUTTONUP   ].Set( this, &Cursor::OnNop            );
			router[ WM_RBUTTONDOWN ].Set( this, &Cursor::OnRButtonDownNop );
			router[ WM_RBUTTONUP   ].Set( this, &Cursor::OnNop            );
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		inline int Input::Cursor::GetWheel() const
		{
			return wheel / WHEEL_SCALE;
		}

		ibool Input::Cursor::OnNop(Window::Param&)
		{
			return true;
		}

		ibool Input::Cursor::OnSetCursor(Window::Param& param)
		{
			if (LOWORD(param.lParam) == HTCLIENT)
			{
				param.lResult = true;
				::SetCursor( hCurrent );
				return true;
			}

			return false;
		}

		ibool Input::Cursor::OnMouseMove(Window::Param&)
		{
			hCurrent = hCursor;
			deadline = ::GetTickCount() + TIME_OUT;
			return true;
		}

		ibool Input::Cursor::OnLButtonDown(Window::Param&)
		{
			deadline = DWORD(~0UL);
			::SetCursor( hCurrent=hCursor );
			return true;
		}

		ibool Input::Cursor::OnRButtonDown(Window::Param&)
		{
			deadline = DWORD(~0UL);
			::SetCursor( hCurrent=hCursor );
			window.SendCommand( IDM_VIEW_MENU );
			return true;
		}

		ibool Input::Cursor::OnRButtonDownNop(Window::Param&)
		{
			window.SendCommand( IDM_VIEW_MENU );
			return true;
		}

		ibool Input::Cursor::OnButtonUp(Window::Param&)
		{
			::SetCursor( hCurrent=hCursor );
			deadline = ::GetTickCount() + TIME_OUT;
			return true;
		}

		ibool Input::Cursor::OnWheel(Window::Param& param)
		{
			wheel += GET_WHEEL_DELTA_WPARAM(param.wParam);
			wheel  = NST_CLAMP(wheel,WHEEL_MIN,WHEEL_MAX);

			return true;
		}

		inline bool Input::CmdKeys::CanPoll() const
		{
			return !keys.Empty() && window.Focused();
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Input::Cursor::AutoHide()
		{
			deadline = DWORD(~0UL);

			if (window.Focused())
			{
				Window::Point pos;
				::GetCursorPos( &pos );

				if (Window::Rect(window.PictureCoordinates()).ScreenTransform(window).Inside( pos ))
					::SetCursor( hCurrent=NULL );
			}
		}

		Input::CmdKeys::CmdKey::CmdKey(const Key& key,uint c)
		: Key(key), cmd(c) {}

		Input::CmdKeys::CmdKeys(Window::Custom& w,DirectX::DirectInput& d)
		: acquired(false), clock(CLOCK_DEFAULT), window(w), directInput(d)
		{
			w.Messages().Hooks().Add( WM_SETFOCUS, this, &CmdKeys::OnFocus );
		}

		void Input::CmdKeys::Update()
		{
			if (!acquired && CanPoll())
			{
				clock = CLOCK_DEFAULT;
				window.StartTimer( this, &CmdKeys::OnTimer, POLL_RAPID );
			}
		}

		void Input::CmdKeys::Add(const Key& key,uint cmd)
		{
			if (key.Assigned() && !key.IsVirtualKey())
				keys.PushBack( CmdKey(key,cmd) );
		}

		void Input::CmdKeys::BeginAdd()
		{
			keys.Clear();
		}

		void Input::CmdKeys::EndAdd()
		{
			keys.Defrag();
			Update();
		}

		void Input::CmdKeys::Acquire()
		{
			acquired = true;
		}

		void Input::CmdKeys::Unacquire()
		{
			acquired = false;
			Update();
		}

		void Input::CmdKeys::OnFocus(Window::Param&)
		{
			Update();
		}

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
				7,6,5,4,3,2,1
			};

			signal = speeds[speed];
		}

		void Input::ClipBoard::Clear()
		{
			paste = false;
			Destroy();
		}

		ibool Input::ClipBoard::CanPaste() const
		{
			return Empty() && ::IsClipboardFormatAvailable( CF_TEXT );
		}

		void Input::ClipBoard::Paste()
		{
			paste = CanPaste();
		}

		uint Input::ClipBoard::Query(const u8* const NST_RESTRICT keyboard,const Type type)
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
										if (p == 165) // ''
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
										operator << (char(0xFF));
									}

									operator << (p);
								}

								if (kana)
									operator << (char(0xFF));

								::GlobalUnlock( handle );
							}
						}

						::CloseClipboard();

						if (Length())
							Io::Screen() << Resource::String( IDS_SCREEN_TEXT_PASTE ).Invoke( Length() );
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

		ibool Input::CmdKeys::ForcePoll() const
		{
			NST_ASSERT( keys.Size() );

			Keys::ConstIterator it = keys.Begin();
			Keys::ConstIterator const end = keys.End();

			do
			{
				if (it->GetState())
				{
					window.PostCommand( it->cmd );
					return true;
				}
			}
			while (++it != end);

			return false;
		}

		inline ibool Input::CmdKeys::Poll() const
		{
			return CanPoll() ? ForcePoll() : false;
		}

		uint Input::CmdKeys::OnTimer()
		{
			if (!acquired)
			{
				if (CanPoll())
				{
					directInput.Poll();

					if (ForcePoll())
					{
						clock = POLL_RAPID;
						return POLL_REST;
					}
					else
					{
						uint next = clock;
						clock = CLOCK_DEFAULT;
						return next;
					}
				}
				else
				{
					directInput.Unacquire();
				}
			}

			return CLOCK_STOP;
		}

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
		polled       ( false ),
		cursor       ( w ),
		directInput  ( w ),
		dialog       ( new Window::Input(directInput,e,cfg) ),
		cmdKeys      ( w, directInput )
		{
			static const Window::Menu::CmdHandler::Entry<Input> commands[] =
			{
				{ IDM_MACHINE_INPUT_AUTOSELECT,              &Input::OnCmdMachineAutoSelectController },
				{ IDM_MACHINE_INPUT_PORT1_UNCONNECTED,       &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT1_PAD1,              &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT1_PAD2,              &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT1_PAD3,              &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT1_PAD4,              &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT1_ZAPPER,            &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT1_PADDLE,            &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT1_POWERPAD,          &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT1_MOUSE,             &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT2_UNCONNECTED,       &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT2_PAD1,              &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT2_PAD2,              &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT2_PAD3,              &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT2_PAD4,              &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT2_ZAPPER,            &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT2_PADDLE,            &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT2_POWERPAD,          &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT2_MOUSE,             &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT3_UNCONNECTED,       &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT3_PAD1,              &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT3_PAD2,              &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT3_PAD3,              &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT3_PAD4,              &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT4_UNCONNECTED,       &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT4_PAD1,              &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT4_PAD2,              &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT4_PAD3,              &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_PORT4_PAD4,              &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_EXP_UNCONNECTED,         &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_EXP_FAMILYTRAINER,       &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_EXP_FAMILYBASICKEYBOARD, &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_EXP_SUBORKEYBOARD,       &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_EXP_DOREMIKKOKEYBOARD,   &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_EXP_HORITRACK,           &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_EXP_PACHINKO,            &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_EXP_PADDLE,              &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_EXP_OEKAKIDSTABLET,      &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_EXP_HYPERSHOT,           &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_EXP_CRAZYCLIMBER,        &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_EXP_MAHJONG,             &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_EXP_EXCITINGBOXING,      &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_EXP_TOPRIDER,            &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_EXP_POKKUNMOGURAA,       &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_INPUT_EXP_PARTYTAP,            &Input::OnCmdMachinePort                 },
				{ IDM_MACHINE_EXT_KEYBOARD_PASTE,            &Input::OnCmdMachineKeyboardPaste        },
				{ IDM_OPTIONS_INPUT,                         &Input::OnCmdOptionsInput                }
			};

			static const Window::Menu::PopupHandler::Entry<Input> popups[] =
			{
				{ Window::Menu::PopupHandler::Pos<IDM_POS_MACHINE,IDM_POS_MACHINE_INPUT,IDM_POS_MACHINE_INPUT_PORT1>::ID, &Input::OnMenuPort1 },
				{ Window::Menu::PopupHandler::Pos<IDM_POS_MACHINE,IDM_POS_MACHINE_INPUT,IDM_POS_MACHINE_INPUT_PORT2>::ID, &Input::OnMenuPort2 },
				{ Window::Menu::PopupHandler::Pos<IDM_POS_MACHINE,IDM_POS_MACHINE_INPUT,IDM_POS_MACHINE_INPUT_PORT3>::ID, &Input::OnMenuPort3 },
				{ Window::Menu::PopupHandler::Pos<IDM_POS_MACHINE,IDM_POS_MACHINE_INPUT,IDM_POS_MACHINE_INPUT_PORT4>::ID, &Input::OnMenuPort4 },
				{ Window::Menu::PopupHandler::Pos<IDM_POS_MACHINE,IDM_POS_MACHINE_INPUT,IDM_POS_MACHINE_INPUT_EXP>::ID, &Input::OnMenuExpPort },
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
			Controllers::Mouse::callback.Set             ( &Callbacks::PollMouse,             &rects );
			Controllers::OekaKidsTablet::callback.Set    ( &Callbacks::PollOekaKidsTablet,    &rects );
			Controllers::HyperShot::callback.Set         ( &Callbacks::PollHyperShot,         this   );
			Controllers::FamilyTrainer::callback.Set     ( &Callbacks::PollFamilyTrainer,     this   );
			Controllers::FamilyKeyboard::callback.Set    ( &Callbacks::PollFamilyKeyboard,    this   );
			Controllers::SuborKeyboard::callback.Set     ( &Callbacks::PollSuborKeyboard,     this   );
			Controllers::DoremikkoKeyboard::callback.Set ( &Callbacks::PollDoremikkoKeyboard, this   );
			Controllers::HoriTrack::callback.Set         ( &Callbacks::PollHoriTrack,         this   );
			Controllers::Pachinko::callback.Set          ( &Callbacks::PollPachinko,          this   );
			Controllers::CrazyClimber::callback.Set      ( &Callbacks::PollCrazyClimber,      this   );
			Controllers::Mahjong::callback.Set           ( &Callbacks::PollMahjong,           this   );
			Controllers::ExcitingBoxing::callback.Set    ( &Callbacks::PollExcitingBoxing,    this   );
			Controllers::TopRider::callback.Set          ( &Callbacks::PollTopRider,          this   );
			Controllers::PokkunMoguraa::callback.Set     ( &Callbacks::PollPokkunMoguraa,     this   );
			Controllers::PartyTap::callback.Set          ( &Callbacks::PollPartyTap,          this   );
			Controllers::VsSystem::callback.Set          ( &Callbacks::PollVsSystem,          this   );
			Controllers::KaraokeStudio::callback.Set     ( &Callbacks::PollKaraokeStudio,     this   );

			menu[IDM_MACHINE_INPUT_AUTOSELECT].Check( cfg["machine autoselect controllers"] != Configuration::NO );

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
					string[13] = '1' + i;
					const GenericString type( cfg[string] );

					Nes::Input::Type controller = Nes::Input::UNCONNECTED;

					switch (i)
					{
						case 0:
						case 1:

                                 if (type == _T( "zapper"   )) { controller = Nes::Input::ZAPPER;   break; }
							else if (type == _T( "paddle"   )) { controller = Nes::Input::PADDLE;   break; }
							else if (type == _T( "powerpad" )) { controller = Nes::Input::POWERPAD; break; }
							else if (type == _T( "mouse"    )) { controller = Nes::Input::MOUSE;    break; }

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
							else if (type == _T( "horitrack"         )) controller = Nes::Input::HORITRACK;
							else if (type == _T( "pachinko"          )) controller = Nes::Input::PACHINKO;
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
			cfg["machine autoselect controllers"].YesNo() = menu[IDM_MACHINE_INPUT_AUTOSELECT].Checked();

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
						case Nes::Input::MOUSE:             type = _T( "mouse"             ); break;
						case Nes::Input::FAMILYTRAINER:     type = _T( "familytrainer"     ); break;
						case Nes::Input::FAMILYKEYBOARD:    type = _T( "familykeyboard"    ); break;
						case Nes::Input::SUBORKEYBOARD:     type = _T( "suborkeyboard"     ); break;
						case Nes::Input::DOREMIKKOKEYBOARD: type = _T( "doremikkokeyboard" ); break;
						case Nes::Input::HORITRACK:         type = _T( "horitrack"         ); break;
						case Nes::Input::PACHINKO:          type = _T( "pachinko"          ); break;
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

					string[13] = '1' + i;
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
				cmdKeys.Acquire();
				UpdateDevices();
			}
		}

		void Input::StopEmulation()
		{
			clipBoard.Clear();
			cmdKeys.Unacquire();
			directInput.Unacquire();
			cursor.Unacquire();
			Window::Menu::EnableAccelerators( true );
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

			directInput.Build( dialog->GetSettings().GetKeys(), Settings::NUM_KEYS );

			static const ushort lut[Settings::NUM_COMMAND_KEYS][2] =
			{
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_OPEN,                  IDM_FILE_OPEN                        },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_LOAD_SCRIPT,           IDM_FILE_LOAD_NSP                    },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_LOAD_STATE,            IDM_FILE_LOAD_NST                    },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_SAVE_SCRIPT,           IDM_FILE_SAVE_NSP                    },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_SAVE_STATE,            IDM_FILE_SAVE_NST                    },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_LOAD_STATE_1,    IDM_FILE_QUICK_LOAD_STATE_SLOT_1     },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_LOAD_STATE_2,    IDM_FILE_QUICK_LOAD_STATE_SLOT_2     },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_LOAD_STATE_3,    IDM_FILE_QUICK_LOAD_STATE_SLOT_3     },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_LOAD_STATE_4,    IDM_FILE_QUICK_LOAD_STATE_SLOT_4     },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_LOAD_STATE_5,    IDM_FILE_QUICK_LOAD_STATE_SLOT_5     },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_LOAD_STATE_6,    IDM_FILE_QUICK_LOAD_STATE_SLOT_6     },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_LOAD_STATE_7,    IDM_FILE_QUICK_LOAD_STATE_SLOT_7     },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_LOAD_STATE_8,    IDM_FILE_QUICK_LOAD_STATE_SLOT_8     },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_LOAD_STATE_9,    IDM_FILE_QUICK_LOAD_STATE_SLOT_9     },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_LOAD_LAST_STATE, IDM_FILE_QUICK_LOAD_STATE_SLOT_NEWEST  },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_SAVE_STATE_1,    IDM_FILE_QUICK_SAVE_STATE_SLOT_1     },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_SAVE_STATE_2,    IDM_FILE_QUICK_SAVE_STATE_SLOT_2     },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_SAVE_STATE_3,    IDM_FILE_QUICK_SAVE_STATE_SLOT_3     },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_SAVE_STATE_4,    IDM_FILE_QUICK_SAVE_STATE_SLOT_4     },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_SAVE_STATE_5,    IDM_FILE_QUICK_SAVE_STATE_SLOT_5     },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_SAVE_STATE_6,    IDM_FILE_QUICK_SAVE_STATE_SLOT_6     },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_SAVE_STATE_7,    IDM_FILE_QUICK_SAVE_STATE_SLOT_7     },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_SAVE_STATE_8,    IDM_FILE_QUICK_SAVE_STATE_SLOT_8     },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_SAVE_STATE_9,    IDM_FILE_QUICK_SAVE_STATE_SLOT_9     },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_QUICK_SAVE_NEXT_STATE, IDM_FILE_QUICK_SAVE_STATE_SLOT_OLDEST  },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_SAVE_SCREENSHOT,       IDM_FILE_SAVE_SCREENSHOT             },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_LAUNCHER,              IDM_FILE_LAUNCHER                    },
				{ Settings::FILE_KEYS    + Settings::FILE_KEY_EXIT,                  IDM_FILE_QUIT                        },
				{ Settings::MACHINE_KEYS + Settings::MACHINE_KEY_POWER,              IDM_MACHINE_POWER                    },
				{ Settings::MACHINE_KEYS + Settings::MACHINE_KEY_RESET_SOFT,         IDM_MACHINE_RESET_SOFT               },
				{ Settings::MACHINE_KEYS + Settings::MACHINE_KEY_RESET_HARD,         IDM_MACHINE_RESET_HARD               },
				{ Settings::MACHINE_KEYS + Settings::MACHINE_KEY_PAUSE,              IDM_MACHINE_PAUSE                    },
				{ Settings::MACHINE_KEYS + Settings::MACHINE_KEY_UNLIMITED_SPRITES,  IDM_MACHINE_OPTIONS_UNLIMITEDSPRITES },
				{ Settings::MACHINE_KEYS + Settings::MACHINE_KEY_CHANGE_DISK_SIDE,   IDM_MACHINE_EXT_FDS_CHANGE_SIDE          },
				{ Settings::NSF_KEYS     + Settings::NSF_KEY_PLAY,                   IDM_MACHINE_NSF_PLAY                 },
				{ Settings::NSF_KEYS     + Settings::NSF_KEY_STOP,                   IDM_MACHINE_NSF_STOP                 },
				{ Settings::NSF_KEYS     + Settings::NSF_KEY_NEXT,                   IDM_MACHINE_NSF_NEXT                 },
				{ Settings::NSF_KEYS     + Settings::NSF_KEY_PREV,                   IDM_MACHINE_NSF_PREV                 },
				{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SCREENSIZE_1X,         IDM_VIEW_WINDOWSIZE_1X               },
				{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SCREENSIZE_2X,         IDM_VIEW_WINDOWSIZE_2X               },
				{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SCREENSIZE_3X,         IDM_VIEW_WINDOWSIZE_3X               },
				{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SCREENSIZE_4X,         IDM_VIEW_WINDOWSIZE_4X               },
				{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SCREENSIZE_5X,         IDM_VIEW_WINDOWSIZE_5X               },
				{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SCREENSIZE_6X,         IDM_VIEW_WINDOWSIZE_6X               },
				{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SCREENSIZE_7X,         IDM_VIEW_WINDOWSIZE_7X               },
				{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SCREENSIZE_8X,         IDM_VIEW_WINDOWSIZE_8X               },
				{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SCREENSIZE_9X,         IDM_VIEW_WINDOWSIZE_9X               },
				{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SCREENSIZE_MAX,        IDM_VIEW_WINDOWSIZE_MAX              },
				{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SHOW_MENU,             IDM_VIEW_MENU                        },
				{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SHOW_STATUSBAR,        IDM_VIEW_STATUSBAR                   },
				{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SHOW_ONTOP,            IDM_VIEW_ON_TOP                      },
				{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_SHOW_FPS,              IDM_VIEW_FPS                         },
				{ Settings::VIEW_KEYS    + Settings::VIEW_KEY_FULLSCREEN,            IDM_VIEW_SWITCH_SCREEN               },
				{ Settings::HELP_KEYS    + Settings::HELP_KEY_HELP,                  IDM_HELP_HELP                        }
			};

			cmdKeys.BeginAdd();

			ACCEL accel[Settings::NUM_COMMAND_KEYS];

			for (uint i=0; i < Settings::NUM_COMMAND_KEYS; ++i)
			{
				const DirectX::DirectInput::Key& key = dialog->GetSettings().GetKey( lut[i][0] );

				cmdKeys.Add( key, lut[i][1] );

				key.GetVirtualKey( accel[i] );
				accel[i].cmd = lut[i][1];
			}

			cmdKeys.EndAdd();

			menu.SetKeys( accel, Settings::NUM_COMMAND_KEYS );
		}

		void Input::OnEmuEvent(Emulator::Event event)
		{
			switch (event)
			{
				case Emulator::EVENT_PORT1_CONTROLLER:
				case Emulator::EVENT_PORT2_CONTROLLER:
				case Emulator::EVENT_PORT3_CONTROLLER:
				case Emulator::EVENT_PORT4_CONTROLLER:
				case Emulator::EVENT_PORT5_CONTROLLER:

					UpdateDevices();
					break;

				case Emulator::EVENT_LOAD:

					if (emulator.Is(Nes::Machine::GAME) && menu[IDM_MACHINE_INPUT_AUTOSELECT].Checked())
						emulator.AutoSelectControllers();

					break;

				case Emulator::EVENT_NETPLAY_LOAD:
				case Emulator::EVENT_NETPLAY_UNLOAD:

					for (uint i=0, enable=(event == Emulator::EVENT_NETPLAY_UNLOAD); i < 4; ++i)
						menu[IDM_POS_MACHINE][IDM_POS_MACHINE_INPUT][IDM_POS_MACHINE_INPUT_PORT1+i].Enable( enable || emulator.GetPlayer() == (i+1) );

					break;

				case Emulator::EVENT_NETPLAY_MODE_ON:
				case Emulator::EVENT_NETPLAY_MODE_OFF:
				{
					const ibool state = (event == Emulator::EVENT_NETPLAY_MODE_OFF);

					menu[IDM_MACHINE_INPUT_AUTOSELECT].Enable( state );
					menu[IDM_POS_MACHINE][IDM_POS_MACHINE_INPUT][IDM_POS_MACHINE_INPUT_EXP].Enable( state );
					menu[IDM_POS_MACHINE][IDM_POS_MACHINE_EXT][IDM_POS_MACHINE_EXT_KEYBOARD].Enable( state );
					menu[IDM_OPTIONS_INPUT].Enable( state );

					menu[ IDM_MACHINE_INPUT_PORT1_UNCONNECTED ].Enable( state );
					menu[ IDM_MACHINE_INPUT_PORT1_ZAPPER      ].Enable( state );
					menu[ IDM_MACHINE_INPUT_PORT1_PADDLE      ].Enable( state );
					menu[ IDM_MACHINE_INPUT_PORT1_POWERPAD    ].Enable( state );
					menu[ IDM_MACHINE_INPUT_PORT1_MOUSE       ].Enable( state );
					menu[ IDM_MACHINE_INPUT_PORT2_UNCONNECTED ].Enable( state );
					menu[ IDM_MACHINE_INPUT_PORT2_ZAPPER      ].Enable( state );
					menu[ IDM_MACHINE_INPUT_PORT2_PADDLE      ].Enable( state );
					menu[ IDM_MACHINE_INPUT_PORT2_POWERPAD    ].Enable( state );
					menu[ IDM_MACHINE_INPUT_PORT2_MOUSE       ].Enable( state );
					menu[ IDM_MACHINE_INPUT_PORT3_UNCONNECTED ].Enable( state );
					menu[ IDM_MACHINE_INPUT_PORT4_UNCONNECTED ].Enable( state );
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
			if (menu[IDM_MACHINE_INPUT_AUTOSELECT].ToggleCheck())
				emulator.AutoSelectControllers();
		}

		void Input::OnCmdMachinePort(uint id)
		{
			static const uchar table[][2] =
			{
				{ 0, Nes::Input::UNCONNECTED       },
				{ 0, Nes::Input::PAD1              },
				{ 0, Nes::Input::PAD2              },
				{ 0, Nes::Input::PAD3              },
				{ 0, Nes::Input::PAD4              },
				{ 0, Nes::Input::ZAPPER            },
				{ 0, Nes::Input::PADDLE            },
				{ 0, Nes::Input::POWERPAD          },
				{ 0, Nes::Input::MOUSE             },
				{ 1, Nes::Input::UNCONNECTED       },
				{ 1, Nes::Input::PAD1              },
				{ 1, Nes::Input::PAD2              },
				{ 1, Nes::Input::PAD3              },
				{ 1, Nes::Input::PAD4              },
				{ 1, Nes::Input::ZAPPER            },
				{ 1, Nes::Input::PADDLE            },
				{ 1, Nes::Input::POWERPAD          },
				{ 1, Nes::Input::MOUSE             },
				{ 2, Nes::Input::UNCONNECTED       },
				{ 2, Nes::Input::PAD1              },
				{ 2, Nes::Input::PAD2              },
				{ 2, Nes::Input::PAD3              },
				{ 2, Nes::Input::PAD4              },
				{ 3, Nes::Input::UNCONNECTED       },
				{ 3, Nes::Input::PAD1              },
				{ 3, Nes::Input::PAD2              },
				{ 3, Nes::Input::PAD3              },
				{ 3, Nes::Input::PAD4              },
				{ 4, Nes::Input::UNCONNECTED       },
				{ 4, Nes::Input::FAMILYTRAINER     },
				{ 4, Nes::Input::FAMILYKEYBOARD    },
				{ 4, Nes::Input::SUBORKEYBOARD     },
				{ 4, Nes::Input::DOREMIKKOKEYBOARD },
				{ 4, Nes::Input::HORITRACK         },
				{ 4, Nes::Input::PACHINKO          },
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
			Application::Instance::GetMainWindow().Post( Application::Instance::WM_NST_COMMAND_RESUME );
		}

		void Input::OnCmdMachineKeyboardPaste(uint)
		{
			clipBoard.Paste();
			Application::Instance::GetMainWindow().Post( Application::Instance::WM_NST_COMMAND_RESUME );
		}

		void Input::OnMenuPort1(Window::Menu::PopupHandler::Param& param)
		{
			uint id;

			switch (emulator.GetController(0))
			{
				case Nes::Input::PAD1:     id = IDM_MACHINE_INPUT_PORT1_PAD1;        break;
				case Nes::Input::PAD2:     id = IDM_MACHINE_INPUT_PORT1_PAD2;        break;
				case Nes::Input::PAD3:     id = IDM_MACHINE_INPUT_PORT1_PAD3;        break;
				case Nes::Input::PAD4:     id = IDM_MACHINE_INPUT_PORT1_PAD4;        break;
				case Nes::Input::ZAPPER:   id = IDM_MACHINE_INPUT_PORT1_ZAPPER;      break;
				case Nes::Input::PADDLE:   id = IDM_MACHINE_INPUT_PORT1_PADDLE;      break;
				case Nes::Input::POWERPAD: id = IDM_MACHINE_INPUT_PORT1_POWERPAD;    break;
				case Nes::Input::MOUSE:    id = IDM_MACHINE_INPUT_PORT1_MOUSE;       break;
				default:                   id = IDM_MACHINE_INPUT_PORT1_UNCONNECTED; break;
			}

			param.menu[id].Check( IDM_MACHINE_INPUT_PORT1_UNCONNECTED, IDM_MACHINE_INPUT_PORT1_MOUSE );
		}

		void Input::OnMenuPort2(Window::Menu::PopupHandler::Param& param)
		{
			uint id;

			switch (emulator.GetController(1))
			{
				case Nes::Input::PAD1:     id = IDM_MACHINE_INPUT_PORT2_PAD1;        break;
				case Nes::Input::PAD2:     id = IDM_MACHINE_INPUT_PORT2_PAD2;        break;
				case Nes::Input::PAD3:     id = IDM_MACHINE_INPUT_PORT2_PAD3;        break;
				case Nes::Input::PAD4:     id = IDM_MACHINE_INPUT_PORT2_PAD4;        break;
				case Nes::Input::ZAPPER:   id = IDM_MACHINE_INPUT_PORT2_ZAPPER;      break;
				case Nes::Input::PADDLE:   id = IDM_MACHINE_INPUT_PORT2_PADDLE;      break;
				case Nes::Input::POWERPAD: id = IDM_MACHINE_INPUT_PORT2_POWERPAD;    break;
				case Nes::Input::MOUSE:    id = IDM_MACHINE_INPUT_PORT2_MOUSE;       break;
				default:                   id = IDM_MACHINE_INPUT_PORT2_UNCONNECTED; break;
			}

			param.menu[id].Check( IDM_MACHINE_INPUT_PORT2_UNCONNECTED, IDM_MACHINE_INPUT_PORT2_MOUSE );
		}

		void Input::OnMenuPort3(Window::Menu::PopupHandler::Param& param)
		{
			uint id;

			switch (emulator.GetController(2))
			{
				case Nes::Input::PAD1: id = IDM_MACHINE_INPUT_PORT3_PAD1;        break;
				case Nes::Input::PAD2: id = IDM_MACHINE_INPUT_PORT3_PAD2;        break;
				case Nes::Input::PAD3: id = IDM_MACHINE_INPUT_PORT3_PAD3;        break;
				case Nes::Input::PAD4: id = IDM_MACHINE_INPUT_PORT3_PAD4;        break;
				default:               id = IDM_MACHINE_INPUT_PORT3_UNCONNECTED; break;
			}

			param.menu[id].Check( IDM_MACHINE_INPUT_PORT3_UNCONNECTED, IDM_MACHINE_INPUT_PORT3_PAD4 );
		}

		void Input::OnMenuPort4(Window::Menu::PopupHandler::Param& param)
		{
			uint id;

			switch (emulator.GetController(3))
			{
				case Nes::Input::PAD1: id = IDM_MACHINE_INPUT_PORT4_PAD1;        break;
				case Nes::Input::PAD2: id = IDM_MACHINE_INPUT_PORT4_PAD2;        break;
				case Nes::Input::PAD3: id = IDM_MACHINE_INPUT_PORT4_PAD3;        break;
				case Nes::Input::PAD4: id = IDM_MACHINE_INPUT_PORT4_PAD4;        break;
				default:               id = IDM_MACHINE_INPUT_PORT4_UNCONNECTED; break;
			}

			param.menu[id].Check( IDM_MACHINE_INPUT_PORT4_UNCONNECTED, IDM_MACHINE_INPUT_PORT4_PAD4 );
		}

		void Input::OnMenuExpPort(Window::Menu::PopupHandler::Param& param)
		{
			uint id;

			switch (emulator.GetController(4))
			{
				case Nes::Input::FAMILYTRAINER:     id = IDM_MACHINE_INPUT_EXP_FAMILYTRAINER;       break;
				case Nes::Input::FAMILYKEYBOARD:    id = IDM_MACHINE_INPUT_EXP_FAMILYBASICKEYBOARD; break;
				case Nes::Input::SUBORKEYBOARD:     id = IDM_MACHINE_INPUT_EXP_SUBORKEYBOARD;       break;
				case Nes::Input::DOREMIKKOKEYBOARD: id = IDM_MACHINE_INPUT_EXP_DOREMIKKOKEYBOARD;   break;
				case Nes::Input::HORITRACK:         id = IDM_MACHINE_INPUT_EXP_HORITRACK;           break;
				case Nes::Input::PACHINKO:          id = IDM_MACHINE_INPUT_EXP_PACHINKO;            break;
				case Nes::Input::PADDLE:            id = IDM_MACHINE_INPUT_EXP_PADDLE;              break;
				case Nes::Input::OEKAKIDSTABLET:    id = IDM_MACHINE_INPUT_EXP_OEKAKIDSTABLET;      break;
				case Nes::Input::HYPERSHOT:         id = IDM_MACHINE_INPUT_EXP_HYPERSHOT;           break;
				case Nes::Input::CRAZYCLIMBER:      id = IDM_MACHINE_INPUT_EXP_CRAZYCLIMBER;        break;
				case Nes::Input::MAHJONG:           id = IDM_MACHINE_INPUT_EXP_MAHJONG;             break;
				case Nes::Input::EXCITINGBOXING:    id = IDM_MACHINE_INPUT_EXP_EXCITINGBOXING;      break;
				case Nes::Input::TOPRIDER:          id = IDM_MACHINE_INPUT_EXP_TOPRIDER;            break;
				case Nes::Input::POKKUNMOGURAA:     id = IDM_MACHINE_INPUT_EXP_POKKUNMOGURAA;       break;
				case Nes::Input::PARTYTAP:          id = IDM_MACHINE_INPUT_EXP_PARTYTAP;            break;
				default:                            id = IDM_MACHINE_INPUT_EXP_UNCONNECTED;         break;
			}

			param.menu[id].Check( IDM_MACHINE_INPUT_EXP_UNCONNECTED, IDM_MACHINE_INPUT_EXP_PARTYTAP );
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

			typedef Window::Input::Settings Settings;
			const Settings::Key* const NST_RESTRICT keys = dialog->GetSettings().GetKeys(Settings::EMULATION_KEYS);

			emulator.ToggleSpeed( keys[Settings::EMULATION_KEY_ALT_SPEED].GetState() );
			emulator.ToggleRewind( keys[Settings::EMULATION_KEY_REWIND].GetState() );

			cmdKeys.Poll();
		}

		inline void Input::CheckPoll()
		{
			if (!polled)
			{
				polled = true;
				ForcePoll();
			}
		}

		bool NST_CALLBACK Input::Callbacks::PollPad(UserData data,Pad& pad,uint index)
		{
			NST_COMPILE_ASSERT
			(
				Settings::TYPE_PAD1 == 0 &&
				Settings::TYPE_PAD2 == 1 &&
				Settings::TYPE_PAD3 == 2 &&
				Settings::TYPE_PAD4 == 3
			);

			NST_ASSERT(index < 4);

			Input& input = *static_cast<Input*>(data);

			input.CheckPoll();

			const Key* const NST_RESTRICT keys = input.dialog->GetSettings().GetKeys(index,0);

			uint buttons = 0;

			keys[ Settings::PAD_KEY_A      ].GetState( buttons, Pad::A      );
			keys[ Settings::PAD_KEY_B      ].GetState( buttons, Pad::B      );
			keys[ Settings::PAD_KEY_SELECT ].GetState( buttons, Pad::SELECT );
			keys[ Settings::PAD_KEY_START  ].GetState( buttons, Pad::START  );
			keys[ Settings::PAD_KEY_UP     ].GetState( buttons, Pad::UP     );
			keys[ Settings::PAD_KEY_DOWN   ].GetState( buttons, Pad::DOWN   );
			keys[ Settings::PAD_KEY_LEFT   ].GetState( buttons, Pad::LEFT   );
			keys[ Settings::PAD_KEY_RIGHT  ].GetState( buttons, Pad::RIGHT  );

			if (input.autoFire.Signaled())
			{
				keys[ Settings::PAD_KEY_AUTOFIRE_A ].GetState( buttons, Pad::A );
				keys[ Settings::PAD_KEY_AUTOFIRE_B ].GetState( buttons, Pad::B );
			}

			pad.buttons = buttons;

			buttons = 0;
			keys[ Settings::PAD_KEY_MIC ].GetState( buttons, Pad::MIC );

			pad.mic = buttons;

			return true;
		}

		bool NST_CALLBACK Input::Callbacks::PollZapper(UserData data,Zapper& zapper)
		{
			POINT mouse;
			::GetCursorPos( &mouse );

			const Rects& rects = *static_cast<Rects*>(data);

			Window::Rect output;
			rects.getOutput( output );

			if (output.Inside( mouse ))
			{
				if (!(::GetAsyncKeyState( Cursor::secondaryButtonId ) & 0x8000U))
				{
					zapper.fire = ::GetAsyncKeyState( Cursor::primaryButtonId ) & 0x8000U;

					if (zapper.x != ~0U)
					{
						Window::Rect input;
						rects.getInput( input );

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
					zapper.fire = true;
					zapper.y = zapper.x = ~0U;
				}
			}
			else
			{
				zapper.fire = false;
			}

			return true;
		}

		bool NST_CALLBACK Input::Callbacks::PollPaddle(UserData data,Paddle& paddle)
		{
			POINT mouse;
			::GetCursorPos( &mouse );

			const Rects& rects = *static_cast<Rects*>(data);

			Window::Rect output;
			rects.getOutput( output );

			if (output.Inside( mouse ))
			{
				Window::Rect input;
				rects.getInput( input );

				paddle.x = uint(input.left) + (uint(mouse.x) - uint(output.left)) * (uint(input.Width())-1) / uint(output.Width());
				paddle.button = ::GetAsyncKeyState( Cursor::primaryButtonId ) & 0x8000U;
			}
			else
			{
				paddle.button = false;
			}

			return true;
		}

		bool NST_CALLBACK Input::Callbacks::PollPowerPad(UserData data,PowerPad& powerPad)
		{
			Input& input = *static_cast<Input*>(data);

			input.CheckPoll();

			const Key* NST_RESTRICT keys = input.dialog->GetSettings().GetKeys(Settings::POWERPAD_KEYS);

			for (uint i=0; i < Settings::POWERPAD_NUM_SIDE_A_KEYS; ++i)
				powerPad.sideA[i] = keys[i].GetState();

			keys += Settings::POWERPAD_NUM_SIDE_A_KEYS;

			for (uint i=0; i < Settings::POWERPAD_NUM_SIDE_B_KEYS; ++i)
				powerPad.sideB[i] = keys[i].GetState();

			return true;
		}

		void Input::Callbacks::PollCursor(UserData data,uint& x,uint& y,uint& button)
		{
			POINT mouse;
			::GetCursorPos( &mouse );

			const Rects& rects = *static_cast<Rects*>(data);

			Window::Rect output;
			rects.getOutput( output );

			if (output.Inside( mouse ))
			{
				Window::Rect input;
				rects.getInput( input );

				x = uint(input.left) + (uint(mouse.x) - uint(output.left)) * (uint(input.Width())-1) / uint(output.Width());
				y = uint(input.top) + (uint(mouse.y) - uint(output.top)) * (uint(input.Height())-1) / uint(output.Height());
				button = ::GetAsyncKeyState( Cursor::primaryButtonId ) & 0x8000U;
			}
			else
			{
				button = 0;
			}
		}

		bool NST_CALLBACK Input::Callbacks::PollMouse(UserData data,Mouse& mouse)
		{
			PollCursor( data, mouse.x, mouse.y, mouse.button );
			return true;
		}

		bool NST_CALLBACK Input::Callbacks::PollOekaKidsTablet(UserData data,OekaKidsTablet& tablet)
		{
			PollCursor( data, tablet.x, tablet.y, tablet.button );
			return true;
		}

		bool NST_CALLBACK Input::Callbacks::PollHoriTrack(UserData data,HoriTrack& horiTrack)
		{
			Input& input = *static_cast<Input*>(data);

			input.CheckPoll();

			const Key* const NST_RESTRICT keys = input.dialog->GetSettings().GetKeys(Settings::HORITRACK_KEYS);

			uint buttons = 0;

			keys[ Settings::HORITRACK_KEY_A      ].GetState( buttons, HoriTrack::BUTTON_A      );
			keys[ Settings::HORITRACK_KEY_B      ].GetState( buttons, HoriTrack::BUTTON_B      );
			keys[ Settings::HORITRACK_KEY_SELECT ].GetState( buttons, HoriTrack::BUTTON_SELECT );
			keys[ Settings::HORITRACK_KEY_START  ].GetState( buttons, HoriTrack::BUTTON_START  );
			keys[ Settings::HORITRACK_KEY_UP     ].GetState( buttons, HoriTrack::BUTTON_UP     );
			keys[ Settings::HORITRACK_KEY_DOWN   ].GetState( buttons, HoriTrack::BUTTON_DOWN   );
			keys[ Settings::HORITRACK_KEY_LEFT   ].GetState( buttons, HoriTrack::BUTTON_LEFT   );
			keys[ Settings::HORITRACK_KEY_RIGHT  ].GetState( buttons, HoriTrack::BUTTON_RIGHT  );

			static ibool speed = false;

			if (keys[Settings::HORITRACK_KEY_SPEED].GetToggle( speed ))
			{
				horiTrack.mode ^= HoriTrack::MODE_LOWSPEED;
				Io::Screen() << Resource::String((horiTrack.mode & HoriTrack::MODE_LOWSPEED) ? IDS_SCREEN_HORITRACK_SPEED_LOW : IDS_SCREEN_HORITRACK_SPEED_HIGH);
			}

			static ibool orientation = false;

			if (keys[Settings::HORITRACK_KEY_ORIENTATION].GetToggle( orientation ))
			{
				horiTrack.mode ^= HoriTrack::MODE_REVERSED;
				Io::Screen() << Resource::String((horiTrack.mode & HoriTrack::MODE_REVERSED) ? IDS_SCREEN_HORITRACK_ORIENTATION_RIGHT : IDS_SCREEN_HORITRACK_ORIENTATION_LEFT);
			}

			horiTrack.buttons = buttons;

			uint mouseButton;
			PollCursor( &input.rects, horiTrack.x, horiTrack.y, mouseButton );
			horiTrack.buttons = (horiTrack.buttons & (HoriTrack::BUTTON_A^0xFF)) | (mouseButton ? HoriTrack::BUTTON_A : 0);

			return true;
		}

		bool NST_CALLBACK Input::Callbacks::PollPachinko(UserData data,Pachinko& pachinko)
		{
			Input& input = *static_cast<Input*>(data);

			input.CheckPoll();

			const Key* const NST_RESTRICT keys = input.dialog->GetSettings().GetKeys(Settings::PACHINKO_KEYS);

			uint buttons = 0;

			keys[ Settings::PACHINKO_KEY_A      ].GetState( buttons, Pachinko::BUTTON_A      );
			keys[ Settings::PACHINKO_KEY_B      ].GetState( buttons, Pachinko::BUTTON_B      );
			keys[ Settings::PACHINKO_KEY_SELECT ].GetState( buttons, Pachinko::BUTTON_SELECT );
			keys[ Settings::PACHINKO_KEY_START  ].GetState( buttons, Pachinko::BUTTON_START  );
			keys[ Settings::PACHINKO_KEY_UP     ].GetState( buttons, Pachinko::BUTTON_UP     );
			keys[ Settings::PACHINKO_KEY_DOWN   ].GetState( buttons, Pachinko::BUTTON_DOWN   );
			keys[ Settings::PACHINKO_KEY_LEFT   ].GetState( buttons, Pachinko::BUTTON_LEFT   );
			keys[ Settings::PACHINKO_KEY_RIGHT  ].GetState( buttons, Pachinko::BUTTON_RIGHT  );

			pachinko.buttons = buttons;
			pachinko.throttle = input.cursor.GetWheel();

			return true;
		}

		bool NST_CALLBACK Input::Callbacks::PollHyperShot(UserData data,HyperShot& hyperShot)
		{
			Input& input = *static_cast<Input*>(data);

			input.CheckPoll();

			const Key* const NST_RESTRICT keys[2] =
			{
				input.dialog->GetSettings().GetKeys(Settings::PAD1_KEYS),
				input.dialog->GetSettings().GetKeys(Settings::PAD2_KEYS)
			};

			uint buttons = 0;

			keys[0][ Settings::PAD_KEY_A ].GetState( buttons, HyperShot::PLAYER1_BUTTON_1 );
			keys[0][ Settings::PAD_KEY_B ].GetState( buttons, HyperShot::PLAYER1_BUTTON_2 );
			keys[1][ Settings::PAD_KEY_A ].GetState( buttons, HyperShot::PLAYER2_BUTTON_1 );
			keys[1][ Settings::PAD_KEY_B ].GetState( buttons, HyperShot::PLAYER2_BUTTON_2 );

			if (input.autoFire.Signaled())
			{
				keys[0][ Settings::PAD_KEY_AUTOFIRE_A ].GetState( buttons, HyperShot::PLAYER1_BUTTON_1 );
				keys[0][ Settings::PAD_KEY_AUTOFIRE_B ].GetState( buttons, HyperShot::PLAYER1_BUTTON_2 );
				keys[1][ Settings::PAD_KEY_AUTOFIRE_A ].GetState( buttons, HyperShot::PLAYER2_BUTTON_1 );
				keys[1][ Settings::PAD_KEY_AUTOFIRE_B ].GetState( buttons, HyperShot::PLAYER2_BUTTON_2 );
			}

			hyperShot.buttons = buttons;

			return true;
		}

		bool NST_CALLBACK Input::Callbacks::PollCrazyClimber(UserData data,CrazyClimber& crazyClimber)
		{
			Input& input = *static_cast<Input*>(data);

			input.CheckPoll();

			const Key* const NST_RESTRICT keys = input.dialog->GetSettings().GetKeys(Settings::CRAZYCLIMBER_KEYS);

			uint buttons = 0;

			keys[ Settings::CRAZYCLIMBER_KEY_LEFT_UP    ].GetState( buttons, CrazyClimber::UP    );
			keys[ Settings::CRAZYCLIMBER_KEY_LEFT_RIGHT ].GetState( buttons, CrazyClimber::RIGHT );
			keys[ Settings::CRAZYCLIMBER_KEY_LEFT_DOWN  ].GetState( buttons, CrazyClimber::DOWN  );
			keys[ Settings::CRAZYCLIMBER_KEY_LEFT_LEFT  ].GetState( buttons, CrazyClimber::LEFT  );

			crazyClimber.left = buttons;

			buttons = 0;

			keys[ Settings::CRAZYCLIMBER_KEY_RIGHT_UP    ].GetState( buttons, CrazyClimber::UP    );
			keys[ Settings::CRAZYCLIMBER_KEY_RIGHT_RIGHT ].GetState( buttons, CrazyClimber::RIGHT );
			keys[ Settings::CRAZYCLIMBER_KEY_RIGHT_DOWN  ].GetState( buttons, CrazyClimber::DOWN  );
			keys[ Settings::CRAZYCLIMBER_KEY_RIGHT_LEFT  ].GetState( buttons, CrazyClimber::LEFT  );

			crazyClimber.right = buttons;

			return true;
		}

		bool NST_CALLBACK Input::Callbacks::PollFamilyTrainer(UserData data,FamilyTrainer& familyTrainer)
		{
			Input& input = *static_cast<Input*>(data);

			input.CheckPoll();

			const Key* NST_RESTRICT keys = input.dialog->GetSettings().GetKeys(Settings::POWERPAD_KEYS);

			for (uint i=0; i < Settings::POWERPAD_NUM_SIDE_A_KEYS; ++i)
				familyTrainer.sideA[i] = keys[i].GetState();

			keys += Settings::POWERPAD_NUM_SIDE_A_KEYS;

			for (uint i=0; i < Settings::POWERPAD_NUM_SIDE_B_KEYS; ++i)
				familyTrainer.sideB[i] = keys[i].GetState();

			return true;
		}

		bool NST_CALLBACK Input::Callbacks::PollFamilyKeyboard(UserData data,FamilyKeyboard& keyboard,uint part,uint mode)
		{
			Input& input = *static_cast<Input*>(data);

			input.CheckPoll();

			const u8* const NST_RESTRICT buffer = input.directInput.GetKeyboardBuffer();

			uint key = 0;

			#define NST_2(a,b) (a | (u16(b) << 8))

			if (input.clipBoard.Query( buffer, ClipBoard::FAMILY ))
			{
				static const u16 asciiMap[FamilyKeyboard::NUM_PARTS][FamilyKeyboard::NUM_MODES][4] =
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
				static const u16 dikMap[FamilyKeyboard::NUM_PARTS][FamilyKeyboard::NUM_MODES][4] =
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

			keyboard.parts[part] = key;

			return true;
		}

		bool NST_CALLBACK Input::Callbacks::PollSuborKeyboard(UserData data,SuborKeyboard& keyboard,uint part,uint mode)
		{
			Input& input = *static_cast<Input*>(data);

			input.CheckPoll();

			const u8* const NST_RESTRICT buffer = input.directInput.GetKeyboardBuffer();

			uint key = 0;

			#define NST_2(a,b) (a | (u16(b) << 8))

			if (input.clipBoard.Query( buffer, ClipBoard::SUBOR ))
			{
				static const u16 asciiMap[SuborKeyboard::NUM_PARTS][SuborKeyboard::NUM_MODES][4] =
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
				static const u16 dikMap[SuborKeyboard::NUM_PARTS][SuborKeyboard::NUM_MODES][4] =
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

			keyboard.parts[part] = key;

			return true;
		}

		bool NST_CALLBACK Input::Callbacks::PollDoremikkoKeyboard(UserData data,DoremikkoKeyboard& doremikko,const uint part,const uint mode)
		{
			Input& input = *static_cast<Input*>(data);

			input.CheckPoll();

			const u8* const NST_RESTRICT keyboard = input.directInput.GetKeyboardBuffer();

			uint keys = 0;

			switch (part)
			{
				case DoremikkoKeyboard::PART_1:

					if (mode == DoremikkoKeyboard::MODE_B)
					{
						if (keyboard[ DIK_Z ] & 0x80) keys |= DoremikkoKeyboard::MODE_B_0;
						if (keyboard[ DIK_S ] & 0x80) keys |= DoremikkoKeyboard::MODE_B_1;
					}
					break;

				case DoremikkoKeyboard::PART_2:

					if (mode == DoremikkoKeyboard::MODE_A)
					{
						if (keyboard[ DIK_X ] & 0x80) keys |= DoremikkoKeyboard::MODE_A_0;
						if (keyboard[ DIK_D ] & 0x80) keys |= DoremikkoKeyboard::MODE_A_1;
						if (keyboard[ DIK_C ] & 0x80) keys |= DoremikkoKeyboard::MODE_A_2;
						if (keyboard[ DIK_V ] & 0x80) keys |= DoremikkoKeyboard::MODE_A_3;
					}
					else
					{
						if (keyboard[ DIK_G ] & 0x80) keys |= DoremikkoKeyboard::MODE_B_0;
						if (keyboard[ DIK_B ] & 0x80) keys |= DoremikkoKeyboard::MODE_B_1;
					}
					break;

				case DoremikkoKeyboard::PART_3:

					if (mode == DoremikkoKeyboard::MODE_A)
					{
						if (keyboard[ DIK_H ] & 0x80) keys |= DoremikkoKeyboard::MODE_A_0;
						if (keyboard[ DIK_N ] & 0x80) keys |= DoremikkoKeyboard::MODE_A_1;
						if (keyboard[ DIK_J ] & 0x80) keys |= DoremikkoKeyboard::MODE_A_2;
						if (keyboard[ DIK_M ] & 0x80) keys |= DoremikkoKeyboard::MODE_A_3;
					}
					else
					{
						if (keyboard[ DIK_COMMA ] & 0x80) keys |= DoremikkoKeyboard::MODE_B_0;
						if (keyboard[ DIK_L     ] & 0x80) keys |= DoremikkoKeyboard::MODE_B_1;
					}
					break;

				case DoremikkoKeyboard::PART_4:

					if (mode == DoremikkoKeyboard::MODE_A)
					{
						if (keyboard[ DIK_PERIOD    ] & 0x80) keys |= DoremikkoKeyboard::MODE_A_0;
						if (keyboard[ DIK_SEMICOLON ] & 0x80) keys |= DoremikkoKeyboard::MODE_A_1;
						if (keyboard[ DIK_SLASH     ] & 0x80) keys |= DoremikkoKeyboard::MODE_A_2;
						if (keyboard[ DIK_Q         ] & 0x80) keys |= DoremikkoKeyboard::MODE_A_3;
					}
					else
					{
						if (keyboard[ DIK_2 ] & 0x80) keys |= DoremikkoKeyboard::MODE_B_0;
						if (keyboard[ DIK_W ] & 0x80) keys |= DoremikkoKeyboard::MODE_B_1;
					}
					break;

				case DoremikkoKeyboard::PART_5:

					if (mode == DoremikkoKeyboard::MODE_A)
					{
						if (keyboard[ DIK_3 ] & 0x80) keys |= DoremikkoKeyboard::MODE_A_0;
						if (keyboard[ DIK_E ] & 0x80) keys |= DoremikkoKeyboard::MODE_A_1;
						if (keyboard[ DIK_4 ] & 0x80) keys |= DoremikkoKeyboard::MODE_A_2;
						if (keyboard[ DIK_R ] & 0x80) keys |= DoremikkoKeyboard::MODE_A_3;
					}
					else
					{
						if (keyboard[ DIK_T ] & 0x80) keys |= DoremikkoKeyboard::MODE_B_0;
						if (keyboard[ DIK_6 ] & 0x80) keys |= DoremikkoKeyboard::MODE_B_1;
					}
					break;

				case DoremikkoKeyboard::PART_6:

					if (mode == DoremikkoKeyboard::MODE_A)
					{
						if (keyboard[ DIK_Y ] & 0x80) keys |= DoremikkoKeyboard::MODE_A_0;
						if (keyboard[ DIK_7 ] & 0x80) keys |= DoremikkoKeyboard::MODE_A_1;
						if (keyboard[ DIK_U ] & 0x80) keys |= DoremikkoKeyboard::MODE_A_2;
						if (keyboard[ DIK_I ] & 0x80) keys |= DoremikkoKeyboard::MODE_A_3;
					}
					else
					{
						if (keyboard[ DIK_9 ] & 0x80) keys |= DoremikkoKeyboard::MODE_B_0;
						if (keyboard[ DIK_O ] & 0x80) keys |= DoremikkoKeyboard::MODE_B_1;
					}
					break;

				case DoremikkoKeyboard::PART_7:

					if (mode == DoremikkoKeyboard::MODE_A)
					{
						if (keyboard[ DIK_0     ] & 0x80) keys |= DoremikkoKeyboard::MODE_A_0;
						if (keyboard[ DIK_P     ] & 0x80) keys |= DoremikkoKeyboard::MODE_A_1;
						if (keyboard[ DIK_MINUS ] & 0x80) keys |= DoremikkoKeyboard::MODE_A_2;
						if (keyboard[ DIK_AT    ] & 0x80) keys |= DoremikkoKeyboard::MODE_A_3;
					}
					break;
			}

			doremikko.keys = keys;

			return true;
		}

		bool NST_CALLBACK Input::Callbacks::PollMahjong(UserData data,Mahjong& mahjong,uint part)
		{
			Input& input = *static_cast<Input*>(data);

			input.CheckPoll();

			const Key* const NST_RESTRICT keys = input.dialog->GetSettings().GetKeys(Settings::MAHJONG_KEYS);

			uint buttons = 0;

			switch (part)
			{
				case Mahjong::PART_1:

					keys[ Settings::MAHJONG_KEY_I ].GetState( buttons, Mahjong::PART_1_I );
					keys[ Settings::MAHJONG_KEY_J ].GetState( buttons, Mahjong::PART_1_J );
					keys[ Settings::MAHJONG_KEY_K ].GetState( buttons, Mahjong::PART_1_K );
					keys[ Settings::MAHJONG_KEY_L ].GetState( buttons, Mahjong::PART_1_L );
					keys[ Settings::MAHJONG_KEY_M ].GetState( buttons, Mahjong::PART_1_M );
					keys[ Settings::MAHJONG_KEY_N ].GetState( buttons, Mahjong::PART_1_N );
					break;

				case Mahjong::PART_2:

					keys[ Settings::MAHJONG_KEY_A ].GetState( buttons, Mahjong::PART_2_A );
					keys[ Settings::MAHJONG_KEY_B ].GetState( buttons, Mahjong::PART_2_B );
					keys[ Settings::MAHJONG_KEY_C ].GetState( buttons, Mahjong::PART_2_C );
					keys[ Settings::MAHJONG_KEY_D ].GetState( buttons, Mahjong::PART_2_D );
					keys[ Settings::MAHJONG_KEY_E ].GetState( buttons, Mahjong::PART_2_E );
					keys[ Settings::MAHJONG_KEY_F ].GetState( buttons, Mahjong::PART_2_F );
					keys[ Settings::MAHJONG_KEY_G ].GetState( buttons, Mahjong::PART_2_G );
					keys[ Settings::MAHJONG_KEY_H ].GetState( buttons, Mahjong::PART_2_H );
					break;

				case Mahjong::PART_3:

					keys[ Settings::MAHJONG_KEY_SELECT ].GetState( buttons, Mahjong::PART_3_SELECT );
					keys[ Settings::MAHJONG_KEY_START  ].GetState( buttons, Mahjong::PART_3_START  );
					keys[ Settings::MAHJONG_KEY_KAN    ].GetState( buttons, Mahjong::PART_3_KAN    );
					keys[ Settings::MAHJONG_KEY_PON    ].GetState( buttons, Mahjong::PART_3_PON    );
					keys[ Settings::MAHJONG_KEY_CHII   ].GetState( buttons, Mahjong::PART_3_CHII   );
					keys[ Settings::MAHJONG_KEY_REACH  ].GetState( buttons, Mahjong::PART_3_REACH  );
					keys[ Settings::MAHJONG_KEY_RON    ].GetState( buttons, Mahjong::PART_3_RON    );
					break;
			}

			mahjong.buttons = buttons;

			return true;
		}

		bool NST_CALLBACK Input::Callbacks::PollExcitingBoxing(UserData data,ExcitingBoxing& excitingBoxing,uint part)
		{
			Input& input = *static_cast<Input*>(data);

			input.CheckPoll();

			const Key* const NST_RESTRICT keys = input.dialog->GetSettings().GetKeys(Settings::EXCITINGBOXING_KEYS);

			uint buttons = 0;

			switch (part)
			{
				case ExcitingBoxing::PART_1:

					keys[ Settings::EXCITINGBOXING_KEY_RIGHT_HOOK ].GetState( buttons, ExcitingBoxing::PART_1_RIGHT_HOOK );
					keys[ Settings::EXCITINGBOXING_KEY_RIGHT_MOVE ].GetState( buttons, ExcitingBoxing::PART_1_RIGHT_MOVE );
					keys[ Settings::EXCITINGBOXING_KEY_LEFT_MOVE  ].GetState( buttons, ExcitingBoxing::PART_1_LEFT_MOVE  );
					keys[ Settings::EXCITINGBOXING_KEY_LEFT_HOOK  ].GetState( buttons, ExcitingBoxing::PART_1_LEFT_HOOK  );
					break;

				case ExcitingBoxing::PART_2:

					keys[ Settings::EXCITINGBOXING_KEY_STRAIGHT   ].GetState( buttons, ExcitingBoxing::PART_2_STRAIGHT   );
					keys[ Settings::EXCITINGBOXING_KEY_RIGHT_JABB ].GetState( buttons, ExcitingBoxing::PART_2_RIGHT_JABB );
					keys[ Settings::EXCITINGBOXING_KEY_BODY       ].GetState( buttons, ExcitingBoxing::PART_2_BODY       );
					keys[ Settings::EXCITINGBOXING_KEY_LEFT_JABB  ].GetState( buttons, ExcitingBoxing::PART_2_LEFT_JABB  );
					break;
			}

			excitingBoxing.buttons = buttons;

			return true;
		}

		bool NST_CALLBACK Input::Callbacks::PollTopRider(UserData data,TopRider& topRider)
		{
			Input& input = *static_cast<Input*>(data);

			input.CheckPoll();

			const Key* const NST_RESTRICT keys = input.dialog->GetSettings().GetKeys(Settings::PAD1_KEYS);

			uint buttons = 0;

			keys[ Settings::PAD_KEY_UP     ].GetState( buttons, TopRider::SHIFT_GEAR  );
			keys[ Settings::PAD_KEY_RIGHT  ].GetState( buttons, TopRider::STEER_RIGHT );
			keys[ Settings::PAD_KEY_DOWN   ].GetState( buttons, TopRider::REAR        );
			keys[ Settings::PAD_KEY_LEFT   ].GetState( buttons, TopRider::STEER_LEFT  );
			keys[ Settings::PAD_KEY_SELECT ].GetState( buttons, TopRider::SELECT      );
			keys[ Settings::PAD_KEY_START  ].GetState( buttons, TopRider::START       );
			keys[ Settings::PAD_KEY_A      ].GetState( buttons, TopRider::ACCEL       );
			keys[ Settings::PAD_KEY_B      ].GetState( buttons, TopRider::BRAKE       );

			topRider.buttons = buttons;

			return true;
		}

		bool NST_CALLBACK Input::Callbacks::PollPokkunMoguraa(UserData data,PokkunMoguraa& pokkunMoguraa,uint row)
		{
			Input& input = *static_cast<Input*>(data);

			input.CheckPoll();

			const Key* const NST_RESTRICT keys = input.dialog->GetSettings().GetKeys(Settings::POKKUNMOGURAA_KEYS);

			uint buttons = 0;

			if (row & PokkunMoguraa::ROW_1)
			{
				keys[ Settings::POKKUNMOGURAA_KEY_ROW_1_1 ].GetState( buttons, PokkunMoguraa::BUTTON_1 );
				keys[ Settings::POKKUNMOGURAA_KEY_ROW_1_2 ].GetState( buttons, PokkunMoguraa::BUTTON_2 );
				keys[ Settings::POKKUNMOGURAA_KEY_ROW_1_3 ].GetState( buttons, PokkunMoguraa::BUTTON_3 );
				keys[ Settings::POKKUNMOGURAA_KEY_ROW_1_4 ].GetState( buttons, PokkunMoguraa::BUTTON_4 );
			}

			if (row & PokkunMoguraa::ROW_2)
			{
				keys[ Settings::POKKUNMOGURAA_KEY_ROW_2_1 ].GetState( buttons, PokkunMoguraa::BUTTON_1 );
				keys[ Settings::POKKUNMOGURAA_KEY_ROW_2_2 ].GetState( buttons, PokkunMoguraa::BUTTON_2 );
				keys[ Settings::POKKUNMOGURAA_KEY_ROW_2_3 ].GetState( buttons, PokkunMoguraa::BUTTON_3 );
				keys[ Settings::POKKUNMOGURAA_KEY_ROW_2_4 ].GetState( buttons, PokkunMoguraa::BUTTON_4 );
			}

			if (row & PokkunMoguraa::ROW_3)
			{
				keys[ Settings::POKKUNMOGURAA_KEY_ROW_3_1 ].GetState( buttons, PokkunMoguraa::BUTTON_1 );
				keys[ Settings::POKKUNMOGURAA_KEY_ROW_3_2 ].GetState( buttons, PokkunMoguraa::BUTTON_2 );
				keys[ Settings::POKKUNMOGURAA_KEY_ROW_3_3 ].GetState( buttons, PokkunMoguraa::BUTTON_3 );
				keys[ Settings::POKKUNMOGURAA_KEY_ROW_3_4 ].GetState( buttons, PokkunMoguraa::BUTTON_4 );
			}

			pokkunMoguraa.buttons = buttons;

			return true;
		}

		bool NST_CALLBACK Input::Callbacks::PollPartyTap(UserData data,PartyTap& partyTap)
		{
			Input& input = *static_cast<Input*>(data);

			input.CheckPoll();

			const Key* const NST_RESTRICT keys = input.dialog->GetSettings().GetKeys(Settings::PARTYTAP_KEYS);

			uint units = 0;

			keys[ Settings::PARTYTAP_UNIT_1 ].GetState( units, PartyTap::UNIT_1 );
			keys[ Settings::PARTYTAP_UNIT_2 ].GetState( units, PartyTap::UNIT_2 );
			keys[ Settings::PARTYTAP_UNIT_3 ].GetState( units, PartyTap::UNIT_3 );
			keys[ Settings::PARTYTAP_UNIT_4 ].GetState( units, PartyTap::UNIT_4 );
			keys[ Settings::PARTYTAP_UNIT_5 ].GetState( units, PartyTap::UNIT_5 );
			keys[ Settings::PARTYTAP_UNIT_6 ].GetState( units, PartyTap::UNIT_6 );

			partyTap.units = units;

			return true;
		}

		bool NST_CALLBACK Input::Callbacks::PollVsSystem(UserData data,VsSystem& vsSystem)
		{
			Input& input = *static_cast<Input*>(data);

			input.CheckPoll();

			const Key* const NST_RESTRICT keys = input.dialog->GetSettings().GetKeys(Settings::EMULATION_KEYS);

			uint buttons = 0;

			keys[ Settings::EMULATION_KEY_INSERT_COIN_1 ].GetState( buttons, VsSystem::COIN_1 );
			keys[ Settings::EMULATION_KEY_INSERT_COIN_2 ].GetState( buttons, VsSystem::COIN_2 );

			vsSystem.insertCoin = buttons;

			return true;
		}

		bool NST_CALLBACK Input::Callbacks::PollKaraokeStudio(UserData data,KaraokeStudio& karaokeStudio)
		{
			Input& input = *static_cast<Input*>(data);

			input.CheckPoll();

			const Key* const NST_RESTRICT keys = input.dialog->GetSettings().GetKeys(Settings::KARAOKESTUDIO_KEYS);

			uint buttons = 0;

			keys[ Settings::KARAOKESTUDIO_MIC ].GetState( buttons, KaraokeStudio::MIC );
			keys[ Settings::KARAOKESTUDIO_A   ].GetState( buttons, KaraokeStudio::A   );
			keys[ Settings::KARAOKESTUDIO_B   ].GetState( buttons, KaraokeStudio::B   );

			karaokeStudio.buttons = buttons;

			return true;
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
