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

#pragma comment(lib,"dinput8")
#pragma comment(lib,"dxguid")

#include "NstApplicationException.hpp"
#include "NstApplicationInstance.hpp"
#include "NstSystemKeyboard.hpp"
#include "NstWindowStruct.hpp"
#include "NstDirectInput.hpp"
#include "NstIoScreen.hpp"
#include "NstIoLog.hpp"

namespace Nestopia
{
	using DirectX::DirectInput;

	struct DirectInput::Joystick::Lookup
	{
		uint (*code)(const void* const) throw();
		ushort offset;
		ushort axis;
		cstring name;
	};

	const DirectInput::Joystick::Lookup DirectInput::Joystick::table[TABLE_KEYS] =
	{
		{ KeyPosDir,   (ushort) DIJOFS_Y,         AXIS_Y,        "+y"   },
		{ KeyPosDir,   (ushort) DIJOFS_X,         AXIS_X,        "+x"   },
		{ KeyNegDir,   (ushort) DIJOFS_Y,         AXIS_Y,        "-y"   },
		{ KeyNegDir,   (ushort) DIJOFS_X,         AXIS_X,        "-x"   },
		{ KeyPosDir,   (ushort) DIJOFS_Z,         AXIS_Z,        "+z"   },
		{ KeyNegDir,   (ushort) DIJOFS_Z,         AXIS_Z,        "-z"   },
		{ KeyPosDir,   (ushort) DIJOFS_RY,        AXIS_RY,       "+ry"  },
		{ KeyPosDir,   (ushort) DIJOFS_RX,        AXIS_RX,       "+rx"  },
		{ KeyPosDir,   (ushort) DIJOFS_RY,        AXIS_RY,       "-ry"  },
		{ KeyNegDir,   (ushort) DIJOFS_RX,        AXIS_RX,       "-rx"  },
		{ KeyPosDir,   (ushort) DIJOFS_RZ,        AXIS_RZ,       "+rz"  },
		{ KeyNegDir,   (ushort) DIJOFS_RZ,        AXIS_RZ,       "-rz"  },
		{ KeyNegDir,   (ushort) DIJOFS_SLIDER(0), AXIS_SLIDER_0, "-s0"  },
		{ KeyPosDir,   (ushort) DIJOFS_SLIDER(0), AXIS_SLIDER_0, "+s0"  },
		{ KeyNegDir,   (ushort) DIJOFS_SLIDER(1), AXIS_SLIDER_1, "-s1"  },
		{ KeyPosDir,   (ushort) DIJOFS_SLIDER(1), AXIS_SLIDER_1, "+s1"  },
		{ KeyPovUp,	   (ushort) DIJOFS_POV(0),    AXIS_POV_0,    "-p0y" },
		{ KeyPovRight, (ushort) DIJOFS_POV(0),    AXIS_POV_0,    "+p0x" },
		{ KeyPovDown,  (ushort) DIJOFS_POV(0),    AXIS_POV_0,    "+p0y" },
		{ KeyPovLeft,  (ushort) DIJOFS_POV(0),    AXIS_POV_0,    "-p0x" },
		{ KeyPovUp,	   (ushort) DIJOFS_POV(1),    AXIS_POV_1,    "-p1y" },
		{ KeyPovRight, (ushort) DIJOFS_POV(1),    AXIS_POV_1,    "+p1x" },
		{ KeyPovDown,  (ushort) DIJOFS_POV(1),    AXIS_POV_1,    "+p1y" },
		{ KeyPovLeft,  (ushort) DIJOFS_POV(1),    AXIS_POV_1,    "-p1x" },
		{ KeyPovUp,	   (ushort) DIJOFS_POV(2),    AXIS_POV_2,    "-p2y" },
		{ KeyPovRight, (ushort) DIJOFS_POV(2),    AXIS_POV_2,    "+p2x" },
		{ KeyPovDown,  (ushort) DIJOFS_POV(2),    AXIS_POV_2,    "+p2y" },
		{ KeyPovLeft,  (ushort) DIJOFS_POV(2),    AXIS_POV_2,    "-p2x" },
		{ KeyPovUp,	   (ushort) DIJOFS_POV(3),    AXIS_POV_3,    "-p3y" },
		{ KeyPovRight, (ushort) DIJOFS_POV(3),    AXIS_POV_3,    "+p3x" },
		{ KeyPovDown,  (ushort) DIJOFS_POV(3),    AXIS_POV_3,    "+p3y" },
		{ KeyPovLeft,  (ushort) DIJOFS_POV(3),    AXIS_POV_3,    "-p3x" }
	};

