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

#pragma once

#ifndef NST_INPUTMANAGER_H
#define NST_INPUTMANAGER_H

#include "NstDirectX.h"
#include "NstManager.h"

class PDXFILE;

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class INPUTMANAGER : public DIRECTINPUT, public MANAGER
{
public:

	INPUTMANAGER(const INT);

	PDXRESULT Poll();

	BOOL OnMouseMove(POINT&);

	inline VOID OnMouseButtonDown(POINT& point)
	{ format.paddle.fire = format.zapper.fire = OnMouseMove(point); }

	inline VOID OnMouseButtonUp()
	{ format.paddle.fire = format.zapper.fire = 0; }

	inline NES::IO::INPUT* GetFormat()
	{ return &format; }

	PDXRESULT PollFamilyKeyboard
	(
     	NES::IO::INPUT::FAMILYKEYBOARD* const,
		const UINT,
		const UINT
	);

private:

	VOID PollPad(const UINT,const UINT);
	VOID PollArcade();
	VOID PollPowerPad();

	PDXRESULT Create  (CONFIGFILE* const);
	PDXRESULT Destroy (CONFIGFILE* const);

	VOID PollJoysticks();
	VOID Reset();
	VOID Clear();
	VOID UpdateDialog();
	VOID UpdateJoystickDevices();
	VOID UpdateDlgButtonTexts();
	
	ULONG IsButtonPressed(const ULONG) const;
	ULONG IsJoystickButtonPressed(ULONG) const;

	VOID ScanInput();
	BOOL DialogProc(HWND,UINT,WPARAM,LPARAM);

	static BOOL CALLBACK StaticKeyPressDialogProc(HWND,UINT,WPARAM,LPARAM);

	enum
	{
		CATEGORY_PAD1,
		CATEGORY_PAD2,
		CATEGORY_PAD3,
		CATEGORY_PAD4,
		CATEGORY_POWERPAD,
		CATEGORY_GENERAL,
		NUM_CATEGORIES
	};

	enum
	{
		KEY_UP         = NES::IO::INPUT::PAD::INDEX_UP,
		KEY_RIGHT      = NES::IO::INPUT::PAD::INDEX_RIGHT,
		KEY_DOWN       = NES::IO::INPUT::PAD::INDEX_DOWN,
		KEY_LEFT       = NES::IO::INPUT::PAD::INDEX_LEFT,
		KEY_SELECT     = NES::IO::INPUT::PAD::INDEX_SELECT,
		KEY_START      = NES::IO::INPUT::PAD::INDEX_START,
		KEY_A          = NES::IO::INPUT::PAD::INDEX_A,
		KEY_B          = NES::IO::INPUT::PAD::INDEX_B,
		KEY_AUTOFIRE_A = 8,
		KEY_AUTOFIRE_B = 9,
		NUM_PAD_KEYS = 10
	};

	enum
	{
		KEY_POWERPAD_SIDE_A_1,
		KEY_POWERPAD_SIDE_A_2,
		KEY_POWERPAD_SIDE_A_3,
		KEY_POWERPAD_SIDE_A_4,
		KEY_POWERPAD_SIDE_A_5,
		KEY_POWERPAD_SIDE_A_6,
		KEY_POWERPAD_SIDE_A_7,
		KEY_POWERPAD_SIDE_A_8,
		KEY_POWERPAD_SIDE_A_9,
		KEY_POWERPAD_SIDE_A_10,
		KEY_POWERPAD_SIDE_A_11,
		KEY_POWERPAD_SIDE_A_12,
		KEY_POWERPAD_SIDE_B_3,
		KEY_POWERPAD_SIDE_B_2,
		KEY_POWERPAD_SIDE_B_8,
		KEY_POWERPAD_SIDE_B_7,
		KEY_POWERPAD_SIDE_B_6,
		KEY_POWERPAD_SIDE_B_5,
		KEY_POWERPAD_SIDE_B_11,
		KEY_POWERPAD_SIDE_B_10,
		NUM_POWERPAD_SIDE_A_KEYS = NES::IO::INPUT::POWERPAD::NUM_SIDE_A_BUTTONS,
		NUM_POWERPAD_SIDE_B_KEYS = NES::IO::INPUT::POWERPAD::NUM_SIDE_B_BUTTONS,
		NUM_POWERPAD_KEYS = NUM_POWERPAD_SIDE_A_KEYS + NUM_POWERPAD_SIDE_B_KEYS
	};

	enum
	{
		KEY_GENERAL_INSERT_COIN_1,    
		KEY_GENERAL_INSERT_COIN_2,
		KEY_GENERAL_TOGGLE_FPS,
		NUM_GENERAL_KEYS
	};
  
	static const CHAR* Key2Text(DWORD);
	static DWORD Text2Key(const PDXSTRING&);

	static const CHAR* Map2Text(const UINT,const UINT);

	struct MAP
	{
		struct CATEGORY
		{
			struct KEY
			{
				KEY()
				:
				key    (0),
				device (NULL)
				{}

				VOID Reset()
				{
					key = 0;
					device = NULL;
				}

				DWORD key;
				LPDIRECTINPUTDEVICE8 device;
			};

			typedef PDXARRAY<KEY> KEYS;

			KEYS keys;
		};

		CATEGORY category[NUM_CATEGORIES];
	};

	struct ACTIVEJOYSTICK
	{
		ACTIVEJOYSTICK()
		:
		device (NULL),
		active (FALSE)
		{
			PDXMemZero( state );
		}

		BOOL operator == (const LPDIRECTINPUTDEVICE8 d) const
		{ return device == d; }

		BOOL active;
		LPDIRECTINPUTDEVICE8 device;
		DIJOYSTATE state;
	};

	typedef PDXARRAY<ACTIVEJOYSTICK> ACTIVEJOYSTICKS;

	HWND hDlg;

	UINT SelectDevice;
	UINT SelectKey;
	BOOL SpeedThrottleKeyDown;
	UINT SpeedThrottle;

	ACTIVEJOYSTICKS ActiveJoysticks;

	MAP map;
	NES::IO::INPUT format;

	UINT AutoFireStep;

	const UINT MenuHeight;
};

#endif
