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

#ifndef NST_HOOK_H
#define NST_HOOK_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
	#ifndef NST_FPTR_MEM_MAP

		class Hook
		{
			class Component {};
			typedef void (Component::*Function)();

			Component* component;
			Function function;

		public:

			template<typename T>
			Hook(T* c,void (T::*t)())
			:
			component ( reinterpret_cast<Component*>(c) ),
			function  ( reinterpret_cast<Function>(t) )
			{
				NST_COMPILE_ASSERT( sizeof(function) == sizeof(t) );
			}

			void Execute() const
			{
				(*component.*function)();
			}

			bool operator == (const Hook& h) const
			{
				return function == h.function && component == h.component;
			}
		};

		#define NES_DECL_HOOK(a_) void Hook_##a_();
		#define NES_HOOK(o_,a_) void o_::Hook_##a_()
		#define NES_CALL_HOOK(o_,a_) o_::Hook_##a_()

	#else

		class Hook
		{
			typedef void* Component;
			typedef void (*Function)(Component);

			Component component;
			Function function;

		public:

			Hook(Component c,Function t)
			:
			component ( c ),
			function  ( t )
			{}

			void Execute() const
			{
				function( component );
			}

			bool operator == (const Hook& h) const
			{
				return function == h.function && component == h.component;
			}
		};

		#define NES_DECL_HOOK(a_)                                    \
                                                                     \
			void Hook_Member_##a_();                                 \
                                                                     \
			template<typename T>                                     \
			static void Hook_Type_##a_(void* instance,void (T::*)()) \
			{                                                        \
				static_cast<T*>(instance)->Hook_Member_##a_();       \
			}                                                        \
                                                                     \
			static void Hook_##a_(void* instance)                    \
			{                                                        \
				Hook_Type_##a_( instance, Hook_Member_##a_ );        \
			}

		#define NES_HOOK(o_,a_) void o_::Hook_Member_##a_()
		#define NES_CALL_HOOK(o_,a_) o_::Hook_Member_##a_()

	#endif
	}
}

#endif
