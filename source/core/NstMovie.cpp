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
frame     (0),
NextFrame (0),
machine   (m),
state     (NOTHING),
stopped   (TRUE)
{}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

MOVIE::~MOVIE()
{
	if (file.IsOpen())
	{
		if (file.Position() >= sizeof(U32) * 2)
		{
			file.Seek( PDXFILE::BEGIN, file.Size() );
			file.Close();
		}
		else
		{
			file.Abort();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MOVIE::Load(const PDXSTRING& FileName)
{
	if (file.IsOpen() && file.Name() == FileName)
		return PDX_OK;

	PDX_TRY(file.Open( FileName, PDXFILE::APPEND ));
	file.Seek( PDXFILE::BEGIN );

	if (file.Readable(sizeof(U32) * 2))
	{
		if (file.Read<U32>() != 0x1A564D4EUL)
			return PDX_FAILURE;

		file.Seek( PDXFILE::CURRENT, sizeof(U32) );
	}
	else
	{
		file << U32(0x1A564D4EUL);
		file << U32(0);
	}
	
	frame      = 0;
	NextFrame  = 0;
	state      = NOTHING;
	stopped    = TRUE;
	
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

VOID MOVIE::Stop()
{
	if (!stopped)
	{
		stopped = TRUE;

		if (state == RECORDING)
		{
			file << U32(frame - PrevFrame);
			file << U8(UPDATE_SAVE_STATE);

			const U32 SavePos = file.Position();

			if (PDX_FAILED(machine->SaveNST( file )))
				return;

			const TSIZE CurrentPos = file.Position();

			file.Seek( PDXFILE::BEGIN, sizeof(U32) );
			file << SavePos;
			file.Seek( PDXFILE::BEGIN, CurrentPos );

			frame = 0;
			PrevFrame = 0;

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

VOID MOVIE::Record()
{
	if (CanRecord())
	{
		stopped   = FALSE;
		state     = RECORDING;
		frame     = 0;
		NextFrame = 0;

		const BOOL started = (file.Position() == sizeof(U32) * 2);
		file.Buffer().Resize( file.Position() );

		if (!started)
		{
			file << U32(1);
			file << U8(UPDATE_SAVE_STATE);
		}

		const U32 SavePos = file.Position();

		if (PDX_FAILED(machine->SaveNST( file )))
			return;

		const TSIZE CurrentPos = file.Position();

		file.Seek( PDXFILE::BEGIN, sizeof(U32) );
		file << SavePos;
		file.Seek( PDXFILE::BEGIN, CurrentPos );

		MsgOutput( started ? "Movie recording started.." : "Movie recording resumed.." );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MOVIE::Play()
{
	if (CanPlay())
	{
		stopped = FALSE;
		state = PLAYING;
		frame = 0;

		file.Seek( PDXFILE::BEGIN, sizeof(U32) * 2 );

		if (PDX_FAILED(machine->LoadNST( file )))
			return;

		U32 next;

		if (!file.Read(next))
			return;

		NextFrame = next;

		MsgOutput( "Movie started, grab your popcorn.." );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MOVIE::Rewind()
{
	if (CanRewind())
	{
		MsgOutput( "Movie rewinded.." );
		file.Seek( PDXFILE::BEGIN, sizeof(U32) * 2 );
		frame = 0;
		NextFrame = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MOVIE::Forward()
{
	if (CanForward())
	{
		file.Seek( PDXFILE::BEGIN, sizeof(U32) );
		file.Seek( PDXFILE::BEGIN, file.Peek<U32>() );

		if (PDX_FAILED(machine->LoadNST( file )))
			return;			

		MsgOutput( "Reached end of movie.." );

		frame = 0;
		NextFrame = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MOVIE::Close()
{
	Stop();
	file.Close();
	state = NOTHING;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MOVIE::ExecuteFrame()
{
	if (!stopped)
	{
		switch (state)
		{
     		case PLAYING:   return ExecuteFrameRead();
     		case RECORDING:	return ExecuteFrameWrite();
		}
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MOVIE::ExecuteFrameRead()
{
	if (frame == NextFrame)
	{
		U8 u;

		if (!file.Read(u))
			return PDX_FAILURE;

		const UINT update = u;

		if (update & UPDATE_SAVE_STATE)
		{
			if (PDX_FAILED(machine->LoadNST( file )))
				return PDX_FAILURE;			

			frame = 0;
		}

		for (UINT i=0; i < 6; ++i)
		{
			if (update & (1U << i))
			{
				if (devices.Size() <= i || !file.Read(devices[i].state))
					return PDX_FAILURE;
			}
		}

		if (file.Eof())
		{
			Stop();
			return PDX_OK;
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
	UINT update = 0;
	
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
		const U32 count = frame - PrevFrame;
		PrevFrame = frame;

		file << count;
		file << U8(update);

		for (UINT i=0; i < devices.Size(); ++i)
		{
			if (update & (1U << i))
				file << U32(devices[i].state);
		}
	}

	++frame;

	return PDX_OK;
}

NES_NAMESPACE_END
