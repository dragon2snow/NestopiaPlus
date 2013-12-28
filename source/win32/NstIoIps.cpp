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

#include "NstObjectBackup.hpp"
#include "NstIoIps.hpp"

namespace Nestopia
{
	namespace Io
	{
		Ips::Blocks::~Blocks()
		{
			for (Iterator it(Begin()), end(End()); it != end; ++it)
				delete [] it->data;
		}

		bool Ips::Create(const void* const source,const void* const target,const uint length,PatchData& patch)
		{
			NST_ASSERT( length && length <= MAX_LENGTH );

			struct Stream
			{
				static void Write(PatchData& patch,const uchar* data,uint length)
				{
					NST_ASSERT( length && length <= 0xFFFF );
					patch.Append( data, length );
				}

				static void Write3(PatchData& patch,const uint value)
				{
					const uint pos = patch.Size();

					patch.Grow( 3 );

					patch[pos+0] = value >> 16;
					patch[pos+1] = value >> 8 & 0xFF;
					patch[pos+2] = value >> 0 & 0xFF;
				}

				static void Write2(PatchData& patch,const uint value)
				{
					const uint pos = patch.Size();

					patch.Grow( 2 );

					patch[pos+0] = value >> 8;
					patch[pos+1] = value >> 0 & 0xFF;
				}

				static void Write1(PatchData& patch,const uint value)
				{
					patch.PushBack( uchar(value) );
				}
			};

			const uint offset = patch.Size();
			patch.Reserve( offset + length );

			Stream::Write3( patch, DATA_ID1 );
			Stream::Write2( patch, DATA_ID2 );

			const uchar* src = static_cast<const uchar*>(source);
			const uchar* dst = static_cast<const uchar*>(target);

			for (uint i=0; i < length; )
			{
				if (src[i] == dst[i])
				{
					++i;
					continue;
				}

				uint j = i++;

				for (uint k=0; i < length; ++i)
				{
					if (src[i] != dst[i])
					{
						k = 0;
					}
					else if (k++ == MIN_EQUAL)
					{
						i -= MIN_EQUAL;
						break;
					}
				}

				do
				{
					if (j == DATA_EOF)
						--j;

					Stream::Write3( patch, j );

					uchar c = dst[j];

					uint k = j;
					const uint stop = NST_MIN(j + MAX_BLOCK,i);

					while (++k != stop && c == dst[k]);

					if (k - j >= MIN_BEG_RUN)
					{
						Stream::Write2( patch, 0     );
						Stream::Write2( patch, k - j );
						Stream::Write1( patch, c     );
					}
					else
					{
						uint l = k;

						if (k + 1 < stop)
						{
							c = dst[k];

							for (l=k++; k < stop; ++k)
							{
								if (c != dst[k])
								{
									c = dst[k];
									l = k;
								}
								else if (k - l == MIN_MID_RUN)
								{
									k = l;
									break;
								}
							}
						}

						if (k == stop && k - l >= MIN_END_RUN)
							k = l;

						if (k == DATA_EOF)
							++k;

						Stream::Write2( patch, k - j  );
						Stream::Write( patch, dst + j, k - j );
					}

					j = k;
				}
				while (j != i);
			}

			Stream::Write3( patch, DATA_EOF );

			return patch.Size() > (offset + 3+2+3);
		}

		void Ips::Parse(const void* const source,const uint size)
		{
			class Stream
			{
				const uchar* data;
				const uchar* const end;

				void Check(const uint length) const
				{
					if (data + length > end)
						throw ERR_CORRUPT;
				}

			public:

				Stream(const void* source,uint size)
				:
				data (static_cast<const uchar*>(source)),
				end  (static_cast<const uchar*>(source) + size)
				{}

				const uchar* Read(const uint length)
				{
					Check( length );

					const uchar* ptr = data;
					data += length;

					return ptr;
				}

				uint Read1()
				{
					Check( 1 );
					return *data++;
				}

				uint Read2()
				{
					Check( 2 );

					const uint value = uint(data[0]) << 8 | data[1];
					data += 2;

					return value;
				}

				uint Read3()
				{
					Check( 3 );

					const uint value = uint(data[0]) << 16 | uint(data[1]) << 8 | data[2];
					data += 3;

					return value;
				}

				operator bool () const
				{
					return data != end;
				}
			};

			if (!size)
				throw ERR_EMPTY;

			blocks.Clear();

			Stream stream( source, size );

			if (stream.Read3() != DATA_ID1 || stream.Read2() != DATA_ID2)
				throw ERR_CORRUPT;

			Block block;

			while (stream)
			{
				block.offset = stream.Read3();

				if (block.offset == DATA_EOF)
					break;

				if ((block.length = stream.Read2()) != 0)
				{
					block.fill = UINT_MAX;
					const uchar* const data = stream.Read( block.length );
					std::memcpy( block.data = new uchar [block.length], data, block.length );
				}
				else if ((block.length = stream.Read2()) != 0)
				{
					block.fill = stream.Read1();
					block.data = NULL;
				}
				else
				{
					throw ERR_CORRUPT;
				}

				blocks.PushBack( block );
			}

			if (blocks.Empty())
				throw ERR_EMPTY;
		}

		void Ips::Patch(void* const target,const uint size) const
		{
			NST_ASSERT( target && size );

			Object::Backup backup( target, size );

			for (Blocks::ConstIterator it(blocks.Begin()), end(blocks.End()); it != end; ++it)
			{
				NST_ASSERT( it->length );

				if (it->offset + it->length <= size)
				{
					if (it->fill == UINT_MAX)
						std::memcpy( static_cast<uchar*>(target) + it->offset, it->data, it->length );
					else
						std::memset( static_cast<uchar*>(target) + it->offset, it->fill, it->length );
				}
				else
				{
					backup.Restore();
					throw ERR_CORRUPT;
				}
			}
		}
	}
}
