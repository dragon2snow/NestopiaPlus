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

#ifndef NST_OBJECT_DELEGATE_H
#define NST_OBJECT_DELEGATE_H

#pragma once

#include "NstMain.hpp"

namespace Nestopia
{
	namespace Object
	{
		template<typename Output=void,typename Input=void> class Delegate
		{
			class Object {};

			typedef Object* Data;
			typedef Output (Object::*Code)(Input);

			Data data;
			Code code;

		public:

			Delegate()
			: data(NULL), code(NULL) {}

			template<typename T>
			Delegate(T* d,Output (T::*c)(Input))
			: data(reinterpret_cast<Data>(d)), code(reinterpret_cast<Code>(c))
			{
				NST_COMPILE_ASSERT( sizeof(code) == sizeof(c) );
				NST_ASSERT( bool(data) == bool(code) );
			}

			Output operator()(Input input) const
			{
				return (*data.*code)( input );
			}

			template<typename T>
			void Set(T* d,Output (T::*c)(Input))
			{
				NST_COMPILE_ASSERT( sizeof(code) == sizeof(c) );
				NST_ASSERT( bool(data) == bool(code) );

				data = reinterpret_cast<Data>(d);
				code = reinterpret_cast<Code>(c);
			}

			ibool operator == (const Delegate& delegate) const
			{
				return code == delegate.code && data == delegate.data;
			}

			ibool operator != (const Delegate& delegate) const
			{
				return code != delegate.code || data != delegate.data;
			}

		private:

			class Dummy;

		public:

			operator const Dummy* () const
			{
				return data ? reinterpret_cast<const Dummy*>(this) : NULL;
			}

			void Unset()
			{
				data = NULL;
				code = NULL;
			}

			template<typename T>
			const Delegate Replace(T* d,Output (T::*c)(Input))
			{
				const Delegate tmp(*this);
				Set( d, c );
				return tmp;
			}

			template<typename T> T* DataPtr() const
			{
				typedef Output (T::*F)(Input);
				NST_COMPILE_ASSERT( sizeof(F) == sizeof(code) );
				return reinterpret_cast<T*>(data);
			}

			void* VoidPtr() const
			{
				return data;
			}

			template<typename T> Output (T::*CodePtr() const) (Input)
			{
				typedef Output (T::*F)(Input);
				NST_COMPILE_ASSERT( sizeof(F) == sizeof(code) );
				return reinterpret_cast<F>(code);
			}
		};

		template<typename Output> class Delegate<Output,void>
		{
			class Object {};

			typedef Object* Data;
			typedef Output (Object::*Code)();

			Data data;
			Code code;

		public:

			Delegate()
			: data(NULL), code(NULL) {}

			template<typename T>
			Delegate(T* d,Output (T::*c)())
			: data(reinterpret_cast<Data>(d)), code(reinterpret_cast<Code>(c))
			{
				NST_COMPILE_ASSERT( sizeof(code) == sizeof(c) );
				NST_ASSERT( bool(data) == bool(code) );
			}

			Output operator () () const
			{
				return (*data.*code)();
			}

			template<typename T>
			void Set(T* d,Output (T::*c)())
			{
				NST_COMPILE_ASSERT( sizeof(code) == sizeof(c) );
				NST_ASSERT( bool(data) == bool(code) );

				data = reinterpret_cast<Data>(d);
				code = reinterpret_cast<Code>(c);
			}

			ibool operator == (const Delegate& delegate) const
			{
				return code == delegate.code && data == delegate.data;
			}

			ibool operator != (const Delegate& delegate) const
			{
				return code != delegate.code || data != delegate.data;
			}

		private:

			class Dummy;

		public:

			operator const Dummy* () const
			{
				return data ? reinterpret_cast<const Dummy*>(this) : NULL;
			}

			void Unset()
			{
				data = NULL;
				code = NULL;
			}

			template<typename T>
			const Delegate Replace(T* d,Output (T::*c)())
			{
				const Delegate tmp(*this);
				Set( d, c );
				return tmp;
			}

			template<typename T> T* DataPtr() const
			{
				typedef Output (T::*F)();
				NST_COMPILE_ASSERT( sizeof(F) == sizeof(code) );
				return reinterpret_cast<T*>(data);
			}

			void* VoidPtr() const
			{
				return data;
			}

			template<typename T> Output (T::*CodePtr() const) ()
			{
				typedef Output (T::*F)();
				NST_COMPILE_ASSERT( sizeof(F) == sizeof(code) );
				return reinterpret_cast<F>(code);
			}
		};

		typedef Delegate<> Procedure;

		template<typename Output,typename Param1,typename Param2> class Delegate2
		{
			class Object {};

			typedef Object* Data;
			typedef Output (Object::*Code)(Param1,Param2);

			Data data;
			Code code;

		public:

			Delegate2()
			: data(NULL), code(NULL) {}

			template<typename T>
			Delegate2(T* d,Output (T::*c)(Param1,Param2))
			: data(reinterpret_cast<Data>(d)), code(reinterpret_cast<Code>(c))
			{
				NST_COMPILE_ASSERT( sizeof(code) == sizeof(c) );
				NST_ASSERT( bool(data) == bool(code) );
			}

			Output operator () (Param1 param1,Param2 param2) const
			{
				return (*data.*code)( param1, param2 );
			}

			template<typename T>
			void Set(T* d,Output (T::*c)(Param1,Param2))
			{
				NST_COMPILE_ASSERT( sizeof(code) == sizeof(c) );
				NST_ASSERT( bool(data) == bool(code) );

				data = reinterpret_cast<Data>(d);
				code = reinterpret_cast<Code>(c);
			}

			ibool operator == (const Delegate2& delegate) const
			{
				return code == delegate.code && data == delegate.data;
			}

			ibool operator != (const Delegate2& delegate) const
			{
				return code != delegate.code || data != delegate.data;
			}

		private:

			class Dummy;

		public:

			operator const Dummy* () const
			{
				return data ? reinterpret_cast<const Dummy*>(this) : NULL;
			}

			void Unset()
			{
				data = NULL;
				code = NULL;
			}

			template<typename T>
			const Delegate2 Replace(T* d,Output (T::*c)(Param1,Param2))
			{
				const Delegate2 tmp(*this);
				Set( d, c );
				return tmp;
			}

			template<typename T> T* DataPtr() const
			{
				typedef Output (T::*F)(Param1,Param2);
				NST_COMPILE_ASSERT( sizeof(F) == sizeof(code) );
				return reinterpret_cast<T*>(data);
			}

			void* VoidPtr() const
			{
				return data;
			}

			template<typename T> Output (T::*CodePtr() const) (Param1,Param2)
			{
				typedef Output (T::*F)(Param1,Param2);
				NST_COMPILE_ASSERT( sizeof(F) == sizeof(code) );
				return reinterpret_cast<F>(code);
			}
		};
	}
}

#endif
