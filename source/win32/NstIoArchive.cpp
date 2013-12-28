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
#pragma comment(lib,"unrar")

#include "NstResourceString.hpp"
#include "NstApplicationInstance.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowDialog.hpp"
#include "NstSystemDll.hpp"
#include "NstObjectPod.hpp"
#include "NstIoFile.hpp"
#include "NstIoLog.hpp"
#include "NstIoArchive.hpp"
#include <initguid.h>
#include "../unrar/unrar.hpp"
#include "../7zip/IArchive.h"

DEFINE_GUID(CLSID_CFormat7z,0x23170F69,0x40C1,0x278A,0x10,0x00,0x00,0x01,0x10,0x07,0x00,0x00);

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

namespace Nestopia
{
	using Io::Archive;

	class Archive::Codec
	{
	public:

		enum Exception
		{
			ERR_DLL,
			ERR_CREATE,
			ERR_OPEN
		};

		virtual ~Codec() {}

		virtual ibool Build(Items&) = 0;
		virtual uint  Extract(uint,void*,uint) = 0;
	};

	class Archive::UnZip : public Codec
	{
		struct Stream
		{
			const char* const buffer;
			uint pos;
			const uint size;

			Stream(const void* b=NULL,uint s=0)
			: buffer(static_cast<const char*>(b)), pos(0), size(s) {}
		};

		Stream stream;
		void* const handle;
		
		ibool Build(Items& files)
		{
			uint numFiles;

			{
				unz_global_info_s info;

				if 
				(
			     	::unzGetGlobalInfo( handle, &info ) != UNZ_OK ||
					info.number_entry == 0 ||
					::unzGoToFirstFile( handle ) != UNZ_OK
				)
					return FALSE;

				numFiles = info.number_entry;
			}

			files.reserve( numFiles );

			char path[_MAX_PATH+1];
			unz_file_info info;
			uint i=0;

			do
			{
				if (::unzGetCurrentFileInfo( handle, &info, path, _MAX_PATH, NULL, 0, NULL, 0 ) == UNZ_OK && info.uncompressed_size && *path)
					files.push_back( Item(this,path,info.uncompressed_size,i) );
			}
			while (++i < numFiles && ::unzGoToNextFile( handle ) == UNZ_OK);

			return !files.empty();
		}

		uint Extract(uint index,void* buffer,uint size)
		{
			if (::unzGoToFirstFile( handle ) == UNZ_OK)
			{
				for (uint i=0; i < index; ++i)
				{
					if (::unzGoToNextFile( handle ) != UNZ_OK)
						return FALSE;
				}

				if (::unzOpenCurrentFile( handle ) == UNZ_OK)
				{
					const int extracted = ::unzReadCurrentFile( handle, buffer, size );
					::unzCloseCurrentFile( handle );
					
					if (extracted == (int) size)
						return size;
				}
			}

			return FALSE;
		}

