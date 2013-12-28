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

#ifndef PDXTREE_H
#define PDXTREE_H

#if !defined(PDXSET_H) && !defined(PDXMAP_H)
#error Do not include PdxTree.h directly!
#endif

#include "PdxAllocator.h"
#include "PdxPair.h"
#include "PdxUtilities.h"	

//////////////////////////////////////////////////////////////////////////////////////////////
// Base tree class
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> class PDXTREE
{
	typedef DATA::NODE NODE;

public:

	enum BALANCE
	{
		LH =  1,
		EH =  0,
		RH = -1
	};

	typedef PDXTREE<DATA>       TREE;
	typedef DATA::VALUE         VALUE;
	typedef DATA::KEY           KEY;
	typedef DATA::PREDICATE   	PREDICATE;
	typedef DATA::COMPARE       COMPARE;
	typedef DATA::ITERATOR      ITERATOR;
	typedef DATA::CONSTITERATOR CONSTITERATOR;

	PDXTREE();
	~PDXTREE();
				 
	VOID Destroy();

	template<class INPUT> ITERATOR Insert(const INPUT& k=INPUT());	
	template<class INPUT> ITERATOR BlockInsert(VOID*,const INPUT& k=INPUT());	
	template<class INPUT> PDXPAIR<ITERATOR,BOOL> SafeInsert(const INPUT& k=INPUT());	

	template<class INPUT> VOID Erase(const INPUT&);
	template<class INPUT> BOOL SafeErase(const INPUT&);
	template<class INPUT> VOID* BlockErase(const INPUT&);

	template<class BEGIN,class END> PDX_NO_INLINE TSIZE InsertSequence(BEGIN,const END&);	
	template<class BEGIN,class END> PDX_NO_INLINE TSIZE SafeInsertSequence(BEGIN,const END&);	
	template<class BEGIN,class END> PDX_NO_INLINE TSIZE EraseSequence(BEGIN,const END&);

	VOID operator = (const TREE&);
	BOOL operator == (const TREE&) const;

	template<class INPUT> VOID Swap(const INPUT&,const INPUT&);

	ITERATOR At(const TSIZE pos)
	{ return ITERATOR(AtNode(pos)); }

	const CONSTITERATOR At(const TSIZE pos) const
	{ return CONSTITERATOR(AtNode(pos)); }
	
	template<class INPUT> inline ITERATOR Find(const INPUT& key)
	{ return Find(root,key); }

	template<class INPUT> inline const CONSTITERATOR Find(const INPUT& key) const
	{ return Find(root,key); }

	template<class INPUT> inline ITERATOR Get(const INPUT& key)
	{ return Get(root,key); }

	template<class INPUT> inline const CONSTITERATOR Get(const INPUT& key) const
	{ return Get(root,key); }

	inline TSIZE Size() const
	{ return size; }

	inline BOOL IsEmpty() const
	{ return size == 0 ? TRUE : FALSE; }

	VOID Reserve(const TSIZE length)
	{ allocator.Reserve(sizeof(NODE) * length); }

	inline ITERATOR Begin()
	{ return ITERATOR(first); }

	inline ITERATOR Current()
	{ return ITERATOR(current);	}

	inline ITERATOR Last()
	{ return ITERATOR(last); }

	inline ITERATOR End()
	{ return ITERATOR(NULL); }

	inline const CONSTITERATOR Begin() const
	{ return CONSTITERATOR(first); }

	inline const CONSTITERATOR Current() const
	{ return CONSTITERATOR(current); }

	inline const CONSTITERATOR Last() const
	{ return CONSTITERATOR(last); }

	inline const CONSTITERATOR End() const
	{ return CONSTITERATOR(NULL); }

private:

	VOID Destroy(NODE* const);
	NODE* RecursiveCopy(const NODE* const,NODE*&);

	VOID SetLeftLinks(NODE* const,NODE* const);
	VOID SetRightLinks(NODE* const,NODE* const);

	template<bool CHECK,class INPUT> NODE* RecursiveInsert(NODE*,const INPUT&);
	template<bool CHECK,class INPUT> NODE* RecursiveErase(NODE* const,const INPUT&);

	NODE* AtNode(TSIZE);
	const NODE* AtNode(TSIZE) const;

	NODE* InsertLeftBalance  (NODE*);
	NODE* InsertRightBalance (NODE*);

	NODE* EraseLeftBalance  (NODE*);
	NODE* EraseRightBalance (NODE*);

	static NODE* RotateLeft  (NODE* const);
	static NODE* RotateRight (NODE* const);

	template<class _NODE,class INPUT> static _NODE Find(_NODE,const INPUT&);
	template<class _NODE,class INPUT> static _NODE Get(_NODE,const INPUT&);

	NODE* root;
	NODE* first;
	NODE* last;
	NODE* current;
	TSIZE size;

	union
	{
		VOID* block;
		BOOL SaveBlock;
	};

	union
	{
		BOOL taller;
		BOOL shorter;
	};

	PDXALLOCATOR<> allocator;
};

