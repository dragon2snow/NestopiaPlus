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

#include "NstIoFile.hpp"
#include "NstIoStream.hpp"

namespace Nestopia
{
	namespace Io
	{
		Stream::Input::Buffer::Buffer()
		: pos(0)
		{
			Initialize();
		}

		Stream::Input::Buffer::Buffer(const File& file)
		{
			Initialize();
			Initialize( file );
		}

		Stream::Input::Buffer::Buffer(Collection::Buffer& buffer)
		{
			Initialize();
			Initialize( buffer );
		}

		inline void Stream::Input::Buffer::operator = (const File& file)
		{
			Clear();
			Initialize( file );
		}

		inline void Stream::Input::Buffer::operator = (Collection::Buffer& buffer)
		{
			Clear();
			Initialize( buffer );
		}

		void Stream::Input::Buffer::Initialize()
		{
			setg( NULL, NULL, NULL );
		}

		void Stream::Input::Buffer::Initialize(Collection::Buffer& buffer)
		{
			pos = 0;
			vector.Import( buffer );
		}

		void Stream::Input::Buffer::Initialize(const File& file)
		{
			try
			{
				pos = 0;
				vector.Resize( file.Size() );
				file.Peek( 0, vector.Ptr(), vector.Size() );
			}
			catch (File::Exception)
			{
				// I/O failure
			}
		}

		void Stream::Input::Buffer::Export(Collection::Buffer& buffer)
		{
			pos = 0;
			buffer.Import( vector );
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		Stream::Input::Buffer::int_type Stream::Input::Buffer::underflow()
		{
			NST_ASSERT( pos <= vector.Size() );
			return pos < vector.Size() ? vector[pos] : traits_type::eof();
		}

		Stream::Input::Buffer::int_type Stream::Input::Buffer::uflow()
		{
			NST_ASSERT( pos <= vector.Size() );
			return pos < vector.Size() ? vector[pos++] : traits_type::eof();
		}

		std::streamsize Stream::Input::Buffer::xsgetn(char* output,std::streamsize count)
		{
			NST_ASSERT( pos <= vector.Size() );

			if (pos + count <= vector.Size())
			{
				std::memcpy( output, vector.Ptr() + pos, count );
				pos += count;
				return count;
			}

			return 0;
		}

		std::streampos Stream::Input::Buffer::seekoff
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
				offset += (long) vector.Size();
			}

			pos = offset;

			NST_ASSERT( pos <= vector.Size() );

			return offset;
		}

		std::streampos Stream::Input::Buffer::seekpos(std::streampos offset,std::ios::openmode mode)
		{
			NST_ASSERT( (mode == std::ios::in) && (int(offset) >= 0 && uint(offset) <= vector.Size()) );
			pos = offset;
			return offset;
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Stream::Input::Buffer::Clear()
		{
			pos = 0;
			vector.Destroy();
		}

		Stream::Input::Input()
		: std::istream(&buffer) {}

		Stream::Input::Input(const File& file)
		: std::istream(&buffer), buffer(file) {}

		Stream::Input::Input(Collection::Buffer& input)
		: std::istream(&buffer), buffer(input) {}

		Stream::Input& Stream::Input::operator = (const File& file)
		{
			buffer = file;
			return *this;
		}

		Stream::Input& Stream::Input::operator = (Collection::Buffer& input)
		{
			buffer = input;
			return *this;
		}

		Stream::Output::Buffer::Buffer()
		: pos(0)
		{
			Initialize();
		}

		Stream::Output::Buffer::Buffer(const File& file)
		{
			Initialize();
			Initialize( file );
		}

		Stream::Output::Buffer::Buffer(Collection::Buffer& buffer)
		{
			Initialize();
			Initialize( buffer );
		}

		inline void Stream::Output::Buffer::operator = (const File& file)
		{
			Initialize( file );
		}

		inline void Stream::Output::Buffer::operator = (Collection::Buffer& buffer)
		{
			Initialize( buffer );
		}

		void Stream::Output::Buffer::Initialize()
		{
			setp( NULL, NULL );
		}

		void Stream::Output::Buffer::Initialize(Collection::Buffer& buffer)
		{
			pos = 0;
			vector.Import( buffer );
		}

		void Stream::Output::Buffer::Initialize(const File& file)
		{
			try
			{
				pos = 0;
				vector.Resize( file.Size() );
				file.Peek( 0, vector.Ptr(), vector.Size() );
			}
			catch (File::Exception)
			{
				vector.Clear();
			}
		}

		void Stream::Output::Buffer::Export(Collection::Buffer& input)
		{
			vector.ShrinkTo( pos );
			pos = 0;
			input.Import( vector );
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		Stream::Output::Buffer::int_type Stream::Output::Buffer::overflow(int_type c)
		{
			if (c != traits_type::eof())
			{
				vector.PushBack( char(c) );
				c = traits_type::not_eof(c);
			}

			return c;
		}

		std::streamsize Stream::Output::Buffer::xsputn(const char* data,std::streamsize count)
		{
			if (pos + count > vector.Size())
			{
				vector.Reserve( (pos + count) * 2 );
				vector.ShrinkTo( pos + count );
			}

			std::memcpy( vector.Ptr() + pos, data, count );
			pos += count;

			return count;
		}

		std::streampos Stream::Output::Buffer::seekoff
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

		std::streampos Stream::Output::Buffer::seekpos(std::streampos p,std::ios::openmode mode)
		{
			NST_ASSERT( (mode == std::ios::out) && (int(p) >= 0 && uint(p) <= vector.Size()) );
			pos = p;
			return p;
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		Stream::Output::Output()
		: std::ostream(&buffer) {}

		Stream::Output::Output(Collection::Buffer& vector)
		: std::ostream(&buffer), buffer(vector) {}

		Stream::Output::Output(const File& file)
		: std::ostream(&buffer), buffer(file) {}

		Stream::Output& Stream::Output::operator = (const File& file)
		{
			buffer = file;
			return *this;
		}

		Stream::Output& Stream::Output::operator = (Collection::Buffer& input)
		{
			buffer = input;
			return *this;
		}
	}
}
