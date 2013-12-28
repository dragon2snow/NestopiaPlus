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

DIRECTINPUT::DIRECTINPUT()
: 
hWnd     (NULL),
device   (NULL),
keyboard (NULL)
{
	memset( KeyboardBuffer, 0x00, sizeof(KeyboardBuffer) );
}

////////////////////////////////////////////////////////////////////////////////////////
// Destructor, release all devices
////////////////////////////////////////////////////////////////////////////////////////

DIRECTINPUT::~DIRECTINPUT()
{
	Destroy();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTINPUT::Error(const CHAR* const msg)
{
	application.LogOutput(PDXSTRING("DIRECTINPUT: ") + msg);
	application.OnError( msg );

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
// Destroyer, release all devices
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTINPUT::Destroy()
{
	joysticks.Destroy();

	if (keyboard)
	{
		keyboard->Unacquire();
		DIRECTX::Release(keyboard,TRUE);
	}

	DIRECTX::Release(device,TRUE);

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTINPUT::Initialize(HWND h)
{
	PDX_ASSERT(h && !device);

	hWnd = h;

	application.LogOutput("DIRECTINPUT: Initializing");

	if (FAILED(DirectInput8Create(application.GetHInstance(),DIRECTINPUT_VERSION,IID_IDirectInput8,PDX_CAST(LPVOID*,&device),NULL)))
		return Error("DirectInput8Create() failed!");

	if (FAILED(device->EnumDevices(DI8DEVCLASS_GAMECTRL,EnumJoysticks,PDX_CAST(LPVOID,this),DIEDFL_ATTACHEDONLY)))
		return Error("IDirectInput8::EnumDevices() failed!");

	PDX_TRY(SetUpKeyboard());
	PDX_TRY(SetCooperativeLevel(DISCL_BACKGROUND));

	AcquireDevices();

	{
		PDXSTRING log;
		
		log  = "DIRECTINPUT: found ";
		log += joysticks.Size();
		log += " attached joystick(s)";

		application.LogOutput( log );
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTINPUT::SetUpKeyboard()
{
	if (!device)
		return PDX_FAILURE;

	if (FAILED(device->CreateDevice( GUID_SysKeyboard, &keyboard, NULL )))
		return Error("IDirectInput8::CreateDevice() failed!");

	if (FAILED(keyboard->SetDataFormat( &c_dfDIKeyboard )))
		return Error("IDirectInputDevice8::SetDataFormat() failed!");

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

UINT DIRECTINPUT::GetDeviceIndex(const LPDIRECTINPUTDEVICE8 device) const
{
	if (device == keyboard)
		return 0;

	for (UINT i=0; i < joysticks.Size(); ++i)
	{
		if (device == joysticks[i].device)
			return (i+1);
	}

	return UINT_MAX;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

LPDIRECTINPUTDEVICE8 DIRECTINPUT::GetDevice(UINT index)
{
	return (!index) ? keyboard : (--index < joysticks.Size() ? joysticks[index].device : NULL);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTINPUT::AcquireDevices()
{
	if (keyboard)
	{
		while (keyboard->Acquire() == DIERR_INPUTLOST)
			Sleep(100);

		for (UINT i=0; i < joysticks.Size(); ++i)
		{
			while (joysticks[i].device->Acquire() == DIERR_INPUTLOST)
				Sleep(100);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTINPUT::SetCooperativeLevel(DWORD flags)
{
	if (!keyboard)
		return PDX_FAILURE;

	flags |= DISCL_NONEXCLUSIVE;

	if (FAILED(keyboard->SetCooperativeLevel(hWnd,flags)))
		return Error("IDirectInputDevice8::SetCooperativeLevel() failed!");

	for (UINT i=0; i < joysticks.Size(); ++i)
	{
		if (FAILED(joysticks[i].device->SetCooperativeLevel(hWnd,flags)))
			return Error("IDirectInputDevice8::SetCooperativeLevel() failed!");
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTINPUT::PollKeyboard()
{
	if (keyboard)
	{
		PDXMemZero( KeyboardBuffer, KEYBOARD_BUFFER_SIZE );

		if (FAILED(keyboard->GetDeviceState( sizeof(KeyboardBuffer), PDX_CAST(LPVOID,KeyboardBuffer) )))
		{
			while (keyboard->Acquire() == DIERR_INPUTLOST)
				Sleep(100);

			keyboard->GetDeviceState( sizeof(KeyboardBuffer), PDX_CAST(LPVOID,KeyboardBuffer) );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL DIRECTINPUT::ScanKeyboard(DWORD& key)
{
	if (!keyboard)
		return PDX_FAILURE;

	PDXMemZero( KeyboardBuffer, KEYBOARD_BUFFER_SIZE );

	if (FAILED(keyboard->GetDeviceState( sizeof(KeyboardBuffer), PDX_CAST(LPVOID,KeyboardBuffer) )))
	{
		while (keyboard->Acquire() == DIERR_INPUTLOST)
			Sleep(100);
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

	key = 0;

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

UINT DIRECTINPUT::ScanJoystick(DWORD& button,LPDIRECTINPUTDEVICE8& device)
{
	DIJOYSTATE state;

	for (UINT i=0; i < joysticks.Size(); ++i)
	{
		device = joysticks[i].device;

		if (FAILED(device->Poll()))
		{
			while (device->Acquire() == DIERR_INPUTLOST)
				Sleep(100);

			if (FAILED(device->Poll()))
				continue;
		}

		if (FAILED(device->GetDeviceState(sizeof(DIJOYSTATE),PDX_CAST(LPVOID,&state))))
			continue;

		const UINT index = i + 1;

		for (UINT j=0; j < joysticks[i].NumButtons; ++j)
		{
			if (state.rgbButtons[j])
			{
				button = j;
				return index;
			}
		}

		const UINT axes = joysticks[i].axes;

		if ( state.lX           && (axes & 0x01) ) { button = 32 + (state.lX           > 0 ? 1 : 0); return index; }
		if ( state.lY           && (axes & 0x02) ) { button = 34 + (state.lY           > 0 ? 1 : 0); return index; }
		if ( state.lZ           && (axes & 0x04) ) { button = 36 + (state.lZ           > 0 ? 1 : 0); return index; }
		if ( state.lRx          && (axes & 0x08) ) { button = 38 + (state.lRx          > 0 ? 1 : 0); return index; }
		if ( state.lRy          && (axes & 0x10) ) { button = 40 + (state.lRy          > 0 ? 1 : 0); return index; }
		if ( state.lRz          && (axes & 0x20) ) { button = 42 + (state.lRz          > 0 ? 1 : 0); return index; }
		if ( state.rglSlider[0] && (axes & 0x40) ) { button = 44 + (state.rglSlider[0] > 0 ? 1 : 0); return index; }
		if ( state.rglSlider[1] && (axes & 0x80) ) { button = 46 + (state.rglSlider[1] > 0 ? 1 : 0); return index; }

		for (UINT j=0; j < joysticks[i].NumPOVs; ++j)
		{
			const DWORD pov = state.rgdwPOV[j];

			if ( ( pov & 0xFFFF ) != 0xFFFFU && ( pov >= 31500U || pov <=  4500U ) ) { button = 48 + (j * 4); return index; } // up
			if ( ( pov & 0xFFFF ) != 0xFFFFU && ( pov >=  4500U && pov <= 13500U ) ) { button = 49 + (j * 4); return index; } // right
			if ( ( pov & 0xFFFF ) != 0xFFFFU && ( pov >= 13500U && pov <= 22500U ) ) { button = 50 + (j * 4); return index; } // down
			if ( ( pov & 0xFFFF ) != 0xFFFFU && ( pov >= 22500U && pov <= 31500U ) ) { button = 51 + (j * 4); return index; } // left
		}
	}

	device = NULL;
	button = 0;

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK DIRECTINPUT::EnumJoysticks(LPCDIDEVICEINSTANCE instance,LPVOID context)
{
	if (!instance || !context)
		return DIENUM_STOP;

	DIRECTINPUT& DirectInput = *PDX_CAST(DIRECTINPUT*,context);

	DirectInput.joysticks.Grow();

	JOYSTICK& joystick = DirectInput.joysticks.Back();

	PDX_ASSERT(DirectInput.device);

	if (FAILED(DirectInput.device->CreateDevice( instance->guidInstance, &joystick.device, NULL )))
		goto hell;

	if (FAILED(joystick.device->SetDataFormat( &c_dfDIJoystick )))
		goto hell;

	{
		DIDEVCAPS caps;
		DIRECTX::InitStruct( caps );

		if (FAILED(joystick.device->GetCapabilities( &caps )))
			goto hell;

		joystick.NumButtons = PDX_MIN(32,caps.dwButtons);
		joystick.NumPOVs    = PDX_MIN(4,caps.dwPOVs);
	}

	{
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

			range.diph.dwSize       = sizeof(range);
			range.diph.dwHeaderSize = sizeof(range.diph);
			range.diph.dwHow        = DIPH_BYOFFSET;
			range.lMin              = -1000;
			range.lMax              = +1000;

			for (UINT i=0; i < 8; ++i)
			{
				range.diph.dwObj = offsets[i];

				if (SUCCEEDED(joystick.device->SetProperty( DIPROP_RANGE, &range.diph )))
					joystick.axes |= (1U << i);
			}
		}

		{
			DIPROPDWORD prop;
			PDXMemZero( prop );

			prop.diph.dwSize       = sizeof(prop);
			prop.diph.dwHeaderSize = sizeof(prop.diph);
			prop.diph.dwHow        = DIPH_BYOFFSET;
			prop.dwData            = 2500;

			for (UINT i=0; i < 8; ++i)
			{
				if (joystick.axes & (1U << i))
				{
					prop.diph.dwObj = offsets[i];
					joystick.device->SetProperty( DIPROP_DEADZONE, &prop.diph );
				}
			}

			prop.diph.dwObj = 0;
			prop.diph.dwHow = DIPH_DEVICE;
			prop.dwData     = DIPROPAXISMODE_ABS;

			joystick.device->SetProperty( DIPROP_AXISMODE, &prop.diph );
		}
	}

	PDXMemCopy( joystick.guid, instance->guidInstance );

	return DIENUM_CONTINUE;

hell:

	DirectInput.joysticks.EraseBack();

	return DIENUM_CONTINUE;
}