		static void* ZCALLBACK OnOpen(void* opaque,cstring,int mode)
		{
			if (mode & ZLIB_FILEFUNC_MODE_WRITE)
				return NULL;
			else
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

		static ulong ZCALLBACK OnStreamRead(voidpf,voidpf object,void* data,ulong length)
		{
			if (object && data)
			{
				Stream& stream = *static_cast<Stream*>( object );

				if (length > stream.size - stream.pos)
					length = stream.size - stream.pos;

				std::memcpy( data, stream.buffer + stream.pos, length );
				stream.pos += length;

				return length;
			}
			else
			{
				return 0UL;
			}
		}

		static long ZCALLBACK OnStreamTell(voidpf,voidpf object)
		{
			if (const Stream* const stream = static_cast<const Stream*>( object ))
				return stream->pos;
			else
				return -1L;
		}

		static long ZCALLBACK OnStreamSeek(voidpf,voidpf object,ulong distance,int origin)
		{
			if (Stream* const stream = static_cast<Stream*>( object ))
			{
				switch (origin)
				{
					case ZLIB_FILEFUNC_SEEK_SET: stream->pos = distance; break;
					case ZLIB_FILEFUNC_SEEK_CUR: stream->pos += distance; break;
					case ZLIB_FILEFUNC_SEEK_END: stream->pos = stream->size + distance; break;
					default: return -1L;
				}

				if (stream->pos <= stream->size)
					return 0L;
				
				stream->pos = stream->size;
			}

			return -1L;
		}

		static ulong ZCALLBACK OnFileRead(voidpf,voidpf file,void* data,ulong size)
		{
			if (file && data)
			{
				try
				{
					return static_cast<const Io::File*>(file)->ReadSome( data, size );
				}
				catch (...)
				{
				}
			}

			return 0UL;
		}

		static long ZCALLBACK OnFileTell(voidpf,voidpf file)
		{
			if (file)
			{
				try
				{
					return (long) static_cast<const Io::File*>(file)->Position();
				}
				catch (...)
				{
				}
			}

			return -1L;
		}

		static long ZCALLBACK OnFileSeek(voidpf,voidpf file,ulong distance,int origin)
		{
			NST_COMPILE_ASSERT
			( 
		     	ZLIB_FILEFUNC_SEEK_SET == Io::File::Offset::BEGIN && 
				ZLIB_FILEFUNC_SEEK_CUR == Io::File::Offset::CURRENT && 
				ZLIB_FILEFUNC_SEEK_END == Io::File::Offset::END 
			);

			if (file && origin < 3)
			{
				try
				{
					static_cast<const Io::File*>(file)->Seek( (Io::File::Offset) origin, distance );
					return 0L;
				}
				catch (...)
				{
				}
			}

			return -1L;
		}

		static void* Create(const Io::File& file)
		{
			zlib_filefunc_def_s def;

			def.zopen_file  = &OnOpen;
			def.zread_file  = &OnFileRead;
			def.zwrite_file = &OnWrite;
			def.ztell_file  = &OnFileTell;
			def.zseek_file  = &OnFileSeek;
			def.zclose_file = &OnClose;
			def.zerror_file = &OnTest;
			def.opaque      = const_cast<void*>(static_cast<const void*>(&file));

			void* handle = ::unzOpen2( NULL, &def );

			if (handle == NULL)
				throw ERR_OPEN;

			return handle;
		}

		static void* Create(Stream& stream)
		{
			zlib_filefunc_def_s def;

			def.zopen_file  = &OnOpen;
			def.zread_file  = &OnStreamRead;
			def.zwrite_file = &OnWrite;
			def.ztell_file  = &OnStreamTell;
			def.zseek_file  = &OnStreamSeek;
			def.zclose_file = &OnClose;
			def.zerror_file = &OnTest;
			def.opaque      = &stream;

			void* handle = ::unzOpen2( NULL, &def );

			if (handle == NULL)
				throw ERR_OPEN;

			return handle;
		}

	public:

		UnZip(const Io::File& file)
		: handle(Create(file)) {}

		UnZip(const void* buffer,uint size)
		: stream(buffer,size), handle(Create(stream)) {}

		~UnZip()
		{
			::unzClose( handle );
		}
	};

	class Archive::UnRar : public Codec
	{
	public:

		UnRar(const String::Path<false>&);

	private:

		String::Path<false> path; 

		void* Open(uint);
		uint  Extract(uint,void*,uint);
		ibool Build(Items&);
	};

	Archive::UnRar::UnRar(const String::Path<false>& string)
	: path(string) 
	{
		static System::Dll dll("unrar.dll");

		if (!dll.IsSupported())
			throw ERR_DLL;
	}

	void* Archive::UnRar::Open(uint mode)
	{
		RAROpenArchiveData data;

		data.ArcName = path;
		data.CmtBuf = NULL;
		data.CmtBufSize = 0;
		data.CmtSize = 0;
		data.CmtState = 0;
		data.OpenMode = mode;
		data.OpenResult = 0;

		return ::RAROpenArchive( &data );
	}

