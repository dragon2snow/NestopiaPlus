////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2005 Martin Freij
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

#pragma comment(lib,"zlibstat")

#include "NstResourceString.hpp"
#include "NstApplicationInstance.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowDialog.hpp"
#include "NstIoFile.hpp"

#ifdef __INTEL_COMPILER
#pragma warning( push )
#pragma warning( disable : 193 )
#endif

#define ZLIB_WINAPI
#define ZCALLBACK NST_CDECL
#include "../zlib/unzip.h"

#ifdef __INTEL_COMPILER
#pragma warning( pop )
#endif

#include "NstIoZip.hpp"

namespace Nestopia
{
	using Io::Zip;
	using Io::File;

	namespace
	{
		static voidpf ZCALLBACK OnOpen(voidpf opaque,cstring,int mode)
		{
			if (mode & ZLIB_FILEFUNC_MODE_WRITE)
				return NULL;

			return opaque;
		}

		static ulong ZCALLBACK OnWrite(voidpf,voidpf,const void*,ulong)
		{
			return 0UL;
		}

		static int ZCALLBACK OnClose(voidpf,voidpf stream)
		{
			return !stream;
		}

		static int ZCALLBACK OnTest(voidpf,voidpf stream)
		{
			return !stream;
		}

		struct onstream
		{
			cstring begin;
			cstring pos;
			cstring end;
		};

		static ulong ZCALLBACK OnStreamRead(voidpf,voidpf object,void* data,ulong size)
		{
			if (object && data && size)
			{
				onstream& stream = *static_cast<onstream*>( object );
				NST_ASSERT( stream.pos >= stream.begin && stream.pos <= stream.end );

				if (size > (ulong) (stream.end - stream.pos))
					size = (ulong) (stream.end - stream.pos);

				std::memcpy( data, stream.pos, size );
				stream.pos += size;

				return size;
			}

			return 0UL;
		}

		static long ZCALLBACK OnStreamTell(voidpf,voidpf object)
		{
			if (const onstream* const stream = static_cast<const onstream*>( object ))
			{
				NST_ASSERT( stream->pos >= stream->begin && stream->pos <= stream->end );
				return stream->pos - stream->begin;
			}

			return -1L;
		}

		static long ZCALLBACK OnStreamSeek(voidpf,voidpf object,ulong distance,int origin)
		{
			if (onstream* const stream = static_cast<onstream*>( object ))
			{
				NST_ASSERT( stream->pos >= stream->begin && stream->pos <= stream->end );

				switch (origin)
				{
					case ZLIB_FILEFUNC_SEEK_SET: 
				
						if (stream->end >= stream->begin + distance)
						{
							stream->pos = stream->begin + distance; 
							return 0L;
						}						
						break;
				
					case ZLIB_FILEFUNC_SEEK_CUR: 
				
						if (stream->end >= stream->pos + distance)
						{
							stream->pos += distance; 
							return 0L;
						}
						break;
				
					case ZLIB_FILEFUNC_SEEK_END: 
				
						if (stream->begin <= stream->end - distance)
						{
							stream->pos = stream->end - distance; 
							return 0L;
						}
						break;
				}
			}

			return -1L;
		}

		static ulong ZCALLBACK OnFileRead(voidpf,voidpf file,void* data,ulong size)
		{
			if (size && file && data)
			{
				try
				{
					static_cast<const File*>(file)->Read( data, size );
					return size;
				}
				catch (...)
				{
				}
			}

			return 0UL;
		}

		static long ZCALLBACK OnFileTell(voidpf,voidpf file)
		{
			return file ? long(static_cast<const File*>(file)->Position()) : -1L;
		}

		static long ZCALLBACK OnFileSeek(voidpf,voidpf file,ulong distance,int origin)
		{
			if (!file)
				return -1L;

			File::Offset dir;

			switch (origin)
			{
				case ZLIB_FILEFUNC_SEEK_SET: dir = File::BEGIN;   break;
				case ZLIB_FILEFUNC_SEEK_CUR: dir = File::CURRENT; break;
				case ZLIB_FILEFUNC_SEEK_END: dir = File::END;     break;
				default: return -1L;
			}

			try
			{
				static_cast<const File*>(file)->Seek( dir, distance );
			}
			catch (...)
			{
				return -1L;
			}

			return 0L;
		}
	}

