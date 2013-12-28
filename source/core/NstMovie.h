////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
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

#pragma once

#ifndef NST_MOVIE_H
#define NST_MOVIE_H

#include "../paradox/PdxFile.h"

////////////////////////////////////////////////////////////////////////////////////////
// Movie
////////////////////////////////////////////////////////////////////////////////////////

NES_NAMESPACE_BEGIN

class MOVIE
{
public:

	MOVIE(MACHINE* const);
	~MOVIE();

	PDXRESULT Load(const PDXSTRING&);
	PDXRESULT ExecuteFrame();
	
	VOID Record();
	VOID Play();
	VOID Stop();
	VOID Rewind();
	VOID Forward();
	VOID Close();

	VOID AddInputDevice(MACHINE::CONTROLLER* const);
	VOID RemoveInputDevices();

	BOOL IsLoaded()    const;
	BOOL IsRecording() const; 
	BOOL IsPlaying()   const; 
	BOOL IsStopped()   const; 
	BOOL IsRewinded()  const;

	BOOL CanStop()    const;
	BOOL CanPlay()    const;
	BOOL CanRecord()  const;
	BOOL CanRewind()  const;
	BOOL CanForward() const;

	const PDXSTRING& FileName() const;

private:

	PDXRESULT ExecuteFrameRead();
	PDXRESULT ExecuteFrameWrite();

	enum STATE
	{
		NOTHING,
		RECORDING,
		PLAYING
	};

	enum UPDATE
	{
		UPDATE_SAVE_STATE = 0x40,
		UPDATE_EOF        = 0x80
	};

	STATE state;
	BOOL  stopped;

  #ifdef PDX_U64_SUPPORT

	U64 frame;

	union
	{
		U64 NextFrame;
		U64 PrevFrame;
	};

  #else

	ULONG frame;

	union
	{
		ULONG NextFrame;
		ULONG PrevFrame;
	};

  #endif

	struct DEVICE
	{
		inline DEVICE(MACHINE::CONTROLLER* const c=NULL,const U32 s=0)
		: device(c), state(s) {}

		MACHINE::CONTROLLER* device;
		U32 state;
	};

	PDXARRAY<DEVICE> devices;

	MACHINE* const machine;
	PDXFILE file;
};

#include "NstMovie.inl"

NES_NAMESPACE_END

#endif
