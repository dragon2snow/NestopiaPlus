////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003 Martin Freij
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
#include "NstInputManager.h"
#include "NstApplication.h"
#include "../paradox/PdxFile.h"
#include "../paradox/PdxSet.h"
#include <WindowsX.h>

#define NST_USE_JOYSTICK 0x80000000UL

const UINT INPUTMANAGER::MenuHeight = GetSystemMetrics(SM_CYMENU);

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NES::IO::INPUT::PAD::Poll(const UINT index) 
{ PDX_CAST(INPUTMANAGER*,device)->Poll(*this,index); }

VOID NES::IO::INPUT::POWERPAD::Poll() 
{ PDX_CAST(INPUTMANAGER*,device)->Poll(*this); }

VOID NES::IO::INPUT::VS::Poll() 
{ PDX_CAST(INPUTMANAGER*,device)->Poll(*this); }

VOID NES::IO::INPUT::FAMILYKEYBOARD::Poll(const UINT part,const UINT mode)
{ PDX_CAST(INPUTMANAGER*,device)->Poll(*this,part,mode); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

INPUTMANAGER::INPUTMANAGER()
: 
MANAGER              (IDD_INPUT), 
hDlg                 (NULL),
AutoFireStep         (0),
SelectDevice         (0),
SelectKey            (0),
SpeedThrottle        (0),
SpeedThrottleKeyDown (FALSE),
SaveSlot             (0),
SaveSlotKeyDown      (FALSE),
LoadSlot             (0),
LoadSlotKeyDown      (FALSE)
{
	format.pad[0].device =
	format.pad[1].device =
	format.pad[2].device =
	format.pad[3].device =
	format.PowerPad.device =
	format.vs.device =
	format.FamilyKeyboard.device = PDX_CAST(VOID*,this);

	map.category[ CATEGORY_PAD1     ].keys.Resize( NUM_PAD_KEYS      );
	map.category[ CATEGORY_PAD2     ].keys.Resize( NUM_PAD_KEYS      );
	map.category[ CATEGORY_PAD3     ].keys.Resize( NUM_PAD_KEYS      );
	map.category[ CATEGORY_PAD4     ].keys.Resize( NUM_PAD_KEYS      );
	map.category[ CATEGORY_POWERPAD ].keys.Resize( NUM_POWERPAD_KEYS );
	map.category[ CATEGORY_GENERAL  ].keys.Resize( NUM_GENERAL_KEYS  );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID INPUTMANAGER::Poll(NES::IO::INPUT::FAMILYKEYBOARD& family,const UINT part,const UINT mode)
{
	PDX_ASSERT(part < NES::IO::INPUT::FAMILYKEYBOARD::NUM_PARTS && mode < NES::IO::INPUT::FAMILYKEYBOARD::NUM_MODES);

    #define NST_2(a,b) (a | (WORD(b) << 8))

	static const WORD FamicomMap[9][2][4] =
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

	const BYTE* const PDX_RESTRICT buffer = GetKeyboardBuffer();

	UINT& key = family.parts[part];
	key = 0x00;

	for (UINT i=0; i < 4; ++i)
	{
		const UINT pushed = 
		(
     		(buffer[(FamicomMap[part][mode][i] & 0x00FF) >> 0] & 0x80) |
     		(buffer[(FamicomMap[part][mode][i] & 0xFF00) >> 8] & 0x80)
		);

		key |= pushed >> ( 7 - ( i + 1 ) );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID INPUTMANAGER::Poll(NES::IO::INPUT::PAD& pad,const UINT index)
{
	PDX_ASSERT(index <= CATEGORY_PAD4);

	const MAP::CATEGORY::KEYS& keys = map.category[index].keys;

	UINT buttons = 0;

	if (IsButtonPressed( keys[ KEY_UP     ] )) buttons  = NES::IO::INPUT::PAD::UP;
	if (IsButtonPressed( keys[ KEY_RIGHT  ] )) buttons |= NES::IO::INPUT::PAD::RIGHT;
	if (IsButtonPressed( keys[ KEY_DOWN   ] )) buttons |= NES::IO::INPUT::PAD::DOWN;
	if (IsButtonPressed( keys[ KEY_LEFT   ] )) buttons |= NES::IO::INPUT::PAD::LEFT;
	if (IsButtonPressed( keys[ KEY_SELECT ] )) buttons |= NES::IO::INPUT::PAD::SELECT;
	if (IsButtonPressed( keys[ KEY_START  ] )) buttons |= NES::IO::INPUT::PAD::START;
	if (IsButtonPressed( keys[ KEY_A      ] )) buttons |= NES::IO::INPUT::PAD::A;
	if (IsButtonPressed( keys[ KEY_B      ] )) buttons |= NES::IO::INPUT::PAD::B;

	if (++AutoFireStep >= 3)
	{
		if (AutoFireStep == 6)
			AutoFireStep = 0;

		if (IsButtonPressed( keys[ KEY_AUTOFIRE_A ] )) buttons |= NES::IO::INPUT::PAD::A;
		if (IsButtonPressed( keys[ KEY_AUTOFIRE_B ] )) buttons |= NES::IO::INPUT::PAD::B;
	}

	pad.buttons = buttons;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID INPUTMANAGER::Poll(NES::IO::INPUT::VS& vs)
{
	vs.InsertCoin = 
	(
     	( IsButtonPressed( map.category[ CATEGORY_GENERAL ].keys[ KEY_GENERAL_INSERT_COIN_1 ] ) ? NES::IO::INPUT::VS::COIN_1 : 0 ) |
     	( IsButtonPressed( map.category[ CATEGORY_GENERAL ].keys[ KEY_GENERAL_INSERT_COIN_2 ] ) ? NES::IO::INPUT::VS::COIN_2 : 0 )
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID INPUTMANAGER::Poll(NES::IO::INPUT::POWERPAD& PowerPad)
{
	for (UINT i=0; i < NUM_POWERPAD_SIDE_A_KEYS; ++i)
		PowerPad.SideA[i] = IsButtonPressed(map.category[CATEGORY_POWERPAD].keys[i]);

	for (UINT i=0; i < NUM_POWERPAD_SIDE_B_KEYS; ++i)
		PowerPad.SideB[i] = IsButtonPressed(map.category[CATEGORY_POWERPAD].keys[NUM_POWERPAD_SIDE_A_KEYS + i]);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

ULONG INPUTMANAGER::IsJoystickButtonPressed(ULONG key) const
{
	PDX_ASSERT(!(key & NST_USE_JOYSTICK));

	const DIJOYSTATE& state = joysticks[key / 64].state;

	key %= 64;

	if (key < 32)
		return state.rgbButtons[key];

	switch (key)
	{
     	case 32: return state.lX < 0;
     	case 33: return state.lX > 0;
    	case 34: return state.lY < 0;
     	case 35: return state.lY > 0;
		case 36: return state.lZ < 0;
		case 37: return state.lZ > 0;
		case 38: return state.lRx < 0;
		case 39: return state.lRx > 0;
     	case 40: return state.lRy < 0;
     	case 41: return state.lRy > 0;
     	case 42: return state.lRz < 0;
     	case 43: return state.lRz > 0;
     	case 44: return state.rglSlider[0] < 0;
     	case 45: return state.rglSlider[0] > 0;
     	case 46: return state.rglSlider[1] < 0;
		case 47: return state.rglSlider[1] > 0;
	}

	key -= 48;

	const DWORD pov = state.rgdwPOV[key / 4];

    switch (key % 4)
	{
		case 0: return ( pov & 0xFFFF ) != 0xFFFFU && ( pov >= 31500U || pov <=  4500U ); // up
		case 1: return ( pov & 0xFFFF ) != 0xFFFFU && ( pov >=  4500U && pov <= 13500U ); // right
		case 2: return ( pov & 0xFFFF ) != 0xFFFFU && ( pov >= 13500U && pov <= 22500U ); // down
		case 3: return ( pov & 0xFFFF ) != 0xFFFFU && ( pov >= 22500U && pov <= 31500U ); // left
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline ULONG INPUTMANAGER::IsButtonPressed(const ULONG key) const
{
	return 
	(
	    (key & NST_USE_JOYSTICK) ?
		IsJoystickButtonPressed(key & ~NST_USE_JOYSTICK) :
	    GetKeyboardBuffer()[key] & 0x80
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID INPUTMANAGER::Create(CONFIGFILE* const ConfigFile)
{
	SelectDevice = 0;
	SelectKey = 0;

	DIRECTINPUT::Initialize( MANAGER::hWnd );

	Reset();

	if (ConfigFile)
	{
		CONFIGFILE& file = *ConfigFile;
	
		PDXARRAY<UINT> indices;

		{
			PDXSTRING string("input device ");
	
			for (UINT i=0; ; ++i)
			{
				string.Resize(13);
				string << i;
	
				const PDXSTRING& input = file[string];
	
				if (input.IsEmpty())
					break;
	
				const JOYSTICK* const it(PDX::Find(joysticks.Begin(),joysticks.End(),CONFIGFILE::ToGUID(input.String())));
				indices.InsertBack( (it != joysticks.End()) ? UINT(it - joysticks.Begin()) : UINT_MAX );
			}
		}
	
		for (UINT i=0; i < NUM_CATEGORIES; ++i)
		{
			for (UINT j=0; j < map.category[i].keys.Size(); ++j)
			{
				const PDXSTRING& text = file[Map2Text(i,j)];

				if (text.Length())
				{
					const ULONG key = Text2Key(text);

					if (key & NST_USE_JOYSTICK)
					{
						const UINT index = (key & ~NST_USE_JOYSTICK) / 64;

						if (indices.Size() <= index || indices[index] == UINT_MAX)
							continue;

						if (PDX_FAILED(joysticks[indices[index]].Create(GetDevice(),DIRECTINPUT::hWnd)))
							continue;

						map.category[i].keys[j] = ((key & ~NST_USE_JOYSTICK) % 64) + (indices[index] * 64) | NST_USE_JOYSTICK;
					}
					else
					{
						map.category[i].keys[j] = key;
					}
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID INPUTMANAGER::Destroy(CONFIGFILE* const ConfigFile)
{
	if (ConfigFile)
	{
		CONFIGFILE& file = *ConfigFile;

		{
			PDXSTRING string("input device ");

			DIDEVICEINSTANCE info;
			DIRECTX::InitStruct(info);

			for (UINT i=0; i < joysticks.Size(); ++i)
			{
				if (joysticks[i].device)
				{
					string.Resize(13);
					string << i;
					file[string] = CONFIGFILE::FromGUID(joysticks[i].guid);
				}
			}
		}

		for (UINT i=0; i < NUM_CATEGORIES; ++i)
			for (UINT j=0; j < map.category[i].keys.Size(); ++j)
				file[Map2Text(i,j)] = Key2Text(map.category[i].keys[j]);
	}

	DIRECTINPUT::Destroy();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID INPUTMANAGER::Reset()
{
	for (UINT i=0; i < NUM_CATEGORIES; ++i)
		for (TSIZE j=0; j < map.category[i].keys.Size(); ++j)
			map.category[i].keys[j] = 0;

	map.category[ CATEGORY_PAD1 ].keys[ KEY_UP         ] = DIK_UP;
	map.category[ CATEGORY_PAD1 ].keys[ KEY_RIGHT      ] = DIK_RIGHT;
	map.category[ CATEGORY_PAD1 ].keys[ KEY_DOWN       ] = DIK_DOWN;
	map.category[ CATEGORY_PAD1 ].keys[ KEY_LEFT       ] = DIK_LEFT;
	map.category[ CATEGORY_PAD1 ].keys[ KEY_SELECT     ] = DIK_RSHIFT;
	map.category[ CATEGORY_PAD1 ].keys[ KEY_START      ] = DIK_RETURN;
	map.category[ CATEGORY_PAD1 ].keys[ KEY_A          ] = DIK_PERIOD;
	map.category[ CATEGORY_PAD1 ].keys[ KEY_B          ] = DIK_COMMA;
	map.category[ CATEGORY_PAD1 ].keys[ KEY_AUTOFIRE_A ] = DIK_L;
	map.category[ CATEGORY_PAD1 ].keys[ KEY_AUTOFIRE_B ] = DIK_K;
															
	map.category[ CATEGORY_PAD2 ].keys[ KEY_UP         ] = DIK_F;
	map.category[ CATEGORY_PAD2 ].keys[ KEY_RIGHT      ] = DIK_B;
	map.category[ CATEGORY_PAD2 ].keys[ KEY_DOWN       ] = DIK_V;
	map.category[ CATEGORY_PAD2 ].keys[ KEY_LEFT       ] = DIK_C;
	map.category[ CATEGORY_PAD2 ].keys[ KEY_SELECT     ] = DIK_A;
	map.category[ CATEGORY_PAD2 ].keys[ KEY_START      ] = DIK_S;
	map.category[ CATEGORY_PAD2 ].keys[ KEY_A          ] = DIK_X;
	map.category[ CATEGORY_PAD2 ].keys[ KEY_B          ] = DIK_Z;
	map.category[ CATEGORY_PAD2 ].keys[ KEY_AUTOFIRE_A ] = DIK_W;
	map.category[ CATEGORY_PAD2 ].keys[ KEY_AUTOFIRE_B ] = DIK_Q;

	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_1  ] = DIK_Q;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_2  ] = DIK_W;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_3  ] = DIK_E;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_4  ] = DIK_R;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_5  ] = DIK_A;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_6  ] = DIK_S;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_7  ] = DIK_D;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_8  ] = DIK_F;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_9  ] = DIK_Z;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_10 ] = DIK_X;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_11 ] = DIK_C;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_12 ] = DIK_V;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_3  ] = DIK_Y;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_2  ] = DIK_U;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_8  ] = DIK_G;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_7  ] = DIK_H;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_6  ] = DIK_J;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_5  ] = DIK_K;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_11 ] = DIK_N;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_10 ] = DIK_M;

	map.category[ CATEGORY_GENERAL ].keys[ KEY_GENERAL_INSERT_COIN_1 ] = DIK_F2;    
	map.category[ CATEGORY_GENERAL ].keys[ KEY_GENERAL_INSERT_COIN_2 ] = DIK_F3;    
	map.category[ CATEGORY_GENERAL ].keys[ KEY_GENERAL_TOGGLE_FPS    ] = DIK_F5;    
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID INPUTMANAGER::Poll()
{
	const BOOL polled = PollKeyboard();

	if (!PollJoysticks() && !polled)
		return;

	const MAP::CATEGORY::KEYS& keys = map.category[CATEGORY_GENERAL].keys;

	if (IsButtonPressed(keys[KEY_GENERAL_TOGGLE_FPS]))
	{
		if (!SpeedThrottleKeyDown)
		{
			SpeedThrottleKeyDown = TRUE;
			application.GetTimerManager().EnableCustomFPS( SpeedThrottle );
			SpeedThrottle ^= 1;
		}
		return;
	}

	SpeedThrottleKeyDown = FALSE;

	if (nes.IsOn() && nes.IsImage())
	{
		if (IsButtonPressed(keys[KEY_GENERAL_SAVE_SLOT]))
		{
			if (!SaveSlotKeyDown)
			{
				SaveSlotKeyDown = TRUE;
				application.OnSaveStateSlot(IDM_FILE_QUICK_SAVE_STATE_SLOT_NEXT);
				SaveSlot ^= 1;
			}
			return;
		}

		SaveSlotKeyDown = FALSE;

		if (IsButtonPressed(keys[KEY_GENERAL_LOAD_SLOT]))
		{
			if (!LoadSlotKeyDown)
			{
				LoadSlotKeyDown = TRUE;
				application.OnLoadStateSlot(IDM_FILE_QUICK_LOAD_STATE_SLOT_LAST);
				LoadSlot ^= 1;
			}
			return;
		}

		LoadSlotKeyDown = FALSE;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL INPUTMANAGER::PollJoysticks()
{
	BOOL active = FALSE;

	const UINT NumDevices = joysticks.Size();

	for (UINT i=0; i < NumDevices; ++i)
	{
		JOYSTICK& joy = joysticks[i];

		if (joy.device)
		{
			if (joy.device->Poll() == DI_OK)
			{
				if (joy.device->GetDeviceState(sizeof(DIJOYSTATE),PDX_CAST(LPVOID,&joy.state)) == DI_OK)
				{
					active = TRUE;
					continue;
				}
			}

			Acquire(joy.device);
			PDXMemZero(joy.state);
		}
	}

	return active;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL INPUTMANAGER::OnMouseMove(POINT& point)
{
	RECT rcClient;

	if (application.IsWindowed())
	{
		GetClientRect( MANAGER::hWnd, &rcClient );
	}
	else
	{
		rcClient.right = application.GetGraphicManager().GetDisplayWidth();
		rcClient.bottom = application.GetGraphicManager().GetDisplayHeight();

		if (application.IsMenuSet())
			point.y += MenuHeight;
	}

	const RECT& rcNes = application.GetGraphicManager().GetNesRect();

	const UINT width  = rcNes.right - rcNes.left;
	const UINT height = rcNes.bottom - rcNes.top;

	const INT y = INT( point.y / ( FLOAT( rcClient.bottom ) / FLOAT( height ) ) );
	const INT x = INT( point.x / ( FLOAT( rcClient.right  ) / FLOAT( width  ) ) );

	if (x >= 0 && x < width && y >= 0 && y < height)
	{
		format.zapper.x = format.paddle.x = x;
		format.zapper.y = y;
		return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK INPUTMANAGER::StaticKeyPressDialogProc(HWND hDlg,UINT uMsg,WPARAM,LPARAM lParam)
{
	static INPUTMANAGER* im = NULL;

	switch (uMsg)
	{
     	case WM_INITDIALOG:
		
			PDX_ASSERT( !im );
			im = PDX_CAST(INPUTMANAGER*,lParam);
			SetTimer( hDlg, 666, 100, NULL );
			SetWindowPos( hDlg, HWND_TOP, 0, 0, 0, 0, SWP_HIDEWINDOW );
			return TRUE;
		
		case WM_KEYDOWN:
			return TRUE;

		case WM_TIMER:
		{
			PDX_ASSERT( im );

			DWORD key = 0;
				
			if (im->ScanKeyboard( key ))
			{
				switch (key)
				{
     				case DIK_LALT:
       				case DIK_0:
					case DIK_1:
					case DIK_2:
					case DIK_3:
					case DIK_4:
					case DIK_5:
					case DIK_6:
					case DIK_7:
					case DIK_8:
					case DIK_9:

						KillTimer( hDlg, 666 );
						application.OnWarning("This key is reserved!");

					case DIK_ESCAPE:

						EndDialog( hDlg, 0 );
						return TRUE;
				}

				im->map.category[im->SelectDevice].keys[im->SelectKey] = key;				
			}
			else
			{
				UINT index;

				if (im->ScanJoystick(key,index))
				{
					key += (64 * index) | NST_USE_JOYSTICK;
					im->map.category[im->SelectDevice].keys[im->SelectKey] = key;
				}
			}

			if (key)
			{
				HWND hMap = GetDlgItem( im->hDlg, IDC_INPUT_MAP );

				ListBox_DeleteString( hMap, im->SelectKey );
				ListBox_InsertString( hMap, im->SelectKey, Key2Text( key ) );	

				EndDialog( hDlg, 0 );
			}
  	
			return TRUE;
		}

		case WM_DESTROY:

			PDX_ASSERT( im );
			im = NULL;
			KillTimer( hDlg, 666 );
			return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL INPUTMANAGER::DialogProc(HWND h,UINT uMsg,WPARAM wParam,LPARAM)
{
	switch (uMsg) 
	{
     	case WM_INITDIALOG:

			hDlg = h;
			SelectKey = 0;

			for (UINT i=0; i < joysticks.Size(); ++i)
				joysticks[i].Create( GetDevice(), DIRECTINPUT::hWnd );

			SetCooperativeLevel(DISCL_NONEXCLUSIVE|DISCL_BACKGROUND);
			UpdateDialog();
    		return TRUE;

		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
       			case IDC_INPUT_DEVICES:

					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						SelectDevice = ListBox_GetCurSel( GetDlgItem( hDlg, IDC_INPUT_DEVICES ) );
						UpdateDlgButtonTexts();
					}
					return TRUE;

				case IDC_INPUT_MAP:

					if (HIWORD(wParam) == CBN_SELCHANGE)
						SelectKey = ListBox_GetCurSel( GetDlgItem( hDlg, IDC_INPUT_MAP ) );

					return TRUE;

				case IDC_INPUT_SET:

					ScanInput();
					return TRUE;

				case IDC_INPUT_SETALL:
				{
					HWND hItem = GetDlgItem( hDlg, IDC_INPUT_MAP );
					const UINT count = ListBox_GetCount( hItem );

					for (SelectKey=0; SelectKey < count;)
					{
						ListBox_SetCurSel( hItem, SelectKey );

						ScanInput();

						const DWORD key = map.category[SelectDevice].keys[SelectKey];

						do 
						{	
							if (key & NST_USE_JOYSTICK) PollJoysticks();
							else                        PollKeyboard();
						} 
						while (IsButtonPressed(key));

						++SelectKey;
					}
				}
				return TRUE;

				case IDC_INPUT_CLEAR:

					Clear();
					return TRUE;

				case IDC_INPUT_OK:

					EndDialog( hDlg, 0 );
					return TRUE;

				case IDC_INPUT_DEFAULT:

					Reset();
					UpdateDialog();
					return TRUE;
			}
			return FALSE;

     	case WM_CLOSE:

     		EndDialog( hDlg, 0 );
     		return TRUE;

		case WM_DESTROY:

			UpdateJoystickDevices();
			SetCooperativeLevel(DISCL_NONEXCLUSIVE|DISCL_FOREGROUND);
			hDlg = NULL;
			return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID INPUTMANAGER::UpdateDialog()
{
	HWND hDevice = GetDlgItem( hDlg, IDC_INPUT_DEVICES );

	ListBox_ResetContent( hDevice );
	
	ListBox_AddString( hDevice, "Pad 1"     );
	ListBox_AddString( hDevice, "Pad 2"     );
	ListBox_AddString( hDevice, "Pad 3"     );
	ListBox_AddString( hDevice, "Pad 4"     );
	ListBox_AddString( hDevice, "Power Pad" );
	ListBox_AddString( hDevice, "General"   );

	ListBox_SetCurSel( hDevice, SelectDevice );

	UpdateDlgButtonTexts();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID INPUTMANAGER::UpdateDlgButtonTexts()
{
	HWND hKeys = GetDlgItem( hDlg, IDC_INPUT_KEYS );
	HWND hMap  = GetDlgItem( hDlg, IDC_INPUT_MAP  );

	ListBox_ResetContent( hKeys );
	ListBox_ResetContent( hMap  );

	switch (SelectDevice)
	{
     	case CATEGORY_PAD1:
    	case CATEGORY_PAD2:
    	case CATEGORY_PAD3:
     	case CATEGORY_PAD4:

			ListBox_InsertString( hKeys, KEY_A,          "A"           );
			ListBox_InsertString( hKeys, KEY_B,          "B"           );
			ListBox_InsertString( hKeys, KEY_SELECT,     "Select"      );
			ListBox_InsertString( hKeys, KEY_START,      "Start"       );
			ListBox_InsertString( hKeys, KEY_UP,         "Up"          );
			ListBox_InsertString( hKeys, KEY_DOWN,       "Down"        );
			ListBox_InsertString( hKeys, KEY_LEFT,       "Left"        );
			ListBox_InsertString( hKeys, KEY_RIGHT,      "Right"       );
			ListBox_InsertString( hKeys, KEY_AUTOFIRE_A, "Auto-Fire A" );
			ListBox_InsertString( hKeys, KEY_AUTOFIRE_B, "Auto-Fire B" );

			ListBox_InsertString( hMap, KEY_A,          Key2Text( map.category[ SelectDevice ].keys[ KEY_A          ] ) );
			ListBox_InsertString( hMap, KEY_B,          Key2Text( map.category[ SelectDevice ].keys[ KEY_B          ] ) );
			ListBox_InsertString( hMap, KEY_SELECT,     Key2Text( map.category[ SelectDevice ].keys[ KEY_SELECT     ] ) );
			ListBox_InsertString( hMap, KEY_START,      Key2Text( map.category[ SelectDevice ].keys[ KEY_START      ] ) );
			ListBox_InsertString( hMap, KEY_UP,         Key2Text( map.category[ SelectDevice ].keys[ KEY_UP         ] ) );
			ListBox_InsertString( hMap, KEY_DOWN,       Key2Text( map.category[ SelectDevice ].keys[ KEY_DOWN       ] ) );
			ListBox_InsertString( hMap, KEY_LEFT,       Key2Text( map.category[ SelectDevice ].keys[ KEY_LEFT       ] ) );
			ListBox_InsertString( hMap, KEY_RIGHT,      Key2Text( map.category[ SelectDevice ].keys[ KEY_RIGHT      ] ) );
			ListBox_InsertString( hMap, KEY_AUTOFIRE_A, Key2Text( map.category[ SelectDevice ].keys[ KEY_AUTOFIRE_A ] ) );
			ListBox_InsertString( hMap, KEY_AUTOFIRE_B, Key2Text( map.category[ SelectDevice ].keys[ KEY_AUTOFIRE_B ] ) );
			break;

		case CATEGORY_POWERPAD:

			ListBox_AddString( hKeys, "Side A - 1"  );
			ListBox_AddString( hKeys, "Side A - 2"  );
			ListBox_AddString( hKeys, "Side A - 3"  );
			ListBox_AddString( hKeys, "Side A - 4"  );
			ListBox_AddString( hKeys, "Side A - 5"  );
			ListBox_AddString( hKeys, "Side A - 6"  );
			ListBox_AddString( hKeys, "Side A - 7"  );
			ListBox_AddString( hKeys, "Side A - 8"  );
			ListBox_AddString( hKeys, "Side A - 9"  );
			ListBox_AddString( hKeys, "Side A - 10" );
			ListBox_AddString( hKeys, "Side A - 11" );
			ListBox_AddString( hKeys, "Side A - 12" );
			ListBox_AddString( hKeys, "Side B - 3"  );
			ListBox_AddString( hKeys, "Side B - 2"  );
			ListBox_AddString( hKeys, "Side B - 8"  );
			ListBox_AddString( hKeys, "Side B - 7"  );
			ListBox_AddString( hKeys, "Side B - 6"  );
			ListBox_AddString( hKeys, "Side B - 5"  );
			ListBox_AddString( hKeys, "Side B - 11" );
			ListBox_AddString( hKeys, "Side B - 10" );

			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_1  ] ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_2  ] ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_3  ] ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_4  ] ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_5  ] ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_6  ] ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_7  ] ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_8  ] ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_9  ] ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_10 ] ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_11 ] ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_12 ] ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_3  ] ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_2  ] ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_8  ] ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_7  ] ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_6  ] ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_5  ] ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_11 ] ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_10 ] ) );
			break;

       	case CATEGORY_GENERAL:

			ListBox_AddString( hKeys, "Insert Coin (slot 1)" );
			ListBox_AddString( hKeys, "Insert Coin (slot 2)" );
			ListBox_AddString( hKeys, "Speed Throttle"       );
			ListBox_AddString( hKeys, "Save To Next Slot"    );
			ListBox_AddString( hKeys, "Load From Last Slot"  );

			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_GENERAL ].keys[ KEY_GENERAL_INSERT_COIN_1 ] ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_GENERAL ].keys[ KEY_GENERAL_INSERT_COIN_2 ] ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_GENERAL ].keys[ KEY_GENERAL_TOGGLE_FPS    ] ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_GENERAL ].keys[ KEY_GENERAL_SAVE_SLOT     ] ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_GENERAL ].keys[ KEY_GENERAL_LOAD_SLOT     ] ) );
			break;
	}

	ListBox_SetCurSel( hMap, SelectKey );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID INPUTMANAGER::Clear()
{
	for (TSIZE i=0; i < map.category[ SelectDevice ].keys.Size(); ++i)
		map.category[ SelectDevice ].keys[ i ] = 0;

	UpdateDlgButtonTexts();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID INPUTMANAGER::ScanInput()
{
	HWND hItem = GetDlgItem( hDlg, IDC_INPUT_KEYPRESS_TEXT );
	SetWindowText( hItem, "Press any key/button to use, ESC to skip.." );
	
	DialogBoxParam
	( 
    	GetModuleHandle(NULL), 
		MAKEINTRESOURCE(IDD_INPUT_KEYPRESS), 
		hDlg, 
		StaticKeyPressDialogProc,
		PDX_CAST(LPARAM,this)
	); 
	
	SetWindowText( hItem, "" );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID INPUTMANAGER::UpdateJoystickDevices()
{
	for (UINT i=0; i < joysticks.Size(); ++i)
		joysticks[i].state.lX = 0;

	typedef PDXSET<const LPDIRECTINPUTDEVICE8> ACTIVEDEVICES;

	ACTIVEDEVICES ActiveDevices;

	for (UINT i=0; i < NUM_CATEGORIES; ++i)
	{
		for (UINT j=0; j < map.category[i].keys.Size(); ++j)
		{
			const DWORD key = map.category[i].keys[j];

			if (key & NST_USE_JOYSTICK)
			{
				const LPDIRECTINPUTDEVICE8 device = joysticks[(key & ~NST_USE_JOYSTICK) / 64].device;

				if (ActiveDevices.Find(device) == ActiveDevices.End())
				{
					ActiveDevices.Insert(device);

					JOYSTICK* const joy = PDX::Find(joysticks.Begin(),joysticks.End(),device);

					if (joy != joysticks.End())
						joy->state.lX = LONG_MAX;
				}
			}
		}
	}

	for (UINT i=0; i < joysticks.Size(); ++i)
	{
		if (joysticks[i].state.lX != LONG_MAX)
			joysticks[i].Release();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

const CHAR* INPUTMANAGER::Map2Text(const UINT category,const UINT key)
{
	PDX_ASSERT(category < NUM_CATEGORIES);

	if (category < NUM_CATEGORIES)
	{
		static CHAR buffer[128];

		static const CHAR* types[NUM_CATEGORIES] =
		{
			"input pad1 ",
			"input pad2 ",
			"input pad3 ",
			"input pad4 ",
			"input powerpad ",
			"input general "
		};

		strcpy( buffer, types[category] );
		const CHAR* button = NULL;

		switch (category)
		{
			case CATEGORY_PAD1:
			case CATEGORY_PAD2:
			case CATEGORY_PAD3:
			case CATEGORY_PAD4:

				switch (key)
				{
					case KEY_A:          button = "a";			 break;
					case KEY_B:          button = "b";			 break;
					case KEY_SELECT:     button = "select";	     break;
					case KEY_START:      button = "start";	     break;
					case KEY_UP:         button = "up";		     break;
					case KEY_DOWN:       button = "down";	     break;
					case KEY_LEFT:       button = "left";	     break;
					case KEY_RIGHT:      button = "right";	     break;
					case KEY_AUTOFIRE_A: button = "auto fire a"; break;
					case KEY_AUTOFIRE_B: button = "auto fire b"; break;
					default: return NULL;
				}
				break;

			case CATEGORY_POWERPAD:

				switch (key)
				{
					case KEY_POWERPAD_SIDE_A_1:	 button = "side a 1";  break;
					case KEY_POWERPAD_SIDE_A_2:	 button = "side a 2";  break;
					case KEY_POWERPAD_SIDE_A_3:	 button = "side a 3";  break;
					case KEY_POWERPAD_SIDE_A_4:	 button = "side a 4";  break;
					case KEY_POWERPAD_SIDE_A_5:	 button = "side a 5";  break;
					case KEY_POWERPAD_SIDE_A_6:	 button = "side a 6";  break;
					case KEY_POWERPAD_SIDE_A_7:	 button = "side a 7";  break;
					case KEY_POWERPAD_SIDE_A_8:	 button = "side a 8";  break;
					case KEY_POWERPAD_SIDE_A_9:	 button = "side a 9";  break;
					case KEY_POWERPAD_SIDE_A_10: button = "side a 10"; break;
					case KEY_POWERPAD_SIDE_A_11: button = "side a 11"; break;
					case KEY_POWERPAD_SIDE_A_12: button = "side a 12"; break;
					case KEY_POWERPAD_SIDE_B_3:  button = "side b 3";  break;
					case KEY_POWERPAD_SIDE_B_2:	 button = "side b 2";  break;
					case KEY_POWERPAD_SIDE_B_8:	 button = "side b 8";  break;
					case KEY_POWERPAD_SIDE_B_7:	 button = "side b 7";  break;
					case KEY_POWERPAD_SIDE_B_6:	 button = "side b 6";  break;
					case KEY_POWERPAD_SIDE_B_5:	 button = "side b 5";  break;
					case KEY_POWERPAD_SIDE_B_11: button = "side b 11"; break;
					case KEY_POWERPAD_SIDE_B_10: button = "side b 10"; break;
					default: return NULL;
				}
				break;

			case CATEGORY_GENERAL:

				switch (key)
				{
					case KEY_GENERAL_INSERT_COIN_1: button = "insert coin 1";       break;
					case KEY_GENERAL_INSERT_COIN_2: button = "insert coin 2";       break;
					case KEY_GENERAL_TOGGLE_FPS:    button = "speed throttle";      break;
					case KEY_GENERAL_SAVE_SLOT:     button = "save to next slot";   break;
					case KEY_GENERAL_LOAD_SLOT:     button = "load from last slot"; break;
					default: return NULL;
				}
				break;
		}

		strcat( buffer, button );

		return buffer;
	}

	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#define NST_CASE(a,b) case a: return b;

const CHAR* INPUTMANAGER::Key2Text(DWORD key)
{
	if (key & NST_USE_JOYSTICK)
	{
		key &= ~NST_USE_JOYSTICK;

		PDXSTRING text;

		text << "(joy ";
		text << (key / 64);
		text << ") ";

		key %= 64;

		switch (key)
		{
     		case 32: text << "-x";  break;
     		case 33: text << "+x";  break;
     		case 34: text << "+y";  break;
     		case 35: text << "-y";  break;
			case 36: text << "+z";  break;
			case 37: text << "-z";  break;
			case 38: text << "-rx"; break;
			case 39: text << "+rx"; break;
			case 40: text << "+ry"; break;
			case 41: text << "-ry"; break;
			case 42: text << "+rz"; break;
			case 43: text << "-rz"; break;
			case 44: text << "-s0"; break;
			case 45: text << "+s0"; break;
			case 46: text << "-s1"; break;
			case 47: text << "+s1"; break;
			case 48: text << "+py"; break;
			case 49: text << "+px"; break;
			case 50: text << "-py"; break;
			case 51: text << "-px"; break;			
			case 52: text << "+py"; break;
			case 53: text << "+px"; break;
			case 54: text << "-py"; break;
			case 55: text << "-px"; break;
			case 56: text << "+py"; break;
			case 57: text << "+px"; break;
			case 58: text << "-py"; break;
			case 59: text << "-px"; break;
			case 60: text << "+py"; break;
			case 61: text << "+px"; break;
			case 62: text << "-py"; break;
			case 63: text << "-px"; break;
			default: text << key;   break;
		}

		static CHAR buffer[24];
		strcpy( buffer, text.String() );

		return buffer;
	}
	else
	{
		switch (key)
		{
			NST_CASE( DIK_ESCAPE       , "ESCAPE"       )      
			NST_CASE( DIK_1            , "1"			)
			NST_CASE( DIK_2            , "2"            )      
			NST_CASE( DIK_3            , "3"            )      
			NST_CASE( DIK_4            , "4"            )      
			NST_CASE( DIK_5            , "5"            )      
			NST_CASE( DIK_6            , "6"            )      
			NST_CASE( DIK_7            , "7"            )      
			NST_CASE( DIK_8            , "8"            )      
			NST_CASE( DIK_9            , "9"            )      
			NST_CASE( DIK_0            , "0"            )      
			NST_CASE( DIK_MINUS        , "-"            )      
			NST_CASE( DIK_EQUALS       , "="            )      
			NST_CASE( DIK_BACK         , "BACK"         )      
			NST_CASE( DIK_TAB          , "TAB"          )      
			NST_CASE( DIK_Q            , "Q"            )      
			NST_CASE( DIK_W            , "W"            )      
			NST_CASE( DIK_E            , "E"            )      
			NST_CASE( DIK_R            , "R"            )      
			NST_CASE( DIK_T            , "T"            )      
			NST_CASE( DIK_Y            , "Y"            )      
			NST_CASE( DIK_U            , "U"            )      
			NST_CASE( DIK_I            , "I"            )      
			NST_CASE( DIK_O            , "O"            )      
			NST_CASE( DIK_P            , "P"            )      
			NST_CASE( DIK_LBRACKET     , "["            )      
			NST_CASE( DIK_RBRACKET     , "]"            )      
			NST_CASE( DIK_RETURN       , "RETURN"       )      
			NST_CASE( DIK_LCONTROL     , "LCTRL"        )      
			NST_CASE( DIK_A            , "A"            )      
			NST_CASE( DIK_S            , "S"            )      
			NST_CASE( DIK_D            , "D"            )      
			NST_CASE( DIK_F            , "F"            )      
			NST_CASE( DIK_G            , "G"            )      
			NST_CASE( DIK_H            , "H"            )      
			NST_CASE( DIK_J            , "J"            )      
			NST_CASE( DIK_K            , "K"            )      
			NST_CASE( DIK_L            , "L"            )      
			NST_CASE( DIK_SEMICOLON    , ";"            )      
			NST_CASE( DIK_APOSTROPHE   , "'"            )      
			NST_CASE( DIK_GRAVE        , "§"            )      
			NST_CASE( DIK_LSHIFT       , "LSHIFT"       )      
			NST_CASE( DIK_BACKSLASH    , "\\"           )      
			NST_CASE( DIK_Z            , "Z"            )      
			NST_CASE( DIK_X            , "X"            )      
			NST_CASE( DIK_C            , "C"            )      
			NST_CASE( DIK_V            , "V"            )      
			NST_CASE( DIK_B            , "B"            )      
			NST_CASE( DIK_N            , "N"            )      
			NST_CASE( DIK_M            , "M"            )      
			NST_CASE( DIK_COMMA        , ","            )      
			NST_CASE( DIK_PERIOD       , "."            )      
			NST_CASE( DIK_SLASH        , "/"            )      
			NST_CASE( DIK_RSHIFT       , "RSHIFT"       )      
			NST_CASE( DIK_MULTIPLY     , "NUMPAD *"     )      
			NST_CASE( DIK_LMENU        , "LMENU"        )      
			NST_CASE( DIK_SPACE        , "SPACE"        )      
			NST_CASE( DIK_CAPITAL      , "CAPS-LOCK"    )      
			NST_CASE( DIK_F1           , "F1"           )      
			NST_CASE( DIK_F2           , "F2"           )      
			NST_CASE( DIK_F3           , "F3"           )      
			NST_CASE( DIK_F4           , "F4"           )      
			NST_CASE( DIK_F5           , "F5"           )      
			NST_CASE( DIK_F6           , "F6"           )      
			NST_CASE( DIK_F7           , "F7"           )      
			NST_CASE( DIK_F8           , "F8"           )      
			NST_CASE( DIK_F9           , "F9"           )      
			NST_CASE( DIK_F10          , "F10"          )      
			NST_CASE( DIK_NUMLOCK      , "NUMLOCK"      )      
			NST_CASE( DIK_SCROLL       , "SCROLL"       )      
			NST_CASE( DIK_NUMPAD7      , "NUMPAD 7"     )      
			NST_CASE( DIK_NUMPAD8      , "NUMPAD 8"     )      
			NST_CASE( DIK_NUMPAD9      , "NUMPAD 9"     )      
			NST_CASE( DIK_SUBTRACT     , "NUMPAD -"     )      
			NST_CASE( DIK_NUMPAD4      , "NUMPAD 4"     )      
			NST_CASE( DIK_NUMPAD5      , "NUMPAD 5"     )      
			NST_CASE( DIK_NUMPAD6      , "NUMPAD 6"     )      
			NST_CASE( DIK_ADD          , "NUMPAD +"     )      
			NST_CASE( DIK_NUMPAD1      , "NUMPAD 1"     )      
			NST_CASE( DIK_NUMPAD2      , "NUMPAD 2"     )      
			NST_CASE( DIK_NUMPAD3      , "NUMPAD 3"     )      
			NST_CASE( DIK_NUMPAD0      , "NUMPAD 0"     )      
			NST_CASE( DIK_DECIMAL      , "NUMPAD ."     )      
			NST_CASE( DIK_OEM_102      , "OEM 102"      )      
			NST_CASE( DIK_F11          , "F11"          )      
			NST_CASE( DIK_F12          , "F12"          )      
			NST_CASE( DIK_F13          , "F13"          )      
			NST_CASE( DIK_F14          , "F14"          )      
			NST_CASE( DIK_F15          , "F15"          )      
			NST_CASE( DIK_KANA         , "KANA"         )      
			NST_CASE( DIK_ABNT_C1      , "ABNT C1"      )      
			NST_CASE( DIK_CONVERT      , "CONVERT"      )      
			NST_CASE( DIK_NOCONVERT    , "NOCONVERT"    )      
			NST_CASE( DIK_YEN          , "YEN"          )      
			NST_CASE( DIK_ABNT_C2      , "ABNT C2"      )      
			NST_CASE( DIK_NUMPADEQUALS , "NUMPAD ="     )      
			NST_CASE( DIK_PREVTRACK    , "PREVTRACK"    )      
			NST_CASE( DIK_AT           , "AT"           )      
			NST_CASE( DIK_COLON        , ":"            )     
			NST_CASE( DIK_UNDERLINE    , "_"            )      
			NST_CASE( DIK_KANJI        , "KANJI"        )      
			NST_CASE( DIK_STOP         , "STOP"         )      
			NST_CASE( DIK_AX           , "AX"           )      
			NST_CASE( DIK_UNLABELED    , "UNLABELED"    )      
			NST_CASE( DIK_NEXTTRACK    , "NEXTTRACK"    )      
			NST_CASE( DIK_NUMPADENTER  , "NUMPAD ENTER" )      
			NST_CASE( DIK_RCONTROL     , "RCTRL"        )      
			NST_CASE( DIK_MUTE         , "MUTE"         )      
			NST_CASE( DIK_CALCULATOR   , "CALCULATOR"   )      
			NST_CASE( DIK_PLAYPAUSE    , "PLAYPAUSE"    )      
			NST_CASE( DIK_MEDIASTOP    , "MEDIASTOP"    )      
			NST_CASE( DIK_VOLUMEDOWN   , "VOLUMEDOWN"   )      
			NST_CASE( DIK_VOLUMEUP     , "VOLUMEUP"     )      
			NST_CASE( DIK_WEBHOME      , "WEBHOME"      )      
			NST_CASE( DIK_NUMPADCOMMA  , "NUMPAD ,"     )      
			NST_CASE( DIK_DIVIDE       , "NUMPAD /"     )      
			NST_CASE( DIK_SYSRQ        , "SYSRQ"        )      
			NST_CASE( DIK_RMENU        , "RMENU"        )      
			NST_CASE( DIK_PAUSE        , "PAUSE"        )      
			NST_CASE( DIK_HOME         , "HOME"         )      
			NST_CASE( DIK_UP           , "UP"           )      
			NST_CASE( DIK_PRIOR        , "PAGE-UP"      )     
			NST_CASE( DIK_LEFT         , "LEFT"         )      
			NST_CASE( DIK_RIGHT        , "RIGHT"        )      
			NST_CASE( DIK_END          , "END"          )      
			NST_CASE( DIK_DOWN         , "DOWN"         )      
			NST_CASE( DIK_NEXT         , "PAGE-DOWN"    )      
			NST_CASE( DIK_INSERT       , "INSERT"       )      
			NST_CASE( DIK_DELETE       , "DELETE"       )      
			NST_CASE( DIK_LWIN         , "LWIN"         )      
			NST_CASE( DIK_RWIN         , "RWIN"         )      
			NST_CASE( DIK_APPS         , "APPS"         )      
			NST_CASE( DIK_POWER        , "POWER"        )      
			NST_CASE( DIK_SLEEP        , "SLEEP"        )      
			NST_CASE( DIK_WAKE         , "WAKE"         )      
			NST_CASE( DIK_WEBSEARCH    , "WEBSEARCH"    )      
			NST_CASE( DIK_WEBFAVORITES , "WEBFAVORITES" )      
			NST_CASE( DIK_WEBREFRESH   , "WEBREFRESH"   )      
			NST_CASE( DIK_WEBSTOP      , "WEBSTOP"      )      
			NST_CASE( DIK_WEBFORWARD   , "WEBFORWARD"   )      
			NST_CASE( DIK_WEBBACK      , "WEBBACK"      )      
			NST_CASE( DIK_MYCOMPUTER   , "MYCOMPUTER"   )      
			NST_CASE( DIK_MAIL         , "MAIL"         )      
			NST_CASE( DIK_MEDIASELECT  , "MEDIASELECT"  )      
		}											
	}

	return "...";
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#undef NST_CASE
#define NST_CASE(a,b) if (text == b) return a;
#define NST_J_CASE(a,b) if (!strcmp( button, a )) return NST_USE_JOYSTICK | (index + b);

DWORD INPUTMANAGER::Text2Key(const PDXSTRING& text)
{
	if (text.IsEmpty() || (text[0] == '.' && text[1] == '.' && text[3] == '.'))
		return 0;

	if (text[0] == '(' && text[1] == 'j' && text[2] == 'o' && text[3] == 'y' && text[4] == ' ')
	{
		UINT stop = 5;

		while (text[stop] != ')' && text[stop] != '\0')
			++stop;

		if (stop == 5 || text[++stop] != ' ')
			return 0;

		const UINT index = strtoul( text.At(5), NULL, 10 ) * 64;
		const CHAR* const button = text.At(stop+1);

		NST_J_CASE( "-x",  32 )
		NST_J_CASE( "+x",  33 )
		NST_J_CASE( "+y",  34 )
		NST_J_CASE( "-y",  35 )
		NST_J_CASE( "+z",  36 )
		NST_J_CASE( "-z",  37 )
		NST_J_CASE( "-rx", 38 )
		NST_J_CASE( "+rx", 39 )
		NST_J_CASE( "+ry", 40 )
		NST_J_CASE( "-ry", 41 )
		NST_J_CASE( "+rz", 42 )
		NST_J_CASE( "-rz", 43 )
		NST_J_CASE( "-s0", 44 )
		NST_J_CASE( "+s0", 45 )
		NST_J_CASE( "-s1", 46 )
		NST_J_CASE( "+s1", 47 )
		NST_J_CASE( "+py", 48 )
		NST_J_CASE( "+px", 49 )
		NST_J_CASE( "-py", 50 )
		NST_J_CASE( "-px", 51 )
		NST_J_CASE( "+py", 52 )
		NST_J_CASE( "+px", 53 )
		NST_J_CASE( "-py", 54 )
		NST_J_CASE( "-px", 55 )
		NST_J_CASE( "+py", 56 )
		NST_J_CASE( "+px", 57 )
		NST_J_CASE( "-py", 58 )
		NST_J_CASE( "-px", 59 )
		NST_J_CASE( "+py", 60 )
		NST_J_CASE( "+px", 61 )
		NST_J_CASE( "-py", 62 )
		NST_J_CASE( "-px", 63 )

		const UINT key = strtoul( text.At(stop), NULL, 10 );

		if (key > 31)
			return 0;

		return NST_USE_JOYSTICK | (index + key);
	}
	else
	{
		NST_CASE( DIK_ESCAPE       , "ESCAPE"       )      
		NST_CASE( DIK_1            , "1"			)
		NST_CASE( DIK_2            , "2"            )      
		NST_CASE( DIK_3            , "3"            )      
		NST_CASE( DIK_4            , "4"            )      
		NST_CASE( DIK_5            , "5"            )      
		NST_CASE( DIK_6            , "6"            )      
		NST_CASE( DIK_7            , "7"            )      
		NST_CASE( DIK_8            , "8"            )      
		NST_CASE( DIK_9            , "9"            )      
		NST_CASE( DIK_0            , "0"            )      
		NST_CASE( DIK_MINUS        , "-"            )      
		NST_CASE( DIK_EQUALS       , "="            )      
		NST_CASE( DIK_BACK         , "BACK"         )      
		NST_CASE( DIK_TAB          , "TAB"          )      
		NST_CASE( DIK_Q            , "Q"            )      
		NST_CASE( DIK_W            , "W"            )      
		NST_CASE( DIK_E            , "E"            )      
		NST_CASE( DIK_R            , "R"            )      
		NST_CASE( DIK_T            , "T"            )      
		NST_CASE( DIK_Y            , "Y"            )      
		NST_CASE( DIK_U            , "U"            )      
		NST_CASE( DIK_I            , "I"            )      
		NST_CASE( DIK_O            , "O"            )      
		NST_CASE( DIK_P            , "P"            )      
		NST_CASE( DIK_LBRACKET     , "["            )      
		NST_CASE( DIK_RBRACKET     , "]"            )      
		NST_CASE( DIK_RETURN       , "RETURN"       )      
		NST_CASE( DIK_LCONTROL     , "LCTRL"        )      
		NST_CASE( DIK_A            , "A"            )      
		NST_CASE( DIK_S            , "S"            )      
		NST_CASE( DIK_D            , "D"            )      
		NST_CASE( DIK_F            , "F"            )      
		NST_CASE( DIK_G            , "G"            )      
		NST_CASE( DIK_H            , "H"            )      
		NST_CASE( DIK_J            , "J"            )      
		NST_CASE( DIK_K            , "K"            )      
		NST_CASE( DIK_L            , "L"            )      
		NST_CASE( DIK_SEMICOLON    , ";"            )      
		NST_CASE( DIK_APOSTROPHE   , "'"            )      
		NST_CASE( DIK_GRAVE        , "§"            )      
		NST_CASE( DIK_LSHIFT       , "LSHIFT"       )      
		NST_CASE( DIK_BACKSLASH    , "\\"           )      
		NST_CASE( DIK_Z            , "Z"            )      
		NST_CASE( DIK_X            , "X"            )      
		NST_CASE( DIK_C            , "C"            )      
		NST_CASE( DIK_V            , "V"            )      
		NST_CASE( DIK_B            , "B"            )      
		NST_CASE( DIK_N            , "N"            )      
		NST_CASE( DIK_M            , "M"            )      
		NST_CASE( DIK_COMMA        , ","            )      
		NST_CASE( DIK_PERIOD       , "."            )      
		NST_CASE( DIK_SLASH        , "/"            )      
		NST_CASE( DIK_RSHIFT       , "RSHIFT"       )      
		NST_CASE( DIK_MULTIPLY     , "NUMPAD *"     )      
		NST_CASE( DIK_LMENU        , "LMENU"        )      
		NST_CASE( DIK_SPACE        , "SPACE"        )      
		NST_CASE( DIK_CAPITAL      , "CAPS-LOCK"    )      
		NST_CASE( DIK_F1           , "F1"           )      
		NST_CASE( DIK_F2           , "F2"           )      
		NST_CASE( DIK_F3           , "F3"           )      
		NST_CASE( DIK_F4           , "F4"           )      
		NST_CASE( DIK_F5           , "F5"           )      
		NST_CASE( DIK_F6           , "F6"           )      
		NST_CASE( DIK_F7           , "F7"           )      
		NST_CASE( DIK_F8           , "F8"           )      
		NST_CASE( DIK_F9           , "F9"           )      
		NST_CASE( DIK_F10          , "F10"          )      
		NST_CASE( DIK_NUMLOCK      , "NUMLOCK"      )      
		NST_CASE( DIK_SCROLL       , "SCROLL"       )      
		NST_CASE( DIK_NUMPAD7      , "NUMPAD 7"     )      
		NST_CASE( DIK_NUMPAD8      , "NUMPAD 8"     )      
		NST_CASE( DIK_NUMPAD9      , "NUMPAD 9"     )      
		NST_CASE( DIK_SUBTRACT     , "NUMPAD -"     )      
		NST_CASE( DIK_NUMPAD4      , "NUMPAD 4"     )      
		NST_CASE( DIK_NUMPAD5      , "NUMPAD 5"     )      
		NST_CASE( DIK_NUMPAD6      , "NUMPAD 6"     )      
		NST_CASE( DIK_ADD          , "NUMPAD +"     )      
		NST_CASE( DIK_NUMPAD1      , "NUMPAD 1"     )      
		NST_CASE( DIK_NUMPAD2      , "NUMPAD 2"     )      
		NST_CASE( DIK_NUMPAD3      , "NUMPAD 3"     )      
		NST_CASE( DIK_NUMPAD0      , "NUMPAD 0"     )      
		NST_CASE( DIK_DECIMAL      , "NUMPAD ."     )      
		NST_CASE( DIK_OEM_102      , "OEM 102"      )      
		NST_CASE( DIK_F11          , "F11"          )      
		NST_CASE( DIK_F12          , "F12"          )      
		NST_CASE( DIK_F13          , "F13"          )      
		NST_CASE( DIK_F14          , "F14"          )      
		NST_CASE( DIK_F15          , "F15"          )      
		NST_CASE( DIK_KANA         , "KANA"         )      
		NST_CASE( DIK_ABNT_C1      , "ABNT C1"      )      
		NST_CASE( DIK_CONVERT      , "CONVERT"      )      
		NST_CASE( DIK_NOCONVERT    , "NOCONVERT"    )      
		NST_CASE( DIK_YEN          , "YEN"          )      
		NST_CASE( DIK_ABNT_C2      , "ABNT C2"      )      
		NST_CASE( DIK_NUMPADEQUALS , "NUMPAD ="     )      
		NST_CASE( DIK_PREVTRACK    , "PREVTRACK"    )      
		NST_CASE( DIK_AT           , "AT"           )      
		NST_CASE( DIK_COLON        , ":"            )     
		NST_CASE( DIK_UNDERLINE    , "_"            )      
		NST_CASE( DIK_KANJI        , "KANJI"        )      
		NST_CASE( DIK_STOP         , "STOP"         )      
		NST_CASE( DIK_AX           , "AX"           )      
		NST_CASE( DIK_UNLABELED    , "UNLABELED"    )      
		NST_CASE( DIK_NEXTTRACK    , "NEXTTRACK"    )      
		NST_CASE( DIK_NUMPADENTER  , "NUMPAD ENTER" )      
		NST_CASE( DIK_RCONTROL     , "RCTRL"        )      
		NST_CASE( DIK_MUTE         , "MUTE"         )      
		NST_CASE( DIK_CALCULATOR   , "CALCULATOR"   )      
		NST_CASE( DIK_PLAYPAUSE    , "PLAYPAUSE"    )      
		NST_CASE( DIK_MEDIASTOP    , "MEDIASTOP"    )      
		NST_CASE( DIK_VOLUMEDOWN   , "VOLUMEDOWN"   )      
		NST_CASE( DIK_VOLUMEUP     , "VOLUMEUP"     )      
		NST_CASE( DIK_WEBHOME      , "WEBHOME"      )      
		NST_CASE( DIK_NUMPADCOMMA  , "NUMPAD ,"     )      
		NST_CASE( DIK_DIVIDE       , "NUMPAD /"     )      
		NST_CASE( DIK_SYSRQ        , "SYSRQ"        )      
		NST_CASE( DIK_RMENU        , "RMENU"        )      
		NST_CASE( DIK_PAUSE        , "PAUSE"        )      
		NST_CASE( DIK_HOME         , "HOME"         )      
		NST_CASE( DIK_UP           , "UP"           )      
		NST_CASE( DIK_PRIOR        , "PAGE-UP"      )     
		NST_CASE( DIK_LEFT         , "LEFT"         )      
		NST_CASE( DIK_RIGHT        , "RIGHT"        )      
		NST_CASE( DIK_END          , "END"          )      
		NST_CASE( DIK_DOWN         , "DOWN"         )      
		NST_CASE( DIK_NEXT         , "PAGE-DOWN"    )      
		NST_CASE( DIK_INSERT       , "INSERT"       )      
		NST_CASE( DIK_DELETE       , "DELETE"       )      
		NST_CASE( DIK_LWIN         , "LWIN"         )      
		NST_CASE( DIK_RWIN         , "RWIN"         )      
		NST_CASE( DIK_APPS         , "APPS"         )      
		NST_CASE( DIK_POWER        , "POWER"        )      
		NST_CASE( DIK_SLEEP        , "SLEEP"        )      
		NST_CASE( DIK_WAKE         , "WAKE"         )      
		NST_CASE( DIK_WEBSEARCH    , "WEBSEARCH"    )      
		NST_CASE( DIK_WEBFAVORITES , "WEBFAVORITES" )      
		NST_CASE( DIK_WEBREFRESH   , "WEBREFRESH"   )      
		NST_CASE( DIK_WEBSTOP      , "WEBSTOP"      )      
		NST_CASE( DIK_WEBFORWARD   , "WEBFORWARD"   )      
		NST_CASE( DIK_WEBBACK      , "WEBBACK"      )      
		NST_CASE( DIK_MYCOMPUTER   , "MYCOMPUTER"   )      
		NST_CASE( DIK_MAIL         , "MAIL"         )      
		NST_CASE( DIK_MEDIASELECT  , "MEDIASELECT"  )      
	}

	return 0;
}
