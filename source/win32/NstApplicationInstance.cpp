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

#pragma comment(lib,"comctl32")

#include "NstResourceCursor.hpp"
#include "NstResourceVersion.hpp"
#include "NstApplicationException.hpp"
#include "NstApplicationInstance.hpp"
#include "NstWindowStruct.hpp"
#include "NstWindowGeneric.hpp"
#include <CommCtrl.h>

#define _WIN32_DCOM
#include <ObjBase.h>

#define NST_MENU_CLASS_NAME "#32768" // name for menus as documented on MSDN
#define NST_APP_CLASS_NAME "Nestopia"
#define NST_STATUS_CLASS_NAME STATUSCLASSNAME
#define NST_IME_CLASS_NAME "IME"
#define NST_APP_MUTEX_NAME "Nestopia Mutex"

namespace Nestopia
{
	using Application::Instance;

	struct Instance::Global
	{
		Global();

		struct Paths
		{
			Paths();

			String::Path<false> path;
		};

		struct Hooks
		{
			Hooks(HINSTANCE const);
			~Hooks();

			void OnCreate(const CREATESTRUCT&,HWND const);
			void OnDestroy(HWND const);

			static LRESULT CALLBACK CBTProc(int,WPARAM,LPARAM);

			enum
			{
				CHILDREN_RESERVE = 8
			};

			typedef Collection::Vector<HWND> Children;

			HHOOK const handle;
			HWND window;
			Children children;
		};

		HINSTANCE const hInstance;
		const Resource::Version version;
		const Paths paths;
		Hooks hooks;
	};

	Instance::Global Instance::global;
	Instance::Events::Callbacks Instance::Events::callbacks;

	Instance::Global::Paths::Paths()
	{
		String::Path<true> buffer;
		buffer.ShrinkTo( ::GetModuleFileName( NULL, buffer, buffer.Capacity() ) );
		path = buffer;
	}

	Instance::Global::Hooks::Hooks(HINSTANCE const hInstance)
	: handle( ::SetWindowsHookEx( WH_CBT, CBTProc, hInstance, ::GetCurrentThreadId() ))
	{
		children.Reserve( CHILDREN_RESERVE );
	}

