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

inline UINT FILEMANAGER::NumRecentFiles() const
{ 
	return RecentFiles.Size(); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline const PDXSTRING& FILEMANAGER::GetRecentFile(const UINT i) const
{ 
	return RecentFiles[i]; 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline const PDXSTRING& FILEMANAGER::GetRecentFile() const
{ 
	PDX_ASSERT(RecentFiles.Size());
	return RecentFiles.Front();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline BOOL FILEMANAGER::UpdatedRecentFiles() const
{ 
	return UpdatedRecentFile; 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline UINT FILEMANAGER::OpenZipFile(const CHAR* const title,const CHAR* const name,const PDXARRAY<PDXSTRING>& extensions,PDXFILE& file)
{ 
	return OpenZipFile( title, name, extensions, file, FALSE ); 
}