	ibool Archive::UnRar::Build(Items& files)
	{
		if (void* const handle = Open( RAR_OM_LIST ))
		{
			try
			{
				Object::Pod<RARHeaderData> header;
				header.CmtBuf = NULL;

				for (uint i=0; ::RARReadHeader( handle, &header ) == 0; ++i)
				{
					if (header.UnpSize && *header.FileName)
						files.push_back( Item(this,header.FileName,header.UnpSize,i) );

					if (::RARProcessFile( handle, RAR_SKIP, NULL, NULL ))
						break;
				}
			}
			catch (...)
			{	
				files.clear();
			}

			::RARCloseArchive( handle );
			
			return !files.empty();
		}

		return FALSE;
	}

	uint Archive::UnRar::Extract(uint index,void* buffer,uint size)
	{
		static struct
		{
			uchar* buffer;
			uint pos;
			uint size;
		}   stream;

		struct Callback
		{
			static int PASCAL OnRead(uchar* input,int size)
			{
				if (input && size > 0 && uint(size) <= stream.size)
				{
					std::memcpy( stream.buffer + stream.pos, input, (uint) size );
					stream.pos += (uint) size;
					return 1;
				}
				else
				{
					return 0;
				}
			};
		};

		uint extracted = 0;

		if (void* const handle = Open( RAR_OM_EXTRACT ))
		{
			stream.buffer = static_cast<uchar*>(buffer);
			stream.pos = 0;
			stream.size = size;

			::RARSetProcessDataProc( handle, Callback::OnRead );

			try
			{
				Object::Pod<RARHeaderData> header;
				header.CmtBuf = NULL;

				for (uint i=0; ; ++i)
				{
					if (::RARReadHeader( handle, &header ))
						break;

					if (i < index)
					{
						if (::RARProcessFile( handle, RAR_SKIP, NULL, NULL ))
							break;
					}
					else
					{
						char path[_MAX_PATH+16];
						char oem[_MAX_PATH+16];
						
						// hmm, doesn't seem to be any way of saving 
						// the uncompressed RAR file into memory only.

						if 
						(
					    	::GetTempPath( _MAX_PATH, oem ) && 
							::GetLongPathName( oem, path, _MAX_PATH ) &&
							std::strcat( path, "nestopia.tmp" ) &&
							::CharToOem( path, oem ) &&
							::RARProcessFile( handle, RAR_EXTRACT, NULL, oem ) == 0 
						)
						{
							if (stream.pos == stream.size)
								extracted = size;

							if (!Io::File::Delete( path ))
								Io::Log() << "Archive: warning, couldn't delete temporary RAR file: " << cstring(path) << '!';
						}

						break;
					}
				}
			}
			catch (...)
			{								
			}
			
			::RARCloseArchive( handle );
		}
		
		return extracted;
	}
  
	class Archive::Un7zip : public Codec
	{
	public:

		Un7zip(const Io::File&);
		Un7zip(const void*,uint);

	private:

		~Un7zip();

		IInArchive& archive;
		IInStream* const stream;

		static IInArchive* Create();

		class InStream : public IInStream, public IStreamGetSize
		{
			ulong refCount;

			HRESULT STDMETHODCALLTYPE QueryInterface(REFGUID,void**)
			{
				return E_NOINTERFACE;
			}

			HRESULT STDMETHODCALLTYPE GetSize(UInt64* save)
			{
				if (save)
				{
					*save = size;
					return S_OK;
				}
				else
				{
					return E_INVALIDARG;
				}
			}

			ulong STDMETHODCALLTYPE AddRef()
			{
				return ++refCount;
			}

			ulong STDMETHODCALLTYPE Release()
			{
				return --refCount;
			}

		protected:

			const uint size;

		public:

			InStream(uint s)
			: refCount(0), size(s) {}
		};

		class InFileStream : public InStream
		{
			const Io::File& file;

			HRESULT STDMETHODCALLTYPE Read(void* data,UInt32 length,UInt32* save)
			{
				if (data != NULL || length == 0)
				{
					try
					{
						length = file.ReadSome( data, length );
					}
					catch (Io::File::Exception)
					{
						return E_FAIL;
					}

					if (save)
						*save = length;

					return S_OK;
				}
				else
				{
					return E_INVALIDARG;
				}
			}

