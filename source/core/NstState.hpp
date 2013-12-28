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

#ifndef NST_STATE_H
#define NST_STATE_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "NstCore.hpp"
#include "NstVector.hpp"
#include "NstStream.hpp"

#define NES_STATE_CHUNK_ID(a_,b_,c_,d_) u32( (a_) | ((b_) << 8) | ((c_) << 16) | ((d_) << 24) )

namespace Nes
{
	namespace Core
	{
		namespace State
		{
			enum
			{
				MIN_CHUNK_SIZE = 4 + 4
			};

			class Saver
			{
			public:

				Saver(StdStream,bool);
				~Saver();
				
				Saver& Begin(dword);
				Saver& End();
				Saver& Write8(uint);
				Saver& Write16(uint);
				Saver& Write32(dword);
				Saver& Write(const void*,dword);
				Saver& Compress(const u8*,dword);

				class Subset
				{
					Saver& saver;

				public:

					Subset(Saver& s,char a,char b,char c,char d)
					: saver(s)
					{
						s.Begin( NES_STATE_CHUNK_ID(a,b,c,d) );
					}

					Subset(Saver& s,dword id)
					: saver(s)
					{
						s.Begin( id );
					}

					Saver& Ref() const
					{
						return saver;
					}

					~Subset()
					{
						saver.End();
					}
				};

			private:

            #ifndef NDEBUG

				Saver(const Saver&);

            #endif

				Stream::Out stream;
				Vector<u32> chunks;
				const ibool useCompression;

			public:

				Saver& Begin(char a,char b,char c,char d)
				{
					return Begin( NES_STATE_CHUNK_ID(a,b,c,d) );
				}

				template<size_t N>
				Saver& Write(const u8 (&data)[N])
				{
					return Write( data, N );
				}

				template<size_t N>
				Saver& Compress(const u8 (&data)[N])
				{
					return Compress( data, N );
				}

				Stream::Out& GetStream()
				{
					return stream;
				}

				const Stream::Out& GetStream() const
				{
					return stream;
				}
			};

			class Loader
			{
			public:

				explicit Loader(StdStream);
				~Loader();

				dword Begin();
				void  End();
				void  DigIn();
				void  DigOut();
				uint  Read8();
				uint  Read16();
				dword Read32();
				void  Read(u8*,dword);
				void  Uncompress(u8*,dword);

				class Subset
				{
					Loader& loader;

				public:

					Subset(Loader& l)
					: loader(l)	
					{
						l.DigIn();
					}

					Loader& Ref() const
					{
						return loader;
					}

					~Subset()
					{
						loader.DigOut();
					}
				};

				template<uint N>
				class Data
				{
					struct Array
					{
						u8 data[N];

						Array(Loader& loader)
						{
							loader.Read( data, N );
						}
					};

					const Array array;

				public:

					Data(Loader& loader)
					: array(loader)	{}

					const u8& operator [] (uint i) const
					{
						NST_ASSERT( i < N );
						return array.data[i];
					}
				};

			private:

            #ifndef NDEBUG

				Loader(const Loader&);
             
            #endif

				void CheckRead(dword);

				Stream::In stream;
				Vector<u32> chunks;
				Vector<u32> lengths;

			public:

				template<size_t N>
				void Read(u8 (&data)[N])
				{
					Read( data, N );
				}

				template<size_t N>
				void Uncompress(u8 (&data)[N])
				{
					Uncompress( data, N );
				}

				Stream::In& GetStream()
				{
					return stream;
				}

				const Stream::In& GetStream() const
				{
					return stream;
				}
			};
		}
	}
}

#endif
