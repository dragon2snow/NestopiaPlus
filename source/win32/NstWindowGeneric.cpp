////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2006 Martin Freij
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
		   
#include "NstObjectPod.hpp"
#include "NstResourceIcon.hpp"
#include "NstApplicationException.hpp"
#include "NstApplicationInstance.hpp"
#include "NstWindowGeneric.hpp"
#include <WindowsX.h>

namespace Nestopia
{
	using namespace Window;

	Generic::LockDraw::LockDraw(HWND const handle)
	: hWnd(handle)
	{
		NST_ASSERT( handle );
		SetWindowRedraw( handle, FALSE );
	}

	Generic::LockDraw::~LockDraw()
	{
		SetWindowRedraw( hWnd, TRUE );
		::InvalidateRect( hWnd, NULL, FALSE );
	}

	uint Generic::Stream::operator >> (ulong& value) const
	{
		String::Stack<16,tchar> string;
		string.ShrinkTo( ::GetWindowText( hWnd, string.Ptr(), 16+1 ) );
		string >> value;
		return string.Length();
	}

	uint Generic::Stream::operator >> (long& value) const
	{
		String::Stack<16,tchar> string;
		string.ShrinkTo( ::GetWindowText( hWnd, string.Ptr(), 16+1 ) );
		string >> value;
		return string.Length();
	}

	void Generic::SetStyle(long style) const
	{
		::SetWindowLongPtr( hWnd, GWL_STYLE, style );
		::SetWindowPos( hWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE );
	}

	void Generic::SetStyle(long style,long exStyle) const
	{
		::SetWindowLongPtr( hWnd, GWL_STYLE, style );
		::SetWindowLongPtr( hWnd, GWL_EXSTYLE, exStyle );
		::SetWindowPos( hWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE );
	}

	void Generic::ModifyStyle(long flag,ibool on) const
	{
		const long style = GetStyle();

		if ((style & flag) != (on ? flag : 0))
			SetStyle( (style & ~flag) | (on ? flag : 0) );
	}

	void Generic::Maximize() const
	{
		::SendMessage( hWnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0 );
	}

	void Generic::Minimize() const
	{
		::SendMessage( hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0 );
	}

	void Generic::Restore() const 
	{
		::SendMessage( hWnd, WM_SYSCOMMAND, SC_RESTORE, 0 );
	}

	ibool Generic::Activate(const ibool sameThread) const
	{
		if (IsEnabled())
		{
			if (Minimized())
			{
				if (sameThread)
					Send( WM_SYSCOMMAND, SC_RESTORE, 0 );
				else
					Post( WM_SYSCOMMAND, SC_RESTORE, 0 );
			}
			else
			{
				::SetForegroundWindow( hWnd );
			}

			return TRUE;
		}

		return FALSE;
	}

	void Generic::ValidateRect() const
	{
		::ValidateRect( hWnd, NULL );
	}

	void Generic::InvalidateRect() const
	{
		::InvalidateRect( hWnd, NULL, FALSE );
	}

	void Generic::Close() const
	{
		::PostMessage( hWnd, WM_CLOSE, 0, 0 );
	}

	void Generic::Invalidate(const ibool erase) const
	{
		::InvalidateRect( hWnd, NULL, erase );
	}

	void Generic::Destroy()
	{
		if (hWnd)
		{
			::DestroyWindow( hWnd );
			hWnd = NULL;
		}
	}

	void Generic::Redraw() const
	{
		::InvalidateRect( hWnd, NULL, FALSE );
	}

	void Generic::Set(const Rect& rect,HWND const zOrder,const DWORD flags) const
	{
		Set( rect.Position(), rect.Size(), zOrder, flags );
	}

	void Generic::Set(const Rect& rect,const DWORD flags) const
	{
		Set( rect.Position(), rect.Size(), NULL, flags|SWP_NOZORDER );
	}

	void Generic::Set(const Point& pos,const Point& size,HWND const zOrder,DWORD flags) const
	{
		::SetWindowPos
		(
			hWnd, 
			zOrder, 
			pos.x, 
			pos.y, 
			size.x, 
			size.y, 
			flags 
		);
	}

	void Generic::Resize(const Point& size,const DWORD flags) const
	{
		::SetWindowPos
		(
			hWnd, 
			NULL, 
			0, 
			0, 
			size.x, 
			size.y, 
			flags|SWP_NOMOVE|SWP_NOZORDER
		);
	}

