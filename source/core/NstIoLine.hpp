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

#ifndef NST_IO_LINE_H
#define NST_IO_LINE_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Io
		{
        #ifndef NST_FPTR_MEM_MAP

			class Line
			{
				class Generic {};

				typedef Generic* Component;
				typedef void (NES_IO_CALL Generic::*Toggler)(Cycle);

				Component component;
				Toggler toggler;

			public:

				Line() {}

				template<typename T> 
				Line(T* c,void (NES_IO_CALL T::*t)(Cycle))
				: 
				component ( reinterpret_cast<Component>(c) ), 
				toggler   ( reinterpret_cast<Toggler>(t)   )
				{ 
					NST_COMPILE_ASSERT( sizeof(toggler) == sizeof(t) );
				}

				template<typename T> 
				void Set(T* c,void (NES_IO_CALL T::*t)(Cycle))
				{ 
					NST_COMPILE_ASSERT( sizeof(toggler) == sizeof(t) );

					component = reinterpret_cast<Component>(c);
					toggler   = reinterpret_cast<Toggler>(t);
				}

				void Invalidate()
				{
					component = NULL;
					toggler = NULL;
				}

				bool InUse() const
				{
					return component != NULL;
				}

				void Toggle(Cycle cycle) const
				{
					(*component.*toggler)( cycle );
				}

				bool SameComponent(const void* ptr) const
				{
					return static_cast<const void*>(component) == ptr;
				}
			};

            #define NES_DECL_LINE(a_) void NES_IO_CALL Line_##a_(Cycle);
            #define NES_LINE(o_,a_) void NES_IO_CALL o_::Line_##a_(Cycle cycle)

        #else

			class Line
			{
				typedef void* Component;
				typedef void (NES_IO_CALL *Toggler)(Component,Cycle);

				Component component;
				Toggler toggler;

			public:

				Line() {}

				Line(Component c,Toggler t)
				: 
				component ( c ), 
				toggler   ( t )
				{}

				void Set(Component c,Toggler t)
				{ 
					component = c;
					toggler   = t;
				}

				void Invalidate()
				{
					component = NULL;
					toggler = NULL;
				}

				bool InUse() const
				{
					return component != NULL;
				}

				void Toggle(Cycle cycle) const
				{
					toggler( component, cycle );
				}

				bool SameComponent(const void* ptr) const
				{
					return component == ptr;
				}
			};

            #define NES_DECL_LINE(a_)													 		 	  \
																						 		 	  \
				void NES_IO_CALL Line_Member_##a_(Cycle); 								 		 	  \
																						 		 	  \
				template<typename T>													 		 	  \
				static void Line_Type_##a_(void* instance,Cycle cycle,void (NES_IO_CALL T::*)(Cycle)) \
				{																		  	  	  	  \
					static_cast<T*>(instance)->Line_Member_##a_( cycle );                             \
				}																		  	  	  	  \
																						  	  	  	  \
				static void NES_IO_CALL Line_##a_(void* instance,Cycle cycle) 			  	  	  	  \
				{																		  	  	  	  \
					Line_Type_##a_( instance, cycle, Line_Member_##a_ );            	  	  	  	  \
				}

            #define NES_LINE(o_,a_) void NES_IO_CALL o_::Line_Member_##a_(Cycle cycle)

        #endif
		}
	}
}

#endif
