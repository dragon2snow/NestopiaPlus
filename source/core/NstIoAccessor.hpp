////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2007 Martin Freij
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

#ifndef NST_IO_ACCESSOR_H
#define NST_IO_ACCESSOR_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Io
		{
		#ifndef NST_NO_FASTDELEGATE

			class Accessor
			{
				class Component {};
				typedef Data (NES_IO_CALL Component::*Function)(Address);

				Component* component;
				Function function;

			public:

				Accessor() {}

				template<typename T>
				Accessor(T* c,Data (NES_IO_CALL T::*t)(Address))
				:
				component ( reinterpret_cast<Component*>(c) ),
				function  ( reinterpret_cast<Function>(t) )
				{
					NST_COMPILE_ASSERT( sizeof(function) == sizeof(t) );
				}

				template<typename T>
				void Set(T* c,Data (NES_IO_CALL T::*t)(Address))
				{
					NST_COMPILE_ASSERT( sizeof(function) == sizeof(t) );

					component = reinterpret_cast<Component*>(c);
					function  = reinterpret_cast<Function>(t);
				}

				Data Fetch(Address address) const
				{
					return (*component.*function)( address );
				}

				bool operator == (const void* ptr) const
				{
					return static_cast<const void*>(component) == ptr;
				}

				template<typename T> struct Type
				{
					typedef Data (NES_IO_CALL T::*Function)(Address);
				};
			};

			#define NES_DECL_ACCESSOR(a_) Data NES_IO_CALL Access_##a_(Address)
			#define NES_DECL_ACCESSOR_T(t_,n_,a_) template<t_ n_> NES_DECL_ACCESSOR(a_)
			#define NES_ACCESSOR(o_,a_) Data NES_IO_CALL o_::Access_##a_(Address address)
			#define NES_ACCESSOR_T(t_,n_,o_,a_) template<t_ n_> NES_ACCESSOR(o_,a_)

		#else

			class Accessor
			{
				typedef void* Component;
				typedef Data (NES_IO_CALL *Function)(Component,Address);

				Component component;
				Function function;

			public:

				Accessor() {}

				Accessor(Component c,Function t)
				:
				component ( c ),
				function  ( t )
				{}

				void Set(Component c,Function t)
				{
					component = c;
					function  = t;
				}

				Data Fetch(Address address) const
				{
					return function( component, address );
				}

				bool operator == (const void* ptr) const
				{
					return component == ptr;
				}

				template<typename T> struct Type
				{
					typedef Data (NES_IO_CALL *Function)(Component,Address);
				};
			};

			#define NES_DECL_ACCESSOR(a_)                                          \
                                                                                   \
				Data NES_IO_CALL Access_M_##a_(Address);                           \
				static Data NES_IO_CALL Access_##a_(void*,Address)

			#define NES_DECL_ACCESSOR_T(t_,n_,a_)                                  \
                                                                                   \
				template<t_ n_> Data NES_IO_CALL Access_M_##a_(Address);           \
				template<t_ n_> static Data NES_IO_CALL Access_##a_(void*,Address)

			#define NES_ACCESSOR(o_,a_)                                            \
                                                                                   \
				Data NES_IO_CALL o_::Access_##a_(void* p_,Address i_)              \
				{                                                                  \
					return static_cast<o_*>(p_)->Access_M_##a_(i_);                \
				}                                                                  \
                                                                                   \
				Data NES_IO_CALL o_::Access_M_##a_(Address address)

			#define NES_ACCESSOR_T(t_,n_,o_,a_)                                    \
                                                                                   \
				template<t_ n_>                                                    \
				Data NES_IO_CALL o_::Access_##a_(void* p_,Address i_)              \
				{                                                                  \
					return static_cast<o_*>(p_)->Access_M_##a_<n_>(i_);            \
				}                                                                  \
                                                                                   \
				template<t_ n_>                                                    \
				Data NES_IO_CALL o_::Access_M_##a_(Address address)

		#endif
		}
	}
}

#endif
