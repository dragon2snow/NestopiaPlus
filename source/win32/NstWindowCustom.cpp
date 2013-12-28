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
#include "NstWindowCustom.hpp"

namespace Nestopia
{
	using Window::Custom;

	Custom::Timers Custom::timers;

	void Custom::StartTimer(const TimerCallback timer,const uint duration) const
	{
		NST_ASSERT( hWnd );

		uint id;

		for (id=0; id < timers.Size() && timers[id] != timer; ++id);

		if (timers.Size() == id)
			timers.PushBack( timer );

		timers[id].active = TRUE;

		if (!::SetTimer( hWnd, id, duration, &TimerProc ))
			throw Application::Exception("SetTimer() failed!");
	}

	ibool Custom::StopTimer(const TimerCallback timer) const
	{
		for (uint id=0; id < timers.Size(); ++id)
		{
			if (timers[id] == timer && timers[id].active)
			{
				timers[id].active = FALSE;
				::KillTimer( hWnd, id );
				return TRUE;
			}
		}

		return FALSE;
	}

	void CALLBACK Custom::TimerProc(HWND hWnd,uint,UINT_PTR id,DWORD)
	{
		NST_ASSERT( id < timers.Size() );

		if (!timers[id]() && timers[id].active)
		{
			timers[id].active = FALSE;
			::KillTimer( hWnd, id );
		}
	}
}
