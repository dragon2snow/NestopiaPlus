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

template<typename T> template<typename Key>
const typename Set<T>::Item* Set<T>::FindItem(Key key) const
{
	const uint pos = LowerBound<Key>( key );
	return pos != array.Size() && array[pos] == key ? array.At(pos) : NULL;
}

template<typename T> template<typename Key>
typename Set<T>::Item& Set<T>::LocateItem(Key key)
{
	const uint pos = LowerBound<Key>( key );
	NST_ASSERT( pos != array.Size() && array[pos] == key );
	return array[pos];
}

template<typename T> template<typename Key>
typename Set<T>::Item& Set<T>::GetItem(Key key,ibool& found)
{
	const uint pos = LowerBound<Key>( key );
	found = pos != array.Size() && array[pos] == key;

	if (!found)
	{
		array.Insert( array.At(pos), NULL, 1 );
		new (static_cast<void*>(array.At(pos))) Item( key );
	}

	return array[pos];
}

template<typename T> template<typename Key>
typename Set<T>::Item& Set<T>::GetItem(Key key)
{
	ibool found;
	return GetItem<Key>( key, found );
}
