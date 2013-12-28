////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
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

#include "NstStream.hpp"
#include "NstIps.hpp"

namespace Nes
{
	namespace Core
	{
		Ips::~Ips()
		{
			Destroy();
		}

		bool Ips::Load(StdStream const stdStream)
		{
			Destroy();

			try
			{
				Stream::In stream( stdStream );

				byte data[8];
				stream.Read( data, 5 );

				if
				(
					data[0] != Ascii<'P'>::V ||
					data[1] != Ascii<'A'>::V ||
					data[2] != Ascii<'T'>::V ||
					data[3] != Ascii<'C'>::V ||
					data[4] != Ascii<'H'>::V
				)
					return false;

				while (!stream.Eof())
				{
					stream.Read( data, 3 );

					if
					(
						data[0] == Ascii<'E'>::V &&
						data[1] == Ascii<'O'>::V &&
						data[2] == Ascii<'F'>::V
					)
						break;

					blocks.push_back(Block());
					Block& block = blocks.back();

					block.data = NULL;
					block.offset = dword(data[0]) << 16 | uint(data[1]) << 8 | data[2];

					stream.Read( data, 2 );
					block.length = uint(data[0]) << 8 | data[1];

					if (block.length)
					{
						block.fill = NO_FILL;
						block.data = new byte [block.length];
						stream.Read( block.data, block.length );
					}
					else
					{
						stream.Read( data, 2 );
						block.length = uint(data[0]) << 8 | data[1];

						if (block.length)
						{
							block.fill = stream.Read8();
						}
						else
						{
							Destroy();
							return false;
						}
					}
				}
			}
			catch (...)
			{
				Destroy();
				return false;
			}

			return true;
		}

		bool Ips::Save(StdStream const stdStream) const
		{
			try
			{
				Stream::Out stream( stdStream );

				byte data[8];

				data[0] = Ascii<'P'>::V;
				data[1] = Ascii<'A'>::V;
				data[2] = Ascii<'T'>::V;
				data[3] = Ascii<'C'>::V;
				data[4] = Ascii<'H'>::V;

				stream.Write( data, 5 );

				for (Blocks::const_iterator it(blocks.begin()), end(blocks.end()); it != end; ++it)
				{
					data[0] = it->offset >> 16 & 0xFF;
					data[1] = it->offset >>  8 & 0xFF;
					data[2] = it->offset >>  0 & 0xFF;

					stream.Write( data, 3 );

					if (it->fill != NO_FILL)
					{
						data[0] = 0;
						data[1] = 0;

						stream.Write( data, 2 );
					}

					data[0] = it->length >> 8 & 0xFF;
					data[1] = it->length >> 0 & 0xFF;

					stream.Write( data, 2 );

					if (it->fill == NO_FILL)
						stream.Write( it->data, it->length );
					else
						stream.Write8( it->fill );
				}

				data[0] = Ascii<'E'>::V;
				data[1] = Ascii<'O'>::V;
				data[2] = Ascii<'F'>::V;

				stream.Write( data, 3 );
			}
			catch (...)
			{
				return false;
			}

			return true;
		}

		bool Ips::Patch(void* const ptr,const dword length,const dword extraOffset) const
		{
			NST_ASSERT( !length || ptr );

			bool patched = false;

			if (length)
			{
				byte* const NST_RESTRICT data = static_cast<byte*>(ptr);

				for (Blocks::const_iterator it(blocks.begin()), end(blocks.end()); it != end; ++it)
				{
					NST_ASSERT( it->length );

					const dword offset = it->offset - extraOffset;

					if (offset < length)
					{
						if (const dword size = NST_MIN(it->length,length-offset))
						{
							patched = true;

							if (it->fill == NO_FILL)
								std::memcpy( data + offset, it->data, size );
							else
								std::memset( data + offset, it->fill, size );
						}
					}
				}
			}

			return patched;
		}

		bool Ips::Create(const void* srcPtr,const void* dstPtr,const dword length)
		{
			NST_ASSERT( !length || (srcPtr && dstPtr) );

			Destroy();

			try
			{
				const byte* const src = static_cast<const byte*>(srcPtr);
				const byte* const dst = static_cast<const byte*>(dstPtr);

				for (dword i=0; i < length; )
				{
					dword j = i++;

					if (src[j] == dst[j])
						continue;

					for (dword k=0; i < length; ++i)
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
						if (j == AsciiId<'F','O','E'>::V)
							--j;

						blocks.push_back(Block());
						Block& block = blocks.back();

						block.data = NULL;
						block.offset = j;

						uint c = dst[j];

						dword k = j;
						const dword stop = NST_MIN(j + MAX_BLOCK,i);

						while (++k != stop && c == dst[k]);

						if (k - j >= MIN_BEG_RUN)
						{
							block.fill = c;
							block.length = k - j;
						}
						else
						{
							dword l = k;

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

							if (k == AsciiId<'F','O','E'>::V)
								++k;

							block.fill = NO_FILL;
							block.length = k - j;

							block.data = new byte [block.length];
							std::memcpy( block.data, dst + j, block.length );
						}

						j = k;
					}
					while (j != i);
				}
			}
			catch (...)
			{
				Destroy();
				return false;
			}

			return true;
		}

		void Ips::Destroy()
		{
			for (Blocks::iterator it(blocks.begin()), end(blocks.end()); it != end; ++it)
				delete [] it->data;

			blocks.clear();
		}
	}
}