			HRESULT STDMETHODCALLTYPE Seek(Int64 offset,UInt32 origin,UInt64* pos)
			{
				NST_COMPILE_ASSERT( Io::File::BEGIN == 0 && Io::File::CURRENT == 1 && Io::File::END == 2 );

				if (origin < 3)
				{
					try
					{
						origin = file.Seek( (Io::File::Offset) origin, (int) offset );
					}
					catch (Io::File::Exception)
					{
						return E_FAIL;
					}

					if (pos)
						*pos = origin;

					return S_OK;
				}
				else
				{
					return E_INVALIDARG;
				}
			}

		public:

			InFileStream(const Io::File& f)
			: InStream(f.Size()), file(f) {}
		};

		class InMemStream : public InStream
		{
			const char* const buffer;
			uint pos;

			HRESULT STDMETHODCALLTYPE Read(void* data,UInt32 length,UInt32* save)
			{
				if (data != NULL || length == 0)
				{
					if (length > size - pos)
						length = size - pos;

					std::memcpy( data, buffer + pos, length );
					pos += length;

					if (save)
						*save = length;

					return S_OK;
				}
				else
				{
					return E_INVALIDARG;
				}
			}

			HRESULT STDMETHODCALLTYPE Seek(Int64 offset,UInt32 origin,UInt64* save)
			{
				if (origin < 3)
				{
					pos = (origin == 0 ? 0 : origin == 1 ? pos : size) + int(offset);

					if (pos > size)
						pos = size;

					if (save)
						*save = pos;

					return S_OK;
				}
				else
				{
					return E_INVALIDARG;
				}
			}

		public:

			InMemStream(const void* b,uint s)
			: InStream(s), buffer(static_cast<const char*>(b)), pos(0) {}
		};

		class OutStream : public IArchiveExtractCallback
		{
			class SeqStream : public ISequentialOutStream
			{
				char* const output;
				uint pos;
				const uint size;
				ulong refCount;

				HRESULT STDMETHODCALLTYPE QueryInterface(REFGUID,void**)
				{
					return E_NOINTERFACE;
				}

				HRESULT STDMETHODCALLTYPE Write(const void* data,UInt32 length,UInt32* save)
				{
					if (data != NULL || size == 0)
					{
						NST_VERIFY( length <= size - pos );

						if (length > size - pos)
							length = size - pos;

						std::memcpy( output + pos, data, length );
						pos += length;

						if (save)
							*save = length;

						return S_OK;
					}
					else
					{
						return E_INVALIDARG;
					}
				}

				ulong STDMETHODCALLTYPE AddRef()
				{
					return ++refCount;
				}

				ulong STDMETHODCALLTYPE Release()
				{
					return --refCount;
				}

			public:

				SeqStream(void* d,uint s)
				: output(static_cast<char*>(d)), pos(0), size(s), refCount(0) {}

				uint Size() const
				{
					return pos;
				}
			};

			SeqStream seqStream;
			const uint index;
			ulong refCount;

			HRESULT STDMETHODCALLTYPE QueryInterface(REFGUID,void**)
			{
				return E_NOINTERFACE;
			}

			HRESULT STDMETHODCALLTYPE PrepareOperation(Int32)
			{
				return S_OK;
			}

			HRESULT STDMETHODCALLTYPE SetTotal(UInt64)
			{
				return S_OK;
			}

			HRESULT STDMETHODCALLTYPE SetCompleted(const UInt64*)
			{
				return S_OK;
			}

			HRESULT STDMETHODCALLTYPE SetOperationResult(Int32)
			{
				return S_OK;
			}

			HRESULT STDMETHODCALLTYPE GetStream(UInt32 id,ISequentialOutStream** ptr,Int32 mode)
			{
				switch (mode)
				{
					case NArchive::NExtract::NAskMode::kExtract:
					case NArchive::NExtract::NAskMode::kTest:

						if (id != index || ptr == NULL)
							return S_FALSE;
						else
							*ptr = &seqStream;
						
					case NArchive::NExtract::NAskMode::kSkip:
						return S_OK;

					default:
						return E_INVALIDARG;
				}
			}

