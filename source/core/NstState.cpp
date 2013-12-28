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

#include <cstdlib>
#include "NstCore.hpp"

#ifndef NST_NO_ZLIB
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define ZLIB_WINAPI
#endif
#include "../zlib/zlib.h"
#endif

#include "NstState.hpp"

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("s", on)
#endif

namespace Nes 
{ 
	namespace Core
	{
		namespace State
		{
			enum Compression
			{
				NO_COMPRESSION,
				ZLIB_COMPRESSION
			};

			Saver::Saver(StdStream p,bool c)
			: stream(p), chunks(1), useCompression(c) 
			{
				chunks[0] = 0;
			}

			Saver::~Saver()
			{
				NST_VERIFY( chunks.Size() == 1 );
			}

			Saver& Saver::Begin(dword id)
			{
				stream.Write32( id );
				chunks << stream.GetPos();
				stream.Write32( 0 );

				return *this;
			}

			Saver& Saver::End()
			{
				NST_VERIFY( chunks.Back() );

				const dword offset = chunks.Pop();
				const dword pos = stream.GetPos();

				stream.SetPos( offset );
				stream.Write32( pos - (offset + 4) );
				stream.SetPos( pos );

				return *this;
			}

			Saver& Saver::Write8(uint data)
			{
				stream.Write8( data );
				return *this;
			}

			Saver& Saver::Write16(uint data)
			{
				stream.Write16( data );
				return *this;
			}

			Saver& Saver::Write32(dword data)
			{
				stream.Write32( data );
				return *this;
			}

			Saver& Saver::Write(const void* data,dword length)
			{
				stream.Write( data, length );
				return *this;
			}

			Saver& Saver::Compress(const u8* const data,const dword length)
			{
				NST_VERIFY( length );

              #ifndef NST_NO_ZLIB

				if (useCompression && length > 1)
				{
					ulong compression = length - 1;
					Vector<u8> buffer( compression );

					if (compress2( buffer.Begin(), &compression, data, length, Z_BEST_COMPRESSION ) == Z_OK && compression)
					{
						stream.Write8( ZLIB_COMPRESSION );
						stream.Write( buffer.Begin(), compression );
						return *this;
					}
				}

              #endif

				stream.Write8( NO_COMPRESSION );
				stream.Write( data, length );

				return *this;
			}

			Loader::Loader(StdStream p)
			: stream(p)
			{
			}

			Loader::~Loader()
			{
			}

			void Loader::DigIn()
			{
				lengths << chunks.Back();
			}

			void Loader::DigOut()
			{
				lengths.Pop();
			}

			dword Loader::Begin()
			{
				dword id = 0;

				if (lengths.Size() == 0 || stream.GetPos() != lengths.Back())
				{
					id = stream.Read32();
					const dword length = stream.Read32();
					chunks << (stream.GetPos() + length);
				}

				return id;
			}

			void Loader::End()
			{
				stream.SetPos( chunks.Pop() );
			}

			void Loader::CheckRead(dword length)
			{
				if (stream.GetPos() + length > chunks.Back())
					throw RESULT_ERR_CORRUPT_FILE;
			}

			uint Loader::Read8()
			{
				CheckRead( 1 );
				return stream.Read8();
			}

			uint Loader::Read16()
			{
				CheckRead( 2 );
				return stream.Read16();
			}

			dword Loader::Read32()
			{
				CheckRead( 4 );
				return stream.Read32();
			}

			void Loader::Read(u8* const data,const dword length)
			{
				CheckRead( length );
				stream.Read( data, length );
			}

			void Loader::Uncompress(u8* const data,const dword length)
			{
				NST_VERIFY( length );

				switch (Read8())
				{		
					case NO_COMPRESSION:

						Read( data, length );
						break;

     				case ZLIB_COMPRESSION:
					{
                      #ifndef NST_NO_ZLIB

						const Vector<u8> buffer( chunks.Back() - stream.GetPos() );
						Read( buffer.Begin(), buffer.Size() );

						ulong uncompressed = length;

						if (uncompress( data, &uncompressed, buffer.Begin(), buffer.Size() ) == Z_OK && uncompressed == length)
							break;
                      #else

						throw RESULT_ERR_UNSUPPORTED;

                      #endif
					}

					default: 
						
						throw RESULT_ERR_CORRUPT_FILE;
				}
			}
		}
	}
}

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif
