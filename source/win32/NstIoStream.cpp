////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2007 Martin Freij
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

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		Stream::Input::Buffer::int_type Stream::Input::Buffer::underflow()
		{
			NST_VERIFY( pos <= vector.Size() );
			return pos < vector.Size() ? vector[pos] : traits_type::eof();
		}

		Stream::Input::Buffer::int_type Stream::Input::Buffer::uflow()
		{
			NST_VERIFY( pos <= vector.Size() );
			return pos < vector.Size() ? vector[pos++] : traits_type::eof();
		}

		std::streamsize Stream::Input::Buffer::xsgetn(char* output,std::streamsize count)
		{
			NST_ASSERT( count >= 0 );
			NST_VERIFY( count + pos <= vector.Size() + 1 );

			if (count + pos > vector.Size())
				count = pos < vector.Size() ? vector.Size() - pos : 0;

			std::memcpy( output, vector.Ptr() + pos, count );
			pos += count;

			return count;
		}

	#if NST_MSVC >= 1400

		std::streamsize Stream::Input::Buffer::_Xsgetn_s(char* output,std::size_t,std::streamsize count)
		{
			return Buffer::xsgetn( output, count );
		}

	#endif

		std::streampos Stream::Input::Buffer::seekoff(std::streamoff off,std::ios::seekdir dir,std::ios::openmode mode)
		{
			NST_ASSERT( (mode == std::ios::in) && (dir == std::ios::beg || dir == std::ios::cur || dir == std::ios::end) );

			if (dir == std::ios::cur)
			{
				off += int(pos);
			}
			else if (dir == std::ios::end)
			{
				off += int(vector.Size());
			}

			NST_VERIFY( off >= 0 && off <= vector.Size() );

			if (off >= 0 && off <= vector.Size())
			{
				pos = off;
				return off;
			}
			else
			{
				pos = BAD_POS;
				return std::streamoff(-1);
			}
		}

		std::streampos Stream::Input::Buffer::seekpos(std::streampos p,std::ios::openmode mode)
		{
			const std::streamoff off(p);

			NST_ASSERT( mode == std::ios::out && off >= 0 );
			NST_VERIFY( vector.Size() >= off );

			if (vector.Size() >= off)
			{
				pos = off;
				return off;
			}
			else
			{
				pos = BAD_POS;
				return std::streamoff(-1);
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
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

		Stream::Input::~Input()
		{
		}

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
			NST_VERIFY( pos != BAD_POS );

			vector.SetTo( pos == BAD_POS ? 0 : pos );
			pos = 0;
			input.Import( vector );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		Stream::Output::Buffer::int_type Stream::Output::Buffer::overflow(int_type c)
		{
			if (!traits_type::eq_int_type(traits_type::eof(),c))
			{
				if (vector.Size() > pos)
				{
					vector[pos++] = c;
				}
				else if (vector.Size() == pos)
				{
					++pos;
					vector.PushBack( c );
				}
				else
				{
					c = traits_type::eof();
				}
			}

			return c;
		}

		std::streamsize Stream::Output::Buffer::xsputn(const char* input,std::streamsize count)
		{
			NST_ASSERT( count >= 0 );
			NST_VERIFY( pos != BAD_POS );

			if (pos != BAD_POS)
			{
				const uint cur = pos;
				pos += count;

				if (pos > vector.Capacity())
					vector.Reserve( pos * 2 );

				if (pos > vector.Size())
					vector.SetTo( pos );

				std::memcpy( vector.Ptr() + cur, input, count );
			}
			else
			{
				count = 0;
			}

			return count;
		}

		std::streampos Stream::Output::Buffer::seekoff(std::streamoff off,std::ios::seekdir dir,std::ios::openmode mode)
		{
			NST_ASSERT( (mode == std::ios::out) && (dir == std::ios::beg || dir == std::ios::cur || dir == std::ios::end) );

			if (dir == std::ios::cur)
			{
				off += int(pos);
			}
			else if (dir == std::ios::end)
			{
				off += int(vector.Size());
			}

			NST_VERIFY( off >= 0 && off <= vector.Size() );

			if (off >= 0 && off <= vector.Size())
			{
				pos = off;
				return off;
			}
			else
			{
				pos = BAD_POS;
				return std::streamoff(-1);
			}
		}

		std::streampos Stream::Output::Buffer::seekpos(std::streampos p,std::ios::openmode mode)
		{
			const std::streamoff off(p);

			NST_ASSERT( mode == std::ios::out && off >= 0 );
			NST_VERIFY( vector.Size() >= off );

			if (vector.Size() >= off)
			{
				pos = off;
				return off;
			}
			else
			{
				pos = BAD_POS;
				return std::streamoff(-1);
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		Stream::Output::Output()
		: std::ostream(&buffer) {}

		Stream::Output::Output(Collection::Buffer& vector)
		: std::ostream(&buffer), buffer(vector) {}

		Stream::Output::Output(const File& file)
		: std::ostream(&buffer), buffer(file) {}

		Stream::Output::~Output()
		{
		}

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