			ulong STDMETHODCALLTYPE AddRef()
			{
				return ++refCount;
			}

			ulong STDMETHODCALLTYPE Release()
			{
				return --refCount;
			}

		public:

			OutStream(uint index,void* data,uint size)
			: seqStream(data,size), index(index), refCount(0) {}

			uint Size() const
			{
				return seqStream.Size();
			}
		};

		void  Open();
		void  Close();
		ibool Build(Items&);
		uint  Extract(uint,void*,uint);
	};

	IInArchive* Archive::Un7zip::Create()
	{
		static System::Dll dll("7zxa.dll");

		if (!dll.IsSupported())
			throw ERR_DLL;
		
		typedef UINT32 (WINAPI *CreateObjectFunc)(const GUID*,const GUID*,void**);
		CreateObjectFunc const CreateObject = (CreateObjectFunc) dll("CreateObject");

		void* object;

		if (CreateObject == NULL || FAILED(CreateObject( &CLSID_CFormat7z, &IID_IInArchive, &object )))
			throw ERR_CREATE;
			
		return static_cast<IInArchive*>(object);
	}

	Archive::Un7zip::Un7zip(const void* data,uint size)
	: archive(*Create()), stream(new (std::nothrow) InMemStream(data,size))
	{
		Open();
	}

	Archive::Un7zip::Un7zip(const Io::File& file)
	: archive(*Create()), stream(new (std::nothrow) InFileStream(file))
	{
		Open();
	}

	Archive::Un7zip::~Un7zip()
	{
		Close();
	}

	void Archive::Un7zip::Open()
	{
		if (stream)
		{
			try
			{
				if (SUCCEEDED(archive.Open( stream, NULL, NULL )))
					return;
			}
			catch (...)
			{
			}

			delete stream;
		}

		archive.Release();

		throw ERR_OPEN;
	}

	void Archive::Un7zip::Close()
	{
		archive.Close();
		delete stream;		
		archive.Release();
	}

	ibool Archive::Un7zip::Build(Items& files)
	{
		UInt32 numFiles;

		if (SUCCEEDED(archive.GetNumberOfItems( &numFiles )) && numFiles)
		{
			files.reserve( numFiles );

			char path[_MAX_PATH+2];
			PROPVARIANT prop;

			for (uint i=0; i < numFiles; ++i)
			{
				prop.vt = VT_EMPTY;

				if (FAILED(archive.GetProperty( i, kpidSize, &prop )) || prop.vt != VT_UI8 || !prop.uhVal.LowPart || prop.uhVal.HighPart)
					continue;
				
				const uint size = prop.uhVal.LowPart;

				prop.vt = VT_EMPTY;

				if (FAILED(archive.GetProperty( i, kpidPath, &prop )) || prop.vt != VT_BSTR || prop.bstrVal == NULL)
					continue;
					
				const ibool good = *prop.bstrVal && 1 < ::WideCharToMultiByte
				(
					CP_OEMCP, 
					0, 
					prop.bstrVal, 
					-1, 
					path, 
					_MAX_PATH+1, 
					NULL, 
					NULL
				);

				::VariantClear( reinterpret_cast<VARIANTARG*>(&prop) );

				if (good)
					files.push_back( Item(this,path,size,i) );
			}
		}

		return FALSE;
	}

	uint Archive::Un7zip::Extract(uint index,void* data,uint size)
	{
		NST_ASSERT( data );

		OutStream outStream( index, data, size );
		const UInt32 indices[1] = {index};
		
		if (SUCCEEDED(archive.Extract( indices, NST_COUNT(indices), 0, &outStream )) && outStream.Size() == size)
			return size;
		else
			return 0;
	}

	Archive::Archive()
	: codec(NULL)
	{
	}

	Archive::Archive(const Io::File& file)
	: codec(NULL)
	{
		Open( &file, NULL, 0 );
	}

