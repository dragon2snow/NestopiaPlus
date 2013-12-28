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

#ifndef NST_ZIPFILE_H
#define NST_ZIPFILE_H

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

namespace UNZIP
{
  #include "zlib\unzip.h"
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class ZIPFILE
{
public:

	ZIPFILE();
	~ZIPFILE();

	PDXRESULT Open(const CHAR* const,const PDXARRAY<PDXSTRING>* const=NULL);
	PDXRESULT Close();

	inline UINT NumFiles() const
	{ return files.Size(); }

	inline const PDXSTRING& FileName(const UINT i) const
	{ return files[i].name; }

	inline TSIZE FileSize(const UINT i) const
	{ return files[i].info.uncompressed_size; }

	PDXRESULT Uncompress(const UINT i,PDXARRAY<CHAR>&);

private:

	UNZIP::unzFile ZipFile;
	UNZIP::unz_global_info_s ZipGlobalInfo;

	struct ENTRY
	{
		PDXSTRING name;
		UNZIP::unz_file_info info;
		UINT index;
	};

	PDXARRAY<ENTRY> files;
};

#endif
