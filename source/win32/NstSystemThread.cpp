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

#include <process.h>
#include "resource/resource.h"
#include "NstApplicationException.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowUser.hpp"
#include "NstSystemThread.hpp"

namespace Nestopia
{
	using System::Thread;

	struct Thread::Initializer
	{
		Thread& thread;
		const Callback callback;

		Initializer(Thread& t,const Callback& c)
		: thread(t), callback(c) {}
	};

	Thread::Event::Event()
	: handle(::CreateEvent(NULL,FALSE,FALSE,NULL))
	{
		if (handle == NULL)
			throw Application::Exception("::CreateEvent() failed!");
	}

	inline Thread::Event::~Event()
	{
		::CloseHandle( handle );
	}

	inline void Thread::Event::Wait() const
	{
		::WaitForSingleObject( handle, INFINITE );
	}

	inline ibool Thread::Event::IsSignaled() const
	{
		return ::WaitForSingleObject( handle, 0 ) == WAIT_OBJECT_0;
	}

	inline void Thread::Event::Release() const
	{
		::SetEvent( handle );
	}

	Thread::Thread()
	: command(NONE), handle(NULL), window(NULL) {}

	Thread::~Thread()
	{
		Destroy();
	}

	void Thread::Create(const Callback& callback,Window::Custom& w,const Startup startup,uint stack)
	{
		NST_VERIFY( window == NULL );

		if (window)
			return;

		window = &w;
		window->Messages().Set( WM_NST_THREAD_EXIT, this, &Thread::OnExit );
		command = SUSPEND;

		Initializer initializer( *this, callback );
		handle = reinterpret_cast<HANDLE>(::_beginthreadex( NULL, stack, Starter, &initializer, 0, &id ));

		if (handle == NULL)
			throw Application::Exception("::_beginthreadex() failed!");

		synchronizer.Wait();

		if (startup == START)
			Resume();
	}

	void Thread::Close(HANDLE& handle)
	{
		for (uint i=10; i; --i)
		{
			DWORD exitCode;

			if (!::GetExitCodeThread( handle, &exitCode ))
			{
				NST_DEBUG_MSG("GetExitCodeThread() failed!");
				break;
			}
			else if (exitCode != STILL_ACTIVE)
			{
				NST_VERIFY( exitCode == EXIT_SUCCESS || exitCode == EXIT_FAILURE );
				break;
			}
			else
			{
				::Sleep( 100 );
			}
		}

		NST_ASSERT( handle );

		::CloseHandle( handle );		
		handle = NULL;
	}

	void Thread::Unhook()
	{
		if (window) 
		{
			window->Messages().Remove( this );
			window = NULL;
		}
	}

	void Thread::Destroy()
	{
		if (handle)
		{
			const ibool suspended = (command == SUSPEND);
			command = TERMINATE;

			if (suspended)
				suspender.Release();

			::WaitForSingleObject( handle, INFINITE );
			Close( handle );
		}

		Unhook();
	}

	void Thread::Suspend()
	{
		NST_VERIFY( id != ::GetCurrentThreadId() && !synchronizer.IsSignaled() );

		if (handle && command == NONE)
		{
			command = SUSPEND;
			synchronizer.Wait();
		}
	}

	void Thread::Resume()
	{
		NST_VERIFY( id != ::GetCurrentThreadId() && !synchronizer.IsSignaled() );

		if (command == SUSPEND)
		{
			NST_VERIFY( handle );

			command = NONE;
			suspender.Release();
		}
	}

	ibool Thread::OnExit(Window::Param& param)
	{
		// A NULL handle means that the master thread has
		// already destroyed this thread

		if (handle)
			Close( handle );

		Unhook();

		switch (LOWORD(param.wParam))
		{
			case ON_EXIT_EXCEPTION:
		
				throw Application::Exception
				( 
					reinterpret_cast<cstring>(param.lParam), 
					static_cast<Application::Exception::Type>(HIWORD(param.wParam))
				);
		
			case ON_EXIT_MESSAGE:
		
				Window::User::Issue
				(
					static_cast<Window::User::Type>(HIWORD(param.wParam)), 
					LOWORD(param.lParam), 
					HIWORD(param.lParam)
				);
		}

		return TRUE;
	}

	inline Thread::Interrupt::Interrupt(Thread& input)
	: thread(input) {}

	uint Thread::Interrupt::ExitSuccess() const
	{
		thread.window->Post
		( 
			WM_NST_THREAD_EXIT, 
			ON_EXIT_SUCCESS, 
			0 
		);	

		return EXIT_SUCCESS;
	}

	uint Thread::Interrupt::ExitFailure(const Application::Exception& exception) const
	{
		thread.window->Post
		(
			WM_NST_THREAD_EXIT, 
			MAKELONG(ON_EXIT_EXCEPTION,exception.GetType()), 
			exception.GetMessageText() 
		);

		return EXIT_FAILURE;
	}

	inline uint Thread::Interrupt::Execute(const Callback execute) const
	{
		try
		{
			execute( *this );
		}
		catch (const Application::Exception& exception)
		{
			return ExitFailure( exception );
		}
		catch (...)
		{
			return ExitFailure( Application::Exception(IDS_ERR_GENERIC) );
		}

		return ExitSuccess();
	}

	void Thread::Interrupt::Acknowledge() const
	{
		NST_ASSERT( thread.id == ::GetCurrentThreadId() );
		NST_VERIFY( thread.handle && thread.command != NONE );

		if (thread.command == SUSPEND)
		{
			thread.synchronizer.Release();
			thread.suspender.Wait();
		}
	}

	uint NST_STDCALL Thread::Starter(void* data)
	{
		return Interrupt( static_cast<Initializer*>(data)->thread ).Execute
		( 
			static_cast<const Initializer*>(data)->callback 
		);
	}
}
