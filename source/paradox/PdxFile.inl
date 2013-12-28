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

#ifndef PDXFILE_H
#error Do not include PdxFile.inl directly!
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Reserve memory for the buffer
//////////////////////////////////////////////////////////////////////////////////////////////

inline VOID PDXFILE::Reserve(const TSIZE size)
{
	PDX_ASSERT((mode==OUTPUT || mode==APPEND) || size <= buffer.Size());

    if (size > buffer.Size())
        buffer.Grow(size - buffer.Size());
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Increase the size of the file
//////////////////////////////////////////////////////////////////////////////////////////////

inline VOID PDXFILE::Grow(const TSIZE size)
{
	PDX_ASSERT((mode==OUTPUT || mode==APPEND) || size <= buffer.Size());

	if (pos+size > buffer.Size())
		buffer.Grow(size);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Return the file name
//////////////////////////////////////////////////////////////////////////////////////////////

inline const PDXSTRING& PDXFILE::FileName()
{ 
	if (filename.IsEmpty())
		name.GetFileName(filename);

	return filename; 
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Return the directory path where the file is
//////////////////////////////////////////////////////////////////////////////////////////////

inline const PDXSTRING& PDXFILE::FilePath()
{ 
	if (filepath.IsEmpty())
		name.GetFilePath(filepath);

	return filepath; 
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Return the .xxx file extension
//////////////////////////////////////////////////////////////////////////////////////////////

inline const PDXSTRING& PDXFILE::FileExtension()
{ 
	if (fileextension.IsEmpty())
		name.GetFileExtension(fileextension);

	return fileextension; 
}
