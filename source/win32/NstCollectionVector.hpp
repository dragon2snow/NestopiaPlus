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

#ifndef NST_COLLECTION_VECTOR_H
#define NST_COLLECTION_VECTOR_H

#pragma once

#include "NstMain.hpp"

namespace Nestopia
{
	namespace Collection
	{
		namespace Private
		{
			class Base
			{
				void Allocate(uint);
				void Reallocate(uint);

			protected:

				union
				{
					void* data;
					char* bytes;
				};

				uint capacity;
				uint size;

				explicit Base(uint);
				explicit Base(const Base&);
				Base(const void* NST_RESTRICT,uint);

				void operator = (const Base&);

				void Assign(const void* NST_RESTRICT,uint);
				void Append(const void* NST_RESTRICT,uint);
				void Insert(void*,const void* NST_RESTRICT,uint);
				void Erase(void*,void*) throw();

				void Import(void*,uint) throw();
				void* Export() throw();

				void Reserve(uint);
				void Resize(uint);
				void Grow(uint);

				ibool Valid(const void*) const throw();
				ibool InBound(const void*) const throw();

				Base()
				: data(NULL), capacity(0), size(0) {}

				~Base()
				{
					NST_ASSERT
					( 
						capacity >= size &&
						bool(data) >= bool(size) && 
						bool(data) >= bool(capacity)
					);

					if (data)
						operator delete (data);
				}

				void Shrink(uint inSize)
				{
					NST_ASSERT( size >= inSize );
					size -= inSize;
				}

			public:

				void Destroy() throw();
				void Defrag();
				void Import(Base&) throw ();

				ibool Empty() const
				{
					return !size;
				}

				void Clear()
				{
					size = 0;
				}
			};
		}

		template<typename T> class Vector : public Private::Base
		{
			typedef Private::Base Base;

		public:

			typedef T Item;
			typedef T Type;
			typedef T* Iterator;
			typedef const T* ConstIterator;

			enum
			{
				ITEM_SIZE = sizeof(Item)
			};

			Vector() {}

			Vector(const Item* items,uint count)
			: Base(items,ITEM_SIZE * count) {}

			explicit Vector(uint count)
			: Base(count * ITEM_SIZE) {}

			explicit Vector(const Vector& vector)
			: Base(vector) {}

			Vector& operator = (const Vector& vector)
			{
				Base::operator = (vector);
				return *this;
			}

			template<typename U>
			Vector& operator << (const U& t)
			{
				NST_COMPILE_ASSERT( ITEM_SIZE == sizeof(char) );
				Base::Append( &t, sizeof(t) );
				return *this;
			}

			void PushBack(const Item& item)
			{
				Base::Append( &item, ITEM_SIZE );
			}

			void PushBack(const Vector& vector)
			{
				Base::Append( vector.data, vector.size );
			}

			void Assign(ConstIterator items,uint count)
			{
				Base::Assign( items, ITEM_SIZE * count );
			}

			void Append(ConstIterator items,uint count)
			{
				Base::Append( items, ITEM_SIZE * count );
			}

			void Insert(Iterator pos,ConstIterator items,uint count)
			{
				Base::Insert( pos, items, ITEM_SIZE * count );
			}

			void Insert(Iterator pos,const Item& item)
			{
				Base::Insert( pos, &item, ITEM_SIZE );
			}

			void Erase(Iterator begin,Iterator end)
			{
				Base::Erase( begin, end );
			}

			void Erase(Iterator offset,uint count=1)
			{
				Base::Erase( offset, offset + count );
			}

			Item& operator [] (uint i)
			{
				return static_cast<Item*>(data)[i];
			}

			const Item& operator [] (uint i) const
			{
				return static_cast<Item*>(data)[i];
			}

			Item* Ptr()
			{
				return static_cast<Item*>(data);
			}

			const Item* Ptr() const
			{
				return static_cast<const Item*>(data);
			}

			void Import(Item* input,uint count)
			{
				Base::Import( input, ITEM_SIZE * count );
			}

			void Import(Vector& vector)
			{
				Base::Import( vector );
			}

			Item* Export()
			{
				return static_cast<Item*>( Base::Export() );
			}

			Iterator Begin()
			{
				return static_cast<Iterator>(data);
			}

			ConstIterator Begin() const
			{
				return static_cast<ConstIterator>(data);
			}

			Iterator End()
			{
				return reinterpret_cast<Iterator>(bytes + size);
			}

			ConstIterator End() const
			{
				return reinterpret_cast<ConstIterator>(bytes + size);
			}

			Iterator At(uint pos)
			{
				return static_cast<Iterator>(data) + pos;
			}

			ConstIterator At(uint pos) const
			{
				return static_cast<ConstIterator>(data) + pos;
			}

			Item& Front()
			{
				NST_ASSERT( size );
				return *static_cast<Item*>(data);
			}

			const Item& Front() const
			{
				NST_ASSERT( size );
				return *static_cast<const Item*>(data);
			}

			Item& Back()
			{
				NST_ASSERT( size );
				return *(reinterpret_cast<Item*>(bytes + size) - 1);
			}

			const Item& Back() const
			{
				NST_ASSERT( size );
				return *(reinterpret_cast<const Item*>(bytes + size) - 1);
			}

			uint Size() const
			{
				NST_ASSERT( size % ITEM_SIZE == 0 );
				return size / ITEM_SIZE;
			}

			uint Length() const
			{
				return Size();
			}

			uint Capacity() const
			{
				NST_ASSERT( capacity % ITEM_SIZE == 0 );
				return capacity / ITEM_SIZE;
			}

			void Reserve(uint count)
			{
				Base::Reserve( count * ITEM_SIZE );
			}

			void Resize(uint count)
			{
				Base::Resize( count * ITEM_SIZE );
			}

			void ShrinkTo(uint count)
			{
				size = count * ITEM_SIZE;
				NST_ASSERT( capacity >= size );
			}

			void Grow(uint count=1)
			{
				Base::Grow( count * ITEM_SIZE );
			}

			void Shrink(uint count=1)
			{
				Base::Shrink( count * ITEM_SIZE );
			}

			ibool InBound(ConstIterator it) const
			{
				return Base::InBound( it );
			}

			ibool Valid(ConstIterator it) const
			{
				return Base::Valid( it );
			}

			template<typename Value>
			ConstIterator Find(const Value&) const;

			template<typename Value>
			Iterator Find(const Value& value)
			{
				ConstIterator const it = static_cast<const Vector*>(this)->Find( value );
				return reinterpret_cast<Item*>(bytes + (reinterpret_cast<const char*>(it) - bytes));
			}
		};

		template<typename T> template<typename Value>
		typename Vector<T>::ConstIterator Vector<T>::Find(const Value& value) const
		{
			ConstIterator const end = End();

			for (ConstIterator it = Ptr(); it != end; ++it)
				if (*it == value)
					return it;

			return NULL;
		}
	}
}

#endif
