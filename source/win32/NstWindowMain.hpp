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

#ifndef NST_WINDOW_MAIN_H
#define NST_WINDOW_MAIN_H

#pragma once

#include "NstObjectStack.hpp"
#include "NstWindowDynamic.hpp"
#include "NstWindowMenu.hpp"
#include "NstManagerVideo.hpp"
#include "NstManagerSound.hpp"
#include "NstManagerInput.hpp"
#include "NstManagerFrameClock.hpp"

namespace Nestopia
{
	namespace Managers
	{
		class Paths;
		class Preferences;
		class Emulator;
	}

	namespace Window
	{
		class Main
		{
		public:

			Main
			(
				Managers::Emulator&,
				const Configuration&,
				Menu&,
				const Managers::Paths&,
				const Managers::Preferences&,
				int
			);

			~Main();

			int  Run();
			void Save(Configuration& cfg) const;
			uint GetMaxMessageLength() const;

		private:

			enum
			{
				CLASS_STYLE	=
				(
					CS_HREDRAW |
					CS_VREDRAW
				),

				WIN_STYLE = 
				(
					WS_OVERLAPPED |
					WS_CAPTION |
					WS_SYSMENU |
					WS_MINIMIZEBOX |
					WS_MAXIMIZEBOX |
					WS_THICKFRAME |
					WS_CLIPCHILDREN
				),

				WIN_EXSTYLE =
				(
					WS_EX_ACCEPTFILES | 
					WS_EX_CLIENTEDGE
				),

				FULLSCREEN_STYLE =
				(
					WS_POPUP | 
					WS_VISIBLE
				),

				FULLSCREEN_EXSTYLE =
				(
			     	0
				)
			};

			enum
			{
				MAXIMIZE = INT_MAX,
				FULLSCREEN_RECOVER_TIME = 1500
			};

			struct State
			{
				inline State();

				ibool menu;
				ibool maximized;
				Rect rect;
			};

			void  UpdateScreenSize(Point) const;
			uint  CalculateScreenScale() const;
			ibool IsScreenMatched(Nes::Machine::Mode) const;
			ibool IsWindowMenuEnabled() const;
			ibool CanRunInBackground();
			void  Resize(uint);
			ibool ToggleMenu();

			ibool OnStartEmulation();
			void  OnStopEmulation();

			void OnReturnInputScreen(Rect&);
			void OnReturnOutputScreen(Rect&);

			ibool OnCommand           (Param&);
			ibool OnEnable            (Param&);
			ibool OnEnterSizeMoveMenu (Param&);       
			ibool OnExitSizeMoveMenu  (Param&);       
			ibool OnActivate          (Param&);
			ibool OnSysCommand        (Param&);
			ibool OnNclButton         (Param&);
			ibool OnDisplayChange     (Param&);
			ibool OnPowerBroadCast    (Param&);

			void OnCmdViewScreenSize 	(uint);
			void OnCmdViewSwitchScreen	(uint);
			void OnCmdViewShowOnTop		(uint);
			void OnCmdViewShowMenu      (uint);

			void OnMenuViewScreenSize(Menu::PopupHandler::Param&);
			void OnEmuEvent(Managers::Emulator::Event);
			void OnAppEvent(Application::Instance::Event,const void*);

			const Managers::Preferences& preferences;

			Dynamic window;
			const Menu& menu;

			Managers::Emulator& emulator;

			Object::Stack<Managers::FrameClock> frameClock;
			Object::Stack<Managers::Sound> sound;
			Object::Stack<Managers::Input> input;
			Object::Stack<Managers::Video> video;

			State state;

			static const char windowName[];

			ibool IsFullscreen() const
			{
				return video->IsFullscreen();
			}

			ibool IsWindowed() const
			{
				return video->IsWindowed();
			}

		public:

			Custom& Get()
			{
				return window;
			}

			const Custom& Get() const
			{
				return window;
			}
		};
	}
}

#endif
