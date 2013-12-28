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

	VOID Poll();

protected:

	enum {KEYBOARD_BUFFER_SIZE=256};

	static VOID PDX_NO_INLINE Acquire(LPDIRECTINPUTDEVICE8);

	PDX_NO_INLINE VOID Initialize(HWND) throw(const CHAR*);
	PDX_NO_INLINE VOID SetCooperativeLevel(DWORD) throw(const CHAR*);
	PDX_NO_INLINE VOID Destroy();

	VOID AcquireDevices();
	
	inline const BYTE* GetKeyboardBuffer() const 
	{ return KeyboardBuffer; }

	BOOL PollKeyboard();
	BOOL ScanKeyboard(DWORD&);
	BOOL ScanJoystick(DWORD&,UINT&);

	inline LPDIRECTINPUT8 GetDevice()
	{ return device; }

	inline LPDIRECTINPUTDEVICE8 GetKeyboard()
	{ return keyboard; }

	HWND hWnd;

private:

	static BOOL CALLBACK EnumJoysticks(LPCDIDEVICEINSTANCE,LPVOID);

	LPDIRECTINPUT8 device;         
	LPDIRECTINPUTDEVICE8 keyboard;

protected:

	struct JOYSTICK
	{
		JOYSTICK();

		~JOYSTICK()
		{ Release(); }

		PDX_NO_INLINE PDXRESULT Create(LPDIRECTINPUT8,HWND,const GUID* const=NULL);

		VOID Release();

		BOOL operator == (const GUID& g) const
		{ return PDXMemCompare(guid,g); }

		inline BOOL operator == (const LPDIRECTINPUTDEVICE8 d) const
		{ return device == d; }

		LPDIRECTINPUTDEVICE8 device;
		DIJOYSTATE state;
		UINT NumButtons;
		UINT NumPOVs;
		UINT axes;
		GUID guid;
	};

	typedef PDXARRAY<JOYSTICK> JOYSTICKS;

	JOYSTICKS joysticks;

private:

	BYTE KeyboardBuffer[KEYBOARD_BUFFER_SIZE];
};
 
#endif
