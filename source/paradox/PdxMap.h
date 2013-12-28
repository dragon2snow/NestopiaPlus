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

#ifndef PDXMAP_H
#define PDXMAP_H

#include "PdxTree.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// Map data class
//////////////////////////////////////////////////////////////////////////////////////////////

template<class T,class K,class P,class C> class PDXMAPDATA
{
	typedef PDXMAPDATA<T,K,P,C> DATA;

	friend class PDXTREE<DATA>;

	typedef T VALUE;
	typedef K KEY;

	struct PREDICATE
	{
		template<class A,class B> 
		inline BOOL operator () (const PDXPAIR<A,B>& pair,const KEY& key) const
		{ return P()(*pair.First(),key); }

		template<class A,class B> 
		inline BOOL operator () (const KEY& key,const PDXPAIR<A,B>& pair) const
		{ return P()(key,*pair.First()); }

		template<class A,class B> 
		inline BOOL operator () (const A& a,const B& b) const
		{ return P()(a,b); }
	};

	struct COMPARE
	{
		template<class A,class B> 
		inline BOOL operator () (const PDXPAIR<A,B>& pair,const KEY& key) const
		{ return C()(*pair.First(),key); }

		template<class A,class B> 
		inline BOOL operator () (const KEY& key,const PDXPAIR<A,B>& pair) const
		{ return C()(key,*pair.First()); }
  
		template<class A,class B>
		inline BOOL operator () (const A& a,const B& b) const
		{ return C()(a,b); }
	};

	class ITERATOR;
	class CONSTITERATOR;

	class NODE
	{
		friend class PDXTREE<DATA>;
		friend class ITERATOR;
		friend class CONSTITERATOR;
		friend class PDXALLOCATOR<>;

		inline VALUE& Value()             
		{ return value; }

		inline const VALUE& Value() const 
		{ return value; }

		inline const KEY& Key() const     
		{ return key; }

		inline VOID* Data()
		{ return (VOID*)(&value); }

		template<class V,class W> 
		inline NODE(const PDXPAIR<V,W>& pair)
		: 
		value   (*pair.Second()), 
		key     (*pair.First()), 
		balance (PDXTREE<DATA>::EH), 
		left    (NULL), 
		right   (NULL),
		next    (NULL),
		prev    (NULL)
		{}

		template<class V> 
		inline NODE(const V& k)
		: 
		key     (k), 
		balance (PDXTREE<DATA>::EH), 
		left    (NULL), 
		right   (NULL), 
		next    (NULL),
		prev    (NULL)
		{}

		PDX_COMPILE_ASSERT(sizeof(VALUE) + sizeof(const KEY) <= INT_MAX);
		enum {SIZE=sizeof(VALUE) + sizeof(const KEY)};

		VALUE     value;
		const KEY key;
		NODE*     left;
		NODE*     right;
		NODE*     next;
		NODE*     prev;

		PDXTREE<DATA>::BALANCE balance;
	};

	class PAIR
	{
	public:

		friend class ITERATOR;
		friend class CONSTITERATOR;

		inline const KEY& Key() const           
		{ return *key; }

		inline VALUE& Value()                   
		{ return *value; }

		inline const VALUE& Value() const 
		{ return *value; }

		inline const KEY& First() const          
		{ return *key; }

		inline VALUE& Second()             
		{ return *value; }

		inline const VALUE& Second() const 
		{ return *value; }

		inline VOID* operator & ()
		{ return (VOID*)(&value); }

		inline const VOID* operator & () const
		{ return (VOID*)(&value); }

	private:

		inline PAIR(const KEY& k,VALUE& v)
		: key(&k), value(&v) {}

		const KEY* const key;
		VALUE* const value;
	};

	class CONSTPAIR
	{
	public:

		friend class ITERATOR;
		friend class CONSTITERATOR;

		inline const KEY& Key() const           
		{ return *key; }

		inline const VALUE& Value() const 
		{ return *value; }

		inline const KEY& First() const          
		{ return *key; }

		inline const VALUE& Second() const 
		{ return *value; }

		inline const VOID* operator & () const
		{ return (VOID*)(&value); }

	private:

		inline CONSTPAIR(const KEY& k,const VALUE& v)
		: key(&k), value(&v) {}

		const KEY* const key;
		const VALUE* const value;
	};

	class ITERATOR
	{
	public:

		friend class CONSTITERATOR;
		friend class PDXTREE<DATA>;

		PDX_DEFAULT_CONSTRUCTOR(ITERATOR);

		inline operator CONSTITERATOR() const
		{ return CONSTITERATOR(*this); }

		inline PAIR operator ->() { PDX_ASSERT(node); return PAIR(node->key,node->value); }
		inline PAIR operator * () { PDX_ASSERT(node); return PAIR(node->key,node->value); }

		inline CONSTPAIR operator ->() const { PDX_ASSERT(node); return CONSTPAIR(node->key,node->value); }
		inline CONSTPAIR operator * () const { PDX_ASSERT(node); return CONSTPAIR(node->key,node->value); }

		inline ITERATOR& operator ++() { PDX_ASSERT(node); node = node->next; return *this; }
		inline ITERATOR& operator --() { PDX_ASSERT(node); node = node->prev; return *this; }

		inline ITERATOR operator ++ (INT)
		{
			const ITERATOR i(*this);
			node = node->next;
			return i;
		}

		inline ITERATOR operator -- (INT)
		{
			const ITERATOR i(*this);
			node = node->prev;
			return i;
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

		VOID operator & () const;

		inline ITERATOR(NODE* const begin)
		: node(begin) {}

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

		inline const CONSTPAIR operator ->() const { PDX_ASSERT(node); return CONSTPAIR(node->key,node->value); }
		inline const CONSTPAIR operator * () const { PDX_ASSERT(node); return CONSTPAIR(node->key,node->value); }

		inline CONSTITERATOR& operator ++() { PDX_ASSERT(node); node = node->next; return *this; }
		inline CONSTITERATOR& operator --() { PDX_ASSERT(node); node = node->prev; return *this; }

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

		inline VOID operator = (ITERATOR iterator)
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

		VOID operator & () const;

		inline CONSTITERATOR(const NODE* const begin)
		: node(begin) {}

		const NODE* node;
	};
};

//////////////////////////////////////////////////////////////////////////////////////////////
// Main map class
//////////////////////////////////////////////////////////////////////////////////////////////

template<class T,class K,class P=PDX::Less<T>,class C=PDX::Equal<T> > 
class PDXMAP : public PDXTREE<PDXMAPDATA<T,K,P,C> >
{
public:

	template<class V,class W> 
	ITERATOR Insert(const V& v=V(),const W& w=W())
	{ return TREE::Insert(PDX::MakePair(&v,&w)); }

	template<class V,class W> 
	PDXPAIR<ITERATOR,BOOL> SafeInsert(const V& v=V(),const W& w=W())
	{ return TREE::SafeInsert(PDX::MakePair(&v,&w)); }

	template<class V> 
	T& operator [] (const V& key)
	{ return (*TREE::SafeInsert(key).First()).Value(); }
};

#endif

