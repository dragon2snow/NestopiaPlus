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

#include "paradox/PdxString.h"
#include "NstZipFile.h"

////////////////////////////////////////////////////////////////////////////////////////
// constructor
////////////////////////////////////////////////////////////////////////////////////////

ZIPFILE::ZIPFILE()
: ZipFile(NULL)
{
	PDXMemZero( ZipGlobalInfo );
}

////////////////////////////////////////////////////////////////////////////////////////
// destructor
////////////////////////////////////////////////////////////////////////////////////////

ZIPFILE::~ZIPFILE()
{ 
	Close(); 
}

////////////////////////////////////////////////////////////////////////////////////////
// open existing zip file
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT ZIPFILE::Open(const CHAR* const filename,const PDXARRAY<PDXSTRING>* const extensions)
{
	PDX_ASSERT(filename);

	Close();

	if (!(ZipFile = UNZIP::unzOpen( filename )))
		return PDX_FAILURE;

	if (UNZIP::unzGetGlobalInfo( ZipFile, &ZipGlobalInfo ) != UNZ_OK)
		return PDX_FAILURE;

	if (UNZIP::unzGoToFirstFile( ZipFile ) != UNZ_OK)
		return PDX_FAILURE;

	PDXSTRING name, match;
	CHAR buffer[512];

	files.Reserve( ZipGlobalInfo.number_entry );

	for (UINT i=0; i < ZipGlobalInfo.number_entry; ++i)
	{
		UNZIP::unz_file_info info;

		if (UNZIP::unzGetCurrentFileInfo( ZipFile, &info, buffer, 512, NULL, 0, NULL, 0) != UNZ_OK)
			break;

		name = buffer;
		BOOL yep = FALSE;

		if (extensions)
		{
			name.GetFileExtension( match );

			for (UINT i=0; i < extensions->Size(); ++i)
			{
				if (match == (*extensions)[i])
				{
					yep = TRUE;
					break;
				}
			}
		}

		if (yep)
		{
			files.Grow();
			files.Back().name  = name;
			files.Back().info  = info;
			files.Back().index = i;
		}

		if (UNZIP::unzGoToNextFile( ZipFile ) != UNZ_OK)
			break;
	}

	if (!files.Size())
		Close();

	return files.Size() ? PDX_OK : PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
// decompress any file inside the zip file
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT ZIPFILE::Uncompress(const UINT index,PDXARRAY<CHAR>& buffer)
{
	PDX_ASSERT(ZipFile && index < files.Size());

	buffer.Clear();

	if (UNZIP::unzGoToFirstFile( ZipFile ) != UNZ_OK)
		return PDX_FAILURE;

	for (UINT i=0; i < files[index].index; ++i)
	{
		if (UNZIP::unzGoToNextFile( ZipFile ) != UNZ_OK)
			return PDX_FAILURE;
	}

	if (UNZIP::unzOpenCurrentFile( ZipFile ) != UNZ_OK)
		return PDX_FAILURE;

	buffer.Resize( files[index].info.uncompressed_size );

	if (UNZIP::unzReadCurrentFile( ZipFile, buffer.Begin(), buffer.Size() ) != buffer.Size())
	{
		UNZIP::unzCloseCurrentFile( ZipFile );
		return PDX_FAILURE;
	}

	if (UNZIP::unzCloseCurrentFile( ZipFile ) != UNZ_OK)
		return PDX_FAILURE;

	return buffer.Size() ? PDX_OK : PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
// close zip file
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT ZIPFILE::Close()
{
	PDXRESULT result = PDX_OK;

	files.Clear();

	if (ZipFile)
	{
		if (UNZIP::unzClose(ZipFile) != UNZ_OK)
			result = PDX_FAILURE;

		ZipFile = NULL;
	}

	return result;
}
