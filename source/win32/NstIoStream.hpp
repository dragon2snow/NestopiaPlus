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

#ifndef NST_IO_STREAM_H
#define NST_IO_STREAM_H

#pragma once

#include <iostream>
#include "NstCollectionVector.hpp"

namespace Nestopia
{
	namespace Io
	{
		class File;

		namespace Stream
		{
			class Output;

			class Input : public std::istream
			{
			public:

				Input();
				explicit Input(const File&);
				explicit Input(Collection::Buffer&); // invalidates input vector
				~Input();

				Input& operator = (const File&);
				Input& operator = (Collection::Buffer&); // invalidates input vector

			private:

				class Buffer : public std::streambuf
				{
				public:

					Buffer();
					explicit Buffer(const File&);
					explicit Buffer(Collection::Buffer&);
					~Buffer();

					inline void operator = (const File&);
					inline void operator = (Collection::Buffer&);

					void Export(Collection::Buffer&); // invalidates stream buffer

				private:

					void Clear();
					void Initialize ();
					void Initialize (const File&);
					void Initialize (Collection::Buffer&);

					uint pos;
					char* data;
					uint size;

					int_type underflow();
					int_type uflow();
					std::streamsize xsgetn(char*,std::streamsize); 
					std::streampos seekoff(std::streamoff,std::ios::seekdir,std::ios::openmode);
					std::streampos seekpos(std::streampos,std::ios::openmode);
				};

				Buffer buffer;

			public:

				void Export(Collection::Buffer& vector)
				{
					buffer.Export( vector );
				}
			};

			class Output : public std::ostream
			{
			public:

				Output();
				explicit Output(const File&);
				explicit Output(Collection::Buffer&); // Invalidates input vector
				~Output();

				Output& operator = (const File&);
				Output& operator = (Collection::Buffer&); // invalidates input vector

			private:

				class Buffer : public std::streambuf
				{
				public:

					Buffer();
					explicit Buffer(const File&);
					explicit Buffer(Collection::Buffer&);

					inline void operator = (const File&);
					inline void operator = (Collection::Buffer&);

					void Export(Collection::Buffer&); // invalidates stream buffer

				private:

					void Initialize ();
					void Initialize (const File&);
					void Initialize (Collection::Buffer&);

					int_type overflow(int_type);
					std::streamsize xsputn(const char*,std::streamsize); 
					std::streampos seekoff(std::streamoff,std::ios::seekdir,std::ios::openmode);
					std::streampos seekpos(std::streampos,std::ios::openmode);
	  
					uint pos;
					Collection::Buffer vector;
				};

				Buffer buffer;

			public:

				void Export(Collection::Buffer& vector)
				{
					buffer.Export( vector );
				}
			};
		}
	}
}

#endif
