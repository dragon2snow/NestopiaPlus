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

#ifndef NST_SYSTEM_THREAD_H
#define NST_SYSTEM_THREAD_H

#pragma once

#include "NstWindowCustom.hpp"

namespace Nestopia
{
	namespace Application
	{
		class Exception;
	}

	namespace System
	{
		class Thread
		{
			enum Command
			{
				NONE,
				SUSPEND,
				TERMINATE
			};

		public:

			Thread();
			~Thread();

			void Suspend();
			void Resume();
			void Destroy();

			enum Startup
			{
				DONT_START,
				START
			};

			class Interrupt
			{
				friend class Thread;

			public:

				void Acknowledge() const;

			private:

				typedef Object::Delegate<void,Interrupt> Callback;

				inline explicit Interrupt(Thread&);

				inline uint Execute(Callback) const;

				uint ExitSuccess() const;
				uint ExitFailure(const Application::Exception&) const;

				const Thread& thread;

			public:

				ibool None() const
				{
					return thread.command == NONE;
				}

				ibool Demanding() const
				{
					return thread.command != NONE;
				}

				ibool DemandingSuspension() const
				{
					return thread.command == SUSPEND;
				}

				ibool DemandingTermination() const
				{
					return thread.command == TERMINATE;
				}
			};

		private:

			typedef Interrupt::Callback Callback;

			void Create(const Callback&,Window::Custom&,Startup,uint);
			void Unhook();

			class Event
			{
				HANDLE const handle;

			public:

				Event();
				inline ~Event();
				inline void Wait() const;
				inline void Release() const;
				inline ibool IsSignaled() const;
			};

			struct Initializer;

			enum OnMsgType
			{
				ON_EXIT_SUCCESS,
				ON_EXIT_EXCEPTION,
				ON_EXIT_MESSAGE
			};

			enum
			{
				WM_NST_THREAD_EXIT = WM_APP + 54
			};

			ibool OnExit(Window::Param&);

			static void Close(HANDLE&);
			static uint NST_STDCALL Starter(void*);

			volatile Command command;
			Event synchronizer;
			Event suspender;
			HANDLE handle;
			uint id;
			Window::Custom* window;

		public:

			template<typename Data,typename Code> void Create
			(
				Data* data,
				Code code,
				Window::Custom& window,
				const Startup startup,
				uint stack=0
			)
			{
				Create( Callback(data,code), window, startup, stack );
			}

			ibool IsIdle() const
			{
				return handle == NULL || command != NONE;
			}

			HANDLE GetHandle() const
			{
				return handle;
			}
		};
	}
}

#endif
