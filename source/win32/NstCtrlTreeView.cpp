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

#include "NstApplicationException.hpp"
#include "NstResourceBitmap.hpp"
#include "NstWindowCustom.hpp"
#include "NstCtrlTreeView.hpp"
#include <CommCtrl.h>

namespace Nestopia
{
	using Window::Control::TreeView;

	TreeView::ImageList::ImageList(const uint x,const uint y,const uint selected,const uint unselected)
	: handle(ImageList_Create(x,y,ILC_COLOR16,0,2))
	{
		if (!handle)
			throw Application::Exception(_T("ImageList_Create() failed!"));

		try
		{
			if
			(
				ImageList_Add( static_cast<HIMAGELIST>(handle), Resource::Bitmap( selected   ), NULL ) == -1 ||
				ImageList_Add( static_cast<HIMAGELIST>(handle), Resource::Bitmap( unselected ), NULL ) == -1
			)
				throw Application::Exception(_T("ImageList_Add() failed!"));
		}
		catch (const Application::Exception& exception)
		{
			ImageList_Destroy( static_cast<HIMAGELIST>(handle) );
			throw exception;
		}
	}

	TreeView::ImageList::~ImageList()
	{
		if (handle)
			ImageList_Destroy( static_cast<HIMAGELIST>(handle) );
	}

	void TreeView::Item::Select() const
	{
		NST_VERIFY( hItem );
		TreeView_SelectItem( root, static_cast<HTREEITEM>(hItem) );
	}

	void TreeView::SetImageList(const ImageList& imageList) const
	{
		NST_VERIFY( imageList.handle );
		TreeView_SetImageList( control, static_cast<HIMAGELIST>(imageList.handle), TVSIL_NORMAL );
	}

	void TreeView::Add(const GenericString text) const
	{
		TVINSERTSTRUCT tv;

		tv.hParent = TVI_ROOT;
		tv.hInsertAfter = TVI_LAST;
		tv.item.mask = TVIF_TEXT;
		tv.item.pszText = const_cast<tchar*>( text.Ptr() );
		tv.item.cchTextMax = text.Length();

		if (TreeView_GetImageList( control, TVSIL_NORMAL ))
		{
			tv.item.mask = TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
			tv.item.iImage = ImageList::UNSELECTED;
			tv.item.iSelectedImage = ImageList::SELECTED;
		}

		TreeView_InsertItem( control, &tv );
	}

	TreeView::Item TreeView::operator [] (const uint index) const
	{
		HTREEITEM hItem = TreeView_GetRoot( control );

		for (uint i=index; i; --i)
			hItem = TreeView_GetNextSibling( control, hItem );

		return Item( control, hItem, hItem ? index : -1 );
	}

	int TreeView::GetIndex(void* const handle) const
	{
		int index = -1;

		if (handle)
		{
			index = 0;

			for (HTREEITEM hItem = TreeView_GetRoot( control ); hItem && hItem != static_cast<HTREEITEM>(handle); ++index)
				hItem = TreeView_GetNextSibling( control, hItem );
		}

		return index;
	}

	TreeView::Item TreeView::Selection() const
	{
		HTREEITEM const hSelection = TreeView_GetSelection( control );
		return Item( control, hSelection, GetIndex( hSelection ) );
	}

	void TreeView::SetBackgroundColor(const uint color) const
	{
		TreeView_SetBkColor( control, color );
	}

	void TreeView::SetTextColor(const uint color) const
	{
		TreeView_SetTextColor( control, color );
	}
}
