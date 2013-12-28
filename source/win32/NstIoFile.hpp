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

#ifndef NST_IO_FILE_H
#define NST_IO_FILE_H

#pragma once

#include "resource/resource.h"
#include "NstString.hpp"
#include "NstObjectRaw.hpp"

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
			File(String::Generic,uint);

			~File();

			void Open(String::Generic,uint);
			void Close();
			uint Seek(Offset,int=0) const;
			void Truncate() const;
			void Truncate(uint) const;
			void Flush() const;
			uint Size() const;

			void Write (const void*,uint) const;
			void Read  (void*,uint) const;
			void Peek  (void*,uint) const;
			void Peek  (uint,void*,uint) const;

		private:

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

				const StreamProxy& operator << (const Object::ConstRaw& buffer) const
				{
					file.Write( buffer, buffer.Size() );
					return *this;
				}

				template<typename T> void operator >> (T& buffer) const
				{
					NST_COMPILE_ASSERT( sizeof(buffer[0]) == sizeof(char) );
					buffer.Resize( file.Size() - file.Position() );
					file.Read( buffer, buffer.Size() );
				}
			};

			class TextProxy
			{
				const File& file;

			public:

				TextProxy(const File& f)
				: file(f) {}

				const TextProxy& operator << (const String::Anything& string) const
				{
					file.Write( string, string.Size() );
					return *this;
				}

				template<typename T> void operator >> (T& string) const
				{
					NST_COMPILE_ASSERT( sizeof(string[0]) == 1 );
					string.Resize( file.Size() - file.Position() );
					file.Read( static_cast<char*>(string), string.Size() );
				}
			};

			void* handle;

		public:

			ibool IsOpen() const
			{
				return handle != reinterpret_cast<void*>(-1);
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

			TextProxy Text() const
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

			static ibool FileExist (cstring);
			static ibool DirExist  (cstring);
			static ibool Delete    (cstring);
		};
	}
}

#endif
