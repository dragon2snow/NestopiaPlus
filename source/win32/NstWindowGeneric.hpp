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

#ifndef NST_WINDOW_GENERIC_H
#define NST_WINDOW_GENERIC_H

#pragma once

#include "NstWindowRect.hpp"
#include "NstString.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Generic
		{
		protected:

			HWND hWnd;

		public:

			ibool IsChild(HWND) const;
			void  Magnify(const Point&) const;
			void  Shrink(const Point&) const;
			void  Set(const Point&,const Point&,HWND,DWORD) const;
			void  Set(const Rect&,HWND,DWORD) const;
			void  Set(const Rect&,DWORD=0) const;
			void  SetNormalWindowRect(const Rect&) const;
			void  SetIcon(uint,uint=ICON_SMALL) const;
			void  Resize(const Point&,DWORD=0) const;
			void  Move(const Point&,DWORD=0) const;
			void  Reorder(HWND,DWORD=0) const;
			Rect  GetNormalWindowRect() const;
			ibool IsCursorInsideClientArea() const;
			void  Maximize() const;
			void  Minimize() const;
			void  Restore() const;
			void  ValidateRect() const;
			void  InvalidateRect() const;
			void  Post(uint) const;
			void  PostCommand(uint) const;
			void  Close() const;
			void  Invalidate(ibool=TRUE) const;
			void  Redraw() const;
			ibool Activate(ibool=TRUE) const;
			void  CopyData(const void*,uint,uint=0,HWND=NULL) const;
			void  Destroy();

			LONG_PTR Send(uint) const;
			LONG_PTR SendCommand(uint) const;

			static void Register
			(
				cstring,
				uint,
				WNDPROC,
				HCURSOR,
				HBRUSH,
				HICON
			);
            
			static Generic Create
			(
				DWORD,
				cstring,
				cstring,
				DWORD,
				uint,
				uint,
				uint,
				uint,
				HWND,
				HMENU,
				void* = NULL
			);

			Generic(HWND h=NULL)
			: hWnd(h) {}

			explicit Generic(cstring className)
			: hWnd(::FindWindow( className, NULL )) 
			{
				NST_ASSERT( className && *className );
			}

			Generic& operator = (HWND h)
			{
				hWnd = h;
				return *this;
			}

			operator HWND () const
			{
				return hWnd;
			}

			ibool Enable(ibool state=TRUE) const
			{
				return ::EnableWindow( hWnd, state );
			}

			ibool Disable() const
			{
				return Enable( FALSE );
			}

			ibool IsEnabled() const
			{
				return ::IsWindowEnabled( hWnd );
			}

			ibool IsDisabled() const
			{
				return !IsEnabled();
			}

			ibool Maximized() const
			{
				return ::IsZoomed( hWnd );
			}

			ibool Minimized() const
			{
				return ::IsIconic( hWnd );
			}

			ibool Restored() const
			{
				return !Maximized() && !Minimized();
			}

			ibool IsForeground() const
			{
				return hWnd && hWnd == ::GetForegroundWindow();
			}

			ibool HasMenu() const
			{
				return ::GetMenu( hWnd ) != NULL;
			}

			Generic GetParent() const
			{
				return Generic( ::GetParent( hWnd ) );
			}

			void Show(int cmdShow) const
			{
				::ShowWindow( hWnd, cmdShow );
			}

			Rect GetWindowRect() const
			{
				return Rect::Window( hWnd );
			}

			Rect GetClientRect() const
			{
				return Rect::Client( hWnd );
			}

			Rect GetScreenRect() const
			{
				return Rect::Screen( hWnd );
			}

			Rect GetRectangle() const
			{
				return GetWindowRect();
			}

			Point GetPosition() const
			{
				return GetWindowRect().Position();
			}

			Point GetSize() const
			{
				return GetWindowRect().Size();
			}

			template<typename T>
			void SetStyle(T style,T exStyle) const
			{
				::SetWindowLongPtr( hWnd, GWL_STYLE, style );
				::SetWindowLongPtr( hWnd, GWL_EXSTYLE, exStyle );
			}

			template<typename Param1,typename Param2>
			LONG_PTR Send(uint uMsg,Param1 param1,Param2 param2) const
			{
				return ::SendMessage
				(
					hWnd, 
					uMsg, 
					(WPARAM) param1, 
					(LPARAM) param2
				);
			}

			template<typename Param1,typename Param2>
			void Post(uint uMsg,Param1 param1,Param2 param2) const
			{
				::PostMessage
				(
					hWnd, 
					uMsg, 
					(WPARAM) param1, 
					(LPARAM) param2
				);
			}

			ibool IsParent(HWND hChild) const
			{
				return ::IsChild( hWnd, hChild );
			}

			class Stream
			{
				HWND const hWnd;

			public:

				Stream(Generic w)
				: hWnd(w.hWnd) {}

				void operator << (const String::Anything& string) const
				{
					::SetWindowText( hWnd, string );
				}

				template<typename T> uint operator >> (T&) const;

				uint operator >> (long&) const;
				uint operator >> (ulong&) const;

				uint operator >> ( schar&  i ) const { long  t; const uint r = operator >> (t); i = ( schar  ) t; return r; }
				uint operator >> ( uchar&  i ) const { ulong t; const uint r = operator >> (t); i = ( uchar  ) t; return r; }
				uint operator >> ( short&  i ) const { long  t; const uint r = operator >> (t); i = ( short  ) t; return r; }
				uint operator >> ( ushort& i ) const { ulong t; const uint r = operator >> (t); i = ( ushort ) t; return r; }
				uint operator >> ( int&    i ) const { long  t; const uint r = operator >> (t); i = ( int    ) t; return r; }
				uint operator >> ( uint&   i ) const { ulong t; const uint r = operator >> (t); i = ( uint   ) t; return r; }

				void Clear() const
				{
					::SetWindowText( hWnd, "" );
				}
			};

			Stream Text() const
			{
				return *this;
			}

			class LockDraw
			{
				HWND const hWnd;

			public:

				LockDraw(HWND);
				~LockDraw();
			};
		};

		template<typename T>
		uint Generic::Stream::operator >> (T& string) const
		{
			uint size = ::GetWindowTextLength( hWnd );
			string.Reserve( size );
			size = ::GetWindowText( hWnd, string, size + 1 );
			string.ShrinkTo( size );
			return size;
		}
	}
}

#endif
