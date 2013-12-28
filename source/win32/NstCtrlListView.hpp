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
							
#ifndef NST_CTRL_LISTVIEW_H
#define NST_CTRL_LISTVIEW_H

#pragma once

#include "NstCtrlStandard.hpp"

namespace Nestopia
{
	namespace Window
	{
		namespace Control
		{
			class ListView : public Generic
			{
				class StyleExProxy
				{
					const Window::Generic control;

				public:

					StyleExProxy(Window::Generic w)
					: control(w) {}

					operator uint () const;
					void operator = (uint) const;
				};

				class ColumnsProxy
				{
					const Window::Generic control;

				public:

					ColumnsProxy(Window::Generic w)
					: control(w) {}

					void Insert(uint,cstring) const;
					void GetOrder(int*,uint) const;
					uint GetIndex(uint) const;
					void Align() const;
					void Clear() const;

					template<typename T,size_t N>
					void Set(const T (&list)[N]) const
					{
						Clear();

						for (uint i=0; i < N; ++i)
							Insert( i, list[i] );
					}
				};

				class Item
				{
					class TextProxy
					{
						typedef String::Smart<_MAX_PATH> Buffer; 

						const Item& item;
						const uint index;

						void GetText(Buffer&) const;

					public:

						TextProxy(const Item& i,uint s)
						: item(i), index(s) {}

						void operator << (cstring) const;

						template<typename T>
						uint operator >> (T& string) const
						{
							Buffer buffer;
							GetText( buffer );
							string = buffer;
							return string.Size();
						}
					};

					class DataProxy
					{
						const Item& item;

					public:

						DataProxy(const Item& i)
						: item(i) {}

						void operator = (LPARAM) const;
						operator LPARAM () const;

						operator void* () const
						{
							return reinterpret_cast<void*>( operator LPARAM() );
						}

						void operator = (const void* data) const
						{
							operator = (reinterpret_cast<LPARAM>(data));
						}
					};

					const Window::Generic control;
					const uint index;

				public:

					Item(Window::Generic w,uint i)
					: control(w), index(i) {}

					ibool Delete() const;
					void  Select(ibool=TRUE) const;
					void  Show() const;
					void  Check(ibool=TRUE) const;
					ibool IsChecked() const;

					uint GetIndex() const
					{
						return index;
					}

					TextProxy Text(uint i=0) const
					{
						return TextProxy( *this, i );
					}

					DataProxy Data() const
					{
						return *this;
					}
				};

				typedef Object::Delegate2<int,const void*,const void*> SortFunction;

				void Sort(const SortFunction&) const;

				static int CALLBACK SortProc(LPARAM,LPARAM,LPARAM);

			public:

				uint  Size() const;
				void  Clear() const;
				void  Reserve(uint) const;
				int   Add(String::Generic=String::Generic(),LPARAM=0,ibool=FALSE) const;
				Item  Selection() const;
				void  SetBkColor(uint) const;
				void  SetTextColor(uint) const;
				void  SetTextBkColor(uint) const;
				ibool HitTest(uint,uint) const;

				ListView(HWND hWnd=NULL)
				: Generic( hWnd ) {}

				ListView(HWND hWnd,uint id)
				: Generic( hWnd, id ) {}

				ColumnsProxy Columns() const
				{
					return control;
				}

				StyleExProxy StyleEx() const
				{
					return control;
				}

				Item operator [] (uint index) const
				{
					return Item( control, index );
				}

				template<typename Data,typename Code>
				void Sort(Data* data,Code code) const
				{
					Sort( SortFunction(data,code) );
				}

				int Add(String::Generic name,const void* const data,const ibool checked=FALSE) const
				{
					return Add( name, reinterpret_cast<LPARAM>(data), checked );
				}
			};
		}
	}
}

#endif
