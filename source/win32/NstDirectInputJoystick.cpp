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

#include "NstWindowStruct.hpp"
#include "NstIoScreen.hpp"
#include "NstDirectInput.hpp"

namespace Nestopia
{
	using DirectX::DirectInput;

	struct DirectInput::Joystick::Lookup
	{
		uint (*code)(const void*) throw();
		ushort offset;
		ushort axis;
		cstring name;
	};

	const DirectInput::Joystick::Lookup DirectInput::Joystick::table[TABLE_KEYS] =
	{
		{ KeyPosDir,   (ushort) DIJOFS_Y,         Caps::AXIS_Y,        "+y"   },
		{ KeyPosDir,   (ushort) DIJOFS_X,         Caps::AXIS_X,        "+x"   },
		{ KeyNegDir,   (ushort) DIJOFS_Y,         Caps::AXIS_Y,        "-y"   },
		{ KeyNegDir,   (ushort) DIJOFS_X,         Caps::AXIS_X,        "-x"   },
		{ KeyPosDir,   (ushort) DIJOFS_Z,         Caps::AXIS_Z,        "+z"   },
		{ KeyNegDir,   (ushort) DIJOFS_Z,         Caps::AXIS_Z,        "-z"   },
		{ KeyPosDir,   (ushort) DIJOFS_RY,        Caps::AXIS_RY,       "+ry"  },
		{ KeyPosDir,   (ushort) DIJOFS_RX,        Caps::AXIS_RX,       "+rx"  },
		{ KeyPosDir,   (ushort) DIJOFS_RY,        Caps::AXIS_RY,       "-ry"  },
		{ KeyNegDir,   (ushort) DIJOFS_RX,        Caps::AXIS_RX,       "-rx"  },
		{ KeyPosDir,   (ushort) DIJOFS_RZ,        Caps::AXIS_RZ,       "+rz"  },
		{ KeyNegDir,   (ushort) DIJOFS_RZ,        Caps::AXIS_RZ,       "-rz"  },
		{ KeyNegDir,   (ushort) DIJOFS_SLIDER(0), Caps::AXIS_SLIDER_0, "-s0"  },
		{ KeyPosDir,   (ushort) DIJOFS_SLIDER(0), Caps::AXIS_SLIDER_0, "+s0"  },
		{ KeyNegDir,   (ushort) DIJOFS_SLIDER(1), Caps::AXIS_SLIDER_1, "-s1"  },
		{ KeyPosDir,   (ushort) DIJOFS_SLIDER(1), Caps::AXIS_SLIDER_1, "+s1"  },
		{ KeyPovUp,	   (ushort) DIJOFS_POV(0),    Caps::AXIS_POV_0,    "-p0y" },
		{ KeyPovRight, (ushort) DIJOFS_POV(0),    Caps::AXIS_POV_0,    "+p0x" },
		{ KeyPovDown,  (ushort) DIJOFS_POV(0),    Caps::AXIS_POV_0,    "+p0y" },
		{ KeyPovLeft,  (ushort) DIJOFS_POV(0),    Caps::AXIS_POV_0,    "-p0x" },
		{ KeyPovUp,	   (ushort) DIJOFS_POV(1),    Caps::AXIS_POV_1,    "-p1y" },
		{ KeyPovRight, (ushort) DIJOFS_POV(1),    Caps::AXIS_POV_1,    "+p1x" },
		{ KeyPovDown,  (ushort) DIJOFS_POV(1),    Caps::AXIS_POV_1,    "+p1y" },
		{ KeyPovLeft,  (ushort) DIJOFS_POV(1),    Caps::AXIS_POV_1,    "-p1x" },
		{ KeyPovUp,	   (ushort) DIJOFS_POV(2),    Caps::AXIS_POV_2,    "-p2y" },
		{ KeyPovRight, (ushort) DIJOFS_POV(2),    Caps::AXIS_POV_2,    "+p2x" },
		{ KeyPovDown,  (ushort) DIJOFS_POV(2),    Caps::AXIS_POV_2,    "+p2y" },
		{ KeyPovLeft,  (ushort) DIJOFS_POV(2),    Caps::AXIS_POV_2,    "-p2x" },
		{ KeyPovUp,	   (ushort) DIJOFS_POV(3),    Caps::AXIS_POV_3,    "-p3y" },
		{ KeyPovRight, (ushort) DIJOFS_POV(3),    Caps::AXIS_POV_3,    "+p3x" },
		{ KeyPovDown,  (ushort) DIJOFS_POV(3),    Caps::AXIS_POV_3,    "+p3y" },
		{ KeyPovLeft,  (ushort) DIJOFS_POV(3),    Caps::AXIS_POV_3,    "-p3x" }
	};

