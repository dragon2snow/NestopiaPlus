////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2007 Martin Freij
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
		class Input : Manager
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

			inline void CheckPoll();
			void ForcePoll();

			void UpdateDevices();
			void UpdateSettings();
			void OnEmuEvent(Emulator::Event);
			void OnMenuKeyboard(Window::Menu::PopupHandler::Param&);
			void OnMenuPort1(Window::Menu::PopupHandler::Param&);
			void OnMenuPort2(Window::Menu::PopupHandler::Param&);
			void OnMenuPort3(Window::Menu::PopupHandler::Param&);
			void OnMenuPort4(Window::Menu::PopupHandler::Param&);
			void OnMenuExpPort(Window::Menu::PopupHandler::Param&);
			void OnCmdMachineAutoSelectController(uint);
			void OnCmdMachinePort(uint);
			void OnCmdMachineAdapter(uint);
			void OnCmdMachineKeyboardPaste(uint);
			void OnCmdOptionsInput(uint);

			class Callbacks;

			struct Rects
			{
				Rects(const Screening&,const Screening&);

				const Screening getInput;
				const Screening getOutput;
			};

			class Cursor
			{
			public:

				explicit Cursor(Window::Custom&);

				void Acquire(Emulator&);
				void Unacquire();
				void AutoHide();

				inline int GetWheel() const;

			private:

				enum
				{
					TIME_OUT    = 3000,
					WHEEL_MIN   = -WHEEL_DELTA*30,
					WHEEL_MAX   = +WHEEL_DELTA*30,
					WHEEL_SCALE = 56
				};

				ibool OnNop            (Window::Param&);
				ibool OnSetCursor      (Window::Param&);
				ibool OnMouseMove      (Window::Param&);
				ibool OnLButtonDown    (Window::Param&);
				ibool OnRButtonDown    (Window::Param&);
				ibool OnRButtonDownNop (Window::Param&);
				ibool OnButtonUp       (Window::Param&);
				ibool OnWheel          (Window::Param&);

				HCURSOR hCursor;
				HCURSOR hCurrent;
				DWORD deadline;
				Window::Custom& window;
				int wheel;

				static const Resource::Cursor gun;

			public:

				bool MustAutoHide() const
				{
					return deadline != DWORD(~0UL) && deadline <= ::GetTickCount();
				}

				static const uint primaryButtonId;
				static const uint secondaryButtonId;
			};

			class CmdKeys
			{
				typedef DirectX::DirectInput::Key Key;

			public:

				CmdKeys(Window::Custom&,DirectX::DirectInput&);

				void BeginAdd();
				void Add(const Key&,uint);
				void EndAdd();
				void Acquire();
				void Unacquire();

				inline void Poll();

			private:

				inline bool CanPoll() const;

				bool ForcePoll();
				void Update();
				uint OnTimer();
				void OnFocus(Window::Param&);

				enum
				{
					POLL_RAPID    = 50,
					POLL_REST     = 1000,
					CLOCK_DEFAULT = 1,
					CLOCK_STOP    = 0
				};

				struct CmdKey : Key
				{
					CmdKey(const Key&,uint);

					uint cmd;
					bool prev;
				};

				typedef Collection::Vector<CmdKey> Keys;

				bool acquired;
				Keys keys;
				uint clock;
				const Window::Custom& window;
				DirectX::DirectInput& directInput;
			};

			class ClipBoard : String::Heap<wchar_t>
			{
				uint pos;
				uchar releasing;
				uchar hold;
				bool shifted;
				bool paste;

			public:

				ClipBoard();

				enum Type
				{
					FAMILY,
					SUBOR
				};

				uint Query(const uchar* NST_RESTRICT,Type);
				bool CanPaste() const;
				void Paste();
				void Clear();
				void operator ++ ();
				inline uint operator * ();
				inline bool Shifted() const;
				inline void Shift();
			};

			class AutoFire
			{
				uint step;
				uint signal;

			public:

				inline AutoFire();
				inline bool Signaled() const;
				void operator = (uint);

				void Step()
				{
					if (++step > signal)
						step = 0;
				}
			};

			Rects rects;
			ibool polled;
			AutoFire autoFire;
			Cursor cursor;
			Nes::Input::Controllers nesControllers;
			DirectX::DirectInput directInput;
			Object::Heap<Window::Input> dialog;
			CmdKeys cmdKeys;
			ClipBoard clipBoard;

		public:

			void Calibrate(bool full)
			{
				directInput.Calibrate( full );
			}

			Nes::Input::Controllers* GetOutput()
			{
				autoFire.Step();
				return &nesControllers;
			}

			void Poll()
			{
				polled ^= true;

				if (polled)
					ForcePoll();

				if (cursor.MustAutoHide())
					cursor.AutoHide();
			}
		};
	}
}

#endif