	IDirectInput8& DirectInput::Base::Create()
	{
		Io::Log() << "DirectInput: initializing..\r\n";

		IDirectInput8* com;

		if (FAILED(::DirectInput8Create( Application::Instance::GetHandle(), DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<void**>(&com), NULL )))
			throw Application::Exception("DirectInput8Create() failed!");

		return *com;
	}

	DirectInput::Base::Base(HWND const h)
	: com(Create()), hWnd(h) {}

	DirectInput::Base::~Base()
	{
		com.Release();
	}

	DirectInput::DirectInput(HWND const hWnd)
	: base(hWnd), keyboard(base)
	{
		Io::Log logfile;

		if (SUCCEEDED(base.com.EnumDevices( DI8DEVCLASS_GAMECTRL, EnumJoysticks, this, DIEDFL_ATTACHEDONLY )))
			logfile << "DirectInput: found " << joysticks.Size() << " attached joystick(s)\r\n";
		else
			logfile << "DirectInput: IDirectInput8::EnumDevices() failed! No joysticks can be used!\r\n";
	}

	DirectInput::~DirectInput()
	{
		for (uint i=joysticks.Size(); i; )
			joysticks[--i].Destruct();
	}

	BOOL CALLBACK DirectInput::EnumJoysticks(LPCDIDEVICEINSTANCE instance,LPVOID context)
	{
		if (instance)
		{
			DirectInput& directInput = *static_cast<DirectInput*>(context);

			if (directInput.joysticks.Size() == MAX_JOYSTICKS)
			{
				Io::Log() << "DirectInput: warning, device count limit reached, stopping enumeration..\r\n";
				return DIENUM_STOP;
			}

			Io::Log() << "DirectInput: enumerating device - name: " 
			          << (*instance->tszProductName ? instance->tszProductName : "unknown")
					  << ", GUID: " 
					  << System::Guid( instance->guidInstance ).GetString()
					  << "\r\n";

			directInput.joysticks.Grow();
			directInput.joysticks.Back().Invalidate();

			try
			{
				directInput.joysticks.Back().Construct( directInput.base, instance->guidInstance );
			}
			catch (Joystick::Exception)
			{
				directInput.joysticks.Shrink();
				Io::Log() << "DirectInput: warning, bogus device, continuing enumeration..\r\n";
			}
		}

		return DIENUM_CONTINUE;
	}

	void DirectInput::Joystick::Unacquire()
	{
		Clear();
		com.Unacquire();
	}

	void DirectInput::Keyboard::Unacquire()
	{
		Clear();
		com.Unacquire();
	}

	void DirectInput::Acquire(const ibool force)
	{
		keyboard.Acquire( force );

		for (Joysticks::Iterator it=joysticks.Begin(), end=joysticks.End(); it != end; ++it)
			(*it)->Acquire( force );
	}

	void DirectInput::Unacquire()
	{
		keyboard.Unacquire();

		for (Joysticks::Iterator it=joysticks.Begin(), end=joysticks.End(); it != end; ++it)
			(*it)->Unacquire();
	}

	inline void DirectInput::Joystick::Enable(ibool enable)
	{
		enabled = enable;
	}

	inline void DirectInput::Keyboard::Enable(ibool enable)
	{
		enabled = enable;
	}

