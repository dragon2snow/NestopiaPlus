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

#ifdef PDX_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#ifndef PDXARRAY_H
#define PDXARRAY_H

#include "PDXMemory.h"
#include "PDXUtilities.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// Static array class
//////////////////////////////////////////////////////////////////////////////////////////////

template<class T,TSIZE LENGTH=0> class PDXARRAY
{
public:

	typedef T                      VALUE;
	typedef PDXARRAY<VALUE,LENGTH> CONTAINER;
	typedef PDXARRAY<VALUE,LENGTH> ARRAY;
	typedef VALUE&                 REFERENCE;
	typedef const VALUE&           CONSTREFERENCE;
	typedef VALUE*                 ITERATOR;
	typedef const VALUE*           CONSTITERATOR;

	enum {SIZE=LENGTH};
	enum {POD=PDX_IS_POD(VALUE)};
	
	PDX_DEFAULT_CONSTRUCTOR(PDXARRAY)

	explicit PDXARRAY(const ARRAY& src)
	{ PDX::ITERATOR::CopyConstructSequence<POD>(array,src.array,src.array+LENGTH); }

	PDXARRAY(const VALUE& value)
	{ PDX::ITERATOR::CopyConstruct<POD>(array,array+LENGTH,value); }

	template<class I,class J> 
	PDXARRAY(I& begin,const J& end)
	{
		PDX_ASSERT(end - begin <= LENGTH);
		PDX::ITERATOR::CopyConstructSequence<POD>(array,begin,end); 
	}

	template<class I,class J> VOID Insert  (ITERATOR const,I,const J);
	template<class I,class J> VOID Replace (ITERATOR const,I,const J);

	VOID Erase(ITERATOR const,CONSTITERATOR const);
	
	inline VOID Erase(ITERATOR const pos)
	{ Erase(pos,pos+1); }							
	
	inline VOID Insert(ITERATOR const pos,const VALUE& value)
	{ Insert(pos,&value,&value+1); }

	BOOL operator == (const ARRAY&) const;

	inline VOID Fill(ITERATOR const begin,CONSTITERATOR const end,const VALUE& value)
	{ 
		PDX_ASSERT(InBound(begin,end)); 
		PDX::ITERATOR::Fill<POD>(begin,end,value); 
	}

	inline VOID operator = (const ARRAY& in)
	{ PDX::ITERATOR::CopySequence<POD>(array,in.array,in.array+LENGTH); }

	inline TSIZE Size() const
	{ return LENGTH; }

	inline VOID Fill(const VALUE& value)
	{ PDX::ITERATOR::Fill<POD>(array,array+LENGTH,value); }

	inline ITERATOR Begin()                                { return array; }
	inline CONSTITERATOR Begin() const                     { return array; }

	inline REFERENCE Front()                               { return *array; }
	inline CONSTREFERENCE Front() const                    { return *array; }

	inline REFERENCE Back()                                { return array[LENGTH-1]; }
	inline CONSTREFERENCE Back() const                     { return array[LENGTH-1]; }

	inline ITERATOR End()                                  { return array+LENGTH; }
	inline CONSTITERATOR End() const                       { return array+LENGTH; }

	inline ITERATOR At(const TSIZE i)                      { PDX_ASSERT(i<=LENGTH); return array+i; }
	inline CONSTITERATOR At(const TSIZE i) const           { PDX_ASSERT(i<=LENGTH); return array+i; }

	inline REFERENCE operator[] (const TSIZE i)            { PDX_ASSERT(i<LENGTH); return array[i]; }
	inline CONSTREFERENCE operator[] (const TSIZE i) const { PDX_ASSERT(i<LENGTH); return array[i]; }

	inline BOOL InBound(CONSTITERATOR const begin,CONSTITERATOR const end) const
	{ return begin <= end && begin >= array && end <= &array[LENGTH]; }

private:

	VALUE array[LENGTH];
};

