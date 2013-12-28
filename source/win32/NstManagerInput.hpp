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

#ifndef NST_MANAGER_INPUT_H
#define NST_MANAGER_INPUT_H

#pragma once

#include "NstDirectInput.hpp"

namespace Nestopia
{
	namespace Resource
	{
		class Cursor;
	}

	namespace Window
	{
		class Input;
	}

	namespace Managers
	{
		class Input
		{
		public:

			typedef Object::Delegate<void,Window::Rect&> Screening;

			Input
			(
		     	Window::Custom&,
				Window::Menu&,
				Emulator&,
				const Configuration&,
				const Screening&,
				const Screening&
			);

			~Input();

			void Save(Configuration&) const;
			void StartEmulation();
			void StopEmulation();

		private:

			void UpdateDevices();
			void UpdateSettings();
			void OnEmuEvent(Emulator::Event);

			void OnCmdMachineAutoSelectController(uint);
			void OnCmdMachinePort(uint);
			void OnCmdOptionsInput(uint);

			void Poll();

			inline void AutoPoll();

			struct Callbacks;

			struct Rects
			{
				Rects(const Screening&,const Screening&);

				const Screening getInput;
				const Screening getOutput;
			};

			class Cursor
			{
			public:

				Cursor(Window::Custom&);

				void Acquire(Emulator&);
				void Unacquire();
				void AutoHide();

			private:

				enum
				{
					TIME_OUT = 3000
				};

				static inline DWORD NextTime();

				inline void Refresh();
				inline void UpdateTime();

				ibool OnNop               (Window::Param&);
				ibool OnSetCursor         (Window::Param&);
				ibool OnMouseMove         (Window::Param&);
				ibool OnLButtonDown       (Window::Param&);
				ibool OnRButtonDown       (Window::Param&);
				ibool OnButtonUp          (Window::Param&);
				void  OnEnterSizeMoveMenu (Window::Param&);
				void  OnExitSizeMoveMenu  (Window::Param&);

				HCURSOR hCurrent;
				HCURSOR hCursor;
				LPARAM pos;
				ibool autoHide;
				DWORD deadline;
				ibool inNonClient;
				Window::Custom& window;

				static const Resource::Cursor gun;

			public:

				static const uint primaryButtonId;
				static const uint secondaryButtonId;

				ibool AutoHidingEnabled() const
				{
					return autoHide;
				}
			};

			struct AutoFire
			{
				inline AutoFire();
				inline ibool Signaled() const;

				uint step;
				uint signal;

				void Step()
				{
					if (++step > signal * 2)
						step = 0;
				}
			};

			Rects rects;
			Window::Menu& menu;
			Emulator& emulator;
			ibool polled;
			AutoFire autoFire;
			Cursor cursor;
			Nes::Input::Controllers nesControllers;
			DirectX::DirectInput directInput;
			Object::Heap<Window::Input> dialog;

		public:

			Nes::Input::Controllers* GetOutput()
			{
				autoFire.Step();

				polled ^= TRUE;

				if (polled)
					Poll();

				return &nesControllers;
			}

			void RefreshCursor()
			{
				if (cursor.AutoHidingEnabled())
					cursor.AutoHide();
			}
		};
	}
}

#endif
