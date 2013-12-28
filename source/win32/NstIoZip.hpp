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

#ifndef NST_IO_ZIP_H
#define NST_IO_ZIP_H

#pragma once

#include <vector>
#include "NstObjectHeap.hpp"
#include "NstObjectRaw.hpp"
#include "NstString.hpp"

struct zlib_filefunc_def_s;

namespace Nestopia
{
	namespace Io
	{
		class File;

		class Zip
		{
		public:

			Zip();	
			explicit Zip(const File&);
			explicit Zip(const Object::ConstRaw);
			~Zip();

			ibool Open(const File&);
			ibool Open(const Object::ConstRaw);
			void  Close();
			uint  Find(String::Generic) const;

			enum 
			{
				NO_SELECTION = INT_MAX,
				NO_FILES = INT_MAX-1
			};

			uint UserSelect() const;
			uint UserSelect(const String::Generic*,uint) const;

		private:

			enum Exception 
			{
				ERR_ZIP
			};

			struct Stream
			{
				cstring begin;
				cstring pos;
				cstring end;
			};

			class Item
			{
				friend class Zip;

				inline Item
				(
					Zip*,
					cstring,
					uint,
					uint,
					uint
				);

				Zip* zip;
				String::Path<false> name;
				uint compressedSize;
				uint uncompressedSize;
				uint index;

			public:

				uint Uncompress(void*) const;

				uint CompressedSize() const
				{
					return compressedSize;
				}

				uint UncompressedSize() const
				{
					return uncompressedSize;
				}

				const String::Path<false>& GetName() const
				{
					return name;
				}
			};

			ibool Open();

			class Gui;

			typedef std::vector<Item> Items;

			void* unzip;
			Object::Heap<zlib_filefunc_def_s> callbacks;
			Items files;
			Stream stream;

		public:

			uint NumFiles() const
			{
				return files.size();
			}

			const Item& operator [] (uint i) const
			{
				return files[i];
			}
		};
	}
}

#endif