//////////////////////////////////////////////////////////////////////////////////////////////
// Insert at any position, elements above the position+length will be lost
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE,TSIZE LENGTH> template<class I,class J> 
VOID PDXARRAY<VALUE,LENGTH>::Insert(ITERATOR const pos,I begin,const J end)
{
	const TSIZE length = begin - end;
	PDX_ASSERT(length <= LENGTH);

	PDX::ITERATOR::Destruct<POD>(array+(LENGTH-length),array+LENGTH);
	PDX::ITERATOR::MemMove(pos+length,pos,array+(LENGTH-length));
	PDX::ITERATOR::CopyConstructSequence<POD>(pos,begin,end);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Erase a range of elements 
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE,TSIZE LENGTH> 
VOID PDXARRAY<VALUE,LENGTH>::Erase(ITERATOR const begin,CONSTITERATOR const end)
{
	PDX_ASSERT(InBound(begin,end));

	PDX::ITERATOR::Destruct<POD>(begin,end);
	PDX::ITERATOR::MemMove(begin,end,array+LENGTH);
	PDX::ITERATOR::Construct<POD>(end,array+LENGTH);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Replace a range of elements
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE,TSIZE LENGTH> template<class I,class J> 
VOID PDXARRAY<VALUE,LENGTH>::Replace(ITERATOR const pos,I begin,const J end)
{
	const TSIZE length = begin - end;
	PDX_ASSERT(length <= LENGTH);
	
	PDX::ITERATOR::Destruct<POD>(pos,pos+length);
   	PDX::ITERATOR::CopyConstructSequence<POD>(pos,begin,end);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Compare containers
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE,TSIZE LENGTH> 
BOOL PDXARRAY<VALUE,LENGTH>::operator == (const ARRAY& container) const
{
	for (TSIZE i=0; i < LENGTH; ++i)
		if (!(array[i] == container.array[i]))
			return FALSE;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Dynamic array class
//////////////////////////////////////////////////////////////////////////////////////////////

template<class T> class PDXARRAY<T,0>
{
public:

	typedef T                 VALUE;
	typedef PDXARRAY<VALUE,0> CONTAINER;
	typedef PDXARRAY<VALUE,0> ARRAY;
	typedef VALUE&            REFERENCE;
	typedef const VALUE&      CONSTREFERENCE;
	typedef VALUE*            ITERATOR;
	typedef const VALUE*      CONSTITERATOR;

	enum {POD=PDX_IS_POD(VALUE)};

	PDXARRAY();

	PDX_NO_INLINE explicit PDXARRAY(const ARRAY&);
	PDX_NO_INLINE PDXARRAY(const TSIZE);
	
	template<class I,class J> PDX_NO_INLINE PDXARRAY(I,const J);

    PDX_NO_INLINE ~PDXARRAY();

    template<class I,class J> PDX_NO_INLINE VOID Insert     (ITERATOR,I,const J);
	template<class I,class J> PDX_NO_INLINE VOID InsertBack (I,const J);
	template<class I,class J> PDX_NO_INLINE VOID Replace    (ITERATOR const,I,const J);	

	PDX_NO_INLINE VOID Erase(ITERATOR const,CONSTITERATOR const);
	PDX_NO_INLINE VOID Defrag();
	PDX_NO_INLINE VOID Destroy();
	
	VOID Reserve(const TSIZE);	
	VOID Clear();
	VOID Resize(const TSIZE);
	VOID Grow(const TSIZE=1);
	VOID InsertBack(const VALUE&);
	
	inline VOID operator += (const VALUE& v)
	{ InsertBack(v); }

	VOID operator = (const ARRAY&);
	VOID operator += (const ARRAY&);
	BOOL operator == (const ARRAY&) const;
	
	inline VOID Insert(ITERATOR const pos,const VALUE& value)
	{ Insert(pos,&value,&value+1); }

	inline VOID InsertBack()
	{ Grow(); }
	
	inline VOID Erase(ITERATOR const pos)
	{ Erase(pos,pos+1); }

	inline VOID EraseFront()
	{ PDX_ASSERT(size); Erase(array,array+1); }

	VOID EraseBack()
	{ 
		PDX_ASSERT(size); 
		PDX::ITERATOR::Destruct<POD>(array[--size]);
	}
	
	VOID Fill(ITERATOR const first,CONSTITERATOR const last,const VALUE& value)
	{ PDX_ASSERT(InBound(first,last)); PDX::ITERATOR::Fill<POD>(first,last,value); }

	inline VOID Fill(const VALUE& value)
	{ Fill(array,array+size,value); }

	inline TSIZE Size() const
	{ return size; }

	inline BOOL IsEmpty() const
	{ return !size; }

	inline TSIZE Capacity() const
	{ return capacity; }

	inline TSIZE BufferSize() const
	{ return sizeof(VALUE) * size; }

	inline ITERATOR Begin()                                { return array; }
	inline CONSTITERATOR Begin() const                     { return array; }
												     								      
	inline REFERENCE Front()                               { PDX_ASSERT(size); return *array; }
	inline CONSTREFERENCE Front() const                    { PDX_ASSERT(size); return *array; }
												      
	inline REFERENCE Back()			                       { PDX_ASSERT(size); return array[size-1]; }
	inline CONSTREFERENCE Back() const                     { PDX_ASSERT(size); return array[size-1]; }

	inline ITERATOR End()                                  { return array+size; }
	inline CONSTITERATOR End() const                       { return array+size; }

	inline ITERATOR At(const TSIZE i)			           { PDX_ASSERT(i<=size); return array+i; }
	inline CONSTITERATOR At(const TSIZE i) const           { PDX_ASSERT(i<=size); return array+i; }

	inline REFERENCE operator[] (const TSIZE i)	  	       { PDX_ASSERT(i<size); return array[i]; }
	inline CONSTREFERENCE operator[] (const TSIZE i) const { PDX_ASSERT(i<size); return array[i]; }

	VOID Hook(const VOID* const,const VOID* const);
	VOID Hook(const VOID* const);
	VOID Hook(ARRAY&);
	VOID UnHook();

	inline BOOL InBound(CONSTITERATOR a,CONSTITERATOR b) const
	{ return PDX::ITERATOR::InBound(a,b,array,&array[size]); }

private:

    #if defined(__INTEL_COMPILER)
    #undef  PDX_ALIGNED_MALLOC
    #define PDX_ALIGNED_MALLOC
    #else
    #ifdef _MSC_VER
    #if _MSC_VER >= 1200
    #undef  PDX_ALIGNED_MALLOC
    #define PDX_ALIGNED_MALLOC
    #endif
    #endif
    #endif

    #ifdef PDX_MEMORY_DEBUG
    
     #define PDX_VOID_ALLOCATE_(n) PDX::Allocate(sizeof(VALUE) * (n))
     #define PDX_FREE_(a)          PDX::Free((VOID*)a)
    
    #elif defined(PDX_ALIGNED_MALLOC)
    
     #define PDX_VOID_ALLOCATE_(n) ((sizeof(VALUE) >= 16) ? _aligned_malloc(sizeof(VALUE) * (n),16) : malloc(sizeof(VALUE) * (n)))
     #define PDX_FREE_(a)          if (sizeof(VALUE) >= 16) _aligned_free((VOID*)a); else free((VOID*)a)
    
    #else
    
     #define PDX_VOID_ALLOCATE_(n) (VOID*) new CHAR[sizeof(VALUE) * (n)]
     #define PDX_FREE_(a)		   delete [] ((CHAR*)a)
    
    #endif

    #define PDX_ALLOCATE_(n) (VALUE*) PDX_VOID_ALLOCATE_(n)

    #define PDX_REALLOCATE_(a,m,n)               \
	{							    	         \
		VOID* const tmp = PDX_VOID_ALLOCATE_(m); \
		memcpy(tmp,a,sizeof(VALUE) * n);         \
		PDX_FREE_(a);		          		     \
		a = PDX_CAST(VALUE*,tmp);          	     \
	} PDX_FORCE_SEMICOLON

	PDX_NO_INLINE VOID ReserveSpace(const TSIZE);
	PDX_NO_INLINE VOID ReserveSpaceDynamic(const TSIZE);
	PDX_NO_INLINE VOID ReserveSpaceDiscard(const TSIZE);
	
	VOID RaiseCapacityLevel(const TSIZE);

	enum {CACHE=1};

	VALUE* array;
	TSIZE size;
	TSIZE capacity;	
};

#ifdef PDX_ALIGNED_MALLOC
#undef PDX_ALIGNED_MALLOC
#include <malloc.h>
#endif

#include "PDXArray.inl"

//////////////////////////////////////////////////////////////////////////////////////////////
// Copy Constructor
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE> 
PDXARRAY<VALUE,0>::PDXARRAY(const ARRAY& container)
: 
array    (PDX_ALLOCATE_(container.size)),
size     (container.size),
capacity (container.size)
{
	PDX::ITERATOR::CopyConstructSequence<POD>(array,container.array,container.array+container.size);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Constructor, resize the container and initialize <length> elements
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE> 
PDXARRAY<VALUE,0>::PDXARRAY(const TSIZE length)
: 
array    (PDX_ALLOCATE_(length)),
size     (length),
capacity (length)
{
	PDX::ITERATOR::Construct<POD>(array,array+size);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Copy Constructor, insert a range of elements
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE> template<class I,class J> 
PDXARRAY<VALUE,0>::PDXARRAY(I begin,const J end)
{
	const TSIZE length = end - begin;
	array = PDX_ALLOCATE_(length);
	size = capacity = length;
	PDX::ITERATOR::CopyConstructSequence<POD>(array,begin,end);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Destructor, call the destructor for each element and free the memory
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE> 
PDXARRAY<VALUE,0>::~PDXARRAY()
{
	if (array)
	{
		PDX::ITERATOR::Destruct<POD>(array,array+size);
		PDX_FREE_(array);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Send it to wonderland
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE> 
VOID PDXARRAY<VALUE,0>::Destroy()
{
	if (array)
	{
		PDX::ITERATOR::Destruct<POD>(array,array+size);
		PDX_FREE_(array);
		array = NULL;
		size = capacity = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Insert a new element into the back of the container
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE> 
VOID PDXARRAY<VALUE,0>::InsertBack(const VALUE& value)
{
	const TSIZE length = size + 1;

	if (length > capacity)
		ReserveSpaceDynamic(length);

	PDX::ITERATOR::CopyConstruct<POD>(array[size],value);
	size = length;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Increase the size of the container
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE> 
VOID PDXARRAY<VALUE,0>::Grow(const TSIZE num)
{
	PDX_ASSERT(num);
	
	const TSIZE length = size + num;
	
	if (length > capacity)
		ReserveSpaceDynamic(length);
	
	PDX::ITERATOR::Construct<POD>(array+size,array+length);
	size = length;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Erase a range of elements
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE> 
VOID PDXARRAY<VALUE,0>::Erase(ITERATOR const begin,CONSTITERATOR const end)
{
	PDX_ASSERT(InBound(begin,end));
	PDX::ITERATOR::Destruct<POD>(begin,end);
	PDX::ITERATOR::MemMove(begin,end,array+size);
	size -= end - begin;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Resize the container
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE> 
VOID PDXARRAY<VALUE,0>::Resize(const TSIZE num)
{
	Reserve(num);

	if (num > size)      PDX::ITERATOR::Construct<POD>(array+size,array+num);
	else if (num < size) PDX::ITERATOR::Destruct<POD>(array+num,array+size);

	size = num;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Replace one sequence with another
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE> template<class I,class J> 
VOID PDXARRAY<VALUE,0>::Replace(ITERATOR const pos,I begin,const J end)
{
	PDX_ASSERT(InBound(pos,pos));
	PDX::ITERATOR::Destruct<POD>(pos,pos+(end-begin));
   	PDX::ITERATOR::CopyConstructSequence<POD>(pos,begin,end);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Compare containers
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE> 
BOOL PDXARRAY<VALUE,0>::operator == (const ARRAY& container) const
{
	if (size != container.size)
		return FALSE;
	
	for (TSIZE i=0; i < size; ++i)
		if (!(array[i] == container.array[i]))
			return FALSE;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Free unused space
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE> 
VOID PDXARRAY<VALUE,0>::Defrag()
{
	if (size)
	{
		if (size < capacity)
		{
       		capacity = size;
			PDX_REALLOCATE_(array,size,size);
		}
	}
	else 
	{
		if (array)
		{
			PDX_FREE_(array);
     		array = NULL;
     		size = capacity = 0;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Insert a range of elements into the back of the container
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE> template<class I,class J> 
VOID PDXARRAY<VALUE,0>::InsertBack(I begin,const J end)
{
	const TSIZE length = size + (end - begin);
	
	if (length > capacity)
		ReserveSpaceDynamic(length);
	
	PDX::ITERATOR::CopyConstructSequence<POD>(array+size,begin,end);
	size = length;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Insert an element into any position in the container
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE> template<class I,class J> 
VOID PDXARRAY<VALUE,0>::Insert(ITERATOR pos,I begin,const J end)
{
	PDX_ASSERT(InBound(pos,pos));

	if (pos == array+size)
	{
		InsertBack(begin,end);
	}
	else 
	{
		const TSIZE length = end - begin;
		const TSIZE newsize = size + length;
	    
		if (newsize > capacity || InBound(begin,end))
		{
     		RaiseCapacityLevel(newsize);
			VALUE* const tmp = PDX_ALLOCATE_(capacity);
			PDX::ITERATOR::MemCopy(tmp,array,pos);
			PDX::ITERATOR::MemCopy(tmp+(length+(pos-array)),pos,array+size);
			pos = tmp+(pos-array);
			PDX_FREE_(array);
	    	array = tmp;
		}
     	else
		{
			PDX::ITERATOR::MemMove(pos+length,pos,array+size);
		}

		PDX::ITERATOR::CopyConstructSequence<POD>(pos,begin,end); 
        size = newsize;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Copy container
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE> 
VOID PDXARRAY<VALUE,0>::operator = (const ARRAY& container)
{
	PDX_ASSERT(this != &container);

	PDX::ITERATOR::Destruct<POD>(array,array+size);
	size = container.size;

	if (capacity < size)
		ReserveSpaceDiscard(size);

	PDX::ITERATOR::CopyConstructSequence<POD>(array,container.array,container.array+container.size);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Insert a container into the back of this one
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE> 
VOID PDXARRAY<VALUE,0>::operator += (const ARRAY& container)
{
	PDX_ASSERT(this != &container);

	const TSIZE length = size + container.size;

	if (length > capacity)
		ReserveSpaceDynamic(length);

	PDX::ITERATOR::CopyConstructSequence<POD>(array+size,container.array,container.array+container.size);
	size = length;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Empty container and call the destructor for each element but keep the memory for future use
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE> 
VOID PDXARRAY<VALUE,0>::Clear()
{
	if (size)
	{
		PDX::ITERATOR::Destruct<POD>(array,array+size);
		size = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Reserve memory big enough to hold <length> elements
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE> 
VOID PDXARRAY<VALUE,0>::ReserveSpace(const TSIZE length)
{
	PDX_ASSERT(length > capacity);

	capacity = length;

	if (size) 
	{
		PDX_REALLOCATE_(array,capacity,size);
	}
	else 
	{
		if (array) PDX_FREE_(array);
		array = PDX_ALLOCATE_(capacity);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Reserve memory big enough to hold <length> elements, do a logarithmic increase of the size 
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE> 
VOID PDXARRAY<VALUE,0>::ReserveSpaceDynamic(const TSIZE length)
{
	PDX_ASSERT(length > capacity);

	RaiseCapacityLevel(length);

	if (size) 
	{
		PDX_REALLOCATE_(array,capacity,size);
	}
	else 
	{
		if (array) PDX_FREE_(array);
		array = PDX_ALLOCATE_(capacity);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Reserve memory big enough to hold <length> elements, always free the previous allocated block
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE> 
VOID PDXARRAY<VALUE,0>::ReserveSpaceDiscard(const TSIZE length)
{
	PDX_ASSERT(length > capacity);
	capacity = length;
	if (array) PDX_FREE_(array);
	array = PDX_ALLOCATE_(capacity);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Slip in a memory chunk and cast it, may be used in conjunction with UnHook()
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE>
inline VOID PDXARRAY<VALUE,0>::Hook(const VOID* const begin,const VOID* const end)
{
	PDX_ASSERT(array == NULL);
	array = PDX_CAST(VALUE*,((VOID*)begin));
	const TSIZE length = PDX_CAST(const CHAR*,end) - PDX_CAST(const CHAR*,begin);
	size = capacity = length / sizeof(VALUE);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Slip in a memory chunk and cast it, may be used in conjunction with UnHook()
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE>
inline VOID PDXARRAY<VALUE,0>::Hook(ARRAY& container)
{
	PDX_ASSERT(array == NULL);
	array = container.array;
	size = container.size;
	capacity = container.capacity;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Slip in a memory chunk and cast it, may be used in conjunction with UnHook()
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE>
inline VOID PDXARRAY<VALUE,0>::Hook(const VOID* const begin)
{
	PDX_ASSERT(array == NULL);
	array = PDX_CAST(VALUE*,((VOID*)begin));
	size = capacity = TSIZE_MAX;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Clear the array without freeing any memory, may be used in conjunction with Hook()
//////////////////////////////////////////////////////////////////////////////////////////////

template<class VALUE>
inline VOID PDXARRAY<VALUE,0>::UnHook()
{
	array = NULL;
	size = capacity = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// No need to keep these alive
//////////////////////////////////////////////////////////////////////////////////////////////

#undef PDX_ALLOCATE_
#undef PDX_FREE_
#undef PDX_REALLOCATE_
#undef PDX_VOID_ALLOCATE_

#endif