	Instance::Global::Hooks::~Hooks()
	{
		if (handle)
			::UnhookWindowsHookEx( handle );
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("t", on)
    #endif

	void Instance::Global::Hooks::OnCreate(const CREATESTRUCT& createStruct,HWND const hWnd)
	{
		NST_ASSERT( hWnd );

		if (window == createStruct.hwndParent || !window)
		{
			{
				enum
				{
					LENGTH = NST_MAX( NST_COUNT(NST_APP_CLASS_NAME) - 1,
             		     	 NST_MAX( NST_COUNT(NST_STATUS_CLASS_NAME) - 1, NST_COUNT(NST_MENU_CLASS_NAME) - 1 ))
				};

				String::Stack<LENGTH> name;
				name.ShrinkTo( ::GetClassName( hWnd, name, LENGTH+1 ) );

				if (name.ExactEqual( NST_MENU_CLASS_NAME ) || name.ExactEqual( NST_STATUS_CLASS_NAME ) || name.ExactEqual( NST_IME_CLASS_NAME ))
					return;

				if (window)
				{
					children.PushBack( hWnd );
					Events::Signal( EVENT_SYSTEM_BUSY );
				}
				else if (name.ExactEqual( NST_APP_CLASS_NAME ))
				{
					window = hWnd;
				}
				else
				{
					return;
				}
			}

			const Events::WindowCreateParam param = 
			{
				hWnd,
				createStruct.cx > 0 ? createStruct.cx : 0,
				createStruct.cy > 0 ? createStruct.cy : 0
			};

			Events::Signal( EVENT_WINDOW_CREATE, &param );
		}
	}

	void Instance::Global::Hooks::OnDestroy(HWND const hWnd)
	{
		NST_ASSERT( hWnd );

		Children::Iterator const child = children.Find( hWnd );

		if (child)
		{
			Events::Signal( EVENT_SYSTEM_BUSY );
		}
		else if (window != hWnd)
		{
			return;
		}

		{
			const Events::WindowDestroyParam param = { hWnd };
			Events::Signal( EVENT_WINDOW_DESTROY, &param );
		}

		if (child)
			children.Erase( child );
		else
			window = NULL;
	}

	LRESULT CALLBACK Instance::Global::Hooks::CBTProc(int nCode,WPARAM wParam,LPARAM lParam)
	{
		NST_COMPILE_ASSERT( HCBT_CREATEWND >= 0 && HCBT_DESTROYWND >= 0 );

		switch (nCode)
		{
			case HCBT_CREATEWND:
		
				NST_ASSERT( lParam );
		
				if (const CREATESTRUCT* const createStruct = reinterpret_cast<const CBT_CREATEWND*>(lParam)->lpcs)
					global.hooks.OnCreate( *createStruct, reinterpret_cast<HWND>(wParam) );
		
				return 0;
		
			case HCBT_DESTROYWND:
		
				NST_VERIFY( wParam );
		
				if (wParam)
					global.hooks.OnDestroy( reinterpret_cast<HWND>(wParam) );
		
				return 0;
		}

		return (nCode < 0) ? ::CallNextHookEx( global.hooks.handle, nCode, wParam, lParam ) : 0;
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("", on)
    #endif

	Instance::Global::Global()
	: 
	hInstance ( ::GetModuleHandle(NULL) ),
	hooks     ( hInstance )
	{
	}

	HINSTANCE Instance::GetHandle()
	{
		return global.hInstance;
	}

	String::Generic Instance::GetVersion()
	{
		return global.version;
	}

	const String::Path<false>& Instance::GetPath()
	{
		return global.paths.path;
	}

	const String::Path<true> Instance::GetPath(const String::Generic file)
	{
		return String::Path<true>( global.paths.path.Directory(), file, "" );
	}

	void Instance::Launch(const String::Generic file,const uint flags)
	{
		NST_ASSERT( global.hooks.window && file.Size() && file.IsNullTerminated() );
		Window::Generic(global.hooks.window).CopyData( file, file.Size() + 1, flags, global.hooks.window );
	}
  
	void Instance::Events::Add(const Callback& callback)
	{
		NST_ASSERT( bool(callback) && !callbacks.Find( callback ) );
		callbacks.PushBack( callback );
	}

	void Instance::Events::Signal(const Event event,const void* param)
	{
		Callbacks::ConstIterator const end = callbacks.End();

		for (Callbacks::ConstIterator it=callbacks.Begin(); it != end; ++it)
			(*it)( event, param );
	}

	void Instance::Events::Remove(const void* const instance)
	{
		for (Callbacks::Iterator it(callbacks.Begin()); it != callbacks.End(); ++it)
		{
			if (it->DataPtr<void>() == instance)
			{
				callbacks.Erase( it );
				break;
			}
		}
	}

	HWND Instance::GetMainWindow()
	{
		return global.hooks.window;
	}

	uint Instance::NumChildWindows()
	{
		return global.hooks.children.Size();
	}

	HWND Instance::GetChildWindow(const uint i)
	{
		return global.hooks.children[i];
	}

	HWND Instance::GetActiveWindow()
	{
		HWND const hWnd = ::GetActiveWindow();
		return hWnd ? hWnd : global.hooks.window;
	}

	ibool Instance::IsAnyChildWindowVisible()
	{
		Global::Hooks::Children::ConstIterator const end = global.hooks.children.End();

		for (Global::Hooks::Children::ConstIterator it = global.hooks.children.Begin(); it != end; ++it)
		{
			if (::IsWindowVisible( *it ))
				return TRUE;
		}

		return FALSE;
	}

	void Instance::ShowChildWindows(uint state)
	{
		state = (state ? SW_SHOWNA : SW_HIDE);
		Global::Hooks::Children::ConstIterator const end = global.hooks.children.End();

		for (Global::Hooks::Children::ConstIterator it = global.hooks.children.Begin(); it != end; ++it)
			::ShowWindow( *it, state );
	}

	Instance::Waiter::Waiter()
	: hCursor(::SetCursor(Resource::Cursor::GetWait())) {}

	Instance::Instance(cstring const cmdLine)
	: cfg(cmdLine)
	{
		if (global.paths.path.Empty())
			throw Exception("::GetModuleFileName() failed!");

		if (global.hooks.handle == NULL)
			throw Exception("SetWindowsHookEx() failed!");

		if (static_cast<const Configuration&>(cfg)["preferences allow multiple instances"] != Configuration::YES)
		{
			::CreateMutex( NULL, TRUE, NST_APP_MUTEX_NAME );

			if (::GetLastError() == ERROR_ALREADY_EXISTS) 
			{
				const Window::Generic window( NST_APP_CLASS_NAME );

				if (window)
				{
					window.Activate( FALSE );

					if (const uint size = cfg.GetStartupFile().Size())
						window.CopyData( cfg.GetStartupFile(), size + 1 );
				}

				throw Exception::QUIT_SUCCESS;
			}
		}

		if (FAILED(::CoInitializeEx( NULL, COINIT_APARTMENTTHREADED )))
			throw Exception("::CoInitializeEx() failed!");

		Window::Struct<INITCOMMONCONTROLSEX> initCtrlEx;
		initCtrlEx.dwICC = ICC_WIN95_CLASSES;
		::InitCommonControlsEx( &initCtrlEx );
	}

	Instance::~Instance()
	{
		::CoUninitialize();
	}
}
