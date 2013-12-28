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

#ifndef NST_COLLECTION_ROUTER_H
#define NST_COLLECTION_ROUTER_H

#pragma once

#include "NstCollectionMap.hpp"
#include "NstObjectDelegate.hpp"

namespace Nestopia
{
	namespace Collection
	{
		template<typename Output,typename Input,typename Key=uint>
		class Router
		{
			typedef typename ConstParam<Key>::Type KeyParam;

		public:

			typedef Object::Delegate<Output,Input> Callback;

			template<typename T> struct Entry
			{
				Key key;
				Output (T::*function)(Input);
			};

			template<typename T> struct HookEntry
			{
				Key key;
				void (T::*function)(Input);
			};

			template<typename Data,typename Code>
			Router(KeyParam,Data*,Code);

			template<typename Data,typename Array>
			Router(Data*,const Array&);

			~Router();

			NST_NO_INLINE void Remove(const void*);
			void RemoveAll(const void*);
			void Defrag();

			template<typename Match>
			const Callback* Find(const Match&,const Key** = NULL) const;

			Callback& operator [] (KeyParam);

		private:

			NST_NO_INLINE void Add(KeyParam,const Callback&);
			NST_NO_INLINE void Set(KeyParam,const Callback&);
			NST_NO_INLINE void Remove(KeyParam,const Callback&);

			template<typename Data>
			void Add(Data*,const Entry<Data>*,uint);

			template<typename Data>
			void Set(Data*,const Entry<Data>*,uint);

			typedef Collection::Map<Key,Callback> Items;

		public:

			typedef typename Items::Entry Item;

		private:

			struct Hook
			{
				typedef Object::Delegate<void,Input> Item;
				typedef Vector<Item> Items;

				Output Invoke(Input);

				Items items;
				Callback main;
				Hook* next;

				Hook()
				: next(NULL) {}
			};

			NST_NO_INLINE void AddHook(KeyParam,const typename Hook::Item&);
			NST_NO_INLINE bool RemoveHook(Item* const,Hook*,typename Hook::Item*);
			NST_NO_INLINE void RemoveHook(KeyParam,const typename Hook::Item&);
			NST_NO_INLINE void RemoveHooks(const void*);

			class HookRouter
			{
				Router& router;

				template<typename Data>
				void Add(Data*,const HookEntry<Data>*,uint);

			public:

				HookRouter(Router& ref)
				: router(ref) {}

				typedef typename Hook::Item Callback;

				template<typename Data,typename Code>
				void Add(KeyParam key,Data* data,Code code)
				{
					router.AddHook( key, Callback(data,code) );
				}

				template<typename Data,typename Hooks>
				void Add(Data* data,const Hooks& hooks)
				{
					Add( data, hooks, sizeof(array(hooks)) );
				}

				void Remove(const void* data)
				{
					router.RemoveHooks( data );
				}
			};

			Items items;
			Hook* hooks;

		public:

			Router()
			: hooks(NULL) {}

			const Item* operator () (KeyParam key) const
			{
				return items.Find( key );
			}

			uint Size() const
			{
				return items.Size();
			}

			HookRouter Hooks()
			{
				return *this;
			}

			template<typename Data,typename Code>
			void Add(KeyParam key,Data* data,Code code)
			{
				Add( key, Callback(data,code) );
			}

			template<typename Data,typename Array>
			void Add(Data* data,const Array& arr)
			{
				Add( data, arr, sizeof(array(arr)) );
			}

			template<typename Data,typename Array,typename HookArray>
			void Add(Data* data,const Array& arr,const HookArray& hookArray)
			{
				Add( data, arr, sizeof(array(arr)) );
				Hooks().Add( data, hookArray );
			}

			template<typename Data,typename Code>
			void Set(KeyParam key,Data* data,Code code)
			{
				Set( key, Callback(data,code) );
			}

			template<typename Data,typename Array>
			void Set(Data* data,const Array& arr)
			{
				Set( data, arr, sizeof(array(arr)) );
			}

			template<typename Data,typename Code>
			void Remove(KeyParam key,Data* data,Code code)
			{
				Remove( key, Callback(data,code) );
			}
		};

		#include "NstCollectionRouter.inl"
	}
}

#endif
