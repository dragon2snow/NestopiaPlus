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

#ifndef PDXALLOCATOR_H
#define PDXALLOCATOR_H

#include <cstdlib>
#include "PdxMemory.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// Low level allocator class
//////////////////////////////////////////////////////////////////////////////////////////////

template<TSIZE ALIGNMENT=0> class PDXALLOCATOR
{
public:

 #ifdef PDX_X86

	inline PDXALLOCATOR()
	: base(NULL) {}

	~PDXALLOCATOR();

	PDX_NO_INLINE VOID  Defrag();
	PDX_NO_INLINE VOID* Allocate(TSIZE);
	PDX_NO_INLINE VOID  Free(VOID* const);
	PDX_NO_INLINE VOID  Reserve(TSIZE);

	VOID Abandon(const VOID* const);

 #else

	inline VOID* Allocate(const TSIZE size)
	{ 
		PDX_ASSERT(size);
		return (VOID*) new CHAR[size];
	}

	inline VOID Free(VOID* const p)
	{ delete [] ((CHAR*)p); }

	inline VOID Free(const VOID* const p)
	{ delete [] ((CHAR*)p); }

	inline VOID Abandon(const VOID* const) {}
	inline VOID Reserve(const TSIZE) {}
	inline VOID Defrag() {}

 #endif

	template<class T> T* New() 
	{ return PDX_CONSTRUCT(*PDX_CAST(T*,Allocate(sizeof(T)))) T; }

	template<class T,class A> T* New(const A& a) 
	{ return PDX_CONSTRUCT(*PDX_CAST(T*,Allocate(sizeof(T)))) T(a); }

	template<class T,class A,class B> T* New(const A& a,const B& b) 
	{ return PDX_CONSTRUCT(*PDX_CAST(T*,Allocate(sizeof(T)))) T(a,b); }

	template<class T,class A,class B,class C> T* New(const A& a,const B& b,const C& c) 
	{ return PDX_CONSTRUCT(*PDX_CAST(T*,Allocate(sizeof(T)))) T(a,b,c); }

	template<class T,class A,class B,class C,class D> T* New(const A& a,const B& b,const C& c,const D& d) 
	{ return PDX_CONSTRUCT(*PDX_CAST(T*,Allocate(sizeof(T)))) T(a,b,c,d); }

	template<class T> VOID Delete(T* const p)
	{
		PDX_COMPILE_ASSERT(sizeof(T));

		if (p)
		{
			p->~T();
			Free((VOID*)p);
		}
	}

 #ifdef PDX_X86

private:

	enum
	{
		OBJECT_SIZE = (sizeof(TSIZE) * 2),
		BASE_SIZE = (sizeof(TSIZE) * 4) + ALIGNMENT
	};

	TSIZE** base;

 #endif
};

#ifdef PDX_X86

#define PDX_ALIGN_ADDRESS_UP(address) PDX_CAST(CHAR*,(PDX_CAST(TSIZE,address) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

#ifdef _DEBUG
#define PDX_DEAD_END ~TSIZE(0)
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////////////////////