	void DirectInput::Optimize(const Key* keys,const uint count)
	{
		keyboard.Enable( FALSE );

		for (Joysticks::Iterator it=joysticks.Begin(), end=joysticks.End(); it != end; ++it)
			(*it)->Enable( FALSE );

		for (const Key* const end = keys + count; keys != end; ++keys)
		{
			if (keyboard.IsAssigned( *keys ))
			{
				keyboard.Enable( TRUE );
			}
			else for (Joysticks::Iterator it=joysticks.Begin(), end=joysticks.End(); it != end; ++it)
			{
				if ((*it)->IsAssigned( *keys ))
				{
					(*it)->Enable( TRUE );
					break;
				}
			}
		}
	}

	DirectInput::ScanResult DirectInput::ScanKey(Key& key,const uint axes) const
	{
		const ScanResult scan = keyboard.Scan( key );

		if (scan != SCAN_GOOD_KEY)
		{
			if (scan == SCAN_NO_KEY)
			{
				for (Joysticks::ConstIterator it=joysticks.Begin(), end=joysticks.End(); it != end; ++it)
				{
					if ((*it)->Scan( key, axes ))
						return SCAN_GOOD_KEY;
				}
			}

			key.Unmap();
		}

		return scan;
	}

	ibool DirectInput::MapKey(Key& key,cstring const name,const System::Guid* const guids,const uint numGuids) const
	{
		key.Unmap();

		if 
		(
			name && *name &&
			(name[0] != '.' || name[1] != '.' || name[2] != '.' || name[3] != '\0')
		)
		{
			if 
			(
				(name[0] == '(') &&
				(name[1] == 'j' || name[1] == 'J') &&
				(name[2] == 'o' || name[2] == 'O') &&
				(name[3] == 'y' || name[3] == 'Y') &&
				(name[4] == ' ') &&
				(name[5] >= '0' && name[5] <= '9') &&
				(
	    			(name[6] == ')' && name[7] == ' ') || 
    		   		(name[6] >= '0' && name[6] <= '9' && name[7] == ')' && name[8] == ' ')
				)
			)
			{
				uint index = name[5] - '0';

				if (name[6] != ')')
					index = (index * 10) + (name[6] - '0');

				if (index < NST_MIN(MAX_JOYSTICKS,numGuids))
				{
					const System::Guid& guid = guids[index];

					for (Joysticks::ConstIterator it=joysticks.Begin(), end=joysticks.End(); it != end; ++it)
					{
						if ((*it)->GetGuid() == guid)
							return (*it)->Map( key, name + (name[7] == ' ' ? 8 : 9) );
					}
				}
			}
			else
			{
				return keyboard.Map( key, System::Keyboard::DikKey( name ) );
			}
		}

		return FALSE;
	}

	DirectInput::KeyName DirectInput::GetKeyName(const Key& key) const
	{
		if (key.IsAssigned())
		{
			if (keyboard.IsAssigned( key ))
			{
				return keyboard.GetName( key );
			}
			else 
			{
				for (uint i=0, n=joysticks.Size(); i < n; ++i)
				{
					if (joysticks[i]->IsAssigned( key ))
						return KeyName("(joy ") << i << ") " << joysticks[i]->GetName( key );
				}

				cstring shortCut;

				switch (key.vKey & 0xFF)
				{
					case FALT:				   shortCut = "ALT+";            break;
					case FSHIFT:			   shortCut = "SHIFT+";          break;
					case FCONTROL:			   shortCut = "CTRL+";           break;
					case FCONTROL|FALT:        shortCut = "CTRL+ALT+";       break;
					case FCONTROL|FSHIFT:      shortCut = "CTRL+SHIFT+";     break;
					case FALT|FSHIFT:		   shortCut = "ALT+SHIFT+";      break;
					case FCONTROL|FALT|FSHIFT: shortCut = "CTRL+ALT+SHIFT+"; break;
					default:                   shortCut = "";                break;
				}

				return KeyName(shortCut) << System::Keyboard::VikName(key.vKey >> 8);
			}
		}

		return "...";
	}

