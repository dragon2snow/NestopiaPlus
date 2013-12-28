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

#ifdef _MSC_VER
#pragma comment(lib,"comctl32")
#endif

#include "NstResourceCursor.hpp"
#include "NstResourceVersion.hpp"
#include "NstApplicationException.hpp"
#include "NstApplicationLanguage.hpp"
#include <CommCtrl.h>
#include <ObjBase.h>

#define NST_MENU_CLASS_NAME   _T("#32768") // name for menus as documented on MSDN
#define NST_APP_CLASS_NAME    _T("Nestopia")
#define NST_STATUS_CLASS_NAME STATUSCLASSNAME
#define NST_IME_CLASS_NAME    _T("IME")
#define NST_APP_MUTEX_NAME    _T("Nestopia Mutex")

namespace Nestopia
{
	namespace Application
	{
		struct Instance::Global
		{
			Global();

			struct Paths
			{
				Paths(HINSTANCE);

				Path path;
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

			const Paths paths;
			const Resource::Version version;
			Language language;
			Hooks hooks;
			IconStyle iconStyle;
		};

		Instance::Global Instance::global;
		Instance::Events::Callbacks Instance::Events::callbacks;

		Instance::Global::Paths::Paths(HINSTANCE const hInstance)
		{
			uint length;

			do
			{
				path.Reserve( path.Capacity() + 255 );
				length = ::GetModuleFileName( hInstance, path.Ptr(), path.Capacity() );
			}
			while (length == path.Capacity());

			path.ShrinkTo( length );
			path.MakePretty();
			path.Defrag();
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
						MAX_LENGTH = NST_MAX( NST_COUNT(NST_APP_CLASS_NAME) - 1,
                                     NST_MAX( NST_COUNT(NST_STATUS_CLASS_NAME) - 1, NST_COUNT(NST_MENU_CLASS_NAME) - 1 ))
					};

					String::Stack<MAX_LENGTH,tchar> name;
					name.ShrinkTo( ::GetClassName( hWnd, name.Ptr(), MAX_LENGTH+1 ) );

					if (name.Equal( NST_MENU_CLASS_NAME ) || name.Equal( NST_STATUS_CLASS_NAME ) || name.Equal( NST_IME_CLASS_NAME ))
						return;

					if (window)
					{
						children.PushBack( hWnd );
						Events::Signal( EVENT_SYSTEM_BUSY );
					}
					else if (name.Equal( NST_APP_CLASS_NAME ))
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
		paths     ( ::GetModuleHandle(NULL) ),
		version   ( paths.path.Ptr(), Resource::Version::PRODUCT ),
		hooks     ( ::GetModuleHandle(NULL) ),
		iconStyle ( ICONSTYLE_NES )
		{
		}

		Instance::Language& Instance::GetLanguage()
		{
			return global.language;
		}

		const String::Heap<char>& Instance::GetVersion()
		{
			return global.version;
		}

		const Path& Instance::GetExePath()
		{
			return global.paths.path;
		}

		const Path Instance::GetExePath(const GenericString file)
		{
			return Path( global.paths.path.Directory(), file );
		}

		const Path Instance::GetLongPath(tstring const shortPath)
		{
			Path longPath;

			longPath.Reserve( 255 );
			uint length = ::GetLongPathName( shortPath, longPath.Ptr(), longPath.Capacity() + 1 );

			if (length > longPath.Capacity() + 1)
			{
				longPath.Reserve( length - 1 );
				length = ::GetLongPathName( shortPath, longPath.Ptr(), longPath.Capacity() + 1 );
			}

			if (!length)
				return shortPath;

			const tchar slashed = GenericString(shortPath).Back();

			longPath.ShrinkTo( length );
			longPath.MakePretty( slashed == '\\' || slashed == '/' );

			return longPath;
		}

		const Path Instance::GetTmpPath(GenericString file)
		{
			Path path;
			path.Reserve( 255 );

			if (uint length = ::GetTempPath( path.Capacity() + 1, path.Ptr() ))
			{
				if (length > path.Capacity() + 1)
				{
					path.Reserve( length - 1 );
					length = ::GetTempPath( path.Capacity() + 1, path.Ptr() );
				}

				if (length)
					path = GetLongPath( path.Ptr() );
			}

			if (path.Length())
				path.MakePretty( true );
			else
				path << ".\\";

			if (file.Empty())
				file = _T("nestopia.tmp");

			path << file;

			return path;
		}

