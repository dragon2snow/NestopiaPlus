///////////////////////////////////////////////////////////////////////////////////////
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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

#include <Windows.h>
#include <Shlwapi.h>
#include "../paradox/PdxCrc32.h"
#include "NstLauncherFileSearch.h"
#include "../NstZipFile.h"
#include "../core/NstNsp.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#define NES_FILE 0x1A53454EUL
#define UNF_FILE 0x46494E55UL
#define FDS_FILE 0x1A534446UL
#define NSF_FILE 0x4D53454EUL
#define ZIP_FILE 0x04034B50UL

#define NST_COMPACT(a,b,c,d) ((U32(a) << 0) | (U32(b) << 8) | (U32(c) << 16) | (U32(d) << 24))

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL LAUNCHERFILESEARCH::GetExtension(const CHAR* const filename,ULONG& compact)
{
	const CHAR* const extension = ::PathFindExtension( filename );

	if (strlen( extension ) == 4)
	{
		compact =
		(
			( tolower( extension[0] ) << (8*0) ) |
			( tolower( extension[1] ) << (8*1) ) |
			( tolower( extension[2] ) << (8*2) ) |
			( tolower( extension[3] ) << (8*3) )
		);

		return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

LAUNCHERFILESEARCH::LAUNCHERFILESEARCH()
: flags(READ_NES|READ_UNF|READ_FDS|READ_NSF|READ_NSP|READ_ZIP) {}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERFILESEARCH::Create(CONFIGFILE* const LoadSettings)
{
	if (LoadSettings)
	{
		try
		{
			Load();
		}
		catch (...)
		{
			entries.Destroy();
			UI::MsgWarning( IDS_LAUNCHERFILES_LOAD_ERROR );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERFILESEARCH::Destroy(CONFIGFILE* const SaveSettings)
{
	if (SaveSettings)
	{
		try
		{
			Save();
		}
		catch (...)
		{
			UI::MsgWarning( IDS_LAUNCHERFILES_SAVE_ERROR );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL LAUNCHERFILESEARCH::AddEntry(const PDXSTRING& full)
{
	const TSIZE NumEntries = entries.Size();

	const CHAR* const file = ::PathFindFileName( full.String() );
	PDXSTRING path( full.Begin(), file );

	ReadFile( path, file );

	return entries.Size() > NumEntries;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERFILESEARCH::RemoveEntry(ENTRY* const entry)
{
	PDX_ASSERT( entry );
	entry->type = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERFILESEARCH::RemoveAllEntries()
{
	entries.Destroy();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL LAUNCHERFILESEARCH::IsDublicate()
{
	if (flags & PATH_NO_DUBLICATES)
	{
		const ULONG crc = PDXCRC32::Compute( buffer.Begin(), buffer.Size() );

		if (crcs.Find( crc ) != crcs.End())
			return TRUE;

		crcs.Insert( crc );
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL LAUNCHERFILESEARCH::IsValid() const
{
	PDX_ASSERT( buffer.Size() >= sizeof(U32) );

	switch (*PDX_CAST(const U32*,buffer.Begin()))
	{
     	case NES_FILE:
		case UNF_FILE:
		case FDS_FILE:
		case NSF_FILE:
		case ZIP_FILE:
			return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL LAUNCHERFILESEARCH::ReadNes(const PDXSTRING&)
{
	enum
	{
		INES_VERTICAL   = b16( 00000000, 00000001 ),
		INES_BATTERY    = b16( 00000000, 00000010 ),
		INES_TRAINER    = b16( 00000000, 00000100 ),
		INES_FOURSCREEN = b16( 00000000, 00001000 ),
		INES_MAPPER_LO  = b16( 00000000, 11110000 ),
		INES_VS         = b16( 00000001, 00000000 ),
		INES_MAPPER_HI  = b16( 11110000, 00000000 )
	};

   #pragma pack(push,1)

	struct HEADER
	{
		U32 signature;
		U8  Num16kPRomBanks;
		U8  Num8kCRomBanks;
		U16 flags;
		U8  Num8kWRamBanks;
		U32 reserved1;
		U16 reserved2;
		U8  reserved3;
	};

   #pragma pack(pop)

	if (buffer.Size() < sizeof(HEADER))
		return FALSE;

	const HEADER& header = *PDX_CAST(const HEADER*,buffer.Begin());

	if (header.signature != NES_FILE)
		return FALSE;

	if (header.reserved1 | header.reserved2 | header.reserved3)
		PDXMemZero( buffer.At(7), 9 );

	entries.Grow();
	ENTRY& entry = entries.Back();

	entry.type = ENTRY::TYPE_NES;

	entry.header.pRomSize = header.Num16kPRomBanks * NES::n16k;
	entry.header.cRomSize = header.Num8kCRomBanks * NES::n8k;
	entry.header.wRamSize = header.Num8kWRamBanks * NES::n8k;
	
	entry.header.mapper =
	( 
		((header.flags & INES_MAPPER_LO) >> 4) |
		((header.flags & INES_MAPPER_HI) >> 8)
	);

	entry.header.mirroring = 
	(
     	(header.flags & INES_FOURSCREEN) ? NES::MIRROR_FOURSCREEN : 
     	(header.flags & INES_VERTICAL) ? NES::MIRROR_VERTICAL : 
		NES::MIRROR_HORIZONTAL
	);

	entry.header.battery = (header.flags & INES_BATTERY) ? TRUE : FALSE;
	entry.header.trainer = (header.flags & INES_TRAINER) ? TRUE : FALSE;
	entry.header.vs      = (header.flags & INES_VS) ? TRUE : FALSE;

	TSIZE length = buffer.Size() - sizeof(HEADER);

	if (length)
	{
		const ULONG crc = PDXCRC32::Compute( buffer.At(sizeof(HEADER)), length );
		entry.dBaseHandle = nes.GetRomDatabase().GetHandle( crc );

		if (!entry.dBaseHandle)
		{
			length = (NES::n16k * header.Num16kPRomBanks) + (NES::n8k * header.Num8kCRomBanks);

			if (length && length <= (buffer.Size() - sizeof(HEADER)))
			{
				const ULONG crc = PDXCRC32::Compute( buffer.At(sizeof(HEADER)), length );
				entry.dBaseHandle = nes.GetRomDatabase().GetHandle( crc );
			}
		}
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL LAUNCHERFILESEARCH::ReadUnf(const PDXSTRING&)
{
	enum {HEADER_SIZE = sizeof(U32) * 8};

	if (buffer.Size() < HEADER_SIZE || *PDX_CAST(const U32*,buffer.Begin()) != UNF_FILE)
		return FALSE;

	const CHAR* iterator = buffer.At(HEADER_SIZE);
	const CHAR* const end = buffer.End();

	entries.Grow();
	ENTRY& entry = entries.Back();

	entry.type = ENTRY::TYPE_UNF;

	TSIZE pRomSize = 0;
	TSIZE cRomSize = 0;

	while (iterator < end)
	{
		const U32 id = PDX_CAST(const U32*,iterator)[0];
		const U32 length = PDX_CAST(const U32*,iterator)[1];
		iterator += sizeof(U32) * 2;

		switch (id)
		{
			case NST_COMPACT('N','A','M','E'):
			{
				if (iterator >= end)
					return TRUE;

				const CHAR* i;

				for (i=iterator; *i != '\0'; ++i)
					if (i >= end) return TRUE;

				entry.name.Set( iterator, i );
				break;
			}

			case NST_COMPACT('P','R','G','0'):
			case NST_COMPACT('P','R','G','1'):
			case NST_COMPACT('P','R','G','2'):
			case NST_COMPACT('P','R','G','3'):
			case NST_COMPACT('P','R','G','4'):
			case NST_COMPACT('P','R','G','5'):
			case NST_COMPACT('P','R','G','6'):
			case NST_COMPACT('P','R','G','7'):
			case NST_COMPACT('P','R','G','8'):
			case NST_COMPACT('P','R','G','9'):
			case NST_COMPACT('P','R','G','A'):
			case NST_COMPACT('P','R','G','B'):
			case NST_COMPACT('P','R','G','C'):
			case NST_COMPACT('P','R','G','D'):
			case NST_COMPACT('P','R','G','E'):
			case NST_COMPACT('P','R','G','F'):

				pRomSize += length;
				entry.header.pRomSize = pRomSize;
				break;

			case NST_COMPACT('C','H','R','0'):
			case NST_COMPACT('C','H','R','1'):
			case NST_COMPACT('C','H','R','2'):
			case NST_COMPACT('C','H','R','3'):
			case NST_COMPACT('C','H','R','4'):
			case NST_COMPACT('C','H','R','5'):
			case NST_COMPACT('C','H','R','6'):
			case NST_COMPACT('C','H','R','7'):
			case NST_COMPACT('C','H','R','8'):
			case NST_COMPACT('C','H','R','9'):
			case NST_COMPACT('C','H','R','A'):
			case NST_COMPACT('C','H','R','B'):
			case NST_COMPACT('C','H','R','C'):
			case NST_COMPACT('C','H','R','D'):
			case NST_COMPACT('C','H','R','E'):
			case NST_COMPACT('C','H','R','F'):

				cRomSize += length;
				entry.header.cRomSize = cRomSize;
				break;

			case NST_COMPACT('T','V','C','I'):

				if (iterator >= end)
					return TRUE;

				if (*PDX_CAST(const U8*,iterator) == 1)
					entry.header.pal = TRUE;
				else
					entry.header.ntsc = TRUE;

				break;
		
			case NST_COMPACT('B','A','T','R'):
		
				entry.header.battery = TRUE;
				break;
		
			case NST_COMPACT('M','I','R','R'):
		
				if (iterator >= end)
					return TRUE;
		
				switch (*PDX_CAST(const U8*,iterator))
				{
    				case 0: entry.header.mirroring = NES::MIRROR_HORIZONTAL; break;
    				case 1: entry.header.mirroring = NES::MIRROR_VERTICAL;   break;
    				case 2: entry.header.mirroring = NES::MIRROR_ZERO;       break;
    				case 3: entry.header.mirroring = NES::MIRROR_ONE;        break;
    				case 4: entry.header.mirroring = NES::MIRROR_FOURSCREEN; break;
				}
				break;
		}

		iterator += length;
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL LAUNCHERFILESEARCH::ReadFds(const PDXSTRING&)
{
	enum {HEADER_SIZE = sizeof(U8) * 16};

	if (buffer.Size() < HEADER_SIZE || *PDX_CAST(const U32*,buffer.Begin()) != FDS_FILE)
		return FALSE;

	entries.Grow();
	ENTRY& entry = entries.Back();

	entry.type = ENTRY::TYPE_FDS;
	entry.header.ntsc = TRUE;
	entry.header.pRomSize = buffer.Size() - HEADER_SIZE;
	entry.header.wRamSize = NES::n32k;

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL LAUNCHERFILESEARCH::ReadNsf(const PDXSTRING&)
{
   #pragma pack(push,1)

	struct HEADER
	{
		U8   pad1[14];
		CHAR name[32];
		U8   pad2[32];
		CHAR copyright[32];
		U8   pad3[12];
		U8   mode;
		U8   pad4[5];
	};

   #pragma pack(pop)

	if (buffer.Size() < sizeof(HEADER) || *PDX_CAST(const U32*,buffer.Begin()) != NSF_FILE)
		return FALSE;

	entries.Grow();
	ENTRY& entry = entries.Back();

	entry.type = ENTRY::TYPE_NSF;	
	entry.header.pRomSize = buffer.Size() - sizeof(HEADER);

	const HEADER& header = *PDX_CAST(const HEADER*,buffer.Begin());
	
	entry.name = header.name;
	entry.copyright = header.copyright;

	if ((header.mode & 0x3) == 0x2)
		entry.header.pal = TRUE;
	else
		entry.header.ntsc = TRUE;

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL LAUNCHERFILESEARCH::ReadNsp(const PDXSTRING&)
{
	NES::NSP nsp;

	{
		PDXFILE file;
		file.Hook( buffer );

		if (PDX_FAILED(nsp.Load( file )))
		{
			file.UnHook();
			return FALSE;
		}

		file.UnHook();
	}

	NES::NSP::CONTEXT context;
	nsp.GetContext( context );

	if (context.ImageFile.IsEmpty())
		return FALSE;
	
	const UINT SaveFlags = flags;
	flags &= ~(READ_NSP|READ_NSF|READ_ANY|PATH_NO_DUBLICATES);

	const TSIZE NumEntries = entries.Size();

	PDXSTRING path;
	ReadFile( path, context.ImageFile.String() );

	flags = SaveFlags;
    
	if (NumEntries == entries.Size())
		return FALSE;

	entries.Back().type |= ENTRY::TYPE_NSP;

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL LAUNCHERFILESEARCH::ReadZip(const PDXSTRING& zipname)
{
	if (buffer.Size() < sizeof(U32) || *PDX_CAST(const U32*,buffer.Begin()) != ZIP_FILE)
		return FALSE;

	ZIPFILE ZipFile;
	
	if (PDX_FAILED(ZipFile.Open( zipname.String(), &extensions )))
		return FALSE;

	const TSIZE NumFiles = ZipFile.NumFiles();

	for (TSIZE i=0; i < NumFiles; ++i)
	{
		const PDXSTRING& filename = ZipFile.FileName( i );

		ULONG compact;

		if (!GetExtension( filename.String(), compact ))
			continue;

		BOOL (LAUNCHERFILESEARCH::*Parse)(const PDXSTRING&);

		switch (compact)
		{
   			case NST_COMPACT('.','n','e','s'): if (flags & READ_NES) Parse = ReadNes; break;
   			case NST_COMPACT('.','u','n','f'): if (flags & READ_UNF) Parse = ReadUnf; break;
   			case NST_COMPACT('.','f','d','s'): if (flags & READ_FDS) Parse = ReadFds; break;
  			case NST_COMPACT('.','n','s','f'): if (flags & READ_NSF) Parse = ReadNsf; break;
   			case NST_COMPACT('.','n','s','p'): if (flags & READ_NSP) Parse = ReadNsp; break;
			default: continue;
		}

		buffer.Clear();
		
		if (PDX_FAILED(ZipFile.Uncompress( i, buffer )))
			continue;

		if (!IsDublicate() && (*this.*Parse)( filename ))
		{
			ENTRY& entry = entries.Back();

			entry.type |= ENTRY::TYPE_ZIP;

			const CHAR* const file = ::PathFindFileName( zipname.String() );

			entry.path.Set( zipname.Begin(), file );
			entry.file << file << "<" << filename << ">";

			if (entry.name.IsEmpty() && !entry.dBaseHandle)
				entry.name.InsertBack('-');

			if (entry.copyright.IsEmpty())
			{
				const CHAR* c;

				if (!entry.dBaseHandle || !(c=nes.GetRomDatabase().Copyright( entry.dBaseHandle )) || *c == '\0')
					entry.copyright.InsertBack('-');
			}
		}
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL LAUNCHERFILESEARCH::ReadAny(const PDXSTRING& filename)
{
	if ((flags & READ_NES) && ReadNes( filename )) return TRUE;
	if ((flags & READ_UNF) && ReadUnf( filename )) return TRUE;
	if ((flags & READ_FDS) && ReadFds( filename )) return TRUE;
	if ((flags & READ_NSF) && ReadNsf( filename )) return TRUE;
	if ((flags & READ_ZIP) && ReadZip( filename )) return FALSE;

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERFILESEARCH::ReadPath(PDXSTRING& path,const CHAR* const sub)
{
	if (IncSubDirs && *sub != '.' && *sub != '\0')
	{
		const TSIZE length = path.Length();

		path << sub;

		if (path.Back() != '\\' && path.Back() != '/')
			path.InsertBack('\\');

		if (SearchPaths.Find( path ) == SearchPaths.End())
		{
			SearchPaths.Insert( path );
			RecursiveSearch( path );
		}

		path.Resize( length );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERFILESEARCH::ReadFile(PDXSTRING& path,const CHAR* const name)
{
	BOOL (LAUNCHERFILESEARCH::*Parse)(const PDXSTRING&) = NULL;

	ULONG compact;

	if (GetExtension( name, compact ))
	{
		if (flags & READ_ANY)
		{
			switch (compact)
			{
    			case NST_COMPACT('.','n','s','p'): if (flags & READ_NSP) Parse = ReadNsp; break;
				default:                                                 Parse = ReadAny; break;
			}
		}
		else
		{
			switch (compact)
			{
    			case NST_COMPACT('.','n','e','s'): if (flags & READ_NES) Parse = ReadNes; break;
    			case NST_COMPACT('.','u','n','f'): if (flags & READ_UNF) Parse = ReadUnf; break;
    			case NST_COMPACT('.','f','d','s'): if (flags & READ_FDS) Parse = ReadFds; break;
    			case NST_COMPACT('.','n','s','f'): if (flags & READ_NSF) Parse = ReadNsf; break;
    			case NST_COMPACT('.','n','s','p'): if (flags & READ_NSP) Parse = ReadNsp; break;
    			case NST_COMPACT('.','z','i','p'): if (flags & READ_ZIP) Parse = ReadZip; break;
			}
		}
	}
	else if (flags & READ_ANY)
	{
		Parse = ReadAny;
	}

	if (!Parse)
		return;
	
	const TSIZE length = path.Length();	
	path << name;

	if (hDlg)
		::SetWindowText( ::GetDlgItem( hDlg, IDC_LAUNCHER_FILESEARCH_FILE ), path.String() );

	if (path.Length() <= MAX_PATH)
	{
		HANDLE hFile = ::CreateFile
		(
		    path.String(),
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD size = ::GetFileSize( hFile, NULL );

			if (size && size <= 0x3FFFFFFFUL)
			{
				DWORD read;
				BOOL valid;
			
				if (Parse != ReadNsp)
				{
					if (buffer.Size() < sizeof(U32))
						buffer.Resize( sizeof(U32) );
			
					valid =
					(
					    size > sizeof(U32) &&
						::ReadFile( hFile, buffer.Begin(), sizeof(U32), &(read=0), NULL ) && 
						read == sizeof(U32) &&
						IsValid()
					);
			
					if (valid)
					{
						buffer.Resize( size );
						size -= sizeof(U32);
			
						valid =
						(
					     	::ReadFile( hFile, buffer.At(sizeof(U32)), size, &(read=0), NULL ) && 
							read == size
						);
					}
				}
				else if (size <= 0xFFFFFFUL)
				{
					buffer.Resize( size );
			
					valid =
					(
					   	::ReadFile( hFile, buffer.Begin(), size, &(read=0), NULL ) && 
						read == size
					);
				}
			
				if (valid && !IsDublicate() && (*this.*Parse)( path ))
				{
					path.Resize( length );

					ENTRY& entry = entries.Back();

					entry.path = path;
					entry.file = name;

					if (entry.name.IsEmpty() && !entry.dBaseHandle)
						entry.name.InsertBack('-');
				}
			}

			::CloseHandle( hFile );
		}
	}

	path.Resize( length );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERFILESEARCH::RecursiveSearch(PDXSTRING& path)
{
	PDX_ASSERT( path.Length() && (path.Back() == '\\' || path.Back() == '/'));

	path << "*.*";

	WIN32_FIND_DATA wfd;	
	HANDLE handle = ::FindFirstFile( path.String(), &wfd );

	path.Resize( path.Size() - 3 );

	if (handle == INVALID_HANDLE_VALUE)
		return;
	
	do
	{
		if (stopped)
			throw (ABORT_SEARCH);

		if (!(wfd.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM)))
		{
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				ReadPath( path, wfd.cFileName );
			else
				ReadFile( path, wfd.cFileName );
		}
	}
	while (::FindNextFile( handle, &wfd ));

	::FindClose( handle );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL LAUNCHERFILESEARCH::Refresh(const PATHS& paths,HWND hParent)
{
	PDX_ASSERT( extensions.IsEmpty() && buffer.IsEmpty() && crcs.IsEmpty() && SearchPaths.IsEmpty() );

	::EnableWindow( hWnd, FALSE );

	InputPaths = &paths;

	if (flags & READ_NES) extensions.InsertBack( "nes" );
	if (flags & READ_UNF) extensions.InsertBack( "unf" );
	if (flags & READ_FDS) extensions.InsertBack( "fds" );
	if (flags & READ_NSF) extensions.InsertBack( "nsf" );
	if (flags & READ_NSP) extensions.InsertBack( "nsp" );

	const UINT result = DialogBoxParam
	(
		UTILITIES::GetInstance(),
		MAKEINTRESOURCE(IDD_LAUNCHER_FILESEARCH),
		hParent,
		DlgProc,
		PDX_CAST(LPARAM,this)
	);

	PDX_ASSERT( !hThread );

	buffer.Destroy();
	SearchPaths.Destroy();
	crcs.Destroy();
	extensions.Destroy();

	entries.Defrag();

	::EnableWindow( hWnd, TRUE );

	if (result == RESULT_ERROR)
		UI::MsgError(IDS_APP_SEARCH_ERROR);

	return result == RESULT_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI LAUNCHERFILESEARCH::ThreadProc(LPVOID p)
{
	PDX_CAST(LAUNCHERFILESEARCH*,p)->ThreadRefresh();
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERFILESEARCH::ThreadRefresh()
{
	UINT result = RESULT_OK;

	const ENTRIES backup( entries );
	entries.Clear();

	try
	{
		PDXSTRING root;

		for (TSIZE i=0; i < InputPaths->Size(); ++i)
		{
			root = (*InputPaths)[i].First();

			if (root.Length())
			{
				if (root.Back() != '\\' && root.Back() != '/')
					root.InsertBack('\\');

				IncSubDirs = (*InputPaths)[i].Second();
				RecursiveSearch( root );
			}
		}
	}
	catch (ABORT_SEARCH_EXCEPTION)
	{
		result = RESULT_ABORTED;
		entries = backup;
	}
	catch (...)
	{
		result = RESULT_ERROR;
		entries = backup;
	}

	PDX_ASSERT( hDlg );
	::PostMessage( hDlg, WM_APP, WPARAM(result), 0 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK LAUNCHERFILESEARCH::DlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	static LAUNCHERFILESEARCH* launcher = NULL; 

	switch (uMsg)
	{
       	case WM_INITDIALOG:
		{
			PDX_ASSERT( !launcher && lParam );

			launcher = PDX_CAST(LAUNCHERFILESEARCH*,lParam);

			launcher->hDlg = hDlg;
			launcher->stopped = FALSE;

			DWORD ThreadID = 0;
			launcher->hThread = ::CreateThread( NULL, 0, ThreadProc, PDX_CAST(LPVOID,launcher), 0, &ThreadID );

			if (!launcher->hThread)
			{
				launcher = NULL;
				::EndDialog( hDlg, RESULT_ERROR );
			}

			return TRUE;
		}

		case WM_APP:
		{
			PDX_ASSERT( launcher && launcher->hThread );

			::WaitForSingleObject( launcher->hThread, INFINITE );
			::CloseHandle( launcher->hThread );
			
			launcher->hThread = NULL;		
			launcher->hDlg = NULL;
			launcher = NULL;

			::EndDialog( hDlg, INT_PTR(wParam) ); 
			
			return TRUE;
		}

		case WM_CLOSE:
		{
			if (launcher)
				launcher->stopped = TRUE;

			return TRUE;
		}

		case WM_COMMAND:
		{
			PDX_ASSERT( launcher );

			if (LOWORD(wParam) == IDC_LAUNCHER_FILESEARCH_ABORT)
			{
   				launcher->stopped = TRUE;
   				return TRUE;		
			}
		}
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL LAUNCHERFILESEARCH::Load()
{
	PDX_ASSERT( entries.IsEmpty() );

	PDXFILE file;

	{
		PDXSTRING filename;
		UTILITIES::GetExeDir( filename );
		filename << "launcher.nsd";

		if (PDX_FAILED(file.Open( filename, PDXFILE::INPUT )))
			return FALSE;
	}

	if (file.Size() < (sizeof(U8) * 10) || file.Read<U32>() != 0x0064736EUL)
		return FALSE;

	PDXARRAY<const CHAR*> SavePaths;

	const UINT PathIndexSize = file.Read<U8>();

	switch (PathIndexSize)
	{
     	case sizeof(U32): SavePaths.Resize( file.Read<U32>() ); break;
		case sizeof(U16): SavePaths.Resize( file.Read<U16>() ); break;
		case sizeof(U8):  SavePaths.Resize( file.Read<U8>()  ); break;
		default:          return FALSE;
	}

	if (SavePaths.IsEmpty())
		return FALSE;

	{
		TSIZE pos = file.Position();

		for (TSIZE i=0; i < SavePaths.Size(); ++i)
		{
			SavePaths[i] = file.At(pos);
			pos += strlen(SavePaths[i]) + 1;

			if (pos >= file.Size())
				return FALSE;
		}

		file.Seek( PDXFILE::BEGIN, pos );
	}

	if (!file.Readable(sizeof(U32)))
		return FALSE;

	entries.Resize( file.Read<U32>() );

	for (TSIZE i=0; i < entries.Size(); ++i)
	{
		ENTRY& entry = entries[i];

		if (!file.Readable(sizeof(CHAR)))
			return FALSE;

		entry.file = file.Text().Read();

		if (!file.Readable(sizeof(U8)))
			return FALSE;

		entry.type = file.Read<U8>();

		if (entry.type & ENTRY::TYPE_NES)
		{
     		if (!file.Readable(sizeof(U8) * 9))
     			return FALSE;

			entry.header.flags = file.Read<U8>();
			entry.header.mapper = file.Read<U8>();
			entry.header.pRomSize = NES::n16k * file.Read<U8>();
			entry.header.cRomSize = NES::n8k * file.Read<U8>();
			entry.header.wRamSize = NES::n8k * file.Read<U8>();

			entry.dBaseHandle = nes.GetRomDatabase().GetHandle( file.Read<U32>() );
		}
		else if (entry.type & ENTRY::TYPE_UNF)
		{
			if (!file.Readable(sizeof(U8) * 9))
				return FALSE;

			entry.header.flags = file.Read<U8>();
			entry.header.pRomSize = file.Read<U32>();
			entry.header.cRomSize = file.Read<U32>();

			if (!file.Readable(sizeof(CHAR)))
				return FALSE;

			file.Text().Read(entry.name);

			if (!file.Readable(sizeof(CHAR)))
				return FALSE;

			file.Text().Read(entry.copyright);
		}
		else if (entry.type & ENTRY::TYPE_FDS)
		{
			if (!file.Readable(sizeof(U32)))
				return FALSE;

			entry.header.pRomSize = file.Read<U32>();
		}
		else
		{
			if (!(entry.type & ENTRY::TYPE_NSF) || !file.Readable((sizeof(U8) * 5) + sizeof(CHAR)))
				return FALSE;

			{
				const UINT flags = file.Read<U8>();
				entry.header.ntsc = (flags & 0x1) >> 0;
				entry.header.pal  = (flags & 0x2) >> 1;
			}

			entry.header.pRomSize = file.Read<U32>();

			file.Text().Read(entry.name);

			if (!file.Readable(sizeof(CHAR)))
				return FALSE;

			file.Text().Read(entry.copyright);
		}

		if (!file.Readable( PathIndexSize ))
			return FALSE;

		TSIZE index;

		switch (PathIndexSize)
		{
       		case sizeof(U32): index = file.Read<U32>(); break;
     		case sizeof(U16): index = file.Read<U16>(); break;
       		default:          index = file.Read<U8>();  break;
		}

		if (index >= SavePaths.Size())
			return FALSE;

		entry.path = SavePaths[index];

		if (entry.name.IsEmpty() && !entry.dBaseHandle)
			entry.name.InsertBack('-');

		if (entry.copyright.IsEmpty())
		{
			const CHAR* c;

			if (!entry.dBaseHandle || !(c=nes.GetRomDatabase().Copyright( entry.dBaseHandle )) || *c == '\0')
				entry.copyright.InsertBack('-');
		}
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL LAUNCHERFILESEARCH::Save() const
{
	PDXFILE file;

	{
		PDXSTRING filename;
		UTILITIES::GetExeDir( filename );
		filename << "launcher.nsd";

		if (PDX_FAILED(file.Open( filename, PDXFILE::OUTPUT )))
			return FALSE;
	}

	file.Write<U32>( 0x0064736EUL );
	
	SAVEPATHS SavePaths;

	for (TSIZE i=0; i < entries.Size(); ++i)
	{
		if (entries[i].type & ENTRY::TYPE_ANY)
			SavePaths.SafeInsert( entries[i].path );
	}

	UINT PathIndexSize;

	if (SavePaths.Size() <= U8_MAX)  
	{
		PathIndexSize = sizeof(U8);
		file.Write( U8(PathIndexSize) );
		file.Write( U8(SavePaths.Size()) );
	}
	else if (SavePaths.Size() <= U16_MAX) 
	{
		PathIndexSize = sizeof(U16);
		file.Write( U8(PathIndexSize) );
		file.Write( U16(SavePaths.Size()) );
	}
	else
	{
		PathIndexSize = sizeof(U32);
		file.Write( U8(PathIndexSize) );
		file.Write( U32(SavePaths.Size()) );
	}

	for (SAVEPATHS::CONSTITERATOR i(SavePaths.Begin()); i != SavePaths.End(); ++i)
		file.Write( (*i).Begin(), (*i).End() + 1 );

	const TSIZE SizePos = file.Position();
	file.Seek( PDXFILE::CURRENT, sizeof(U32) );

	U32 NumEntries = 0;

	for (TSIZE i=0; i < entries.Size(); ++i)
	{
		const ENTRY& entry = entries[i];

		if (!(entry.type & ENTRY::TYPE_ANY))
			continue;

		++NumEntries;

		file.Text().Write( entry.file );
		file.Write( U8(entry.type) );

		if (entry.type & ENTRY::TYPE_NES)
		{
			PDX_ASSERT
			( 
		       	!(entry.header.pRomSize % NES::n16k) && 
				!(entry.header.cRomSize % NES::n8k ) && 
				!(entry.header.wRamSize % NES::n8k )
			);

			file.Write( U8(entry.header.flags)  );
			file.Write( U8(entry.header.mapper) );
			file.Write( U8(entry.header.pRomSize / NES::n16k) );
			file.Write( U8(entry.header.cRomSize / NES::n8k ) );
			file.Write( U8(entry.header.wRamSize / NES::n8k ) );

			U32 crc = 0;

			if (entry.dBaseHandle)
				crc = nes.GetRomDatabase().Crc( entry.dBaseHandle );

			file.Write( crc );
		}
		else if (entry.type & ENTRY::TYPE_UNF)
		{
			file.Write( U8(entry.header.flags) );
			file.Write( U32(entry.header.pRomSize) );
			file.Write( U32(entry.header.cRomSize) );
			file.Text().Write( entry.name );
			file.Text().Write( entry.copyright );
		}
		else if (entry.type & ENTRY::TYPE_FDS)
		{
			file.Write( U32(entry.header.pRomSize) );
		}
		else
		{
			PDX_ASSERT(entry.type & ENTRY::TYPE_NSF);

			file.Write( U8((entry.header.ntsc ? 0x1 : 0x0) | (entry.header.pal ? 0x2 : 0x0))  );
			file.Write( U32(entry.header.pRomSize) );
			file.Text().Write( entry.name );
			file.Text().Write( entry.copyright );
		}

		const TSIZE PathIndex = SavePaths.Get( entry.path ) - SavePaths.Begin();

		switch (PathIndexSize)
		{
     		case sizeof(U32): file.Write( U32 (PathIndex) ); break;
			case sizeof(U16): file.Write( U16 (PathIndex) ); break;
			default:          file.Write( U8  (PathIndex) ); break;
		}
	}

	file.Seek( PDXFILE::BEGIN, SizePos );
	file.Write( NumEntries );
	file.Seek( PDXFILE::END, 0 );

	return TRUE;
}