	IDirectInputDevice8& DirectInput::Keyboard::Create(IDirectInput8& base)
	{
		IDirectInputDevice8* com;

		if (FAILED(base.CreateDevice( GUID_SysKeyboard, &com, NULL )))
			throw Application::Exception("IDirectInput8::CreateDevice() failed!");

		if (FAILED(com->SetDataFormat( &c_dfDIKeyboard )))
			throw Application::Exception("IDirectInputDevice8::SetDataFormat() failed!");

		return *com;
	}

	ibool DirectInput::Keyboard::MustPoll(IDirectInputDevice8& com)
	{
		Window::Struct<DIDEVCAPS> caps;
		return FAILED(com.GetCapabilities( &caps )) || (caps.dwFlags & (DIDC_POLLEDDATAFORMAT|DIDC_POLLEDDEVICE));
	}

	DirectInput::Keyboard::Keyboard(Base& base)
	: enabled(TRUE), com(Create(base.com)), mustPoll(MustPoll(com))
	{
		try
		{
			if (mustPoll)
				Io::Log() << "DirectInput: warning, keyboard is a polled device!\r\n";

			SetCooperativeLevel( base.hWnd );
			Clear();
		}
		catch (...)
		{
			com.Release();
			throw;
		}
	}

	DirectInput::Keyboard::~Keyboard()
	{
		com.Unacquire();
		com.Release();
	}

	void DirectInput::Keyboard::SetCooperativeLevel(HWND const hWnd,const DWORD flags) const
	{
		if (FAILED(com.SetCooperativeLevel( hWnd, flags )))
			throw Application::Exception("IDirectInputDevice8::SetCooperativeLevel() failed!");
	}

	ibool DirectInput::Keyboard::Map(Key& key,const uint code) const
	{
		if (code && code <= 0xFF && code != DIK_LWIN)
		{
			key.data = buffer + code;
			key.code = KeyDown;
			return TRUE;
		}

		return FALSE;
	}

	DirectInput::ScanResult DirectInput::Keyboard::Scan(Key& key) const
	{
		NST_COMPILE_ASSERT( Buffer::SIZE % 4 == 0 );

		for (uint i=0; i < Buffer::SIZE; i += 4)
		{
			if (const DWORD mask = *reinterpret_cast<const DWORD*>(buffer+i) & 0x80808080U)
			{
				i += (mask & 0x80U) ? 0 : (mask & 0x8000U) ? 1 : (mask & 0x800000U) ? 2 : 3;
				return Map( key, i ) ? SCAN_GOOD_KEY : SCAN_INVALID_KEY;
			}
		}

		return SCAN_NO_KEY;
	}

	ibool DirectInput::Keyboard::IsAssigned(const Key& key) const
	{
		return key.data >= buffer && key.data < (buffer + Buffer::SIZE);
	}

	cstring DirectInput::Keyboard::GetName(const Key& key) const
	{
		NST_VERIFY( IsAssigned(key) );
		return System::Keyboard::DikName( key.data - buffer );
	}

	void DirectInput::Keyboard::Clear()
	{
		std::memset( buffer, 0, Buffer::SIZE );
	}

	void DirectInput::Keyboard::Acquire(const ibool force)
	{
		if (enabled | force)
		{
			com.Acquire();
			Clear();
		}
	}

	DirectInput::Joystick::Caps::Caps(IDirectInputDevice8& com,const GUID& g)
	: mustPoll(FALSE), guid(g), axes(0) 
	{
		try
		{
			Context context( *this, com );

			if (FAILED(com.EnumObjects( EnumObjectsProc, &context, DIDFT_ALL )) || !(context.numButtons|axes))
				throw ERR_API;
		}
		catch (...)
		{
			com.Release();
			throw;
		}
	}

