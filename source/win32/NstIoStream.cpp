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

#include "NstIoFile.hpp"
#include "NstIoStream.hpp"

namespace Nestopia
{
	using Io::Stream::Input;
	using Io::Stream::Output;
	using Io::File;

	Input::Buffer::Buffer()
	: pos(0), data(NULL), size(0)
	{
		Initialize();
	}

	Input::Buffer::Buffer(const File& file)
	{
		Initialize();
		Initialize( file );
	}

	Input::Buffer::Buffer(Collection::Buffer& buffer)
	{
		Initialize();
		Initialize( buffer );
	}

	Input::Buffer::~Buffer()
	{
		delete [] static_cast<char*>(data);
	}

	inline void Input::Buffer::operator = (const File& file)
	{
		Clear();
		Initialize( file );
	}

	inline void Input::Buffer::operator = (Collection::Buffer& buffer)
	{
		Clear();
		Initialize( buffer );
	}

	void Input::Buffer::Initialize()
	{
		setg( NULL, NULL, NULL );
	}
  
	void Input::Buffer::Initialize(Collection::Buffer& buffer)
	{
		pos = 0;
		size = buffer.Size();
		data = buffer.Export();
	}

	void Input::Buffer::Initialize(const File& file)
	{
		try
		{
			Collection::Buffer buffer( file.Size() );
			file.Peek( 0, buffer, buffer.Size() );
			Initialize( buffer );
		}
		catch (File::Exception)
		{		
			// I/O failure
		}
	}

	void Input::Buffer::Export(Collection::Buffer& vector)
	{
		pos = 0;
		vector.Import( data, size );
		data = NULL;
		size = 0;
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("t", on)
    #endif

	Input::Buffer::int_type Input::Buffer::underflow()
	{
		NST_ASSERT( pos <= size );
		return pos < size ? data[pos] : traits_type::eof();
	}

	Input::Buffer::int_type Input::Buffer::uflow()
	{	
		NST_ASSERT( pos <= size );
		return pos < size ? data[pos++] : traits_type::eof();
	}

	std::streamsize Input::Buffer::xsgetn(char* output,std::streamsize count)
	{
		NST_ASSERT( pos <= size );

		if (pos + count <= size)
		{
			std::memcpy( output, data + pos, count );
			pos += count;
			return count;
		}

		return 0;
	}

	std::streampos Input::Buffer::seekoff
	(
		std::streamoff offset,
		std::ios::seekdir dir,
		std::ios::openmode mode
	)
	{
		NST_ASSERT
		( 
			(mode == std::ios::in) &&
			(dir == std::ios::beg || dir == std::ios::cur || dir == std::ios::end)
		);

		if (dir == std::ios::cur)
		{
			offset += (long) pos;
		}
		else if (dir == std::ios::end)
		{
			offset += (long) size;
		}

		pos = offset;

		NST_ASSERT( pos <= size );

		return offset;
	}

	std::streampos Input::Buffer::seekpos(std::streampos offset,std::ios::openmode mode)
	{
		NST_ASSERT( (mode == std::ios::in) && (int(offset) >= 0 && uint(offset) <= size) );
		pos = offset;
		return offset;
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("", on)
    #endif

	void Input::Buffer::Clear()
	{
		delete [] data;
		data = NULL;
		size = 0;
		pos = 0;
	}
  
	Input::Input()
	: std::istream(&buffer) {}

	Input::Input(const File& file)
	: std::istream(&buffer), buffer(file) {}

	Input::Input(Collection::Buffer& input)
	: std::istream(&buffer), buffer(input) {}

	Input::~Input()
	{
	}

	Input& Input::operator = (const File& file)
	{
		buffer = file;
		return *this;
	}

	Input& Input::operator = (Collection::Buffer& input)
	{
		buffer = input;
		return *this;
	}

	Output::Buffer::Buffer()
	: pos(0)
	{
		Initialize();
	}

	Output::Buffer::Buffer(const File& file)
	{
		Initialize();
		Initialize( file );
	}

	Output::Buffer::Buffer(Collection::Buffer& buffer)
	{
		Initialize();
		Initialize( buffer );
	}

	inline void Output::Buffer::operator = (const File& file)
	{
		Initialize( file );
	}

	inline void Output::Buffer::operator = (Collection::Buffer& buffer)
	{
		Initialize( buffer );
	}

	void Output::Buffer::Initialize()
	{
		setp( NULL, NULL );
	}
  
	void Output::Buffer::Initialize(Collection::Buffer& buffer)
	{
		pos = 0;
		vector.Import( buffer );
	}

	void Output::Buffer::Initialize(const File& file)
	{
		try
		{
			pos = 0;
			vector.Resize( file.Size() );
			file.Peek( 0, vector, vector.Size() );
		}
		catch (File::Exception)
		{		
			vector.Clear();
		}
	}

	void Output::Buffer::Export(Collection::Buffer& input)
	{
		vector.ShrinkTo( pos );
		pos = 0;
		input.Import( vector );
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("t", on)
    #endif

	Output::Buffer::int_type Output::Buffer::overflow(int_type c)
	{
		if (c != traits_type::eof()) 
		{
			vector.PushBack( char(c) );
			c = traits_type::not_eof(c);
		}

		return c;
	}

	std::streamsize Output::Buffer::xsputn(const char* data,std::streamsize count) 	
	{
		if (pos + count > vector.Size())
		{
			vector.Reserve( (pos + count) * 2 );
			vector.ShrinkTo( pos + count );
		}

		std::memcpy( vector + pos, data, count );
		pos += count;

		return count;
	}

	std::streampos Output::Buffer::seekoff
	(
		std::streamoff offset,
		std::ios::seekdir dir,
		std::ios::openmode mode
	)
	{
		NST_ASSERT
		( 
			(mode == std::ios::out) &&
			(dir == std::ios::beg || dir == std::ios::cur || dir == std::ios::end)
		);

		if (dir == std::ios::cur)
		{
			offset += (long) pos;
		}
		else if (dir == std::ios::end)
		{
			offset += (long) vector.Size();
		}

		pos = offset;

		NST_ASSERT( pos <= vector.Size() );
	  
		return pos;
	}

	std::streampos Output::Buffer::seekpos(std::streampos p,std::ios::openmode mode)
	{
		NST_ASSERT( (mode == std::ios::out) && (int(p) >= 0 && uint(p) <= vector.Size()) );
		pos = p;  
		return p;
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("", on)
    #endif

	Output::Output()
	: std::ostream(&buffer)	{}

	Output::Output(Collection::Buffer& vector)
	: std::ostream(&buffer), buffer(vector)	{}

	Output::Output(const File& file)
	: std::ostream(&buffer), buffer(file) {}

	Output::~Output()
	{
	}

	Output& Output::operator = (const File& file)
	{
		buffer = file;
		return *this;
	}

	Output& Output::operator = (Collection::Buffer& input)
	{
		buffer = input;
		return *this;
	}
}
