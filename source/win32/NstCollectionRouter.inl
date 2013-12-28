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

template<typename Output,typename Input,typename Key> template<typename Data,typename Code>
Router<Output,Input,Key>::Router(KeyParam key,Data* const data,Code code)
: hooks(NULL)
{
	Add( key, Callback(data,code) );
}

template<typename Output,typename Input,typename Key> template<typename Data,uint COUNT>
Router<Output,Input,Key>::Router(Data* const data,const Entry<Data>(&list)[COUNT])
: hooks(NULL)
{
	Add( data, list, COUNT );
}

template<typename Output,typename Input,typename Key>
Router<Output,Input,Key>::~Router()
{
	for (Hook* it = hooks; it; )
	{
		Hook* const him = it;
		it = it->next;
		delete him;
	}
}

template<typename Output,typename Input,typename Key>
Output Router<Output,Input,Key>::Hook::Invoke(Input input)
{
	for (uint i=0; i < items.Size(); ++i)
		items[i]( input );
				 
	return main ? main( input ) : Output(0);
}

template<typename Output,typename Input,typename Key>
void Router<Output,Input,Key>::Add(KeyParam key,const Callback& callback)
{
	ibool found;
	Callback& item = items( key, found );

	if (found)
	{
		NST_ASSERT
		( 
			hooks && 
			item.CodePtr<Hook>() == &Hook::Invoke &&
			!item.DataPtr<Hook>()->main
		);

		item.DataPtr<Hook>()->main = callback;
	}
	else
	{
		item = callback;
	}
}

template<typename Output,typename Input,typename Key> template<typename Data>
void Router<Output,Input,Key>::Add(Data* const data,const Entry<Data>* list,const uint count)
{
	items.Reserve( items.Size() + count );

	for (const Entry<Data>* const end = list + count; list != end; ++list)
		Add( list->key, Callback(data,list->function) );
}

template<typename Output,typename Input,typename Key> template<typename Data>
void Router<Output,Input,Key>::HookRouter::Add(Data* const data,const HookEntry<Data>* list,const uint count)
{
	for (const HookEntry<Data>* const end = list + count; list != end; ++list)
		router.AddHook( list->key, Callback(data,list->function) );
}

template<typename Output,typename Input,typename Key>
void Router<Output,Input,Key>::Set(KeyParam key,const Callback& callback)
{
	if (Item* const item = items.Find( key ))
	{
		if (item->value.CodePtr<Hook>() == &Hook::Invoke)
			item->value.DataPtr<Hook>()->main = callback;
		else
			item->value = callback;
	}
	else
	{
		Add( key, callback );
	}
}

template<typename Output,typename Input,typename Key> template<typename Data>
void Router<Output,Input,Key>::Set(Data* const data,const Entry<Data>* list,const uint count)
{
	for (const Entry<Data>* const end = list + count; list != end; ++list)
		Set( list->key, Callback(data,list->function) );
}

template<typename Output,typename Input,typename Key>
void Router<Output,Input,Key>::Remove(KeyParam key,const Callback& callback)
{
	if (Item* const item = items.Find( key ))
	{
		if (item->value == callback)
		{
			items.Array().Erase( item );
		}
		else if 
		(
	     	item->value.CodePtr<Hook>() == &Hook::Invoke &&
			item->value.DataPtr<Hook>()->main == callback
		)
		{
			item->value.DataPtr<Hook>()->main.Reset();
		}
	}
}

template<typename Output,typename Input,typename Key>
void Router<Output,Input,Key>::Remove(const void* const data)
{
	for (uint i=0; i < items.Size(); )
	{
		const Callback& callback = items[i].value;

		if (callback.DataPtr<void>() == data)
		{
			items.Array().Erase( items.At(i) );
		}
		else
		{
			if 
			(
     			callback.CodePtr<Hook>() == &Hook::Invoke &&
     			callback.DataPtr<Hook>()->main.DataPtr<void>() == data
    		)
     			callback.DataPtr<Hook>()->main.Reset();
		
			++i;
		}
	}
}