	Zip::Zip()
	: unzip(NULL)
	{
	}

	Zip::Zip(const File& f)
	: unzip(NULL)
	{
		Open( f );
	}

	Zip::Zip(const Object::ConstRaw data)
	: unzip(NULL)
	{
		Open( data );
	}

	Zip::~Zip()
	{
		Close();
	}

	ibool Zip::Open(const File& file)
	{
		Close();

		callbacks->opaque = const_cast<void*>(static_cast<const void*>(&file));

		callbacks->zopen_file  = &OnOpen;
		callbacks->zclose_file = &OnClose;
		callbacks->zwrite_file = &OnWrite;
		callbacks->zerror_file = &OnTest;
		callbacks->zread_file  = &OnFileRead;
		callbacks->ztell_file  = &OnFileTell;
		callbacks->zseek_file  = &OnFileSeek;

		return Open();
	}

	ibool Zip::Open(const Object::ConstRaw data)
	{
		Close();

		if (data.Empty())
			return FALSE;

		stream.begin = data;
		stream.pos = stream.begin;
		stream.end = stream.begin + data.Size();

		callbacks->opaque = &stream;

		callbacks->zopen_file  = &OnOpen;
		callbacks->zclose_file = &OnClose;
		callbacks->zwrite_file = &OnWrite;
		callbacks->zerror_file = &OnTest;
		callbacks->zread_file  = &OnStreamRead;
		callbacks->ztell_file  = &OnStreamTell;
		callbacks->zseek_file  = &OnStreamSeek;

		return Open();
	}

	ibool Zip::Open()
	{
		if ((unzip = ::unzOpen2( NULL, &(*callbacks) )) == NULL)
			return FALSE;

		try
		{
			unz_global_info_s info;

			if 
			(
		     	::unzGetGlobalInfo( unzip, &info ) != UNZ_OK ||
				!info.number_entry ||
				::unzGoToFirstFile( unzip ) != UNZ_OK
			)
				throw ERR_ZIP;

			files.reserve( info.number_entry );

			unz_file_info fileInfo;
			char fileName[_MAX_PATH+1];

			for (uint index=0;;)
			{
				fileName[0] = '\0';

				if (::unzGetCurrentFileInfo( unzip, &fileInfo, fileName, _MAX_PATH, NULL, 0, NULL, 0 ) != UNZ_OK)
					throw ERR_ZIP;

				if (fileInfo.compressed_size && fileInfo.uncompressed_size)
				{
					const Item item
					( 
				     	this, 
						fileName, 
						fileInfo.compressed_size, 
						fileInfo.uncompressed_size, 
						index 
					);

					if (item.GetName().Size())
						files.push_back( item );
				}

				if (++index == info.number_entry)
					break;

				if (::unzGoToNextFile( unzip ) != UNZ_OK)
					throw ERR_ZIP;
			}
		}
		catch (Exception)
		{
			Close();
			return FALSE;
		}

		return TRUE;
	}

	void Zip::Close()
	{
		if (unzip)
		{
			files.clear();

			::unzClose( unzip );
			unzip = NULL;
		}
	}

	inline Zip::Item::Item
	(
		Zip* const z,
		cstring const n,
		const uint cz,
		const uint uz,
		const uint i
	)
	: 
	zip              ( z ),
	name             ( n ),
	compressedSize   ( cz ),
	uncompressedSize ( uz ),
	index            ( i )
	{}

	uint Zip::Item::Uncompress(void* const data) const
	{
		NST_ASSERT( zip && zip->unzip && uncompressedSize );

		unzFile const unzip = zip->unzip;

		if (::unzGoToFirstFile( unzip ) != UNZ_OK)
			return 0;

		for (uint i=0; i < index; ++i)
		{
			if (::unzGoToNextFile( unzip ) != UNZ_OK)
				return 0;
		}

		if (::unzOpenCurrentFile( unzip ) != UNZ_OK)
			return 0;

		int size = ::unzReadCurrentFile( unzip, data, uncompressedSize );

		if (size < 0)
			size = 0;

		::unzCloseCurrentFile( unzip );

		return size;
	}