		void Instance::Events::Add(const Callback& callback)
		{
			NST_ASSERT( bool(callback) && !callbacks.Find( callback ) );
			callbacks.PushBack( callback );
		}

		void Instance::Events::Signal(const Event event,const void* param)
		{
			for (Callbacks::ConstIterator it=callbacks.Begin(), end=callbacks.End(); it != end; ++it)
				(*it)( event, param );
		}

		void Instance::Events::Remove(const void* const instance)
		{
			for (Callbacks::Iterator it(callbacks.Begin()); it != callbacks.End(); ++it)
			{
				if (it->VoidPtr() == instance)
				{
					callbacks.Erase( it );
					break;
				}
			}
		}

		Window::Generic Instance::GetMainWindow()
		{
			return global.hooks.window;
		}

		uint Instance::NumChildWindows()
		{
			return global.hooks.children.Size();
		}

		Window::Generic Instance::GetChildWindow(uint i)
		{
			return global.hooks.children[i];
		}

		Window::Generic Instance::GetActiveWindow()
		{
			HWND hWnd = ::GetActiveWindow();
			return hWnd ? hWnd : global.hooks.window;
		}

		Instance::IconStyle Instance::GetIconStyle()
		{
			return global.iconStyle;
		}

		void Instance::SetIconStyle(IconStyle style)
		{
			global.iconStyle = style;
		}

		void Instance::ShowChildWindows(uint state)
		{
			state = (state ? SW_SHOWNA : SW_HIDE);

			for (Global::Hooks::Children::ConstIterator it=global.hooks.children.Begin(), end=global.hooks.children.End(); it != end; ++it)
				::ShowWindow( *it, state );
		}

		Instance::Waiter::Waiter()
		: hCursor(::SetCursor(Resource::Cursor::GetWait())) {}

		Instance::Locker::Locker()
		: hWnd(GetMainWindow()), enabled(::IsWindowEnabled(hWnd))
		{
			::EnableWindow( hWnd, false );
			::LockWindowUpdate( hWnd );
		}

		ibool Instance::Locker::CheckInput(int vKey) const
		{
			if (::GetAsyncKeyState( vKey ) & 0x8000)
			{
				while (::GetAsyncKeyState( vKey ) & 0x8000)
					::Sleep( 10 );

				return true;
			}

			return false;
		}

		Instance::Locker::~Locker()
		{
			::LockWindowUpdate( NULL );
			::EnableWindow( hWnd, enabled );
			for (MSG msg;::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE|PM_QS_INPUT ););
		}

		Instance::Instance()
		{
			if (global.paths.path.Empty())
				throw Exception(_T("::GetModuleFileName() failed!"));

			if (global.hooks.handle == NULL)
				throw Exception(_T("SetWindowsHookEx() failed!"));

			if (static_cast<const Configuration&>(cfg)["preferences allow multiple instances"] != Configuration::YES)
			{
				::CreateMutex( NULL, true, NST_APP_MUTEX_NAME );

				if (::GetLastError() == ERROR_ALREADY_EXISTS)
				{
					if (Window::Generic window = Window::Generic::Find( NST_APP_CLASS_NAME ))
					{
						window.Activate();

						if (const uint length = cfg.GetStartupFile().Length())
						{
							COPYDATASTRUCT cds;

							cds.dwData = COPYDATA_OPENFILE_ID;
							cds.cbData = (length + 1) * sizeof(tchar);
							cds.lpData = const_cast<tchar*>(cfg.GetStartupFile().Ptr());

							window.Send( WM_COPYDATA, 0, &cds );
						}
					}

					throw Exception();
				}
			}

			global.language.Load( cfg );

			if (FAILED(::CoInitializeEx( NULL, COINIT_APARTMENTTHREADED )))
				throw Exception(_T("::CoInitializeEx() failed!"));

			Object::Pod<INITCOMMONCONTROLSEX> initCtrlEx;

			initCtrlEx.dwSize = sizeof(initCtrlEx);
			initCtrlEx.dwICC = ICC_WIN95_CLASSES;
			::InitCommonControlsEx( &initCtrlEx );
		}

		Instance::~Instance()
		{
			::CoUninitialize();
		}

		void Instance::Save()
		{
			global.language.Save( cfg );
		}
	}
}