	DirectInput::Joystick::Caps::Caps(const Device& device,const GUID& g)
	: guid(g), axis(0)
	{
		Window::Struct<DIDEVCAPS> caps;

		if (FAILED(device->GetCapabilities( &caps )))
			throw ERR_API;

		mustPoll = (caps.dwFlags & DIDC_POLLEDDATAFORMAT) != 0;
		numButtons = (ushort) NST_MIN(NUM_BUTTONS,caps.dwButtons);

		for (uint i=0, n=NST_MIN(NUM_POVS,caps.dwPOVs); i < n; ++i)
			axis |= AXIS_POV_0 << i;

		if (caps.dwAxes) 
			device->EnumObjects( EnumDeviceObjectsProc, &axis, DIDFT_AXIS );

		if (!numButtons && !axis)
			throw ERR_API;
	}

	BOOL CALLBACK DirectInput::Joystick::Caps::EnumDeviceObjectsProc(LPCDIDEVICEOBJECTINSTANCE info,LPVOID ref)
	{
		ushort& axis = *static_cast<ushort*>(ref);

		uint flag;

		     if ( info->guidType == GUID_XAxis  ) flag = AXIS_X;
		else if ( info->guidType == GUID_YAxis  ) flag = AXIS_Y;
		else if ( info->guidType == GUID_ZAxis  ) flag = AXIS_Z;
		else if ( info->guidType == GUID_RxAxis ) flag = AXIS_RX;
		else if ( info->guidType == GUID_RyAxis ) flag = AXIS_RY;
		else if ( info->guidType == GUID_RzAxis ) flag = AXIS_RZ;
		else if ( info->guidType == GUID_Slider ) 
		{
			flag = (axis & AXIS_SLIDER_0) ? AXIS_SLIDER_1 : AXIS_SLIDER_0;
		}
		else
		{
			return DIENUM_CONTINUE;
		}

		axis |= flag;

		return DIENUM_CONTINUE;
	}

	DirectInput::Device DirectInput::Joystick::Create(const Manager& manager,const GUID& guid)
	{
		Device device;

		if (FAILED(manager->CreateDevice( guid, &device, NULL )))
			throw ERR_API;

		if (FAILED(device->SetDataFormat( &c_dfDIJoystick )))
			throw ERR_API;

		if (FAILED(device->SetCooperativeLevel( manager.Window(), DISCL_BACKGROUND|DISCL_NONEXCLUSIVE )))
			throw ERR_API;

		return device;
	}

	DirectInput::Joystick::Joystick(const Manager& manager,const GUID& guid)
	: Device(Create(manager,guid)), enabled(TRUE), caps(*this,guid)
	{
		static const short offsets[] =
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

		Object::Pod<DIPROPRANGE> range;

		range.diph.dwSize = sizeof(range);
		range.diph.dwHeaderSize = sizeof(range.diph);
		range.diph.dwHow = DIPH_BYOFFSET;
		range.lMin = AXIS_MIN_RANGE;
		range.lMax = AXIS_MAX_RANGE;

		Object::Pod<DIPROPDWORD> data;

		data.diph.dwSize = sizeof(data);
		data.diph.dwHeaderSize = sizeof(data.diph);
		data.diph.dwHow = DIPH_BYOFFSET;
		data.dwData = AXIS_DEADZONE;

		for (uint i=0; i < NST_COUNT(offsets); ++i)
		{
			if (caps.axis & (1U << i))
			{
				data.diph.dwObj = range.diph.dwObj = offsets[i];

				if 
				(
					FAILED(pointer->SetProperty( DIPROP_RANGE, &range.diph )) ||
					FAILED(pointer->SetProperty( DIPROP_DEADZONE, &data.diph ))
				)
					throw ERR_API;
			}
		}

		Clear();
	}