template<TSIZE ALIGNMENT>
PDXALLOCATOR<ALIGNMENT>::~PDXALLOCATOR()
{ 
	if (base) 
	{
		PDX_ASSERT(*base && !base[0][0]);

		if (base[0][1])
			free(*base);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Release unused memory
//////////////////////////////////////////////////////////////////////////////////////////////

template<TSIZE ALIGNMENT>
VOID PDXALLOCATOR<ALIGNMENT>::Defrag()
{
	PDX_ASSERT(!base || (base && *base));

	if (base && !base[0][0] && base[0][1])
	{
		free(*base);
		base = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Leave the pointer without freeing it
//////////////////////////////////////////////////////////////////////////////////////////////

template<TSIZE ALIGNMENT>
VOID PDXALLOCATOR<ALIGNMENT>::Abandon(const VOID* const ptr)
{
	PDX_ASSERT(ptr);

	if (base)
	{
		const TSIZE* const end = PDX_CAST(const TSIZE*,*(PDX_CAST(const TSIZE*,ptr)-1));
		const TSIZE* const start = PDX_CAST(const TSIZE*,*end);

		if (*base == start)
			base = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Allocator routine
//////////////////////////////////////////////////////////////////////////////////////////////

template<TSIZE ALIGNMENT>
VOID* PDXALLOCATOR<ALIGNMENT>::Allocate(TSIZE size)
{
	// layed out in memory as follows:
	//
	// [0] size
	// [1] capacity
	// [2] (0) base address
	// [3] last object address
	// ..
	// [x] (z) object end address
	// [y] aligned object address
	// ..
	// [z] (0) base address

	PDX_ASSERT(size && (!base || (base && *base)));

    #ifdef _DEBUG
	size += sizeof(TSIZE);
    #endif

	const TSIZE pos = size;
	size += OBJECT_SIZE;

	TSIZE capacity = 1;

	if (base)
	{
		// See if there's any space left

		capacity = base[0][1];

		if (&PDX_CAST(const CHAR*,base[0])[BASE_SIZE+capacity] > &PDX_CAST(const CHAR*,base)[size])
			goto heaven;
  
		// If this block has already been allocated 
		// but don't have any references, delete it

		if (!base[0][0])
		{
			PDX_ASSERT(base[0][1]);
			free(*base);
		}
	}

	{
		{
			// Dynamically increase the size for every
			// allocation we make

			const TSIZE total = capacity + size;

			while (capacity < total)
				capacity += capacity;
		}

		TSIZE* const mem = PDX_CAST(TSIZE*,malloc(BASE_SIZE + capacity));
		
		if (!mem) 
		{
            #ifdef _DEBUG
			PDX_HALT;
            #else
			exit(EXIT_FAILURE);
            #endif
		}

		mem[0] = 0;
		mem[1] = capacity;
		mem[3] = PDX_CAST(TSIZE,mem);
		
		base = PDX_CAST(TSIZE**,&mem[3]);
	}

 heaven:

	PDX_ASSERT(*base);

	CHAR* object;

	if (ALIGNMENT)
	{
		// Align the pointer and fill the skipped bytes with zeroes 
		// so we can keep track of the base pointer later on

		object = PDX_CAST(CHAR*,PDX_ALIGN_ADDRESS_UP(&base[2]));
		const TSIZE alignment = object - PDX_CAST(CHAR*,&base[2]);
		memset(&base[1],0,alignment);
		base[0][0] += alignment + size;
	}
	else
	{
		object = PDX_CAST(CHAR*,&base[2]);
		base[0][0] += size;
	}

	*(PDX_CAST(TSIZE*,object) - 1) = PDX_CAST(TSIZE,&object[pos]);

 #ifdef _DEBUG
	*(PDX_CAST(TSIZE*,&object[pos]) - 1) = PDX_DEAD_END;
 #endif

	*PDX_CAST(TSIZE*,&object[pos]) = PDX_CAST(TSIZE,*base);
	base = PDX_CAST(TSIZE**,&object[pos]);

	base[0][2] = PDX_CAST(TSIZE,base);

	return PDX_CAST(VOID*,object);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Free the allocated data
//////////////////////////////////////////////////////////////////////////////////////////////

template<TSIZE ALIGNMENT>
VOID PDXALLOCATOR<ALIGNMENT>::Free(VOID* const ptr)
{
	PDX_ASSERT(!base || (base && *base));

	if (ptr)
	{
		TSIZE* const end = PDX_CAST(TSIZE*,*(PDX_CAST(TSIZE*,ptr)-1));
		PDX_ASSERT(*(end-1) == PDX_DEAD_END);

		TSIZE* const start = PDX_CAST(TSIZE*,*end);
		PDX_ASSERT(start[0] && start[1]);

		TSIZE size = OBJECT_SIZE + (PDX_CAST(CHAR*,end) - PDX_CAST(CHAR*,ptr));
		PDX_ASSERT(size);

		if (ALIGNMENT)
		{
			// Locate the unaligned position 

			const CHAR* notaligned = PDX_CAST(CHAR*,ptr) - (sizeof(TSIZE) * 2);

			while (PDX_CAST(TSIZE*,*PDX_CAST(const TSIZE*,notaligned--)) != start)
				++size;
		}

		PDX_ASSERT(*start >= size);

		const TSIZE newsize = *start - size;

		if (newsize) 
		{
			// Try to detect if this is the last object in the block. if it is we can
			// decrease the base pointer and try make some room for the next object

			*start = newsize;

			if (base && *base == start && start[2] == PDX_CAST(TSIZE,end))
			{
				base = PDX_CAST(TSIZE**,PDX_CAST(TSIZE*,ptr)-2);
				start[2] = PDX_CAST(TSIZE,base);
			}
		}
		else 
		{
			if (base && *base == start)
				base = NULL;

			free(start);
 		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Reserve memory
//////////////////////////////////////////////////////////////////////////////////////////////

template<TSIZE ALIGNMENT>
VOID PDXALLOCATOR<ALIGNMENT>::Reserve(TSIZE size)
{
	PDX_ASSERT(size && (!base || (base && *base)));

    #ifdef _DEBUG
	size += sizeof(TSIZE);
    #endif

	size += OBJECT_SIZE;

	if (!base || base[0][1] < base[0][0] + size)
	{
		TSIZE capacity = 1;

		if (base)
		{
			capacity = base[0][1];

    		// If this block has already been allocated 
   	       	// but don't have any references, delete it

			if (!base[0][0])
			{
				PDX_ASSERT(base[0][1]);
				free(*base);
			}
		}

		// Dynamically increase the size for every
		// allocation we make

		size += capacity;

		while (capacity < size)
			capacity += capacity;

		TSIZE* const mem = PDX_CAST(TSIZE*,malloc(BASE_SIZE+capacity));
		
		if (!mem) 
		{
         #ifdef _DEBUG
			PDX_HALT;
         #else
			exit(EXIT_FAILURE);
         #endif
		}

		mem[0] = 0;
		mem[1] = capacity;
		mem[3] = PDX_CAST(TSIZE,mem);
		
		base = PDX_CAST(TSIZE**,&mem[3]);
	}
}

#undef PDX_ALIGN_ADDRESS_UP
#undef PDX_DEAD_END

#endif
#endif
