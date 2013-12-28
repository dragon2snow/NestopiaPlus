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
#include "NstDirectInput.hpp"
#include "NstIoLog.hpp"

namespace Nestopia
{
	using DirectX::DirectInput;

	DirectInput::Manager::Manager(HWND const handle)
	: hWnd(handle)
	{
		Io::Log() << "DirectInput: initializing..\r\n";

		if (FAILED(::DirectInput8Create( Application::Instance::GetHandle(), DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<void**>(&pointer), NULL )))
		{
			pointer = NULL;
			throw Application::Exception("DirectInput8Create() failed!");
		}
	}

	DirectInput::DirectInput(HWND const handle)
	: manager(handle), keyboard(manager)
	{
		Io::Log logfile;

		if (SUCCEEDED(manager->EnumDevices( DI8DEVCLASS_GAMECTRL, EnumJoysticks, this, DIEDFL_ATTACHEDONLY )))
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
				directInput.joysticks.Back().Construct( directInput.manager, instance->guidInstance );
			}
			catch (Joystick::Exception)
			{
				directInput.joysticks.Shrink();
				Io::Log() << "DirectInput: warning, bogus device, continuing enumeration..\r\n";
			}
		}

		return DIENUM_CONTINUE;
	}

	void DirectInput::Acquire(const ibool force)
	{
		keyboard.Acquire( force );

		for (Joysticks::Iterator it=joysticks.Begin(), end=joysticks.End(); it != end; ++it)
			(*it)->Acquire( force );
	}

	void DirectInput::Unacquire() const
	{
		keyboard.Unacquire();

		for (Joysticks::ConstIterator it=joysticks.Begin(), end=joysticks.End(); it != end; ++it)
			(*it)->Unacquire();
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

	DirectInput::ScanResult DirectInput::ScanKey(Key& key) const
	{
		const ScanResult scan = keyboard.Scan( key );

		if (scan != SCAN_GOOD_KEY)
		{
			if (scan == SCAN_NO_KEY)
			{
				for (Joysticks::ConstIterator it=joysticks.Begin(), end=joysticks.End(); it != end; ++it)
				{
					if ((*it)->Scan( key ))
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
						if ((*it)->Guid() == guid)
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

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("t", on)
    #endif

	void DirectInput::Poll(const PollChoice method)
	{
		keyboard.Poll( method );

		for (Joysticks::Iterator it=joysticks.Begin(), end=joysticks.End(); it != end; ++it)
			(*it)->Poll( method );
	}

    #ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4146 )
    #endif

	uint DirectInput::KeyDown(const void* const data) throw() 
	{
    #ifdef NST_UNSIGNED_NEG
		return -(uint(*static_cast<const BYTE*>(data)) >> 7);
    #else
		return (*static_cast<const BYTE*>(data) & 0x80) ? ~0U : 0U;
    #endif
	}

	uint DirectInput::KeyPosDir(const void* const data) throw()        
	{
    #ifdef NST_SIGN_SHIFT
		return (uint) (-*static_cast<const long*>(data) >> (sizeof(long) * CHAR_BIT - 1));
    #else
		return (*static_cast<const long*>(data) > 0) ? ~0U : 0U;
    #endif
	}

    #ifdef _MSC_VER
    #pragma warning( pop )
    #endif

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
}
