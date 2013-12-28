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

#ifndef NST_DIRECTINPUT_H
#define NST_DIRECTINPUT_H

#undef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800

#include <dinput.h>

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class DIRECTINPUT
{
public:

	DIRECTINPUT();
	~DIRECTINPUT();

	PDXRESULT Poll();
	PDXRESULT SetCooperativeLevel(DWORD);

	VOID AcquireDevices();

	enum {KEYBOARD_BUFFER_SIZE=256};

protected:

	PDXRESULT Initialize(HWND);
	PDXRESULT Destroy();
	
	inline const BYTE* GetKeyboardBuffer() const 
	{ return KeyboardBuffer; }

	VOID PollKeyboard();
	BOOL ScanKeyboard(DWORD&);
	UINT ScanJoystick(DWORD&,LPDIRECTINPUTDEVICE8&);

	UINT GetDeviceIndex(const LPDIRECTINPUTDEVICE8) const;
	LPDIRECTINPUTDEVICE8 GetDevice(const UINT); 

	HWND hWnd;

private:

	static PDXRESULT Error(const CHAR* const);

	PDXRESULT SetUpKeyboard();

	static BOOL CALLBACK EnumJoysticks(LPCDIDEVICEINSTANCE,LPVOID);

	LPDIRECTINPUT8 device;         
	LPDIRECTINPUTDEVICE8 keyboard;
	BYTE KeyboardBuffer[KEYBOARD_BUFFER_SIZE];
	DWORD NumKeys;

	struct JOYSTICK
	{
		JOYSTICK()
		: device(NULL) {}

		~JOYSTICK()
		{
			if (device)
			{
				device->Unacquire();
				DIRECTX::Release(device,TRUE);
			}
		}

		GUID guid;
		LPDIRECTINPUTDEVICE8 device;
	};

protected:

	typedef PDXARRAY<JOYSTICK> JOYSTICKS;

	JOYSTICKS joysticks;
};
 
#endif
