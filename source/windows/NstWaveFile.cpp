// Copyright (C) 2003 Martin Freij
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

#include "NstApplication.h"
#include "NstWaveFile.h"

////////////////////////////////////////////////////////////////////////////////////////
// constructor
////////////////////////////////////////////////////////////////////////////////////////

WAVEFILE::WAVEFILE()
: mmio(NULL)
{
	PDXMemZero( mmck     );
	PDXMemZero( mmckInfo );
	PDXMemZero( mmckRiff );
}

////////////////////////////////////////////////////////////////////////////////////////
// destructor
////////////////////////////////////////////////////////////////////////////////////////

WAVEFILE::~WAVEFILE()
{
	Close();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT WAVEFILE::Open(const CHAR* const f,const WAVEFORMATEX& WaveFormat)
{
	PDX_ASSERT(!mmio);

	{
		PDXSTRING file(f);

		if (!(mmio = mmioOpen( file.Begin(), NULL, MMIO_ALLOCBUF|MMIO_READWRITE|MMIO_CREATE )))
			return application.OnWarning("mmioOpen() failed!");
	}

	PDXMemZero( mmck     );
	PDXMemZero( mmckRiff );
	PDXMemZero( mmckInfo );

	mmckRiff.fccType = mmioFOURCC('W','A','V','E');
	mmckRiff.cksize = 0;

	if (mmioCreateChunk( mmio, &mmckRiff, MMIO_CREATERIFF) != MMSYSERR_NOERROR)
	{
		application.OnWarning("mmioCreateChunk() failed!");
		goto hell;
	}

	mmck.ckid = mmioFOURCC('f','m','t',' ');
	mmck.cksize	= sizeof(PCMWAVEFORMAT);

	if (mmioCreateChunk( mmio, &mmck, 0 ) != MMSYSERR_NOERROR)
	{
		application.OnWarning("mmioCreateChunk() failed!");
		goto hell;
	}

	if (mmioWrite( mmio, HPSTR(&WaveFormat), sizeof(PCMWAVEFORMAT)) != sizeof(PCMWAVEFORMAT))
	{
		application.OnWarning("mmioWrite() failed!");
		goto hell;
	}

	if (mmioAscend( mmio, &mmck, 0 ) != MMSYSERR_NOERROR)
	{
		application.OnWarning("mmioAscend() failed!");
		goto hell;
	}

	MMCKINFO ck;
	PDXMemZero( ck );

	ck.ckid = mmioFOURCC('f','a','c','t');

	if (mmioCreateChunk( mmio, &ck, 0 ) != MMSYSERR_NOERROR)
	{
		application.OnWarning("mmioCreateChunk() failed!");
		goto hell;
	}

	const DWORD fact = DWORD(-1);

	if (mmioWrite( mmio, HPSTR(&fact), sizeof(fact)) != sizeof(fact))
	{
		application.OnWarning("mmioWrite() failed!");
		goto hell;
	}

	if (mmioAscend( mmio, &ck, 0 ) != MMSYSERR_NOERROR)
	{
		application.OnWarning("mmioAscend() failed!");
		goto hell;
	}

	mmck.ckid = mmioFOURCC('d','a','t','a');
	mmck.cksize = 0;

	if (mmioCreateChunk( mmio, &mmck, 0 ) != MMSYSERR_NOERROR)
	{
		application.OnWarning("mmioCreateChunk() failed!");
		goto hell;
	}

	if (mmioGetInfo( mmio, &mmckInfo, 0 ) != MMSYSERR_NOERROR)
	{
		application.OnWarning("mmioGetInfo() failed!");
		goto hell;
	}

	return PDX_OK;

hell:

	if (mmio)
	{
		mmioClose( mmio, 0 );
		mmio = NULL;
	}

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT WAVEFILE::Write(const VOID* const d,const DWORD size)
{
	PDX_ASSERT(mmio);

	const BYTE* const data = PDX_CAST(const BYTE*,d);

	for (DWORD i=0; i < size; ++i)
	{
		if (mmckInfo.pchNext == mmckInfo.pchEndWrite)
		{
			mmckInfo.dwFlags |= MMIO_DIRTY;

			if (mmioAdvance( mmio, &mmckInfo, MMIO_WRITE ) != MMSYSERR_NOERROR)
				return application.OnWarning("mmioAdvance() failed!");
		}

		*((BYTE*)mmckInfo.pchNext) = data[i];
		(BYTE*)mmckInfo.pchNext++;
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT WAVEFILE::Close()
{
	if (!mmio)
		return PDX_OK;

	mmckInfo.dwFlags |= MMIO_DIRTY;

	if (mmioSetInfo( mmio, &mmckInfo, 0 ) != MMSYSERR_NOERROR)
	{
		application.OnWarning("mmioSetInfo() failed!");
		goto hell;
	}

	if (mmioAscend( mmio, &mmck, 0 ) != MMSYSERR_NOERROR)
	{
		application.OnWarning("mmioAscend() failed!");
		goto hell;
	}

	if (mmioAscend( mmio, &mmckRiff, 0 ) != MMSYSERR_NOERROR)
	{
		application.OnWarning("mmioAscend() failed!");
		goto hell;
	}

	mmioSeek( mmio, 0, SEEK_SET );

	if (mmioDescend( mmio, &mmckRiff, NULL, 0 ) != MMSYSERR_NOERROR)
	{
		application.OnWarning("mmioDescend() failed!");
		goto hell;
	}

	mmck.ckid = mmioFOURCC('f','a','c','t');

	if (mmioDescend( mmio, &mmck, &mmckRiff, MMIO_FINDCHUNK ) == MMSYSERR_NOERROR) 
	{
		const DWORD samples = 0;
		mmioWrite( mmio, HPSTR(&samples), sizeof(DWORD) );
		mmioAscend( mmio, &mmck, 0 ); 
	}

	if (mmioAscend( mmio, &mmckRiff, 0 ) != MMSYSERR_NOERROR)
	{
		application.OnWarning("mmioAscend() failed!");
		goto hell;
	}

	if (mmio)
	{
		mmioClose( mmio, 0 );
		mmio = NULL;
	}

	return PDX_OK;

hell:

	if (mmio)
	{
		mmioClose( mmio, 0 );
		mmio = NULL;
	}

	return PDX_FAILURE;
}