	Archive::Archive(const void* raw,uint size)
	: codec(NULL)
	{
		Open( NULL, raw, size );
	}

	Archive::~Archive()
	{
		Close();
	}

	ibool Archive::Open(const Io::File& file)
	{
		Close();
		return Open( &file, NULL, 0 );
	}

	ibool Archive::Open(const void* raw,uint size)
	{
		Close();
		return Open( NULL, raw, size );
	}

	ibool Archive::Open(const Io::File* const file,const void* const raw,uint size)
	{
		try
		{
			if (raw)
			{
				if (size >= sizeof(u32))
				{
					switch (*static_cast<const u32*>(raw))
					{
						case FILE_ID_ZIP:
					
							codec = new UnZip( raw, size );
							break;
					
						case FILE_ID_7Z:
					
							codec = new Un7zip( raw, size );
							break;
					}
				}
			}
			else switch (file->Peek<u32>())
			{
		     	case FILE_ID_ZIP:
				
					codec = new UnZip( *file );
					break;

				case FILE_ID_7Z:

					codec = new Un7zip( *file );
					break;

			    case FILE_ID_RAR:
				
					codec = new UnRar( file->GetName() );
					break;
			}

			return codec ? codec->Build( files ) : FALSE;
		}
		catch (...)
		{
		}

		Close();

		return FALSE;
	}

	void Archive::Close()
	{
		if (codec)
		{
			delete codec;
			codec = NULL;
		}

		files.clear();
	}

	inline Archive::Item::Item
	(
		Codec* const c,
		cstring const n,
		const uint s,
		const uint i
	)
	: 
	codec     ( c ),
	name      ( n ),
	size      ( s ),
	index     ( i )
	{}

	uint Archive::Item::Uncompress(void* const data) const
	{
		NST_ASSERT( data && codec && size );
		return codec->Extract( index, data, size );
	}

	uint Archive::Find(const String::Generic name) const
	{
		for (uint i=0, n=files.size(); i < n; ++i)
		{
			if (files[i].GetName() == name)
				return i;
		}

		return NO_FILES;
	}

	class Archive::Gui
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

	struct Archive::Gui::Handlers
	{
		static const Window::MsgHandler::Entry<Gui> messages[];
		static const Window::MsgHandler::Entry<Gui> commands[];
	};

	const Window::MsgHandler::Entry<Archive::Gui> Archive::Gui::Handlers::messages[] =
	{
		{ WM_INITDIALOG, &Gui::OnInitDialog },
		{ WM_CLOSE,      &Gui::OnCmdCancel  }
	};

	const Window::MsgHandler::Entry<Archive::Gui> Archive::Gui::Handlers::commands[] =
	{
		{ IDC_COMPRESSED_FILE_OK,     &Gui::OnCmdOk     },
		{ IDC_COMPRESSED_FILE_CANCEL, &Gui::OnCmdCancel }
	};

	Archive::Gui::Gui(const Items& f,const String::Generic* e,const uint c)
	: 
	dialog ( IDD_COMPRESSED_FILE, this, Handlers::messages, Handlers::commands ), 
	files  ( f ),
	filter ( e, c )
	{
		NST_ASSERT( bool(e) >= bool(c) );
	}

	uint Archive::Gui::Open()
	{
		return dialog.Open();
	}

	ibool Archive::Gui::OnInitDialog(Window::Param&)
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

	ibool Archive::Gui::OnCmdOk(Window::Param&)
	{
		dialog.Close( dialog.ListBox(IDC_COMPRESSED_FILE_LIST).Selection().Data() );
		return TRUE;
	}

	ibool Archive::Gui::OnCmdCancel(Window::Param&)
	{
		dialog.Close( NO_SELECTION );
		return TRUE;
	}

	uint Archive::UserSelect() const
	{
		return files.size() > 1 ? Gui( files ).Open() : files.size() ? 0 : NO_FILES;
	}

	uint Archive::UserSelect(const String::Generic* const filter,const uint count) const
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