	uint Zip::Find(const String::Generic name) const
	{
		for (uint i=0, n=files.size(); i < n; ++i)
		{
			if (files[i].GetName() == name)
				return i;
		}

		return NO_FILES;
	}

	class Zip::Gui
	{
	public:

		Gui
		(
			const Items&,
			const String::Generic* =NULL,
			const uint=0
		);

		uint Open();

	private:

		struct Handlers;

		ibool OnInitDialog (Window::Param&);
		ibool OnCmdOk      (Window::Param&);
		ibool OnCmdCancel  (Window::Param&);

		Window::Dialog dialog;
		const Items& files;

		struct Filter
		{
			const String::Generic* const extensions;
			const uint count;

			Filter(const String::Generic* e,const uint c)
			: extensions(e), count(c) {}
		};

		const Filter filter;
	};

	struct Zip::Gui::Handlers
	{
		static const Window::MsgHandler::Entry<Gui> messages[];
		static const Window::MsgHandler::Entry<Gui> commands[];
	};

	const Window::MsgHandler::Entry<Zip::Gui> Zip::Gui::Handlers::messages[] =
	{
		{ WM_INITDIALOG, &Gui::OnInitDialog },
		{ WM_CLOSE,      &Gui::OnCmdCancel  }
	};

	const Window::MsgHandler::Entry<Zip::Gui> Zip::Gui::Handlers::commands[] =
	{
		{ IDC_COMPRESSED_FILE_OK,     &Gui::OnCmdOk     },
		{ IDC_COMPRESSED_FILE_CANCEL, &Gui::OnCmdCancel }
	};

	Zip::Gui::Gui(const Items& f,const String::Generic* e,const uint c)
	: 
	dialog ( IDD_COMPRESSED_FILE, this, Handlers::messages, Handlers::commands ), 
	files  ( f ),
	filter ( e, c )
	{
		NST_ASSERT( bool(e) >= bool(c) );
	}

	uint Zip::Gui::Open()
	{
		return dialog.Open();
	}

	ibool Zip::Gui::OnInitDialog(Window::Param&)
	{
		const Window::Control::ListBox listBox( dialog.ListBox(IDC_COMPRESSED_FILE_LIST) );

		if (filter.count)
		{
			for (Items::const_iterator it(files.begin()); it != files.end(); ++it)
			{
				const String::Generic extension( it->GetName().Extension() );

				for (uint i=0; i < filter.count; ++i)
				{
					if (filter.extensions[i] == extension)
					{
						listBox.Add( it->GetName() ).Data() = it - files.begin();
						break;
					}
				}
			}
		}
		else
		{
			listBox.Reserve( files.size() );

			for (Items::const_iterator it(files.begin()); it != files.end(); ++it)
				listBox.Add( it->GetName() ).Data() = it - files.begin();
		}

		NST_VERIFY( listBox.Size() );

		listBox[0].Select();

		return TRUE;
	}

	ibool Zip::Gui::OnCmdOk(Window::Param&)
	{
		dialog.Close( dialog.ListBox(IDC_COMPRESSED_FILE_LIST).Selection().Data() );
		return TRUE;
	}

	ibool Zip::Gui::OnCmdCancel(Window::Param&)
	{
		dialog.Close( NO_SELECTION );
		return TRUE;
	}

	uint Zip::UserSelect() const
	{
		return files.size() > 1 ? Gui( files ).Open() : files.size() ? 0 : NO_FILES;
	}

	uint Zip::UserSelect(const String::Generic* const filter,const uint count) const
	{
		if (filter && count)
		{
			uint match = 0;

			for (Items::const_iterator it(files.begin()); it != files.end(); ++it)
			{
				const String::Generic extension( it->GetName().Extension() );

				for (uint i=0; i < count; ++i)
				{
					if (filter[i] == extension)
					{
						if (match)
							return Gui( files, filter, count ).Open();
						else
							match = it - files.begin() + 1;
					}
				}
			}

			return match ? match-1 : NO_FILES;
		}
		else
		{
			return UserSelect();
		}		
	}
}
