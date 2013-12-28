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

#include "NstDirectX.h"
#include "NstApplication.h"

#pragma comment(lib,"dinput8")
#pragma comment(lib,"dxguid")

////////////////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////////////////

DIRECTINPUT::JOYSTICK::JOYSTICK()
: 
device     (NULL), 
NumButtons (0), 
NumPOVs    (0),
axes       (0) 
{
	PDXMemZero( state );
	PDXMemZero( guid );
}

////////////////////////////////////////////////////////////////////////////////////////
// Release
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTINPUT::JOYSTICK::Release()
{
	if (device)
	{
		device->Unacquire();
		DIRECTX::Release(device,TRUE);
	}

	PDXMemZero( state );
}

////////////////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////////////////

DIRECTINPUT::DIRECTINPUT()
: 
hWnd     (NULL),
device   (NULL),
keyboard (NULL)
{
	PDXMemZero( KeyboardBuffer, KEYBOARD_BUFFER_SIZE );
}

////////////////////////////////////////////////////////////////////////////////////////
// Destructor, release all devices
////////////////////////////////////////////////////////////////////////////////////////

DIRECTINPUT::~DIRECTINPUT()
{
	Destroy();
}

////////////////////////////////////////////////////////////////////////////////////////
// Destroyer, release all devices
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTINPUT::Destroy()
{
	joysticks.Clear();

	if (keyboard)
	{
		keyboard->Unacquire();
		DIRECTX::Release(keyboard,TRUE);
	}

	DIRECTX::Release(device,TRUE);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTINPUT::Initialize(HWND h)
{
	PDX_ASSERT(h && !device && !keyboard && joysticks.IsEmpty());

	hWnd = h;

	application.LogFile().Output("DIRECTINPUT: initializing");

	if (FAILED(DirectInput8Create(GetModuleHandle(NULL),DIRECTINPUT_VERSION,IID_IDirectInput8,PDX_CAST(LPVOID*,&device),NULL)) || !device)
		throw ("DirectInput8Create() failed!");

	if (device->EnumDevices(DI8DEVCLASS_GAMECTRL,EnumJoysticks,PDX_CAST(LPVOID,this),DIEDFL_ATTACHEDONLY) == DI_OK)
	{
		application.LogFile().Output
		( 
			"DIRECTINPUT: found ",
			joysticks.Size(),
			" attached joystick(s)"
		);
	}
	else
	{
		application.LogFile().Output
		(
	     	"IDirectInput8::EnumDevices() failed! No joysticks can be used!"
		);
	}

	if (FAILED(device->CreateDevice( GUID_SysKeyboard, &keyboard, NULL )) || !keyboard)
		throw ("IDirectInput8::CreateDevice() failed!");

	if (FAILED(keyboard->SetDataFormat( &c_dfDIKeyboard )))
		throw ("IDirectInputDevice8::SetDataFormat() failed!");

	if (FAILED(keyboard->SetCooperativeLevel( hWnd, DISCL_FOREGROUND|DISCL_NONEXCLUSIVE )))
		throw ("IDirectInputDevice8::SetCooperativeLevel() failed!");

	keyboard->Acquire();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTINPUT::Acquire(LPDIRECTINPUTDEVICE8 input)
{
	PDX_ASSERT(input);

	if (input->Acquire() == DIERR_INPUTLOST)
	{
		Sleep(50);

		for (ULONG i=0; input->Acquire() == DIERR_INPUTLOST && i < 0xFFFFUL; ++i)
			Sleep(20);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTINPUT::AcquireDevices()
{
	PDX_ASSERT(keyboard);

	Acquire(keyboard);

	for (UINT i=0; i < joysticks.Size(); ++i)
	{
		if (joysticks[i].device)
			Acquire(joysticks[i].device);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTINPUT::SetCooperativeLevel(DWORD flags)
{
	PDX_ASSERT(keyboard);

	flags &= ~DISCL_EXCLUSIVE;
	flags |= DISCL_NONEXCLUSIVE;

	keyboard->Unacquire();

	if (FAILED(keyboard->SetCooperativeLevel( hWnd, flags )))
		throw ("IDirectInputDevice8::SetCooperativeLevel() failed!");

	keyboard->Acquire();

	for (UINT i=0; i < joysticks.Size(); ++i)
	{
		if (joysticks[i].device)
		{
			joysticks[i].device->Unacquire();

			if (FAILED(joysticks[i].device->SetCooperativeLevel( hWnd, flags )))
				throw ("IDirectInputDevice8::SetCooperativeLevel() failed!");

			joysticks[i].device->Acquire();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL DIRECTINPUT::PollKeyboard()
{
	PDX_ASSERT(keyboard);

	if (FAILED(keyboard->GetDeviceState( sizeof(KeyboardBuffer), PDX_CAST(LPVOID,KeyboardBuffer) )))
	{
		Acquire( keyboard );
		PDXMemZero( KeyboardBuffer, KEYBOARD_BUFFER_SIZE );
		return FALSE;
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL DIRECTINPUT::ScanKeyboard(DWORD& key)
{
	PDX_ASSERT(keyboard);

	key = 0;

	PDXMemZero( KeyboardBuffer, KEYBOARD_BUFFER_SIZE );

	if (FAILED(keyboard->GetDeviceState( sizeof(KeyboardBuffer), PDX_CAST(LPVOID,KeyboardBuffer) )))
	{
		Acquire(keyboard);
	}
	else
	{
		for (UINT i=0; i < KEYBOARD_BUFFER_SIZE; ++i)
		{
			if (KeyboardBuffer[i] & 0x80)
			{
				key = i;
				return TRUE;
			}
		}
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL DIRECTINPUT::ScanJoystick(DWORD& button,UINT& index)
{
	for (index=0; index < joysticks.Size(); ++index)
	{
		JOYSTICK& joy = joysticks[index];

		if (!joy.device)
			continue;

		if (FAILED(joy.device->Poll()))
		{
			Acquire(joy.device);
			continue;
		}

		if (FAILED(joy.device->GetDeviceState(sizeof(DIJOYSTATE),PDX_CAST(LPVOID,&joy.state))))
			continue;

		for (UINT i=0; i < joy.NumButtons; ++i)
		{
			if (joy.state.rgbButtons[i])
			{
				button = i;
				return TRUE;
			}
		}

		const UINT axes = joy.axes;

		if ( joy.state.lX           && (axes & 0x01) ) { button = 32 + (joy.state.lX           > 0 ? 1 : 0); return TRUE; }
		if ( joy.state.lY           && (axes & 0x02) ) { button = 34 + (joy.state.lY           > 0 ? 1 : 0); return TRUE; }
		if ( joy.state.lZ           && (axes & 0x04) ) { button = 36 + (joy.state.lZ           > 0 ? 1 : 0); return TRUE; }
		if ( joy.state.lRx          && (axes & 0x08) ) { button = 38 + (joy.state.lRx          > 0 ? 1 : 0); return TRUE; }
		if ( joy.state.lRy          && (axes & 0x10) ) { button = 40 + (joy.state.lRy          > 0 ? 1 : 0); return TRUE; }
		if ( joy.state.lRz          && (axes & 0x20) ) { button = 42 + (joy.state.lRz          > 0 ? 1 : 0); return TRUE; }
		if ( joy.state.rglSlider[0] && (axes & 0x40) ) { button = 44 + (joy.state.rglSlider[0] > 0 ? 1 : 0); return TRUE; }
		if ( joy.state.rglSlider[1] && (axes & 0x80) ) { button = 46 + (joy.state.rglSlider[1] > 0 ? 1 : 0); return TRUE; }

		for (UINT i=0; i < joy.NumPOVs; ++i)
		{
			const DWORD pov = joy.state.rgdwPOV[i];

			if ( ( pov & 0xFFFF ) != 0xFFFFU && ( pov >= 31500U || pov <=  4500U ) ) { button = 48 + (i * 4); return TRUE; } // up
			if ( ( pov & 0xFFFF ) != 0xFFFFU && ( pov >=  4500U && pov <= 13500U ) ) { button = 49 + (i * 4); return TRUE; } // right
			if ( ( pov & 0xFFFF ) != 0xFFFFU && ( pov >= 13500U && pov <= 22500U ) ) { button = 50 + (i * 4); return TRUE; } // down
			if ( ( pov & 0xFFFF ) != 0xFFFFU && ( pov >= 22500U && pov <= 31500U ) ) { button = 51 + (i * 4); return TRUE; } // left
		}
	}

	button = 0;
	index = 0;

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTINPUT::JOYSTICK::Create(LPDIRECTINPUT8 dinput,HWND hWnd,const GUID* const inguid)
{
	if (device)
		return PDX_OK;

	PDXMemZero( state );

	PDX_ASSERT(dinput && hWnd);

	if (!dinput || !hWnd)
		return PDX_FAILURE;

	if (inguid)
		guid = *inguid;

	if (FAILED(dinput->CreateDevice( guid, &device, NULL )) || !device)
		return PDX_FAILURE;

	if (FAILED(device->SetDataFormat( &c_dfDIJoystick )))
	{
		DIRECTX::Release(device,TRUE);
		return PDX_FAILURE;
	}

	if (FAILED(device->SetCooperativeLevel( hWnd, DISCL_NONEXCLUSIVE|DISCL_FOREGROUND )))
	{
		DIRECTX::Release(device,TRUE);
		return PDX_FAILURE;
	}

	{
		DIDEVCAPS caps;
		DIRECTX::InitStruct( caps );

		if (FAILED(device->GetCapabilities( &caps )))
		{
     		DIRECTX::Release(device,TRUE);
     		return PDX_FAILURE;
		}

		NumButtons = PDX_MIN(32,caps.dwButtons);
		NumPOVs = PDX_MIN(4,caps.dwPOVs);
	}

	static const LONG offsets[8] =
	{
		DIJOFS_X,
		DIJOFS_Y,
		DIJOFS_Z,
		DIJOFS_RX,
		DIJOFS_RY,
		DIJOFS_RZ,
		DIJOFS_SLIDER(0),
		DIJOFS_SLIDER(1)
	};

	{
		DIPROPRANGE range;
		PDXMemZero( range );

		range.diph.dwSize = sizeof(range);
		range.diph.dwHeaderSize = sizeof(range.diph);
		range.diph.dwHow = DIPH_BYOFFSET;
		range.lMin = -1000;
		range.lMax = +1000;

		for (UINT i=0; i < 8; ++i)
		{
			range.diph.dwObj = offsets[i];

			if (SUCCEEDED(device->SetProperty( DIPROP_RANGE, &range.diph )))
				axes |= (1U << i);
		}
	}

	{
		DIPROPDWORD prop;
		PDXMemZero( prop );

		prop.diph.dwSize = sizeof(prop);
		prop.diph.dwHeaderSize = sizeof(prop.diph);
		prop.diph.dwHow = DIPH_BYOFFSET;
		prop.dwData = 2500;

		for (UINT i=0; i < 8; ++i)
		{
			if (axes & (1U << i))
			{
				prop.diph.dwObj = offsets[i];
				device->SetProperty( DIPROP_DEADZONE, &prop.diph );
			}
		}

		prop.diph.dwObj = 0;
		prop.diph.dwHow = DIPH_DEVICE;
		prop.dwData = DIPROPAXISMODE_ABS;

		device->SetProperty( DIPROP_AXISMODE, &prop.diph );
	}

	device->Acquire();

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK DIRECTINPUT::EnumJoysticks(LPCDIDEVICEINSTANCE instance,LPVOID context)
{
	if (!instance || !context)
		return DIENUM_STOP;

	DIRECTINPUT& DirectInput = *PDX_CAST(DIRECTINPUT*,context);
	PDX_ASSERT(DirectInput.device);

	DirectInput.joysticks.Grow();

	if (PDX_FAILED(DirectInput.joysticks.Back().Create( DirectInput.device, DirectInput.hWnd, &instance->guidInstance )))
	{
		DirectInput.joysticks.EraseBack();
		return DIENUM_CONTINUE;
	}

	DirectInput.joysticks.Back().Release();

	return DIENUM_CONTINUE;
}
