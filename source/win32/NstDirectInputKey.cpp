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

#include "NstDirectInput.hpp"
#include "NstSystemKeyboard.hpp"

namespace Nestopia
{
	using DirectX::DirectInput;

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
