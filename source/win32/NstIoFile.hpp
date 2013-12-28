////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
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

#ifndef NST_IO_FILE_H
#define NST_IO_FILE_H

#pragma once

#include "resource/resource.h"
#include "NstString.hpp"

namespace Nestopia
{
	namespace Io
	{
		class File : Sealed
		{
		public:

			enum
			{
				READ = 0x01,
				WRITE = 0x02,
				EMPTY = 0x04,
				EXISTING = 0x08,
				SEQUENTIAL_ACCESS = 0x10,
				RANDOM_ACCESS = 0x20,
				COLLECT = READ|EXISTING|SEQUENTIAL_ACCESS,
				DUMP = WRITE|EMPTY|SEQUENTIAL_ACCESS
			};

			enum
			{
				UTF16_LE = 0xFEFF,
				UTF16_BE = 0xFFFE
			};

			enum
			{
				MAX_SIZE = 0x7FFFFFFF
			};

			enum Offset
			{
				BEGIN,
				CURRENT,
				END
			};

			enum Exception
			{
				ERR_READ = IDS_FILE_ERR_READ,
				ERR_WRITE = IDS_FILE_ERR_WRITE,
				ERR_SEEK = IDS_FILE_ERR_READ,
				ERR_OPEN = IDS_FILE_ERR_OPEN,
				ERR_NOT_FOUND = IDS_FILE_ERR_NOTFOUND,
				ERR_ALREADY_EXISTS = IDS_FILE_ERR_ALREADYEXISTS,
				ERR_TOO_BIG = IDS_FILE_ERR_TOOBIG
			};

			File();
			File(GenericString,uint);

			~File();

			void Open(GenericString,uint);
			void Close();
			uint Seek(Offset,int=0) const;
			void Truncate() const;
			void Truncate(uint) const;
			void Flush() const;
			uint Size() const;

			void Write    (const void*,uint) const;
			void Read     (void*,uint) const;
			uint ReadSome (void*,uint) const;
			void Peek     (void*,uint) const;
			void Peek     (uint,void*,uint) const;

			void ReadText (String::Heap<char>&,uint=UINT_MAX) const;
			void ReadText (String::Heap<wchar_t>&,uint=UINT_MAX) const;
			void WriteText (cstring,uint,ibool=FALSE) const;
			void WriteText (wstring,uint,ibool=FALSE) const;

			static void ParseText (const void*,uint,String::Heap<char>&);
			static void ParseText (const void*,uint,String::Heap<wchar_t>&);

		private:

			static void EndianSwap(void*,void*);

			class Proxy
			{
				const File& file;

			public:

				Proxy(const File& file)
				: file(file) {}

				void operator = (int pos) const
				{
					file.Seek( BEGIN, pos );
				}

				void operator += (int count) const
				{
					file.Seek( CURRENT, count );
				}

				void operator -= (int count) const
				{
					file.Seek( CURRENT, -count );
				}

				operator uint () const
				{
					return file.Seek( CURRENT );
				}
			};

			class StreamProxy
			{
				const File& file;

			public:

				StreamProxy(const File& f)
				: file(f) {}

				template<typename T>
				const StreamProxy& operator << (const T& buffer) const
				{
					file.Write( buffer.Ptr(), buffer.Length() * sizeof(buffer[0]) );
					return *this;
				}

				template<typename T> void operator >> (T& buffer) const
				{
					NST_COMPILE_ASSERT( sizeof(buffer[0]) == sizeof(char) );
					buffer.Resize( file.Size() - file.Position() );
					file.Read( buffer.Ptr(), buffer.Length() );
				}
			};

			void* handle;
			Path name;

		public:

			ibool IsOpen() const
			{
				return handle != reinterpret_cast<void*>(-1);
			}

			const Path& GetName() const
			{
				return name;
			}

			template<typename T>
			const File& operator << (const T& t) const
			{
				Write( &t, sizeof(t) );
				return *this;
			}

			template<typename T>
			const File& operator >> (T& t) const
			{
				Read( &t, sizeof(t) );
				return *this;
			}

			template<typename T>
			const File& Peek(T& t) const
			{
				Peek( &t, sizeof(t) );
				return *this;
			}

			template<typename T> T Read() const
			{
				T t;
				Read( &t, sizeof(t) );
				return t;
			}

			template<typename T> T Peek() const
			{
				T t;
				Peek( &t, sizeof(t) );
				return t;
			}

			StreamProxy Stream() const
			{
				return *this;
			}

			Proxy Position() const
			{
				return *this;
			}

			void Rewind() const
			{
				Seek( BEGIN );
			}

			static ibool FileExist     (tstring);
			static ibool DirExist      (tstring);
			static ibool Delete        (tstring);
			static ibool FileProtected (tstring);
		};
	}
}

#endif
