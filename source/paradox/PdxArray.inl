//////////////////////////////////////////////////////////////////////////////////////////////
//
// Paradox Library - general purpose C++ utilities
//
// Copyright (C) 2003 Martin Freij
//
// This file is part of Paradox Library.
// 
// Paradox Library is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// Paradox Library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Paradox Library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PDXARRAY_H
#error Do not include PdxArray.inl directly!
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE> 
inline PDXARRAY<VALUE,0>::PDXARRAY()
: array(NULL), size(0), capacity(0) {}

//////////////////////////////////////////////////////////////////////////////////////////////
// Raise the capacity integer to be big enough to hold n elements
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE> 
inline VOID PDXARRAY<VALUE,0>::RaiseCapacityLevel(const TSIZE length)
{
	if (capacity < CACHE)
		capacity = CACHE;
	
	while (capacity < length)
		capacity += capacity;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Make sure memory is big enough to hold n elements
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE> 
inline VOID PDXARRAY<VALUE,0>::Reserve(const TSIZE length)
{
	if (length > capacity)
		ReserveSpace(length);
}
