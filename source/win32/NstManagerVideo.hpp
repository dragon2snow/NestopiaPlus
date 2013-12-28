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

#ifndef NST_MANAGER_VIDEO_H
#define NST_MANAGER_VIDEO_H

#pragma once

#include "NstWindowStatusBar.hpp"
#include "NstObjectHeap.hpp"
#include "NstDirect2d.hpp"
#include "../core/api/NstApiNsf.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Menu;
		class Video;
	}

	namespace Managers
	{
		class Video
		{
		public:

			typedef DirectX::Direct2D::Mode Mode;
			typedef Window::Point Point;
			typedef Window::Rect Rect;

			Video(Window::Custom&,Window::Menu&,Emulator&,const Paths&,const Configuration&);
			~Video();

			enum 
			{
				STRETCHED = INT_MAX,
				NES_WIDTH = Nes::Video::Output::WIDTH,
				NES_HEIGHT = Nes::Video::Output::HEIGHT
			};

			void StartEmulation();
			void StopEmulation();
			void SwitchScreen();
			void SetFullscreenScale(uint);
			void Save(Configuration&) const;
			
			static Point GetDisplayMode();
			
			const Rect GetNesScreen() const;
			const Rect GetNesScreen(Nes::Machine::Mode) const;

		private:

			typedef Application::Instance Instance;

			enum
			{
				DEFAULT_BPP = 16,
				MIN_DIALOG_WIDTH = 640,
				MIN_DIALOG_HEIGHT = 480,
				STATUSBAR_WIDTH = 11,
				SCREEN_TEXT_DURATION = 2000
			};

			void SwitchFullscreen(const Mode&);
			void ToggleFps(ibool);
			void UpdateScreen();
			
			NST_NO_INLINE void RepairScreen();

			ibool OnPaint         (Window::Param&);
			ibool OnEraseBkGnd    (Window::Param&);     
			void  OnEnterSizeMove (Window::Param&);
			void  OnExitSizeMove  (Window::Param&);
			
			void  OnEmuEvent (Emulator::Event);
			void  OnAppEvent (Instance::Event,const void*);
			
			void  OnMenuOptions          (uint);
			void  OnMenuSaveScreenShot   (uint);
			void  OnMenuUnlimitedSprites (uint);
			void  OnMenuToggleStatusBar  (uint);
			void  OnMenuToggleFps        (uint);

			void  OnScreenText(const String::Generic&);

			ibool OnTimerFps();
			ibool OnTimerText();

			struct Callbacks;

			struct Fps
			{
				inline Fps();

				enum
				{
					UPDATE_INTERVAL = 2000
				};

				uint frame;
			};

			struct Nsf
			{
				inline Nsf();

				void Load (Nes::Nsf);
				void Update (Nes::Nsf);

				String::Heap text;
				uint songTextOffset;
			};

			Emulator& emulator;
			Window::Custom& window;
			const Window::Menu& menu;
			Fps fps;
			Window::StatusBar statusBar;
			DirectX::Direct2D direct2d;
			Object::Heap<Window::Video> dialog;
			Nes::Video::Output nesOutput;
			Nsf nsf;
			ibool sizingMoving;
			uint fullscreenScale;
			const Paths& paths;
			const uint childWindowSwitchCount;

		public:

			Nes::Video::Output* GetOutput()
			{
				return direct2d.IsValidScreen() ? &nesOutput : NULL;
			}

			ibool IsWindowed() const
			{
				return direct2d.IsWindowed();
			}

			ibool IsFullscreen() const
			{
				return !IsWindowed();
			}

			ibool IsVSyncEnabled() const
			{
				return direct2d.IsVSyncEnabled();
			}

			uint GetFullscreenScale() const
			{
				return fullscreenScale;
			}

			const Rect GetScreenRect() const
			{
				Rect rect( direct2d.GetScreenRect() );

				if (direct2d.IsWindowed())
				{
					::ClientToScreen( window, reinterpret_cast<POINT*>( &rect.left  ) );
					::ClientToScreen( window, reinterpret_cast<POINT*>( &rect.right ) );
				}

				return rect;
			}

			ibool MustClearFrameScreen() const
			{
				return IsFullscreen() && fullscreenScale != STRETCHED;
			}

			void ClearScreen()
			{
				if (!direct2d.ClearScreen())
					RepairScreen();
			}

			void PresentScreen()
			{
				if (!direct2d.PresentScreen())
					RepairScreen();
			}
		};
	}
}

#endif
