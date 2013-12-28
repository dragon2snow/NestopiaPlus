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

#include "NstApplicationException.hpp"
#include "NstSystemKeyboard.hpp"
#include "NstWindowStruct.hpp"
#include "NstDirectInput.hpp"

namespace Nestopia
{
	using DirectX::DirectInput;

	DirectInput::Keyboard::Keyboard(const Manager& manager)
	: enabled(TRUE), mustPoll(FALSE)
	{
		Clear();

		if (FAILED(manager->CreateDevice( GUID_SysKeyboard, &pointer, NULL )))
		{
			pointer = NULL;
			throw Application::Exception("IDirectInput8::CreateDevice() failed!");
		}

		if (FAILED(pointer->SetDataFormat( &c_dfDIKeyboard )))
			throw Application::Exception("IDirectInputDevice8::SetDataFormat() failed!");

		{
			Window::Struct<DIDEVCAPS> caps;

			if (FAILED(pointer->GetCapabilities( &caps )) || (caps.dwFlags & DIDC_POLLEDDATAFORMAT))
				mustPoll = TRUE;
		}

		SetCooperativeLevel( manager.Window() );
	}

	void DirectInput::Keyboard::SetCooperativeLevel(HWND const hWnd,const DWORD flags) const
	{
		if (FAILED(pointer->SetCooperativeLevel( hWnd, flags )))
			throw Application::Exception("IDirectInputDevice8::SetCooperativeLevel() failed!");
	}

	DirectInput::Keyboard::~Keyboard()
	{
		if (pointer)
		{
			pointer->Unacquire();
			pointer->Release();
			pointer = NULL;
		}
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
			pointer->Acquire();
			Clear();
		}
	}
}