template<typename Output,typename Input,typename Key>
void Router<Output,Input,Key>::RemoveAll(const void* const data)
{
	RemoveHooks( data );
	Remove( data );
}

template<typename Output,typename Input,typename Key>
void Router<Output,Input,Key>::AddHook(KeyParam key,const Hook::Item& newItem)
{
	Hook* hook;

	ibool found;
	Callback& callback = items( key, found );

	if (found && callback.CodePtr<Hook>() == &Hook::Invoke)
	{
		hook = callback.DataPtr<Hook>();
		NST_ASSERT( hook && hook->items.Size() );
	}
	else
	{
		NST_ASSERT( !found || callback );

		hook = new Hook;

		if (Hook* it = hooks)
		{
			while (it->next)
				it = it->next;

			it->next = hook;
		}
		else
		{
			hooks = hook;
		}

		if (found)
			hook->main = callback;
			
		callback.Set( hook, &Hook::Invoke );
	}
	
	hook->items.PushBack( newItem );
}

template<typename Output,typename Input,typename Key>
ibool Router<Output,Input,Key>::RemoveHook(Item* const mainItem,Hook* const hook,Hook::Item* const hookItem)
{
	NST_ASSERT( mainItem );

	hook->items.Erase( hookItem );

	if (hook->items.Size())
		return FALSE;

	if (hook->main)
		mainItem->value = hook->main;
	else
		items.Array().Erase( mainItem );

	if (hooks == hook)
	{
		hooks = hook->next;
	}
	else
	{
		Hook* it = hooks;

		while (it->next != hook)
			it = it->next;

		it->next = hook->next;
	}

	delete hook;
	return TRUE;
}

template<typename Output,typename Input,typename Key>
void Router<Output,Input,Key>::RemoveHook(KeyParam key,const Hook::Item& item)
{
	if (Item* const mainItem = items.Find( key ))
	{
		NST_ASSERT( mainItem->value.CodePtr<Hook>() == &Hook::Invoke );

		Hook* const hook = mainItem->value.DataPtr<Hook>();
		NST_ASSERT( hook );

		if (Hook::Item* const hookItem = hook->items.Find( item ))
			RemoveHook( mainItem, hook, hookItem );
	}
}

template<typename Output,typename Input,typename Key>
void Router<Output,Input,Key>::RemoveHooks(const void* const data)
{
	for (Item* mainItem = items.Begin(); mainItem != items.End(); ++mainItem)
	{
		if (mainItem->value.CodePtr<Hook>() == &Hook::Invoke)
		{
			Hook* const hook = mainItem->value.DataPtr<Hook>();
			NST_ASSERT( hook );

			for (uint i=0; i < hook->items.Size(); )
			{
				Hook::Item* const hookItem = hook->items.At(i);

				if (hookItem->DataPtr<void>() == data)
				{
					if (RemoveHook( mainItem, hook, hookItem ))
						break;
				}
				else
				{
					++i;
				}
			}
		}
	}
}

template<typename Output,typename Input,typename Key> template<typename Match>
typename const Router<Output,Input,Key>::Callback* Router<Output,Input,Key>::Find
(
    const Match& match,
	const Key** key
)   const
{
	if (const Item* const item = items.Find( match ))
	{
		if (key)
			*key = &item->key;

		if (item->value.CodePtr<Hook>() == &Hook::Invoke)
			return &item->value.DataPtr<Hook>()->main;
		else
			return &item->value;
	}

	return NULL;
}

template<typename Output,typename Input,typename Key>
typename Router<Output,Input,Key>::Callback& Router<Output,Input,Key>::operator [] (KeyParam key)
{
	Callback& callback = items.Locate( key );

	if (callback.CodePtr<Hook>() == &Hook::Invoke)
		return callback.DataPtr<Hook>()->main;

	return callback;
}