//////////////////////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> 
PDXTREE<DATA>::PDXTREE()
:
root      (NULL),
first     (NULL),
last      (NULL),
current	  (NULL),
SaveBlock (FALSE),
size      (0)
{}

//////////////////////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> 
PDXTREE<DATA>::~PDXTREE()
{
	Destroy(root);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Destroyer
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> 
VOID PDXTREE<DATA>::Destroy()
{
	Destroy(root);
	
	root      = NULL;
	first     = NULL;
	last      = NULL;
	current   = NULL;
	SaveBlock = FALSE;
	size      = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Recursive node destroy
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> 
VOID PDXTREE<DATA>::Destroy(NODE* const node)
{
	if (node)
	{
		if (node->left) Destroy(node->left);
		if (node->right) Destroy(node->right);
		allocator.Delete(node);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Insert if input type doesn't already exist
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> template<class INPUT>
PDXPAIR<PDXTREE<DATA>::ITERATOR,BOOL> PDXTREE<DATA>::SafeInsert(const INPUT& key)
{
	const TSIZE oldsize = size;
	taller = FALSE;	
	root = RecursiveInsert<true>(root,key);
	return PDXPAIR<ITERATOR,BOOL>(ITERATOR(current),PDX_TO_BOOL(oldsize != size));
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Insert a sequence of values
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> template<class BEGIN,class END>
TSIZE PDXTREE<DATA>::InsertSequence(BEGIN begin,const END& end)
{
	const TSIZE length = end - begin;
	allocator.Reserve(sizeof(NODE) * length);

	for (; begin != end; ++begin)
		Insert(*begin);

	return length;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Insert a sequence of non existing values 
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> template<class BEGIN,class END>
TSIZE PDXTREE<DATA>::SafeInsertSequence(BEGIN begin,const END& end)
{
	TSIZE length = end - begin;
	allocator.Reserve(sizeof(NODE) * length);

	length = 0;

	for (; begin != end; ++begin)
		length += SafeInsert(*begin).Second();

	return length;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Erase if the input value exists
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> template<class INPUT>
BOOL PDXTREE<DATA>::SafeErase(const INPUT& key)
{
	const TSIZE oldsize = size;	
	shorter = FALSE;
	root = RecursiveErase<true>(root,key);
	return PDX_TO_BOOL(oldsize != size);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Erase sequence of existing values
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> template<class BEGIN,class END>
TSIZE PDXTREE<DATA>::EraseSequence(BEGIN begin,const END& end)
{
	TSIZE length = 0;

	for (; begin != end; ++begin)
		length += PDX_TO_BOOL(SafeErase(*begin).node);

	return length;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Copy tree
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> 
VOID PDXTREE<DATA>::operator = (const TREE& tree)
{
	Destroy(root);
	allocator.Reserve(tree.size * sizeof(NODE));

	{
		NODE* prev = NULL;
		root = RecursiveCopy(tree.root,prev);
	}

	size  = tree.size;

	if (size)
	{
		first = root;

		while (first->left)
			first = first->left;

		last = root;

		while (last->right)
			last = last->right;

		current = last;
	}
	else
	{
		first = last = current = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Compare tree
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> 
BOOL PDXTREE<DATA>::operator == (const TREE& tree) const
{
	if (size != tree.size)
		return FALSE;

	if (!root)
		return TRUE;

	for (const NODE* a=first, b=tree.first; a != last; a=a->next, b=b->next)
		if (!(a->value == b->value))
			return FALSE;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Recursive node inserter function
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> template<bool CHECK,class INPUT>
PDXTREE<DATA>::NODE* PDXTREE<DATA>::RecursiveInsert(NODE* parent,const INPUT& key)
{
	if (!parent)
	{
		++size;
		taller = TRUE;

		if (block)
		{
			current = PDX_CAST(NODE*,block);
			PDX_CONSTRUCT(*current) NODE(key);
		}
		else
		{
			current = allocator.New<NODE>(key);
		}

		if (size == 1) 
			first = last = current;

		return current;
	}

	if (PREDICATE()(key,parent->Key()))
	{
		parent->left = RecursiveInsert<CHECK>(parent->left,key);

		if (taller)
		{
			NODE* const node = parent->left;

			if (!node->prev && !node->next)
				SetLeftLinks(node,parent);

			switch (parent->balance)
			{
     	   		case LH: parent = InsertLeftBalance(parent); break;
     			case EH: parent->balance = LH; break;
     			case RH: parent->balance = EH; taller = FALSE; break;
			}
		}
	}
	else if (!CHECK || PREDICATE()(parent->Key(),key))
	{
		PDX_ASSERT(!COMPARE()(parent->Key(),key));

		parent->right = RecursiveInsert<CHECK>(parent->right,key);

		if (taller)
		{
			NODE* const node = parent->right;

			if (!node->prev && !node->next)
				SetRightLinks(node,parent);

			switch (parent->balance)
			{
    			case LH: parent->balance = EH; taller = FALSE; break;
     			case EH: parent->balance = RH; break;
     			case RH: parent = InsertRightBalance(parent); break;
			}
		}
	}
	else if (CHECK)
	{
		current = parent;
	}

	return parent;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Insert a node from the left into the linked list
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA>
VOID PDXTREE<DATA>::SetLeftLinks(NODE* const node,NODE* const parent)
{
	node->next = parent;
	node->prev = parent->prev;

	if (node->prev) node->prev->next = node;
	if (node->next) node->next->prev = node;

	if (first)
	{
		if (first == node->next)
		{
			first = node;
		}
		else if (last == node->prev)
		{
			last = node;
		}
	}
	else
	{
		first = last = node;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Insert a node from the right into the linked list
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA>
VOID PDXTREE<DATA>::SetRightLinks(NODE* const node,NODE* const parent)
{
	node->next = parent->next;
	node->prev = parent;

	if (node->prev) node->prev->next = node;
	if (node->next) node->next->prev = node;

	if (first)
	{
		if (first == node->next)
		{
			first = node;
		}
		else if (last == node->prev)
		{
			last = node;
		}
	}
	else
	{
		first = last = node;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Recursive node erasor function
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> template<bool CHECK,class INPUT>
PDXTREE<DATA>::NODE* PDXTREE<DATA>::RecursiveErase(NODE* const node,const INPUT& key)
{
	if (CHECK && !node)
	{
		shorter = FALSE;
		return NULL;
	}

	if (PREDICATE()(key,node->Key()))
	{
		node->left = RecursiveErase<CHECK>(node->left,key);
		if (shorter) return EraseRightBalance(node);
	}
	else if (PREDICATE()(node->Key(),key))
	{
		node->right = RecursiveErase<CHECK>(node->right,key);
		if (shorter) return EraseLeftBalance(node);
	}
	else
	{
		PDX_ASSERT(COMPARE()(node->Key(),key));

		if (!node->right || !node->left)
		{
			NODE* const newroot = node->right ? node->right : node->left;

			NODE*& prev = node->prev ? node->prev->next : first;
			NODE*& next = node->next ? node->next->prev : last;

			prev = node->next;
			next = node->prev;

			current = (current == node) ? (next ? next : prev) : current;

			if (SaveBlock)
			{
				block = PDX_CAST(VOID*,node);
				allocator.Abandon(block);
			}
			else
			{
				allocator.Delete(node);
			}

			shorter = TRUE;
			--size;

			return newroot;
		}
		else
		{
			NODE* exchange = node->left;

			while (exchange->right)
				exchange = exchange->right;

			{
				CHAR tmp[NODE::SIZE];
				memcpy(tmp,node->Data(),NODE::SIZE);
				memcpy(node->Data(),exchange->Data(),NODE::SIZE);
				memcpy(exchange->Data(),tmp,NODE::SIZE);
			}

			node->left = RecursiveErase<CHECK>(node->left,exchange->Key());
			if (shorter) return EraseRightBalance(node);
		}
	}

	return node;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Node inserter left balance function
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> 
PDXTREE<DATA>::NODE* PDXTREE<DATA>::InsertLeftBalance(NODE* node)
{
	NODE* const left = node->left;

	switch (left->balance)
	{
     	case LH: 

	     	node->balance = EH;
     		left->balance = EH;
     		node = RotateRight(node);
     		taller = FALSE;
     		break;

     	case RH:
		{
			NODE* const right = left->right;

			switch (right->balance)
			{
	     		case LH:

		     		node->balance = RH;
		     		left->balance = EH;
		     		break;

		     	case EH:

	       			node->balance = EH;
	       			left->balance = EH;
	     			break;

	       		case RH:

	       			node->balance = EH;
	       			left->balance = LH;
	     			break;
			}

			right->balance = EH;
			node->left = RotateLeft(node->left);
			node = RotateRight(node);
			taller = FALSE;
			break;
		}
     
        #ifdef _DEBUG
     	default: PDX_BREAK;
        #endif
	}

	return node;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Node inserter right balance function
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> 
PDXTREE<DATA>::NODE* PDXTREE<DATA>::InsertRightBalance(NODE* node)
{
	NODE* const right = node->right;

	switch (right->balance)
	{
    	case RH: 

	     	node->balance = EH;
     		right->balance = EH;
     		node = RotateLeft(node);
     		taller = FALSE;
     		break;

    	case LH:
		{
			NODE* const left = right->left;

			switch (left->balance)
			{
	     		case RH:

	     			node->balance = LH;
	     			right->balance = EH;
	       			break;

	     		case EH:

	     			node->balance = EH;
	     			right->balance = EH;
	     			break;

	     		case LH:

	     			node->balance = EH;
	       			right->balance = RH;
	       			break;
			}

	   		left->balance = EH;
			node->right = RotateRight(node->right);
			node = RotateLeft(node);
			taller = FALSE;
			break;
		}
        
        #ifdef _DEBUG
    	default: PDX_BREAK;
        #endif
	}

	return node;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Node erasor left balance function
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> 
PDXTREE<DATA>::NODE* PDXTREE<DATA>::EraseLeftBalance(NODE* node)
{
	switch (node->balance)
	{
    	case RH:

     		node->balance = EH;
     		break;

     	case EH:

     		node->balance = LH;
     		shorter = FALSE;
     		break;

     	case LH:
		{
			NODE* const left = node->left;

			if (left->balance == RH)
			{
				NODE* const right = left->right;

				switch (right->balance)
				{
	     			case RH: left->balance = LH; node->balance = EH; break;
	       			case EH: node->balance = EH; left->balance = EH; break;
	       			case LH: node->balance = RH; left->balance = EH; break;
				}

				right->balance = EH;
				node->left = RotateLeft(left);
				node = RotateRight(node);
			}
			else
			{
				switch (left->balance)
				{
	     			case RH:
	       			case LH:

	     				node->balance = EH;
	     				left->balance = EH;
	       				break;

	     			case EH:

	      				node->balance = LH;
       					left->balance = RH;
		     			shorter = FALSE;
		     			break;
	     		}

				node = RotateRight(node);
			}

			break;
		}
	}

	return node;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Node erasor right balance function
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> 
PDXTREE<DATA>::NODE* PDXTREE<DATA>::EraseRightBalance(NODE* node)
{
	switch (node->balance)
	{
     	case LH:

     		node->balance = EH;
       		break;

     	case EH:

     		node->balance = RH;
       		shorter = FALSE;
     		break;

    	case RH:
		{
			NODE* const right = node->right;

			if (right->balance == LH)
			{
				NODE* const left = right->left;

				switch (left->balance)
				{
	     			case LH: right->balance = RH; node->balance = EH; break;
	     			case EH: node->balance = EH; right->balance = EH; break;
	       			case RH: node->balance = LH; right->balance = EH; break;
				}

				left->balance = EH;
				node->right = RotateRight(right);
				node = RotateLeft(node);
			}
			else
			{
				switch (right->balance)
				{
	     			case LH:
	     			case RH:

		     			node->balance = EH;
		     			right->balance = EH;
		     			break;

		     		case EH:

		     			node->balance = RH;
		       			right->balance = LH;
		       			shorter = FALSE;
		       			break;
				}

				node = RotateLeft(node);
			}

			break;
		}
	}

	return node;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Recursive value copy function
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> 
PDXTREE<DATA>::NODE* PDXTREE<DATA>::RecursiveCopy(const NODE* const from,NODE*& prev)
{
	NODE* const to = allocator.New<NODE>(PDXPAIR<const VALUE* const,const KEY* const>(&from->value,&from->Key()));

	if (from->left) 
		to->left = RecursiveCopy(from->left,prev);

	if (prev) 
		prev->next = to;

	to->prev = prev;
	prev = to;

	if (from->right) 
		to->right = RecursiveCopy(from->right,prev);

	return to;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Swap two nodes
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> template<class INPUT>
VOID PDXTREE<DATA>::Swap(const INPUT& key1,const INPUT& key2)
{
	NODE* const node1 = Get(root,key1);
	PDX_ASSERT(node1);

	NODE* const node2 = Get(root,key2);
	PDX_ASSERT(node2);

	CHAR tmp[NODE::SIZE];
	memcpy(tmp,node1->Data(),NODE::SIZE);
	memcpy(node1->Data(),node2->Data(),NODE::SIZE);
	memcpy(node2->Data(),tmp,NODE::SIZE);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Retrieve a node based on index in sorted order 
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA>
PDXTREE<DATA>::NODE* PDXTREE<DATA>::AtNode(const TSIZE pos)
{	
	PDX_ASSERT(size > pos);

	NODE* node;

	if (pos <= size/2)
	{
		node = first;

		for (TSIZE i=0; i < pos; ++i)
		{
			PDX_ASSERT(node);
			node = node->next;
		}
	}
	else
	{
		node = last;

		for (TSIZE i=size-1; i > pos; --i)
		{
			PDX_ASSERT(node);
			node = node->prev;
		}
	}

	return node;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Retrieve a node based on index in sorted order, const version
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA>
const PDXTREE<DATA>::NODE* PDXTREE<DATA>::AtNode(const TSIZE pos) const
{	
	PDX_ASSERT(size > pos);

	const NODE* node;

	if (pos <= size/2)
	{
		node = first;

		for (TSIZE i=0; i < pos; ++i)
		{
			PDX_ASSERT(node);
			node = node->next;
		}
	}
	else
	{
		node = last;

		for (TSIZE i=size-1; i > pos; --i)
		{
			PDX_ASSERT(node);
			node = node->prev;
		}
	}

	return node;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Do a binary search for the given key
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> template<class _NODE,class INPUT> 
_NODE PDXTREE<DATA>::Find(_NODE node,const INPUT& key)
{
	for (;;)
	{
		if (!node || COMPARE()(node->Key(),key)) 
			break;

		node = PREDICATE()(node->Key(),key) ? node->right : node->left;
	}

	return node;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Do a binary search for the given key which MUST exist!
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> template<class _NODE,class INPUT>
_NODE PDXTREE<DATA>::Get(_NODE node,const INPUT& key)
{
	for (;;)
	{
		PDX_ASSERT(node);

		if (COMPARE()(node->Key(),key)) 
			break;

		node = PREDICATE()(node->Key(),key) ? node->right : node->left;
	}

	return node;
}

#include "PdxTree.inl"

#endif




