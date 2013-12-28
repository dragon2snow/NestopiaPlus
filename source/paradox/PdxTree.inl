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

#ifndef PDXTREE_H
#error Do not include PdxTree.inl directly!
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Insert key
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> template<class INPUT>
inline PDXTREE<DATA>::ITERATOR PDXTREE<DATA>::Insert(const INPUT& key)
{
	taller = FALSE;
	root = RecursiveInsert<false>(root,key);
	PDX_ASSERT(root);
	return ITERATOR(current);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Insert allocated key
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> template<class INPUT>
inline PDXTREE<DATA>::ITERATOR PDXTREE<DATA>::BlockInsert(VOID* const b,const INPUT& key)
{
	PDX_ASSERT(b);

	taller = FALSE;
	block = b;
	root = RecursiveInsert<false>(root,key);
	block = NULL;
	PDX_ASSERT(root);

	return ITERATOR(current);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Erase key
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> template<class INPUT>
inline VOID PDXTREE<DATA>::Erase(const INPUT& key)
{
	shorter = FALSE;
	root = RecursiveErase<false>(root,key);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Erase allocated key
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> template<class INPUT>
inline VOID* PDXTREE<DATA>::BlockErase(const INPUT& key)
{
	shorter = FALSE;
	SaveBlock = TRUE;
	root = RecursiveErase<false>(root,key);
	VOID* const b = block;
	SaveBlock = FALSE;
	return b;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Rotate left
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> 
inline PDXTREE<DATA>::NODE* PDXTREE<DATA>::RotateLeft(NODE* const node)
{
	NODE* const tmp = node->right;
	node->right	= tmp->left;
	tmp->left = node;
	return tmp;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Rotate right
//////////////////////////////////////////////////////////////////////////////////////////////

template<class DATA> 
inline PDXTREE<DATA>::NODE* PDXTREE<DATA>::RotateRight(NODE* const node)
{
	NODE* const tmp = node->left;
	node->left = tmp->right;
	tmp->right = node;
	return tmp;
}
