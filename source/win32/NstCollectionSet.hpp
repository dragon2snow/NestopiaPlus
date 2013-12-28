////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
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

#ifndef NST_COLLECTION_SET_H
#define NST_COLLECTION_SET_H

#pragma once

#include <new>
#include "NstCollectionVector.hpp"

namespace Nestopia
{
	namespace Collection
	{
		template<typename T> class Set
		{
			Vector<T> array;

		public:

			typedef typename Vector<T>::Item Item;
			typedef typename Vector<T>::Iterator Iterator;
			typedef typename Vector<T>::ConstIterator ConstIterator;

			enum
			{
				ITEM_SIZE = Vector<T>::ITEM_SIZE
			};

		private:

			typedef typename ConstParam<Item>::Type ItemParam;

			template<typename Key>
			NST_FORCE_INLINE uint LowerBound(Key key) const
			{
				uint left = 0;
				uint right = array.Size();

				while (left < right) 
				{
					const uint middle = (left + right) >> 1;

					if (array[middle] < key)
						left = middle + 1;
					else
						right = middle;
				}

				return left;
			}

			template<typename Key>
			Item& GetItem(Key);

			template<typename Key>
			Item& GetItem(Key,ibool&);

			template<typename Key>
			const Item* FindItem(Key) const;

			template<typename Key>
			Item& LocateItem(Key);

		public:

			template<typename Key>
			Item& operator () (const Key& key)
			{
				return GetItem<typename ConstParam<Key>::Type>( key );
			}

			template<typename Key>
			Item& operator () (const Key& key,ibool& found)
			{
				return GetItem<typename ConstParam<Key>::Type>( key, found );
			}

			template<typename Key>
			Iterator Insert(const Key& key)
			{
				ibool found;
				Item& item = (*this)(key,found);
				return !found ? &item : NULL;
			}

			template<typename Key>
			ConstIterator Find(const Key& key) const
			{
				return FindItem<typename ConstParam<Key>::Type>( key );
			}
  
			template<typename Key>
			Iterator Find(const Key& key)
			{
       			return const_cast<Iterator>(FindItem<typename ConstParam<Key>::Type>( key ));
			}

			template<typename Key>
			Item& Locate(const Key& key)
			{
				return LocateItem<typename ConstParam<Key>::Type>( key );
			}

			Vector<T>& Array()
			{
				return array;
			}

			const Vector<T>& Array() const
			{
				return array;
			}

			Item* Ptr()
			{
				return array.Ptr();
			}

			const Item* Ptr() const
			{
				return array.Ptr();
			}

			Item& operator [] (uint i)
			{
				return array[i];
			}

			const Item& operator [] (uint i) const
			{
				return array[i];
			}

			Iterator Begin()
			{
				return array.Begin();
			}

			ConstIterator Begin() const
			{
				return array.Begin();
			}

			Iterator End()
			{
				return array.End();
			}

			ConstIterator End() const
			{
				return array.End();
			}

			Iterator At(uint pos)
			{
				return array.At( pos );
			}

			ConstIterator At(uint pos) const
			{
				return array.At( pos );
			}

			Item& Front()
			{
				return array.Front();
			}

			const Item& Front() const
			{
				return array.Front();
			}

			Item& Back()
			{
				return array.Back();
			}

			const Item& Back() const
			{
				return array.Back();
			}

			uint Size() const
			{
				return array.Size();
			}

			uint Capacity() const
			{
				return array.Capacity();
			}

			ibool Empty() const
			{
				return array.Empty();
			}

			void Reserve(uint length)
			{
				array.Reserve( length );
			}

			void Defrag()
			{
				array.Defrag();
			}

			void Clear()
			{
				array.Clear();
			}

			void Destroy()
			{
				array.Destroy();
			}
		};

        #include "NstCollectionSet.inl"
	}
}

#endif
