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

#include "NstTypes.h"
#include "NstMachine.h"
#include "NstMovie.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

MOVIE::MOVIE(MACHINE* const m)
:				   
frame       (0),
NextFrame   (0),
machine     (m),
UpdateState (0),
state       (NOTHING),
stopped     (TRUE)
{}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

MOVIE::~MOVIE()
{
	if (file.IsOpen() && !stopped)
	{
		if (state == RECORDING)
		{
			Close();
		}
		else
		{
			MsgOutput( "File broken, movie stopped.." );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MOVIE::Load(const CHAR* const FileName)
{
	if (state == PLAYING && file.Name() == FileName)
		return PDX_OK;

	Close();
	state = NOTHING;

	if (PDX_FAILED(file.Open( FileName, PDXFILE::INPUT )))
		return PDX_FAILURE;

	if (!file.Readable(sizeof(U32)) || file.Read<U32>() != 0x1A564D4EUL)
		return PDX_FAILURE;

	frame     = 0;
	NextFrame = 0;
	state     = PLAYING;
	stopped   = TRUE;
	
	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MOVIE::Save(const CHAR* const FileName)
{
	if (state == RECORDING && file.Name() == FileName)
		return PDX_OK;

	Close();
	state = NOTHING;

	if (PDX_FAILED(file.Open( FileName, PDXFILE::OUTPUT )))
		return PDX_FAILURE;

	file << U32(0x1A564D4EUL);

	frame     = 0;
	PrevFrame = 0;
	state     = RECORDING;
	stopped   = TRUE;

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MOVIE::AddInputDevice(MACHINE::CONTROLLER* const device)
{
	PDX_ASSERT( devices.Size() < 6 );
	devices.InsertBack( DEVICE( device, 0 ) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MOVIE::RemoveInputDevices()
{
	devices.Clear();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MOVIE::Start()
{
	PDX_ASSERT( state != NOTHING );

	if (stopped)
	{
		stopped = FALSE;

		if (state == RECORDING)
		{
			UpdateState = UPDATE_SAVE_STATE;
			MsgOutput( frame ? "Movie recording resumed.." : "Movie recording started.." );
		}
		else
		{
			Rewind();
			MsgOutput( "Movie started, grab your popcorn.." );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MOVIE::Stop()
{
	if (!stopped)
	{
		stopped = TRUE;
	
		if (state == RECORDING)
		{
			MsgOutput( "Movie recording paused.." );
		}
		else if (state == PLAYING)
		{
			MsgOutput( "Movie stopped.." );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MOVIE::Close()
{
	Stop();

	if (file.IsOpen())
	{
		if (state == RECORDING)
		{
			if (frame)
			{
				const U32 count = frame - PrevFrame;
				PrevFrame = frame;
				file << count;
			}

			file << U8(UPDATE_EOF);
		}

		file.Close();
	}
  
	state = NOTHING;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MOVIE::Rewind()
{
	PDX_ASSERT( state != NOTHING );

	frame = 0;
	NextFrame = 0;

	if (file.IsOpen())
	{
		file.Seek( PDXFILE::BEGIN, sizeof(U32) );
		MsgOutput( "Movie rewinded.." );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MOVIE::ExecuteFrame()
{
	PDX_ASSERT( state != NOTHING );

	switch (state)
	{
    	case PLAYING:   return ExecuteFrameRead();
		case RECORDING:	return ExecuteFrameWrite();
	}

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MOVIE::ExecuteFrameRead()
{
	if (stopped)
		return PDX_OK;

	if (frame == NextFrame)
	{
		U8 update;

		if (!file.Read(update))
			return PDX_FAILURE;

		if (update & UPDATE_EOF)
		{
			Stop();
			return PDX_OK;
		}

		if ((update & UPDATE_SAVE_STATE) && PDX_FAILED(machine->LoadNST( file )))
			return PDX_FAILURE;			

		for (UINT i=0; i < 6; ++i)
		{
			if (update & (1U << i))
			{
				if (devices.Size() <= i || !file.Read(devices[i].state))
					return PDX_FAILURE;
			}
		}

		U32 next;

		if (!file.Read(next))
			return PDX_FAILURE;

		NextFrame = frame + next;	
	}

	++frame;

	for (UINT i=0; i < devices.Size(); ++i)
		devices[i].device->SetState( devices[i].state );

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MOVIE::ExecuteFrameWrite()
{
	if (!stopped)
	{
		UINT update = UpdateState;
	
		for (UINT i=0; i < devices.Size(); ++i)
		{
			const ULONG state = devices[i].device->GetState();
	
			if (devices[i].state != state)
			{
				devices[i].state = state;
				update |= (1U << i);
			}
		}
	
		if (update)
		{
			if (frame)
			{
				const U32 count = frame - PrevFrame;
				PrevFrame = frame;
				file << count;
			}
	
			file << U8(update);
	
			if (update & UPDATE_SAVE_STATE)
			{
				UpdateState = 0;
				PDX_TRY(machine->SaveNST( file ));
			}
	
			for (UINT i=0; i < devices.Size(); ++i)
			{
				if (update & (1U << i))
					file << U32(devices[i].state);
			}
		}
	}

	++frame;

	return PDX_OK;
}

NES_NAMESPACE_END