	IDirectInputDevice8& DirectInput::Joystick::Create(Base& base,const GUID& guid)
	{
		IDirectInputDevice8* com;

		if (FAILED(base.com.CreateDevice( guid, &com, NULL )))
			throw ERR_API;

		if 
		(
			FAILED(com->SetDataFormat( &c_dfDIJoystick )) ||
			FAILED(com->SetCooperativeLevel( base.hWnd, DISCL_BACKGROUND|DISCL_NONEXCLUSIVE ))
		)
		{
			com->Release();
			throw ERR_API;
		}

		return *com;
	}

	DirectInput::Joystick::Joystick(Base& base,const GUID& guid)
	: enabled(TRUE), com(Create(base,guid)), caps(com,guid) {}

	DirectInput::Joystick::~Joystick()
	{
		com.Unacquire();
		com.Release();
	}

	BOOL CALLBACK DirectInput::Joystick::Caps::EnumObjectsProc(LPCDIDEVICEOBJECTINSTANCE info,LPVOID ref)
	{
		Context& context = *static_cast<Context*>(ref);

		if (info->guidType == GUID_Button)
		{
			++context.numButtons;
		}
		else
		{
			uint flag;

			     if ( info->guidType == GUID_XAxis  ) flag = AXIS_X;
			else if ( info->guidType == GUID_YAxis  ) flag = AXIS_Y;
			else if ( info->guidType == GUID_ZAxis  ) flag = AXIS_Z;
			else if ( info->guidType == GUID_RxAxis ) flag = AXIS_RX;
			else if ( info->guidType == GUID_RyAxis ) flag = AXIS_RY;
			else if ( info->guidType == GUID_RzAxis ) flag = AXIS_RZ;
			else if ( info->guidType == GUID_POV    )
			{
				if (context.caps.axes & AXIS_POV_3)
				{
					return DIENUM_CONTINUE;
				}
				else if (context.caps.axes & AXIS_POV_2)
				{
					flag = AXIS_POV_3;
				}
				else if (context.caps.axes & AXIS_POV_1)
				{
					flag = AXIS_POV_2;
				}
				else if (context.caps.axes & AXIS_POV_0)
				{
					flag = AXIS_POV_1;
				}
				else
				{
					flag = AXIS_POV_0;
				}
			}
			else if ( info->guidType == GUID_Slider ) 
			{
				if (context.caps.axes & AXIS_SLIDER_1)
				{
					return DIENUM_CONTINUE;
				}
				else if (context.caps.axes & AXIS_SLIDER_0)
				{
					flag = AXIS_SLIDER_1;
				}
				else
				{
					flag = AXIS_SLIDER_0;
				}
			}
			else
			{
				return DIENUM_CONTINUE;
			}

			if (info->dwType & DIDFT_AXIS)
			{
				{
					Object::Pod<DIPROPRANGE> diprg;

					diprg.diph.dwSize       = sizeof(diprg); 
					diprg.diph.dwHeaderSize = sizeof(diprg.diph); 
					diprg.diph.dwHow        = DIPH_BYID; 
					diprg.diph.dwObj        = info->dwType;
					diprg.lMin              = AXIS_MIN_RANGE; 
					diprg.lMax              = AXIS_MAX_RANGE; 

					if (FAILED(context.com.SetProperty( DIPROP_RANGE, &diprg.diph )))
					{
						if (FAILED(context.com.GetProperty( DIPROP_RANGE, &diprg.diph )) || diprg.lMin >= 0 || diprg.lMax <= 0)
						{
							Io::Log() << "DirectInput: warning, SetProperty(DIPROP_RANGE) failed, skipping axis..\r\n";
							return DIENUM_CONTINUE;
						}
					}
				}
				{
					Object::Pod<DIPROPDWORD> diprd;

					diprd.diph.dwSize       = sizeof(diprd);
					diprd.diph.dwHeaderSize = sizeof(diprd.diph);
					diprd.diph.dwHow        = DIPH_BYID; 
					diprd.diph.dwObj        = info->dwType;
					diprd.dwData            = AXIS_DEADZONE;

					if (FAILED(context.com.SetProperty( DIPROP_DEADZONE, &diprd.diph )))
						Io::Log() << "DirectInput: warning, SetProperty(DIPROP_DEADZONE) failed, device might not work..\r\n";
				}
			}

			context.caps.axes |= flag;
		}

		if (info->dwFlags & DIDOI_POLLED)
			context.caps.mustPoll = TRUE;

		return DIENUM_CONTINUE;
	}

