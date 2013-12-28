////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2005 Martin Freij
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

#include "NstObjectPod.hpp"
#include "NstApplicationInstance.hpp"
#include "NstDialogBrowse.hpp"
#include <Windows.h>
#include <CommDlg.h>
#include <ShlObj.h>

namespace Nestopia
{
	using namespace Window;

	const Browser::Path Browser::OpenFile(cstring const filter,const Path dir,const Path ext)
	{
		Path path;

		Object::Pod<OPENFILENAME> ofn;

		ofn.lStructSize     = sizeof(ofn);
		ofn.hwndOwner       = Application::Instance::GetActiveWindow();
		ofn.lpstrFile       = path;
		ofn.nMaxFile        = path.Capacity();
		ofn.lpstrInitialDir	= dir.Size() ? static_cast<cstring>(dir) : ".";
		ofn.Flags           = OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;

		if (filter)
		{
			ofn.lpstrFilter = filter;
			ofn.nFilterIndex = 1;
		}

		if (ext.Size())
			ofn.lpstrDefExt = ext;

		if (::GetOpenFileName( &ofn ))
			path.Validate();
		else
			path.Clear();

		return path;
	}

	const Browser::Path Browser::SaveFile(cstring const filter,const Path dir,const Path ext,Path path)
	{		
		Object::Pod<OPENFILENAME> ofn;

		ofn.lStructSize     = sizeof(ofn);
		ofn.hwndOwner       = Application::Instance::GetActiveWindow();
		ofn.lpstrFile       = path;
		ofn.nMaxFile        = path.Capacity();
		ofn.lpstrInitialDir	= dir.Size() ? static_cast<cstring>(dir) : ".";
		ofn.Flags           = OFN_EXPLORER|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY;

		if (filter)
		{
			ofn.lpstrFilter = filter;
			ofn.nFilterIndex = 1;
		}

		if (ext.Size())
			ofn.lpstrDefExt = ext;

		if (::GetSaveFileName( &ofn ))
			path.Validate();
		else
			path.Clear();

		return path;
	}

	const Browser::Path Browser::SelectDirectory()
	{
		Path path;

		Object::Pod<BROWSEINFO> bi;

		bi.hwndOwner	  = Application::Instance::GetActiveWindow();
		bi.pszDisplayName = path;
		bi.ulFlags		  = BIF_RETURNONLYFSDIRS;

		if (LPITEMIDLIST const idl = ::SHBrowseForFolder( &bi ))
		{
			if (::SHGetPathFromIDList( idl, path ) && path.Validate())
				path.Directory().Validate();
			else
				path.Clear();

			IMalloc* pMalloc;

			if (SUCCEEDED(::SHGetMalloc( &pMalloc ))) 
			{
				pMalloc->Free( idl );
				pMalloc->Release();
			}
		}
		else
		{
			path.Clear();
		}

		return path;
	}
}
