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
#include <WindowsX.h>

#define NST_USE_JOYSTICK 0x80000000UL

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT NES::IO::INPUT::FAMILYKEYBOARD::Poll(const UINT part,const UINT mode)
{
	return device ? PDX_CAST(INPUTMANAGER*,device)->PollFamilyKeyboard(this,part,mode) : PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT INPUTMANAGER::PollFamilyKeyboard(NES::IO::INPUT::FAMILYKEYBOARD* const nes,const UINT part,const UINT mode)
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

	const BYTE* const buffer = GetKeyboardBuffer();

	UINT& key = nes->parts[part];
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

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

ULONG INPUTMANAGER::IsJoystickButtonPressed(ULONG key) const
{
	PDX_ASSERT(!(key & NST_USE_JOYSTICK));

	UINT index = 0;

	if (key >= 36)
	{
		index = key / 36;
		key %= 36;
	}

	switch (key)
	{
     	case 32: return ActiveJoysticks[index].state.lX < 0;
     	case 33: return ActiveJoysticks[index].state.lX > 0;
     	case 34: return ActiveJoysticks[index].state.lY < 0;
     	case 35: return ActiveJoysticks[index].state.lY > 0;
	}

	return ActiveJoysticks[index].state.rgbButtons[key];
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

ULONG INPUTMANAGER::IsButtonPressed(const ULONG key) const
{
	if (key & NST_USE_JOYSTICK)
	{
		return IsJoystickButtonPressed(key & ~NST_USE_JOYSTICK);
	}
	else
	{
		return GetKeyboardBuffer()[key] & 0x80;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID INPUTMANAGER::PollPad(const UINT pad,const UINT index)
{
	PDX_ASSERT(pad < 4);
	PDX_ASSERT(index <= CATEGORY_PAD4);

	format.pad[pad].buttons =
	(
     	( IsButtonPressed( map.category[ index ].keys[ KEY_UP     ].key ) ? NES::IO::INPUT::PAD::UP     : 0 ) |
		( IsButtonPressed( map.category[ index ].keys[ KEY_RIGHT  ].key ) ? NES::IO::INPUT::PAD::RIGHT  : 0 ) |
		( IsButtonPressed( map.category[ index ].keys[ KEY_DOWN   ].key ) ? NES::IO::INPUT::PAD::DOWN   : 0 ) |
		( IsButtonPressed( map.category[ index ].keys[ KEY_LEFT   ].key ) ? NES::IO::INPUT::PAD::LEFT   : 0 ) |
		( IsButtonPressed( map.category[ index ].keys[ KEY_SELECT ].key ) ? NES::IO::INPUT::PAD::SELECT : 0 ) |
		( IsButtonPressed( map.category[ index ].keys[ KEY_START  ].key ) ? NES::IO::INPUT::PAD::START  : 0 ) |
		( IsButtonPressed( map.category[ index ].keys[ KEY_A      ].key ) ? NES::IO::INPUT::PAD::A      : 0 ) |
		( IsButtonPressed( map.category[ index ].keys[ KEY_B      ].key ) ? NES::IO::INPUT::PAD::B      : 0 )
	);

	if (++AutoFireStep >= 3)
	{
		if (AutoFireStep == 6)
			AutoFireStep = 0;

		if (IsButtonPressed(map.category[index].keys[KEY_AUTOFIRE_A].key))
			format.pad[pad].buttons |= NES::IO::INPUT::PAD::A;

		if (IsButtonPressed(map.category[index].keys[KEY_AUTOFIRE_B].key))
			format.pad[pad].buttons |= NES::IO::INPUT::PAD::B;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID INPUTMANAGER::PollArcade()
{
	format.vs.InsertCoin = 
	(
     	( IsButtonPressed( map.category[ CATEGORY_GENERAL ].keys[ KEY_GENERAL_INSERT_COIN_1 ].key ) ? NES::IO::INPUT::VS::COIN_1 : 0 ) |
     	( IsButtonPressed( map.category[ CATEGORY_GENERAL ].keys[ KEY_GENERAL_INSERT_COIN_2 ].key ) ? NES::IO::INPUT::VS::COIN_2 : 0 )
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID INPUTMANAGER::PollPowerPad()
{
	for (UINT i=0; i < NUM_POWERPAD_SIDE_A_KEYS; ++i)
		format.PowerPad.SideA[i] = IsButtonPressed(map.category[CATEGORY_POWERPAD].keys[i].key);

	for (UINT i=0; i < NUM_POWERPAD_SIDE_B_KEYS; ++i)
		format.PowerPad.SideB[i] = IsButtonPressed(map.category[CATEGORY_POWERPAD].keys[NUM_POWERPAD_SIDE_A_KEYS + i].key);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

INPUTMANAGER::INPUTMANAGER(const INT id,const UINT chunk)
: 
MANAGER      (id,chunk), 
hDlg         (NULL),
AutoFireStep (0),
MenuHeight   (GetSystemMetrics(SM_CYMENU))
{
	map.category[ CATEGORY_PAD1     ].keys.Resize( NUM_PAD_KEYS      );
	map.category[ CATEGORY_PAD2     ].keys.Resize( NUM_PAD_KEYS      );
	map.category[ CATEGORY_PAD3     ].keys.Resize( NUM_PAD_KEYS      );
	map.category[ CATEGORY_PAD4     ].keys.Resize( NUM_PAD_KEYS      );
	map.category[ CATEGORY_POWERPAD ].keys.Resize( NUM_POWERPAD_KEYS );
	map.category[ CATEGORY_GENERAL  ].keys.Resize( NUM_GENERAL_KEYS  );

	format.FamilyKeyboard.device = PDX_CAST(VOID*,this);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT INPUTMANAGER::Create(PDXFILE* const file)
{
	SelectDevice = 0;
	SelectKey    = 0;

	PDX_TRY(DIRECTINPUT::Initialize( MANAGER::hWnd ));

	if (file)
	{
		for (UINT i=0; i < NUM_CATEGORIES; ++i)
		{
			for (UINT j=0; j < map.category[i].keys.Size(); ++j)
			{
				MAP::CATEGORY::KEY& key = map.category[i].keys[j];

				key.device = NULL;

				const UINT index = file->Read<U8>();

				if (index != 0xFF)
				{
					key.device = GetDevice(index);
					key.key = file->Read<U32>();
				}

				if (!key.device)
					key.key = 0;
			}
		}
	}
	else
	{
		Reset();
	}

	UpdateJoystickDevices();

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT INPUTMANAGER::Destroy(PDXFILE* const file)
{
	if (file)
	{
		for (UINT i=0; i < NUM_CATEGORIES; ++i)
		{
			for (UINT j=0; j < map.category[i].keys.Size(); ++j)
			{
				const U32 key = map.category[i].keys[j].key;

				if (!key)
				{
					file->Write(U8(0xFF));
				}
				else
				{
					LPDIRECTINPUTDEVICE8 device = map.category[i].keys[j].device;

					if (!device)
					{
						file->Write(U8(0));
						file->Write(key);
					}
					else
					{
						const UINT index = GetDeviceIndex(device);

						if (index == UINT_MAX)
						{
							file->Write(U8(0xFF));
						}
						else
						{
							file->Write(U8(index));
							file->Write(key);
						}
					}
				}
			}
		}
	}
  
	return DIRECTINPUT::Destroy();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID INPUTMANAGER::Reset()
{
	for (UINT i=0; i < NUM_CATEGORIES; ++i)
		for (TSIZE j=0; j < map.category[i].keys.Size(); ++j)
			map.category[i].keys[j].Reset();

	map.category[ CATEGORY_PAD1 ].keys[ KEY_UP         ].key = DIK_UP;
	map.category[ CATEGORY_PAD1 ].keys[ KEY_RIGHT      ].key = DIK_RIGHT;
	map.category[ CATEGORY_PAD1 ].keys[ KEY_DOWN       ].key = DIK_DOWN;
	map.category[ CATEGORY_PAD1 ].keys[ KEY_LEFT       ].key = DIK_LEFT;
	map.category[ CATEGORY_PAD1 ].keys[ KEY_SELECT     ].key = DIK_RSHIFT;
	map.category[ CATEGORY_PAD1 ].keys[ KEY_START      ].key = DIK_RETURN;
	map.category[ CATEGORY_PAD1 ].keys[ KEY_A          ].key = DIK_PERIOD;
	map.category[ CATEGORY_PAD1 ].keys[ KEY_B          ].key = DIK_COMMA;
	map.category[ CATEGORY_PAD1 ].keys[ KEY_AUTOFIRE_A ].key = DIK_L;
	map.category[ CATEGORY_PAD1 ].keys[ KEY_AUTOFIRE_B ].key = DIK_K;
																
	map.category[ CATEGORY_PAD2 ].keys[ KEY_UP         ].key = DIK_F;
	map.category[ CATEGORY_PAD2 ].keys[ KEY_RIGHT      ].key = DIK_B;
	map.category[ CATEGORY_PAD2 ].keys[ KEY_DOWN       ].key = DIK_V;
	map.category[ CATEGORY_PAD2 ].keys[ KEY_LEFT       ].key = DIK_C;
	map.category[ CATEGORY_PAD2 ].keys[ KEY_SELECT     ].key = DIK_A;
	map.category[ CATEGORY_PAD2 ].keys[ KEY_START      ].key = DIK_S;
	map.category[ CATEGORY_PAD2 ].keys[ KEY_A          ].key = DIK_X;
	map.category[ CATEGORY_PAD2 ].keys[ KEY_B          ].key = DIK_Z;
	map.category[ CATEGORY_PAD2 ].keys[ KEY_AUTOFIRE_A ].key = DIK_W;
	map.category[ CATEGORY_PAD2 ].keys[ KEY_AUTOFIRE_B ].key = DIK_Q;

	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_1  ].key = DIK_Q;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_2  ].key = DIK_W;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_3  ].key = DIK_E;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_4  ].key = DIK_R;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_5  ].key = DIK_A;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_6  ].key = DIK_S;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_7  ].key = DIK_D;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_8  ].key = DIK_F;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_9  ].key = DIK_Z;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_10 ].key = DIK_X;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_11 ].key = DIK_C;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_12 ].key = DIK_V;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_3  ].key = DIK_Y;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_2  ].key = DIK_U;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_8  ].key = DIK_G;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_7  ].key = DIK_H;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_6  ].key = DIK_J;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_5  ].key = DIK_K;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_11 ].key = DIK_N;
	map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_10 ].key = DIK_M;

	map.category[ CATEGORY_GENERAL ].keys[ KEY_GENERAL_INSERT_COIN_1 ].key = DIK_F2;    
	map.category[ CATEGORY_GENERAL ].keys[ KEY_GENERAL_INSERT_COIN_2 ].key = DIK_F3;    
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT INPUTMANAGER::Poll()
{
	PollKeyboard();
	PollJoysticks();
	
	if (nes->ConnectedController(4) != NES::CONTROLLER_KEYBOARD)
	{
		for (UINT i=0; i < 4; ++i)
		{
			switch (nes->ConnectedController(i))
			{
    			case NES::CONTROLLER_PAD1: PollPad( i, 0 ); continue;
       			case NES::CONTROLLER_PAD2: PollPad( i, 1 ); continue;
       			case NES::CONTROLLER_PAD3: PollPad( i, 2 ); continue;
    			case NES::CONTROLLER_PAD4: PollPad( i, 3 ); continue;
			}
		}

		if (nes->IsAnyControllerConnected(NES::CONTROLLER_POWERPAD))
			PollPowerPad();

		if (nes->IsVs())
			PollArcade();
	}
  
	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID INPUTMANAGER::PollJoysticks()
{
	const UINT NumDevices = ActiveJoysticks.Size();

	for (UINT i=0; i < NumDevices; ++i)
	{
		if (FAILED(ActiveJoysticks[i].device->Poll()))
		{
			while (ActiveJoysticks[i].device->Acquire() == DIERR_INPUTLOST)
				Sleep(100);

			if (FAILED(ActiveJoysticks[i].device->Poll()))
			{
				PDXMemZero( ActiveJoysticks[i].state );
				continue;
			}
		}

		if (FAILED(ActiveJoysticks[i].device->GetDeviceState( sizeof(DIJOYSTATE), PDX_CAST(LPVOID,&ActiveJoysticks[i].state) )))
		{
			PDXMemZero( ActiveJoysticks[i].state );
			continue;
		}
	}
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
		rcClient.right = application.GetDisplayWidth();
		rcClient.bottom = application.GetDisplayHeight();

		if (application.IsMenuSet())
			point.y += MenuHeight;
	}

	const RECT& rcNes = application.NesRect();

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

				im->map.category[ im->SelectDevice ].keys[ im->SelectKey ].key = key;				
				im->map.category[ im->SelectDevice ].keys[ im->SelectKey ].device = NULL;				
			}
			else
			{
				LPDIRECTINPUTDEVICE8 device;
				UINT index;

				if (index = im->ScanJoystick( key, device ))
				{
					key |= NST_USE_JOYSTICK;
					
					const DWORD KeyValue = key + (36 * (index-1));

					im->map.category[ im->SelectDevice ].keys[ im->SelectKey ].key = KeyValue;
					im->map.category[ im->SelectDevice ].keys[ im->SelectKey ].device = device;
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

			ListBox_InsertString( hMap, KEY_A,          Key2Text( map.category[ SelectDevice ].keys[ KEY_A          ].key ) );
			ListBox_InsertString( hMap, KEY_B,          Key2Text( map.category[ SelectDevice ].keys[ KEY_B          ].key ) );
			ListBox_InsertString( hMap, KEY_SELECT,     Key2Text( map.category[ SelectDevice ].keys[ KEY_SELECT     ].key ) );
			ListBox_InsertString( hMap, KEY_START,      Key2Text( map.category[ SelectDevice ].keys[ KEY_START      ].key ) );
			ListBox_InsertString( hMap, KEY_UP,         Key2Text( map.category[ SelectDevice ].keys[ KEY_UP         ].key ) );
			ListBox_InsertString( hMap, KEY_DOWN,       Key2Text( map.category[ SelectDevice ].keys[ KEY_DOWN       ].key ) );
			ListBox_InsertString( hMap, KEY_LEFT,       Key2Text( map.category[ SelectDevice ].keys[ KEY_LEFT       ].key ) );
			ListBox_InsertString( hMap, KEY_RIGHT,      Key2Text( map.category[ SelectDevice ].keys[ KEY_RIGHT      ].key ) );
			ListBox_InsertString( hMap, KEY_AUTOFIRE_A, Key2Text( map.category[ SelectDevice ].keys[ KEY_AUTOFIRE_A ].key ) );
			ListBox_InsertString( hMap, KEY_AUTOFIRE_B, Key2Text( map.category[ SelectDevice ].keys[ KEY_AUTOFIRE_B ].key ) );
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

			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_1  ].key ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_2  ].key ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_3  ].key ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_4  ].key ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_5  ].key ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_6  ].key ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_7  ].key ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_8  ].key ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_9  ].key ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_10 ].key ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_11 ].key ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_A_12 ].key ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_3  ].key ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_2  ].key ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_8  ].key ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_7  ].key ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_6  ].key ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_5  ].key ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_11 ].key ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_POWERPAD ].keys[ KEY_POWERPAD_SIDE_B_10 ].key ) );
			break;

       	case CATEGORY_GENERAL:

			ListBox_AddString( hKeys, "Insert Coin (slot 1)" );
			ListBox_AddString( hKeys, "Insert Coin (slot 2)" );

			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_GENERAL ].keys[ KEY_GENERAL_INSERT_COIN_1 ].key ) );
			ListBox_AddString( hMap, Key2Text( map.category[ CATEGORY_GENERAL ].keys[ KEY_GENERAL_INSERT_COIN_2 ].key ) );
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
	{
		map.category[ SelectDevice ].keys[ i ].key = 0;
		map.category[ SelectDevice ].keys[ i ].device = NULL;
	}

	UpdateDlgButtonTexts();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID INPUTMANAGER::ScanInput()
{
	HWND hItem = GetDlgItem( hDlg, IDC_INPUT_KEYPRESS_TEXT );
	SetWindowText( hItem, "Press any key/button to use, ESC to abort.." );
	
	DialogBoxParam
	( 
    	application.GetHInstance(), 
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
	ActiveJoysticks.Clear();

	ACTIVEJOYSTICK joy;

	for (UINT i=0; i < NUM_CATEGORIES; ++i)
	{
		for (TSIZE j=0; j < map.category[i].keys.Size(); ++j)
		{
			joy.device = map.category[i].keys[j].device;

			if (joy.device && joy.device != GetDevice(0) && PDX::Find(ActiveJoysticks.Begin(),ActiveJoysticks.End(),joy) == ActiveJoysticks.End())
				ActiveJoysticks.InsertBack(joy);
		}
	}
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

		text = "(joy ";

	     	 if (key < 36*1) { text << "0) "; key -= 36*0; }
		else if (key < 36*2) { text << "1) "; key -= 36*1; }
		else if (key < 36*3) { text << "2) "; key -= 36*2; }
		else if (key < 36*4) { text << "3) "; key -= 36*3; }
		else if (key < 36*5) { text << "4) "; key -= 36*4; }
		else if (key < 36*6) { text << "5) "; key -= 36*5; }
		else                 { text << "6) "; key -= 36*6; }

		switch (key)
		{
     		case 32: text << "-x"; break;
     		case 33: text << "+x"; break;
     		case 34: text << "+y"; break;
     		case 35: text << "-y"; break;
			default: text << key;  break;
		}

		static CHAR buffer[16];
		strcpy( buffer, text );

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