	ibool DirectInput::Joystick::Scan(Key& key,const uint axes) const
	{
		for (uint i=0; i < Caps::NUM_BUTTONS; ++i)
		{
			if (state.rgbButtons[i] & 0x80)
			{
				key.data = state.rgbButtons + i;
				key.code = KeyDown;
				return TRUE;
			}
		}

		for (uint i=0; i < TABLE_KEYS; ++i)
		{
			if (caps.axes & axes & table[i].axis)
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

				if (index < Caps::NUM_BUTTONS)
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
					if (caps.axes & table[--i].axis)
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
			com.Acquire();
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

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("t", on)
    #endif

	NST_FORCE_INLINE void DirectInput::Keyboard::Poll(ibool force)
	{
		if (enabled | force)
		{	
			if (mustPoll)
				com.Poll();

			if (SUCCEEDED(com.GetDeviceState( Buffer::SIZE, buffer )))
				return;

			Acquire();
		}
	}

	NST_FORCE_INLINE void DirectInput::Joystick::Poll(const ibool force)
	{
		if (enabled | force)
		{
			HRESULT hResult;

			if 
			(
		     	(caps.mustPoll && FAILED(hResult=com.Poll())) || 
				FAILED(hResult=com.GetDeviceState( sizeof(state), &state ))
			)
				OnError( hResult );
		}
	}

	void DirectInput::Poll(const PollChoice method)
	{
		keyboard.Poll( method );

		for (Joysticks::Iterator it=joysticks.Begin(), end=joysticks.End(); it != end; ++it)
			(*it)->Poll( method );
	}

	uint DirectInput::KeyDown(const void* const data) throw() 
	{
		return 0U - (uint(*static_cast<const BYTE*>(data)) >> 7);
	}

	uint DirectInput::KeyPosDir(const void* const data) throw()        
	{
    #ifdef NST_SIGN_SHIFT
		return (uint) (-*static_cast<const long*>(data) >> (sizeof(long) * CHAR_BIT - 1));
    #else
		return (*static_cast<const long*>(data) > 0) ? ~0U : 0U;
    #endif
	}

	uint DirectInput::KeyNegDir(const void* const data) throw()
	{
    #ifdef NST_SIGN_SHIFT
		return (uint) (*static_cast<const long*>(data) >> (sizeof(long) * CHAR_BIT - 1));
    #else
		return (*static_cast<const long*>(data) < 0) ? ~0U : 0U;
    #endif
	}

	uint DirectInput::KeyPovUp(const void* const data) throw()
	{
		const DWORD pov = *static_cast<const DWORD*>(data); 

		return 
		(
			(pov & Joystick::POV_CENTER) != Joystick::POV_CENTER && 
			(pov >= Joystick::POV_UPLEFT || pov <= Joystick::POV_UPRIGHT)
		)	? ~0U : 0U;
	}

	uint DirectInput::KeyPovRight(const void* const data) throw()
	{
		const DWORD pov = *static_cast<const DWORD*>(data); 

		return 
		(
			(pov & Joystick::POV_CENTER) != Joystick::POV_CENTER && 
			(pov >= Joystick::POV_UPRIGHT && pov <= Joystick::POV_DOWNRIGHT) 
		)   ? ~0U : 0U;
	}

	uint DirectInput::KeyPovDown(const void* const data) throw()
	{
		const DWORD pov = *static_cast<const DWORD*>(data); 

		return 
		(
			(pov & Joystick::POV_CENTER) != Joystick::POV_CENTER && 
			(pov >= Joystick::POV_DOWNRIGHT && pov <= Joystick::POV_DOWNLEFT)
		)   ? ~0U : 0U;
	}

	uint DirectInput::KeyPovLeft(const void* const data) throw()
	{
		const DWORD pov = *static_cast<const DWORD*>(data); 

		return 
		(
			(pov & Joystick::POV_CENTER) != Joystick::POV_CENTER && 
			(pov >= Joystick::POV_DOWNLEFT && pov <= Joystick::POV_UPLEFT)
		)   ? ~0U : 0U;
	}

	uint DirectInput::KeyNone(const void* const) throw() 
	{ 
		return 0U; 
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("", on)
    #endif

	ibool DirectInput::Key::MapVirtualKey(const uint code,const uint vk1,const uint vk2,const uint vk3)
	{
		Unmap();

		if (!code || code > 0xFF)
			return FALSE;

		vKey = code << 8;

		if (vk1 == VK_MENU || vk2 == VK_MENU || vk3 == VK_MENU) 
			vKey |= FALT;

		if (vk1 == VK_SHIFT || vk2 == VK_SHIFT || vk3 == VK_SHIFT) 
			vKey |= FSHIFT;

		if (vk1 == VK_CONTROL || vk2 == VK_CONTROL || vk3 == VK_CONTROL) 
			vKey |= FCONTROL;

		// forbidden keys:
		//
		// ALT+F4
		// ALT+F6
		// ALT+TAB
		// ALT+SPACE
		// ALT+ESC
		// CTRL+F4
		// CTRL+ESC
		// CTRL+ALT+DELETE
		// LWIN

		switch (code)
		{
			case VK_F4:
		
				if (vKey & FCONTROL)
					return FALSE;
		
			case VK_F6:
			case VK_TAB:
			case VK_SPACE:
		
				if (vKey & FALT)
					return FALSE;
		
				break;
		
			case VK_ESCAPE:
		
				if (vKey & (FCONTROL|FALT))
					return FALSE;
		
				break;		
		
			case VK_DELETE:
		
				if ((vKey & (FCONTROL|FALT)) == (FCONTROL|FALT))
					return FALSE;
		
				break;
		
			case VK_LWIN:
				return FALSE;
		}

		return TRUE;
	}

	ibool DirectInput::Key::MapVirtualKey(cstring name)
	{
		Unmap();

		if (!name || !*name)
			return FALSE;

		uint vk[3] = {0,0,0};

		for (uint i=0; i < 3; ++i)
		{
			if 
			(
				!vk[0] &&
				(name[0] == 'a' || name[0] == 'A') &&
				(name[1] == 'l' || name[1] == 'L') &&
				(name[2] == 't' || name[2] == 'T') &&
				(name[3] == '+')
			)
			{
				vk[0] = VK_MENU;
				name += 4;
			}
			else if 
			(
				!vk[1] &&
				(name[0] == 'c' || name[0] == 'C') &&
				(name[1] == 't' || name[1] == 'T') &&
				(name[2] == 'r' || name[2] == 'R') &&
				(name[3] == 'l' || name[3] == 'L') &&
				(name[4] == '+')
			)
			{
				vk[1] = VK_CONTROL;
				name += 5;
			}
			else if 
			(
				!vk[2] &&
				(name[0] == 's' || name[0] == 'S') &&
				(name[1] == 'h' || name[1] == 'H') &&
				(name[2] == 'i' || name[2] == 'I') &&
				(name[3] == 'f' || name[3] == 'F') &&
				(name[4] == 't' || name[4] == 'T') &&
				(name[5] == '+')
			)
			{
				vk[2] = VK_SHIFT;
				name += 6;
			}
		}

		return MapVirtualKey( System::Keyboard::VikKey( name ), vk[0], vk[1], vk[2] );
	}

	ibool DirectInput::Key::GetVirtualKey(ACCEL& accel) const
	{
		if (code == KeyNone && data)
		{
			accel.fVirt = (BYTE) ((vKey & 0xFF) | FVIRTKEY);
			accel.key = (WORD) (vKey >> 8);
			return TRUE;
		}

		accel.fVirt = 0;
		accel.key = 0;
		return FALSE;
	}
}