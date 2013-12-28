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

#ifndef PDXQUICKSORT_H
#define PDXQUICKSORT_H

#include "PdxUtilities.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// Quick sort class
//////////////////////////////////////////////////////////////////////////////////////////////

class PDXQUICKSORT : PDX_STATIC_CLASS
{
public:

	template<class I> 
	static inline VOID Sort(I* const begin,const I* const end)
	{ if (begin != end) Go(begin,end,DefaultPredicate(*begin)); }

	template<class I,class P> 
	static inline VOID Sort(I* const begin,const I* const end,P predicate)
	{ if (begin != end) Go(begin,end,predicate); }

private:

	template<class V,class P> 
	static inline BOOL PredicateLeftTmp(const V& value,const CHAR* const tmp,P predicate)
	{ return predicate(value,*PDX_CAST(const V* const,tmp)); }

	template<class V,class P> 
	static inline BOOL PredicateRightTmp(const CHAR* const tmp,const V& value,P predicate)
	{ return predicate(*PDX_CAST(const V* const,tmp),value); }

	template<class T> 
	static inline PDX::Less<T> DefaultPredicate(const T&)
	{ return PDX::Less<T>(); }

	template<class I,class P> 
	static PDX_NO_INLINE VOID PDX_STDCALL Go(I* const,const I* const,P);

	enum { MINSIZE = 16 };
	enum { STACKSIZE = PDX_SIZE_BITS(TSIZE) };
};

//////////////////////////////////////////////////////////////////////////////////////////////
// Quick sort without the pesky recursion
//////////////////////////////////////////////////////////////////////////////////////////////

template<class I,class P> 
VOID PDX_STDCALL PDXQUICKSORT::Go(I* const iterator,const I* const last,P predicate)
{
	const TSIZE EndIdx = (last - iterator) - 1;

	TSIZE lo = 0;
	TSIZE hi = EndIdx;

	if (hi-lo > MINSIZE)
	{
		CHAR pivot[sizeof(*iterator)];
		TSIZE stack[STACKSIZE];

    	TSIZE counter = 2;

		while (counter)
		{
			const TSIZE mi = (lo+hi) / 2;

			if (predicate(iterator[mi],iterator[lo]))   
				PDX::Swap(iterator[mi],iterator[lo]);        

			if (predicate(iterator[hi],iterator[mi]))
			{
				PDX::Swap(iterator[mi],iterator[hi]);

				if (predicate(iterator[mi],iterator[lo]))
					PDX::Swap(iterator[mi],iterator[lo]);
			}

			memcpy(pivot,&(iterator[mi]),sizeof(iterator[mi]));

			TSIZE left = lo + 1;
			TSIZE right = hi;

			while (left <= right)
			{
				while (PredicateLeftTmp(iterator[left],pivot,predicate))
					++left;

				while (PredicateRightTmp(pivot,iterator[right],predicate))
					--right;

				if (left <= right)
					PDX::Swap(iterator[left++],iterator[right--]);
			}

			if ((right-lo) <= MINSIZE)
			{
				if ((hi-left) <= MINSIZE)
				{
					hi = stack[--counter];
					lo = stack[--counter];
				}
				else
				{
					lo = left;
				}
			}
			else if ((hi-left) <= MINSIZE)
			{
				hi = right;
			}
			else if ((right-lo) > (hi-left))
			{
				stack[counter++] = lo;     
				stack[counter++] = right;
				lo = left;
			}
			else
			{
				stack[counter++] = left;
				stack[counter++] = hi;   
				hi = right;
			}
		}
	}

	{
		const TSIZE tresh = PDX_MIN(EndIdx,MINSIZE);

		TSIZE idx = 0;

		for (TSIZE i=1; i <= tresh; ++i)
			if (predicate(iterator[i],iterator[idx]))
				idx = i;

		if (idx != 0)
			PDX::Swap(iterator[idx],iterator[0]);
	}

	for (TSIZE i=1; i <= EndIdx; ++i)
	{
		TSIZE j = i - 1;

		while (predicate(iterator[i],iterator[j]))
			--j;

		while ((++j) < i)
			PDX::Swap(iterator[i],iterator[j]);
	}
}

#endif
