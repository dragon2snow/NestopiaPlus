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

#ifndef NST_COLLECTION_MAP_H
#define NST_COLLECTION_MAP_H

#pragma once

#include "NstCollectionSet.hpp"

namespace Nestopia
{
	namespace Collection
	{
		template<typename Key,typename Value,bool B>
		struct MapEntry
		{
			const Key key;
			Value value;

			template<typename T>
			MapEntry(const T& t)
			: key(t) {}

			template<typename T>
			ibool operator == (const T& t) const
			{
				return key == t;
			}

			template<typename T>
			ibool operator < (const T& t) const
			{
				return key < t;
			}
		};

		template<typename Key,typename Value>
		struct MapEntry<Key,Value,true>
		{
			const Key key;
			Value value;

			template<typename T>
			MapEntry(const T& t)
			: key(t), value(t) {}

			template<typename T>
			ibool operator == (const T& t) const
			{
				return key == t;
			}

			template<typename T>
			ibool operator < (const T& t) const
			{
				return key < t;
			}
		};

		template<typename K,typename V,bool B=false> class Map
		{
		public:

			typedef K Key;
			typedef V Value;

			enum
			{
				VALUE_SIZE = sizeof(Value)
			};

			typedef MapEntry<K,V,B> Entry;

		private:

			Set<Entry> set;

		public:

			typedef typename Set<Entry>::Iterator Iterator;
			typedef typename Set<Entry>::ConstIterator ConstIterator;

			template<typename T>
			Iterator Insert(const T& key)
			{
				return set.Insert( key );
			}
  
			template<typename T>
			Value& operator () (const T& key)
			{
				return set( key ).value;
			}

			template<typename T>
			Value& operator () (const T& key,ibool& found)
			{
				return set( key, found ).value;
			}

			template<typename T>
			Iterator Find(const T& key)
			{
				return set.Find( key );
			}

			template<typename T>
			ConstIterator Find(const T& key) const
			{
				return set.Find( key );
			}

			template<typename T>
			Value& Locate(const T& key)
			{
				return set.Locate( key ).value;
			}

			Vector<Entry>& Array()
			{
				return set.Array();
			}

			const Vector<Entry>& Array() const
			{
				return set.Array();
			}

			Entry* Ptr()
			{
				return set.Ptr();
			}

			const Entry* Ptr() const
			{
				return set.Ptr();
			}

			Entry& operator [] (uint i)
			{
				return set[i];
			}

			const Entry& operator [] (uint i) const
			{
				return set[i];
			}

			Iterator Begin()
			{
				return set.Begin();
			}

			ConstIterator Begin() const
			{
				return set.Begin();
			}

			Iterator End()
			{
				return set.End();
			}

			ConstIterator End() const
			{
				return set.End();
			}

			Iterator At(uint pos)
			{
				return set.At( pos );
			}

			ConstIterator At(uint pos) const
			{
				return set.At( pos );
			}

			Value& Front()
			{
				return set.Front().value;
			}

			const Value& Front() const
			{
				return set.Front().value;
			}

			Value& Back()
			{
				return set.Back().value;
			}

			const Value& Back() const
			{
				return set.Back().value;
			}

			uint Size() const
			{
				return set.Size();
			}

			uint Capacity() const
			{
				return set.Capacity();
			}

			ibool Empty() const
			{
				return set.Empty();
			}

			void Reserve(uint length)
			{
				set.Reserve( length );
			}

			void Defrag()
			{
				set.Defrag();
			}

			void Clear()
			{
				set.Clear();
			}

			void Destroy()
			{
				set.Destroy();
			}
		};
	}
}

#endif
