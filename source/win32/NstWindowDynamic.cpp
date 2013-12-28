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
#include "NstWindowParam.hpp"
#include "NstWindowDynamic.hpp"

namespace Nestopia
{
	using namespace Window;

	Dynamic::Instances Dynamic::instances;

	Dynamic::Dynamic()
	{
		Initialize();
	}

	Dynamic::~Dynamic()
	{
		Destroy();
	}

	void Dynamic::Initialize()
	{
		Messages().Add( WM_NCDESTROY, this, &Dynamic::OnNcDestroy );
	}

	void Dynamic::Create(Context& create)
	{
		Generic::Register
		(
			create.className,
			create.classStyle,
			WndProcCreate,
			create.hCursor,
			create.hBackground,
			create.hIcon
		);

		className = create.className;

		Generic::Create
		(
			create.exStyle,
			create.className,
			create.windowName,
			create.winStyle,
			create.x,
			create.y,
			create.width,
			create.height,
			create.hParent,
			create.hMenu,
			this
		);

		if (hWnd == NULL)
			throw Application::Exception(_T("CreateWindowEx() failed!"));
	}

	void Dynamic::Destroy()
	{
		Custom::Destroy();

		if (className.Length())
		{
			Unregister( className.Ptr() );
			className.Clear();
		}
	}

	void Dynamic::OnCreate(HWND const handle)
	{
		NST_ASSERT( handle && !hWnd );

		hWnd = handle;
		instances.PushBack( this );

		const LONG_PTR ptr = reinterpret_cast<LONG_PTR>( instances.Size() == 1 ? WndProcSingle : WndProcMulti );

		if (!::SetWindowLongPtr( hWnd, GWLP_WNDPROC, ptr ))
			throw Application::Exception(_T("SetWindowLongPtr() failed!"));
	}

	ibool Dynamic::OnNcDestroy(Param&)
	{
		NST_ASSERT( hWnd );

		Instances::Iterator instance;

		for (instance = instances.Ptr(); (*instance)->hWnd != hWnd; ++instance);

		instances.Erase( instance );
		hWnd = NULL;

		return false;
	}

	#ifdef NST_PRAGMA_OPTIMIZE
	#pragma optimize("t", on)
	#endif

	LRESULT CALLBACK Dynamic::WndProcSingle(HWND hWnd,uint uMsg,WPARAM wParam,LPARAM lParam)
	{
		if (const MsgHandler::Item* const item = instances.Front()->msgHandler( uMsg ))
		{
			Param param = {wParam,lParam,0,hWnd};

			if (item->value( param ))
				return param.lResult;
		}

		return ::DefWindowProc( hWnd, uMsg, wParam, lParam );
	}

	LRESULT CALLBACK Dynamic::WndProcMulti(HWND hWnd,uint uMsg,WPARAM wParam,LPARAM lParam)
	{
		Instances::ConstIterator instance;

		for (instance = instances.Ptr(); (*instance)->hWnd != hWnd; ++instance);

		if (const MsgHandler::Item* const item = (*instance)->msgHandler( uMsg ))
		{
			Param param = {wParam,lParam,0,hWnd};

			if (item->value( param ))
				return param.lResult;
		}

		return ::DefWindowProc( hWnd, uMsg, wParam, lParam );
	}

	#ifdef NST_PRAGMA_OPTIMIZE
	#pragma optimize("", on)
	#endif

	LRESULT CALLBACK Dynamic::WndProcCreate(HWND hWnd,uint uMsg,WPARAM wParam,LPARAM lParam)
	{
		if (uMsg == WM_CREATE && lParam)
		{
			if (Dynamic* const instance = static_cast<Dynamic*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams))
			{
				instance->OnCreate( hWnd );

				if (const MsgHandler::Item* const item = instance->msgHandler( uMsg ))
				{
					Param param = {wParam,lParam,0,hWnd};

					if (item->value( param ))
						return param.lResult;
				}
			}
		}

		return ::DefWindowProc( hWnd, uMsg, wParam, lParam );
	}
}
