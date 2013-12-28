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

#ifndef NST_APPLICATION_INSTANCE_H
#define NST_APPLICATION_INSTANCE_H

#pragma once

#include "NstObjectDelegate.hpp"
#include "NstApplicationConfiguration.hpp"
#include <Windows.h>

namespace Nestopia
{
	namespace Application
	{
		class Instance
		{
		public:

			explicit Instance(cstring);
			~Instance();

			static HINSTANCE GetHandle();

			static const String::Path<false>& GetPath();
			static const String::Path<true> GetPath(String::Generic);

			static String::Generic GetVersion();

			static HWND  GetActiveWindow();
			static uint  NumChildWindows();
			static void  ShowChildWindows(uint=TRUE);
			static HWND  GetChildWindow(uint);
			static ibool IsAnyChildWindowVisible();
			static HWND  GetMainWindow();

			static void Launch(String::Generic,uint=0);

			enum
			{
				WM_NST_OPEN = WM_APP + 54
			};

			enum Event
			{
				EVENT_WINDOW_CREATE = 1,
				EVENT_WINDOW_DESTROY,
				EVENT_SYSTEM_BUSY,
				EVENT_FULLSCREEN,
				EVENT_DESKTOP
			};

			class Events : Sealed
			{
			public:

				struct WindowCreateParam
				{
					HWND hWnd;
					uint x;
					uint y;
				};

				struct WindowDestroyParam
				{
					HWND hWnd;
				};

			private:

				typedef Object::Delegate2<void,Event,const void*> Callback;
				typedef Collection::Vector<Callback> Callbacks;

				static void Add(const Callback&);

				static Callbacks callbacks;

			public:

				static void Remove(const void*);
				static void Signal(Event,const void* = NULL);

				template<typename Data,typename Code>
				static void Add(Data* data,Code code)
				{
					Add( Callback(data,code) );
				}
			};

		private:

			Configuration cfg;

			struct Global;
			static Global global;

		public:

			Configuration& GetConfiguration()
			{
				return cfg;
			}

			static cstring GetClassName()
			{
				return "Nestopia";
			}

			class Waiter
			{
				HCURSOR const hCursor;

			public:

				Waiter();

				~Waiter()
				{
					::SetCursor( hCursor );
				}
			};
		};
	}
}

#endif
