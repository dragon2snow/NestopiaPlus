//////////////////////////////////////////////////////////////////////////////////////////////
//
// Paradox Library - general purpose C++ utilities
//
// Copyright (C) 2003 Martin Freij
//
// This file is part of Paradox Library.
// 
// Paradox Library is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// Paradox Library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Paradox Library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//////////////////////////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include "PdxFile.h"
#include "PdxPair.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// Return the file zie
//////////////////////////////////////////////////////////////////////////////////////////////

static BOOL PDX_STDCALL GetFileSize(FILE* const file,TSIZE& size)
{
	PDX_ASSERT(file);

	if (fseek(file,0,SEEK_END) != 0)
		return FALSE;
	
	fpos_t pos;

	if (fgetpos(file,&pos) != 0)
		return FALSE;

	if (pos > TSIZE_MAX)
		return FALSE;

	size = pos;

	rewind(file);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Constructor, set mode to read or/and write
//////////////////////////////////////////////////////////////////////////////////////////////

PDXFILE::PDXFILE(const MODE m)
: pos(0), mode(m), open(FALSE) {}

//////////////////////////////////////////////////////////////////////////////////////////////
// Constructor, open the file and set mode to read or/and write
//////////////////////////////////////////////////////////////////////////////////////////////

PDXFILE::PDXFILE(const CHAR* const n,const MODE m)
: pos(0), mode(m), name(n), open(FALSE) { Open(); }

//////////////////////////////////////////////////////////////////////////////////////////////
// Destructor, close the file
//////////////////////////////////////////////////////////////////////////////////////////////

PDXFILE::~PDXFILE()
{
	if ((mode==OUTPUT || mode==APPEND) && open && pos)
	{
		const PDXRESULT result = WriteBuffer();
		PDX_ASSERT(PDX_SUCCEEDED(result));
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Open file
//////////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT PDXFILE::Open(const CHAR* const string)
{
	PDX_ASSERT(!open);

	if (string)
		name = string;

	if (name.IsEmpty())
		return PDX_FILE_NOT_FOUND;

	switch (mode)
	{
     	case INPUT:
		case APPEND:
		{
			const PDXRESULT result = ReadBuffer();
			open = (result == PDX_OK);
			pos = (mode == APPEND) ? buffer.Size() : 0;
			return result;
		}

		default:
		{
			PDX_ASSERT(mode == OUTPUT);
			buffer.Reserve(CACHE);
			pos = 0;
			open = TRUE;
			return PDX_OK;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Close file
//////////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT PDXFILE::Close()
{
	PDXRESULT result = PDX_OK;

	if ((mode==OUTPUT || mode==APPEND) && open && pos)
		result = WriteBuffer();	

	pos = 0;
	open = FALSE;
	buffer.Destroy();

	return result;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Move the file position pointer
//////////////////////////////////////////////////////////////////////////////////////////////

TSIZE PDXFILE::Seek(const POSITION t,const LONG p)
{
	PDX_ASSERT(open);

	const TSIZE old = pos;

	switch (t)
	{
    	case BEGIN:   pos  = p; break;
		case CURRENT: pos += p; break;
		case END:     pos  = buffer.Size() + p; break;
        #ifdef _DEBUG
		default: PDX_BREAK_MSG("Invalid position type!");
        #endif
	}

	Reserve(pos);

	return old;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////

VOID PDXFILE::Hook(BUFFER& b,const MODE m,const CHAR* const n)
{
	pos = 0;
	mode = m;
	open = TRUE;

	buffer.Destroy();
	buffer.Hook(b);

	if (n)
		name = n;

	filename.Clear();
	filepath.Clear();
	fileextension.Clear();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////

VOID PDXFILE::UnHook()
{
	buffer.UnHook();
	pos = 0;
	open = FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Read a string
//////////////////////////////////////////////////////////////////////////////////////////////

PDXSTRING& PDXFILE::TEXTPROXY::Read(PDXSTRING& string)
{
	PDX_ASSERT(file.open);
	string = file.buffer.At(file.pos);
	file.pos += string.Length() + 1;
	return string;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Read a C string
//////////////////////////////////////////////////////////////////////////////////////////////

const CHAR* PDXFILE::TEXTPROXY::Read(const CHAR*& string)
{
	PDX_ASSERT(file.open);
	string = file.buffer.At(file.pos);
	file.pos += strlen(string) + 1;
	PDX_ASSERT(file.pos <= file.buffer.Size());
	return string;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Read a C string
//////////////////////////////////////////////////////////////////////////////////////////////

const CHAR* PDXFILE::TEXTPROXY::Read()
{
	PDX_ASSERT(file.open);
	const CHAR* const string = file.buffer.At(file.pos);
	file.pos += strlen(string) + 1;
	PDX_ASSERT(file.pos <= file.buffer.Size());
	return string;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Write a C string
//////////////////////////////////////////////////////////////////////////////////////////////

VOID PDXFILE::TEXTPROXY::Write(const CHAR* const begin,const CHAR* const end)
{
	PDX_ASSERT(file.open && (file.mode==OUTPUT || file.mode==APPEND));
	const TSIZE length = end - begin;
	file.Grow(length+1);
	memcpy(file.buffer.At(file.pos),begin,length);
	file.pos += length;
	file.buffer[file.pos++] = '\0';
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Dump the real physical file's content into the temporary buffer
//////////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT PDXFILE::ReadBuffer()
{
	PDX_ASSERT(name.Size());

	FILE* file;
	
	if (NULL == (file = fopen(name.String(),"rb")))
		return PDX_FILE_NOT_FOUND;

	{
		TSIZE size;

		if (!GetFileSize(file,size))
		{
			fclose(file);
			return PDX_FAILURE;
		}

		buffer.Resize(size);
		buffer.Defrag();

		fread(buffer.Begin(),size,1,file);
	}

	if (fclose(file) != 0)
		return PDX_FAILURE;

	return PDX_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Dump the buffer's content to the real "physical" file
//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#include <Windows.h>
#endif

PDXRESULT PDXFILE::WriteBuffer() const
{
	PDX_ASSERT(name.Size());

	FILE* file;
	
	if (!(file = fopen(name.String(),"wb")))
	{
    #ifdef _WIN32

		PDXSTRING tmp;
		const PDXSTRING* path;

		if (filepath.IsEmpty())
		{
			name.GetFilePath(tmp);
			path = &tmp;
		}
		else
		{
			path = &filepath;
		}

		if (GetFileAttributes(path->String()) != INVALID_FILE_ATTRIBUTES)
			return PDX_FAILURE;
		
		if (!CreateDirectory(path->String(),NULL))
			return PDX_FAILURE;
		
		if (!(file = fopen(name.String(),"wb")))
			return PDX_FAILURE;

    #else

		return PDX_FAILURE;

    #endif
	}

	fwrite(buffer.Begin(),pos,1,file);

	if (fclose(file) != 0)
		return PDX_FAILURE;

	return PDX_OK;
}