	void Generic::Move(const Point& pos,const DWORD flags) const
	{
		::SetWindowPos
		(
			hWnd, 
			NULL, 
			pos.x, 
			pos.y, 
			0, 
			0, 
			flags|SWP_NOSIZE|SWP_NOZORDER
		);
	}

	void Generic::Reorder(HWND const zOrder,const DWORD flags) const
	{
		::SetWindowPos
		(
			hWnd, 
			zOrder, 
			0, 
			0, 
			0, 
			0, 
			flags|SWP_NOSIZE|SWP_NOMOVE 
		);
	}

	void Generic::Magnify(const Point& size) const
	{
		Resize( GetNormalWindowRect().Size() + size, SWP_FRAMECHANGED );
	}

	void Generic::Shrink(const Point& size) const
	{
		const Point current( GetNormalWindowRect() );

		if (current >= size)
			Resize( current - size, SWP_FRAMECHANGED );
	}

	ibool Generic::IsChild(HWND const hChild) const
	{
		return hChild && hWnd && hChild != hWnd && ::IsChild( hWnd, hChild );
	}

	LONG_PTR Generic::Send(const uint uMsg) const
	{
		return ::SendMessage( hWnd, uMsg, 0, 0 );
	}

	LONG_PTR Generic::SendCommand(const uint id) const
	{
		return ::SendMessage( hWnd, WM_COMMAND, id, 0 );
	}

	void Generic::Post(const uint uMsg) const
	{
		::PostMessage( hWnd, uMsg, 0, 0 );
	}

	void Generic::PostCommand(const uint id) const
	{
		::PostMessage( hWnd, WM_COMMAND, id, 0 );
	}

	void Generic::Register
	(
		tstring const className,
		const uint classStyle,
		WNDPROC const wndProc,
		HCURSOR const hCursor,
		HBRUSH const hBrush,
		HICON const hIcon
	)
	{
		NST_ASSERT( className && *className );

		Object::Pod<WNDCLASSEX> winClass;

		winClass.cbSize        = sizeof(winClass);
		winClass.style         = classStyle;
		winClass.lpfnWndProc   = wndProc;
		winClass.hInstance     = Application::Instance::GetHandle();
		winClass.hCursor       = hCursor;
		winClass.hbrBackground = hBrush;
		winClass.lpszClassName = className;
		winClass.hIcon         = 
		winClass.hIconSm       = hIcon;

		if (!RegisterClassEx( &winClass ))
			throw Application::Exception(_T("RegisterClassEx() failed!"));
	}

	void Generic::Unregister(tstring className)
	{
		NST_ASSERT( className && *className );

		::UnregisterClass( className, Application::Instance::GetHandle() );
	}

	Generic Generic::Create
	(
		const DWORD exStyle,
		tstring const className,
		tstring const windowName,
		const DWORD style,
		const uint x,
		const uint y,
		const uint width,
		const uint height,
		HWND const hParent,
		HMENU const hMenu,
		void* const param
	)
	{
		NST_ASSERT( className && *className );

		HWND const hWnd = CreateWindowEx
		( 
			exStyle,                       
			className,         
			windowName,          
			style,
			x,
			y,
			width,
			height,
			hParent,
			hMenu,
			Application::Instance::GetHandle(),
			param
		);

		if (!hWnd)
			throw Application::Exception(_T("CreateWindowEx() failed!"));

		return hWnd;
	}

	Rect Generic::GetNormalWindowRect() const
	{
		WINDOWPLACEMENT winPlacement;

		winPlacement.length = sizeof(winPlacement);

		if (::GetWindowPlacement( hWnd, &winPlacement ))
			return winPlacement.rcNormalPosition;

		return 0;
	}

	void Generic::SetNormalWindowRect(const Rect& rect) const
	{
		WINDOWPLACEMENT winPlacement;

		winPlacement.length = sizeof(winPlacement);
		winPlacement.rcNormalPosition = rect;

		::SetWindowPlacement( hWnd, &winPlacement );
	}

	void Generic::SetIcon(const uint id,const uint type) const
	{
		Send( WM_SETICON, type, (HICON) Resource::Icon(id) );
	}

	ibool Generic::IsCursorInsideClientArea() const
	{
		Point pos;
		::GetCursorPos( &pos );
		return Rect::Screen( hWnd ).IsInside( pos );
	}
}
