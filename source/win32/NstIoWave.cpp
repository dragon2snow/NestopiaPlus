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

#include "NstIoWave.hpp"

namespace Nestopia
{
	using Io::Wave;

	Wave::Wave()
	: handle(NULL) {}

	Wave::~Wave()
	{
		try
		{
			Close();
		}
		catch (Exception)
		{
		}
	}

	void Wave::CreateChunk(MMCKINFO& chunk,const FOURCC id,const uint size) const
	{
		chunk.ckid = id;
		chunk.cksize = size;

		if (::mmioCreateChunk( handle, &chunk, 0 ) != MMSYSERR_NOERROR)
			throw ERR_OPEN;
	}

	template<typename T> 
	void Wave::WriteChunk(const T& t,const int size) const
	{
		if (::mmioWrite( handle, reinterpret_cast<const char*>(&t), size ) != size)
			throw ERR_OPEN;
	}

	void Wave::AscendChunk(MMCKINFO& chunk) const
	{
		if (::mmioAscend( handle, &chunk, 0 ) != MMSYSERR_NOERROR)
			throw ERR_OPEN;
	}

	void Wave::Open(const GenericString name,const WAVEFORMATEX& waveFormat)
	{
		NST_ASSERT( waveFormat.wFormatTag == WAVE_FORMAT_PCM );

		Close();

		fileName = name;
		handle = ::mmioOpen( fileName.Ptr(), NULL, OPEN_FLAGS );

		if (!handle)
			throw ERR_OPEN;

		try
		{
			chunkRiff.fccType = mmioFOURCC('W','A','V','E');
			chunkRiff.cksize = 0;

			if (::mmioCreateChunk( handle, &chunkRiff, MMIO_CREATERIFF ) != MMSYSERR_NOERROR)
				throw ERR_OPEN;

			MMCKINFO chunk;

			CreateChunk( chunk, mmioFOURCC('f','m','t',' '), sizeof(PCMWAVEFORMAT) );
			WriteChunk( waveFormat, sizeof(PCMWAVEFORMAT) );
			AscendChunk( chunk );

			CreateChunk( chunk, mmioFOURCC('f','a','c','t') );
			WriteChunk( DWORD(-1) );
			AscendChunk( chunk );

			CreateChunk( chunkData, mmioFOURCC('d','a','t','a') );

			if (::mmioGetInfo( handle, &output, 0 ) != MMSYSERR_NOERROR)
				throw ERR_OPEN;
		}
		catch (Exception)
		{
			Abort();
			throw ERR_OPEN;
		}
	}

	void Wave::Write(const void* const data,const uint length)
	{
		NST_ASSERT( handle );

		const char* input = static_cast<const char*>(data);
		const char* const end = input + length;

		while (input != end)
		{
			if (output.pchNext == output.pchEndWrite)
			{
				output.dwFlags |= MMIO_DIRTY;

				if (::mmioAdvance( handle, &output, MMIO_WRITE ) != MMSYSERR_NOERROR)
				{
					Abort();
					throw ERR_WRITE;
				}
			}

			*output.pchNext++ = *input++;
		}
	}

	void Wave::Close()
	{
		if (handle)
		{
			try
			{
				output.dwFlags |= MMIO_DIRTY;

				if (::mmioSetInfo( handle, &output, 0 ) != MMSYSERR_NOERROR)
					throw ERR_WRITE;

				AscendChunk( chunkData );
				AscendChunk( chunkRiff );

				::mmioSeek( handle, 0, SEEK_SET );

				if (::mmioDescend( handle, &chunkRiff, NULL, 0 ) != MMSYSERR_NOERROR)
					throw ERR_WRITE;

				MMCKINFO chunkFact;

				chunkFact.ckid = mmioFOURCC('f','a','c','t');
				chunkFact.cksize = 0;

				if (::mmioDescend( handle, &chunkFact, &chunkRiff, MMIO_FINDCHUNK ) == MMSYSERR_NOERROR) 
				{
					WriteChunk( DWORD(0) );
					AscendChunk( chunkFact );
				}

				AscendChunk( chunkRiff );

				::mmioClose( handle, 0 );
				handle = NULL;
			}
			catch (Exception)
			{
				Abort();
				throw ERR_FINALIZE;
			}
		}
	}

	void Wave::Abort()
	{
		NST_ASSERT( handle && fileName.Length() );

		::mmioClose( handle, 0 );
		handle = NULL;
	}
}
