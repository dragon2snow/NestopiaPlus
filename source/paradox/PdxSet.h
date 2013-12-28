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

#ifndef PDXSET_H
#define PDXSET_H

#include "PdxTree.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// Set data
//////////////////////////////////////////////////////////////////////////////////////////////

template<class T,class P,class C> class PDXSETDATA
{
	typedef PDXSETDATA<T,P,C> DATA;

	friend class PDXTREE<DATA>;

	typedef T VALUE;
	typedef T KEY;
	typedef P PREDICATE;
	typedef C COMPARE;

	class ITERATOR;
	class CONSTITERATOR;

	class NODE
	{
		friend class PDXTREE<DATA>;
		friend class ITERATOR;
		friend class CONSTITERATOR;
		friend class PDXALLOCATOR<>;

		template<class V> 
		inline NODE(const V& v) 
		: 
     	value   (v), 
		balance (PDXTREE<DATA>::EH), 
		left    (NULL), 
		right   (NULL), 
		next    (NULL), 
		prev    (NULL) 
		{}

		template<class V> 
		inline NODE(const PDXPAIR<const V* const,const V* const>& v) 
		: 
     	value   (*v.First()), 
		balance (PDXTREE<DATA>::EH), 
		left    (NULL), 
		right   (NULL), 
		next    (NULL), 
		prev    (NULL) 
		{}

		inline KEY& Key()               
		{ return value; }

		inline const KEY& Key() const   
		{ return value; }

		inline VALUE& Value()             
		{ return value; }

		inline const VALUE& Value() const 
		{ return value; }

		inline VOID* Data()
		{ return (VOID*)(&value); }

		PDX_COMPILE_ASSERT(sizeof(VALUE) < INT_MAX);
		enum {SIZE=sizeof(VALUE)};

		VALUE value;
		NODE* left;
		NODE* right;
		NODE* next;
		NODE* prev;

		PDXTREE<DATA>::BALANCE balance;		
	};

	class ITERATOR
	{
	public:

		friend class CONSTITERATOR;
		friend class PDXTREE<DATA>;

		PDX_DEFAULT_CONSTRUCTOR(ITERATOR);

		inline operator CONSTITERATOR()
		{ return CONSTITERATOR(*this); }

		inline const VALUE& operator ->() const { PDX_ASSERT(node); return node->value; }
		inline const VALUE& operator * () const { PDX_ASSERT(node); return node->value; }

		inline VALUE& operator ->() { PDX_ASSERT(node); return node->value; }
		inline VALUE& operator * () { PDX_ASSERT(node); return node->value; }

		inline ITERATOR& operator ++() { PDX_ASSERT(node); node = node->next; return *this; }
		inline ITERATOR& operator --() { PDX_ASSERT(node); node = node->prev; return *this; }

		inline ITERATOR operator ++ (INT)
		{
			const ITERATOR i(*this);
			PDX_ASSERT(node);
			node = node->next;
			return i;
		}

		inline ITERATOR operator -- (INT)
		{
			const ITERATOR i(*this);
			PDX_ASSERT(node);
			node = node->prev;
			return i;
		}

		inline VOID operator += (const TSIZE n)
		{
			for (TSIZE i=0; i < n; ++i) 
			{
				PDX_ASSERT(node);
				node = node->next;
			}
		}

		inline VOID operator -= (const TSIZE n)
		{
			for (TSIZE i=0; i < n; ++i) 
			{
				PDX_ASSERT(node);
				node = node->prev;
			}
		}

		inline BOOL operator < (const CONSTITERATOR iterator) const
		{ return node ? (iterator.node ? (*node < *iterator.node) : TRUE) : (node != iterator.node); }

		inline BOOL operator == (const CONSTITERATOR iterator) const { return node == iterator.node; }
		inline BOOL operator != (const CONSTITERATOR iterator) const { return node != iterator.node; }
		inline BOOL operator >  (const CONSTITERATOR iterator) const { return iterator < *this; }
		inline BOOL operator <= (const CONSTITERATOR iterator) const { return !(iterator < *this); }
		inline BOOL operator >= (const CONSTITERATOR iterator) const { return !(*this < iterator); }

		TSIZE operator - (const CONSTITERATOR iterator) const
		{
			TSIZE i=0;

			for (const NODE* n = iterator.node; n != node; ++i)
			{
				PDX_ASSERT(n);
				n = n->next;
			}

			return i;
		}
  
	private:

		inline ITERATOR(NODE* const n)
		: node(n) {}

		NODE* node;
	};

	class CONSTITERATOR	
	{
	public:

		friend class ITERATOR;
		friend class PDXTREE<DATA>;

		PDX_DEFAULT_CONSTRUCTOR(CONSTITERATOR);

		inline CONSTITERATOR(const ITERATOR iterator)
		: node(iterator.node) {}

		inline const VALUE& operator ->() const { PDX_ASSERT(node); return node->value; }
		inline const VALUE& operator * () const { PDX_ASSERT(node); return node->value; }

		inline const CONSTITERATOR& operator ++() { PDX_ASSERT(node); node = node->next; return *this; }
		inline const CONSTITERATOR& operator --() { PDX_ASSERT(node); node = node->prev; return *this; }

		inline const CONSTITERATOR operator ++ (INT)
		{
			const CONSTITERATOR i(*this);
			PDX_ASSERT(node);
			node = node->next;
			return i;
		}

		inline const CONSTITERATOR operator -- (INT)
		{
			const CONSTITERATOR i(*this);
			PDX_ASSERT(node);
			node = node->prev;
			return i;
		}

		inline VOID operator += (const ULONG n)
		{
			for (ULONG i=0; i < n; ++i) 
			{
				PDX_ASSERT(node);
				node = node->next;
			}
		}

		inline VOID operator -= (const ULONG n)
		{
			for (ULONG i=0; i < n; ++i) 
			{
				PDX_ASSERT(node);
				node = node->prev;
			}
		}

		inline VOID operator = (const ITERATOR iterator)                  
		{ node = iterator.node; }

		inline BOOL operator < (const CONSTITERATOR iterator) const
		{ return node ? (iterator.node ? (*node < *iterator.node) : TRUE) : (node != iterator.node); }

		inline BOOL operator == (const CONSTITERATOR iterator) const { return node == iterator.node; }
		inline BOOL operator != (const CONSTITERATOR iterator) const { return node != iterator.node; }
		inline BOOL operator >  (const CONSTITERATOR iterator) const { return iterator < *this; }
		inline BOOL operator <= (const CONSTITERATOR iterator) const { return !(iterator < *this); }
		inline BOOL operator >= (const CONSTITERATOR iterator) const { return !(*this < iterator); }

		TSIZE operator - (const CONSTITERATOR iterator) const
		{
			TSIZE i=0;

			for (const NODE* n = iterator.node; n != node; ++i)
			{
				PDX_ASSERT(n);
				n = n->next;
			}

			return i;
		}

	private:

		inline CONSTITERATOR(const NODE* const n)
		: node(n) {}

		const NODE* node;
	};
};

//////////////////////////////////////////////////////////////////////////////////////////////
// Set class
//////////////////////////////////////////////////////////////////////////////////////////////

template<class T,class P=PDX::Less<T>,class C=PDX::Equal<T> > 
class PDXSET : public PDXTREE<PDXSETDATA<T,P,C> >
{
private:

	typedef PDXTREE<PDXSETDATA<T,P,C> > OLDMAN;

public:

	template<class I>
	inline T& operator [] (const I& i)
	{ return *Get(i); }

	template<class I>
	inline const T& operator [] (const I& i) const
	{ return *Get(i); }

	inline VOID operator = (const PDXSET<T,P,C>& set)
	{
		*PDX_STATIC_CAST(OLDMAN*,this) = *PDX_STATIC_CAST(const OLDMAN*,&set);
	}
};

#endif