	DirectInput::Joystick::~Joystick()
	{
		if (pointer)
		{
			pointer->Unacquire();
			pointer->Release();
			pointer = NULL;
		}
	}

	ibool DirectInput::Joystick::Scan(Key& key) const
	{
		for (uint i=caps.numButtons; i; )
		{
			if (state.rgbButtons[--i] & 0x80)
			{
				key.data = state.rgbButtons + i;
				key.code = KeyDown;
				return TRUE;
			}
		}

		for (uint i=TABLE_KEYS; i; )
		{
			if (caps.axis & table[--i].axis)
			{
				key.data = reinterpret_cast<const BYTE*>(&state) + table[i].offset;
				key.code = table[i].code;

				if (key.GetState())
					return TRUE;
			}
		}

		return FALSE;
	}

	ibool DirectInput::Joystick::Map(Key& key,cstring const name) const
	{
		if (*name)
		{
			if (name[0] >= '0' && name[0] <= '9')
			{
				uint index = name[0] - '0';

				if (name[1] >= '0' && name[1] <= '9')
					index = (index * 10) + (name[1] - '0');

				if (index < caps.numButtons)
				{
					key.data = state.rgbButtons + index;
					key.code = KeyDown;
					return TRUE;
				}
			}
			else
			{
				for (uint i=TABLE_KEYS; i; )
				{
					if (caps.axis & table[--i].axis)
					{
						if (String::Compare( name, table[i].name ) == 0)
						{
							key.data = reinterpret_cast<const BYTE*>(&state) + table[i].offset;
							key.code = table[i].code;
							return TRUE;
						}
					}
				}
			}
		}

		return FALSE;
	}

	ibool DirectInput::Joystick::IsAssigned(const Key& key) const
	{
		return 
		(
			key.data >= reinterpret_cast<const BYTE*>(&state) &&
			key.data <  reinterpret_cast<const BYTE*>(&state) + sizeof(state)
		);
	}

	cstring DirectInput::Joystick::GetName(const Key& key) const
	{
		NST_VERIFY( IsAssigned(key) );

		if (key.code == KeyDown)
		{
			static char button[3] = "xx";

			const uint index = key.data - state.rgbButtons;
			button[0] = (char) (index < 10 ? '0' + index : '0' + index / 10);
			button[1] = (char) (index < 10 ? '\0'        : '0' + index % 10);

			return button;
		}

		for (const Lookup* it = table; ; ++it)
		{
			if (key.data == reinterpret_cast<const BYTE*>(&state) + it->offset && key.code == it->code)
				return it->name;
		}
	}

	void DirectInput::Joystick::Clear()
	{
		state.Clear();
		state.rgdwPOV[3] = state.rgdwPOV[2] = state.rgdwPOV[1] = state.rgdwPOV[0] = ~DWORD(0); 
	}

	void DirectInput::Joystick::Acquire(const ibool force)
	{
		if (enabled | force)
		{
			pointer->Acquire();
			Clear();
		}
	}

	void DirectInput::Joystick::OnError(const HRESULT hResult)
	{
		NST_ASSERT( FAILED(hResult) );

		switch (hResult)
		{		
			case DIERR_INPUTLOST:
			case DIERR_NOTACQUIRED:
		
				Acquire( TRUE );
				break;

			case DIERR_UNPLUGGED:

				enabled = FALSE;
				Clear();
				Io::Screen() << "Error! Joystick unplugged!";
				break;
		}
	}
}
