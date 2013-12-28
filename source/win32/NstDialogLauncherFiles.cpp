////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2006 Martin Freij
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

#include <map>
#include <set>
#include "NstWindowUser.hpp"
#include "NstIoFile.hpp"
#include "NstIoArchive.hpp"
#include "NstSystemThread.hpp"
#include "NstApplicationInstance.hpp"
#include "NstIoLog.hpp"
#include "NstManagerPaths.hpp"
#include "NstDialogLauncher.hpp"
#include "../core/NstCrc32.hpp"
#include "../core/api/NstApiCartridge.hpp"

namespace Nestopia
{
	using namespace Window;

	Launcher::List::Files::Entry::Entry(uint t)
	: 
	file       (0),
	path       (0),
	name       (0),
	maker      (0),
	dBaseEntry (NULL),
	pRom       (0), 
	cRom       (0), 
	wRam       (0), 
	mapper     (0), 
	type       (u8(t)), 
	flags      (0) 
	{}

	class Launcher::List::Files::Inserter
	{
	public:

		typedef Paths::Settings Settings;
		typedef Settings::Include Include;

		Inserter
		(
			Strings&,
			Entries&,
			const Include,
			const Nes::Cartridge::Database&
		);

		ibool Add(const GenericString);

	protected:

		void ReadFile(tstring const,const Dialog&);

		enum Stop
		{
			STOP_SEARCH
		};

		enum
		{
			PATH_NOT_ADDED
		};

		struct PathInfo
		{
			Path string;
			ibool incSubDir;
			uint reference;
		};

		const Include include;
		uint compressed;
		PathInfo path;
		Strings strings;
		Entries entries;
		Strings& saveStrings;
		Entries& saveEntries;

	private:

		enum Type
		{
			TYPE_INVALID,
			TYPE_PROCESSED
		};

		typedef Collection::Set<u32> FileCheckSums;
		typedef Collection::Buffer Buffer;
		typedef Type (Inserter::*Parser)();

		Parser GetParser() const;

		inline u32 Crc(const uint,const uint) const;

		ibool IsFileUnique();
		ibool PrepareFile(const uint=1,const uint=0);
		Type  ParseAny();
		Type  ParseNes();
		Type  ParseUnf();
		Type  ParseFds();
		Type  ParseNsf();
		Type  ParseIps();
		Type  ParseNsp();
		Type  ParseArchive();
		void  AddEntry(const Entry&);

		Buffer buffer;
		FileCheckSums parsedFiles;
		const Nes::Cartridge::Database& imageDatabase;
	};

	Launcher::List::Files::Inserter::Inserter
	(
		Strings& v,
		Entries& e,
		const Include i,
		const Nes::Cartridge::Database& r
	)
	: 
	saveStrings   ( v ), 
	saveEntries   ( e ), 
	include       ( i ),
	imageDatabase ( r )
	{}

	Launcher::List::Files::Inserter::Parser Launcher::List::Files::Inserter::GetParser() const
	{
		if (const uint extension = path.string.Extension().Id())
		{
			if (include[Include::ANY])
			{
				if (extension == NST_FOURCC('n','s','p','\0'))
					return include[Include::NSP] ? &Inserter::ParseNsp : NULL;
				else
					return &Inserter::ParseAny;
			}
			else
			{
				switch (extension)
				{
					case NST_FOURCC('n','e','s','\0'): return include[Include::NES] ? &Inserter::ParseNes : NULL;
					case NST_FOURCC('u','n','f','\0'):
					case NST_FOURCC('u','n','i', 'f'): return include[Include::UNF] ? &Inserter::ParseUnf : NULL;
					case NST_FOURCC('f','d','s','\0'): return include[Include::FDS] ? &Inserter::ParseFds : NULL;
					case NST_FOURCC('n','s','f','\0'): return include[Include::NSF] ? &Inserter::ParseNsf : NULL;
					case NST_FOURCC('i','p','s','\0'): return include[Include::IPS] ? &Inserter::ParseIps : NULL;
					case NST_FOURCC('n','s','p','\0'): return include[Include::NSP] ? &Inserter::ParseNsp : NULL;
					case NST_FOURCC('z','i','p','\0'):
					case NST_FOURCC('r','a','r','\0'):
					case NST_FOURCC('7','z','\0','\0'): return include[Include::ARCHIVE] ? &Inserter::ParseArchive : NULL;
				}
			}
		}
		else if (include[Include::ANY])
		{
			return &Inserter::ParseAny; 
		}

		return NULL;
	}

	void Launcher::List::Files::Inserter::AddEntry(const Entry& entry)
	{
		entries.PushBack( entry );

		Entry& back = entries.Back();

		back.file = (strings << path.string.File());

		if (path.reference == PATH_NOT_ADDED)
			path.reference = (strings << path.string.Directory());

		NST_ASSERT( path.string.Directory() == strings[path.reference] );

		back.path = path.reference;

		if (entries.Size() == Header::MAX_ENTRIES)
			throw STOP_SEARCH;
	}

	ibool Launcher::List::Files::Inserter::Add(const GenericString fileName)
	{
		NST_ASSERT( fileName.Length() && fileName.Length() <= _MAX_PATH );

		if (entries.Size() == Header::MAX_ENTRIES)
			return FALSE;

		compressed = 0;
		path.string = fileName;
		strings = saveStrings;

		{
			const int index = strings.Find( path.string.Directory() );

			if (index != Strings::NONE)
				path.reference = index;
			else
				path.reference = PATH_NOT_ADDED;
		}

		if (Parser const parser = GetParser())
		{
			try
			{
				(*this.*parser)();
			}
			catch (Io::File::Exception)
			{
				return FALSE;
			}
			catch (Stop)
			{
				// reached file count limit
			}

			if (entries.Size())
			{
				saveEntries.PushBack( entries );
				saveStrings = strings;
				return TRUE;
			}
		}

		return FALSE;
	}

	void Launcher::List::Files::Inserter::ReadFile(tstring const fileName,const Dialog& dialog)
	{
		path.string.File() = fileName;

		if (Parser const parser = GetParser())
		{
			if (dialog)
				dialog.Control( IDC_LAUNCHER_FILESEARCH_FILE ).Text() << path.string.Ptr();

			compressed = 0;
			buffer.Clear();

			try
			{
				(*this.*parser)();
			}
			catch (Io::File::Exception)
			{
				// file I/O failure, just skip it
			}
		}

		path.string.File().Clear();
	}

	inline u32 Launcher::List::Files::Inserter::Crc(const uint start,const uint length) const
	{
		NST_ASSERT( buffer.Size() );
		return Nes::Core::Crc32::Compute( &buffer[start], length );
	}

	ibool Launcher::List::Files::Inserter::IsFileUnique()
	{
		NST_ASSERT( buffer.Size() );
		return !include[Include::UNIQUE] || parsedFiles.Insert(Crc(0,buffer.Size()));
	}

	ibool Launcher::List::Files::Inserter::PrepareFile(const uint minSize,const uint fileId)
	{
		NST_ASSERT( path.string.Length() && minSize && minSize >= bool(fileId) * 4U );

		if (buffer.Empty())
		{
			const Io::File file( path.string, Io::File::COLLECT );
			const uint size = file.Size();

			if (size >= minSize && (!fileId || fileId == file.Peek<u32>()))
			{
				buffer.Resize( size );
				file.Read( buffer.Ptr(), size );
				return TRUE;
			}

			return FALSE;
		}

		return
		(
			buffer.Size() >= minSize &&
			(!fileId || fileId == reinterpret_cast<const u32&>( buffer.Front() ))
		);
	}

	Launcher::List::Files::Inserter::Type Launcher::List::Files::Inserter::ParseNes() 
	{ 
		if (PrepareFile( 16, Managers::Paths::File::FILEID_INES ))
		{
			if (IsFileUnique())
			{
				enum
				{
					INES_VERTICAL   = 0x0001,
					INES_BATTERY    = 0x0002,
					INES_TRAINER    = 0x0004,
					INES_FOURSCREEN = 0x0008,
					INES_MAPPER_LO  = 0x00F0,
					INES_VS         = 0x0100,
					INES_MAPPER_HI  = 0xF000,
					INES_PAL        = 0x1
				};

            #pragma pack(push,1)

				struct Header
				{
					u32 signature;
					u8  num16kPRomBanks;
					u8  num8kCRomBanks;
					u16 flags;
					u8  num8kWRamBanks;
					u8  pal;
					u32 reserved1;
					u16 reserved2;
				};

            #pragma pack(pop)

				NST_COMPILE_ASSERT( sizeof(Header) == 16 );

				const Header& header = reinterpret_cast<const Header&>( buffer.Front() );

				if (header.reserved1 | header.reserved2)
					std::memset( buffer.Ptr() + 7, 0x00, sizeof(Header) - 7 );

				Entry entry( Entry::NES | compressed );

				entry.pRom = header.num16kPRomBanks * 16;
				entry.cRom = header.num8kCRomBanks * 8;
				entry.wRam = header.num8kWRamBanks * 8;

				entry.mapper = u8
				( 
					((header.flags & INES_MAPPER_LO) >> 4) |
					((header.flags & INES_MAPPER_HI) >> 8)
				);

				entry.bits.mirroring = 
				(
		 			(header.flags & INES_FOURSCREEN) ? Entry::MIRROR_FOURSCREEN : 
		     		(header.flags & INES_VERTICAL)   ? Entry::MIRROR_VERTICAL   : 
		                                           	   Entry::MIRROR_HORIZONTAL
				);

				entry.bits.battery = ( header.flags & INES_BATTERY ) != 0;
				entry.bits.trainer = ( header.flags & INES_TRAINER ) != 0;
				entry.bits.vs      = ( header.flags & INES_VS      ) != 0;
				entry.bits.pal     = ( header.pal & INES_PAL       ) != 0;

				entry.dBaseEntry = imageDatabase.FindEntry( buffer.Ptr(), buffer.Size(), entry.pRom + entry.cRom );

				AddEntry( entry );
			}

			return TYPE_PROCESSED; 
		}

		return TYPE_INVALID;
	}

	Launcher::List::Files::Inserter::Type Launcher::List::Files::Inserter::ParseUnf() 
	{ 
		if (PrepareFile( 32, Managers::Paths::File::FILEID_UNIF ))
		{
			if (IsFileUnique())
			{
				Entry entry( Entry::UNF | compressed );

				cstring const end = buffer.End() - 8;
				uint pRom = 0, cRom = 0;

				for (char* it = buffer.At(32); it <= end; it += 8 + reinterpret_cast<const u32&>(it[4]))
				{
					switch (reinterpret_cast<const u32&>(*it))
					{
						case NST_FOURCC('N','A','M','E'):
					
							// limit to 255 characters by looking at the first byte only
							if (it+1 < end && reinterpret_cast<const u8&>(it[4]) > 1)
							{
								// in case string is not terminated
								it[8-1 + NST_MIN( it[4], end-it )] = '\0';
								entry.name = (strings << (it+8));
							}
							break;
					
						case NST_FOURCC('P','R','G','0'):
						case NST_FOURCC('P','R','G','1'):
						case NST_FOURCC('P','R','G','2'):
						case NST_FOURCC('P','R','G','3'):
						case NST_FOURCC('P','R','G','4'):
						case NST_FOURCC('P','R','G','5'):
						case NST_FOURCC('P','R','G','6'):
						case NST_FOURCC('P','R','G','7'):
						case NST_FOURCC('P','R','G','8'):
						case NST_FOURCC('P','R','G','9'):
						case NST_FOURCC('P','R','G','A'):
						case NST_FOURCC('P','R','G','B'):
						case NST_FOURCC('P','R','G','C'):
						case NST_FOURCC('P','R','G','D'):
						case NST_FOURCC('P','R','G','E'):
						case NST_FOURCC('P','R','G','F'):
					
							entry.pRom = (u16) ((pRom += reinterpret_cast<const u32&>(it[4])) / Nes::Core::SIZE_1K);
							break;
					
						case NST_FOURCC('C','H','R','0'):
						case NST_FOURCC('C','H','R','1'):
						case NST_FOURCC('C','H','R','2'):
						case NST_FOURCC('C','H','R','3'):
						case NST_FOURCC('C','H','R','4'):
						case NST_FOURCC('C','H','R','5'):
						case NST_FOURCC('C','H','R','6'):
						case NST_FOURCC('C','H','R','7'):
						case NST_FOURCC('C','H','R','8'):
						case NST_FOURCC('C','H','R','9'):
						case NST_FOURCC('C','H','R','A'):
						case NST_FOURCC('C','H','R','B'):
						case NST_FOURCC('C','H','R','C'):
						case NST_FOURCC('C','H','R','D'):
						case NST_FOURCC('C','H','R','E'):
						case NST_FOURCC('C','H','R','F'):
					
							entry.cRom = (u16) ((cRom += reinterpret_cast<const u32&>(it[4])) / Nes::Core::SIZE_1K);
							break;
					
						case NST_FOURCC('T','V','C','I'):
					
							if (it < end)
							{
								switch (it[8])
								{
									case 1: entry.bits.pal = TRUE; break;
									case 2:	entry.bits.pal = TRUE;								
									default: entry.bits.ntsc = TRUE; break;
								}
							}
							break;
					
						case NST_FOURCC('B','A','T','R'):
					
							entry.bits.battery = TRUE;
							break;
					
						case NST_FOURCC('M','I','R','R'):
					
							if (it < end)
							{
								switch (it[8])
								{
									case 0: entry.bits.mirroring = Entry::MIRROR_HORIZONTAL; break;
									case 1: entry.bits.mirroring = Entry::MIRROR_VERTICAL;   break;
									case 2: entry.bits.mirroring = Entry::MIRROR_ZERO;       break;
									case 3: entry.bits.mirroring = Entry::MIRROR_ONE;        break;
									case 4: entry.bits.mirroring = Entry::MIRROR_FOURSCREEN; break;
									case 5: entry.bits.mirroring = Entry::MIRROR_CONTROLLED; break;
								}
							}
							break;
					}				
				}

				AddEntry( entry );
			}

			return TYPE_PROCESSED; 
		}

		return TYPE_INVALID;
	}

	Launcher::List::Files::Inserter::Type Launcher::List::Files::Inserter::ParseFds() 
	{ 
		const ibool hasHeader = PrepareFile( 16, Managers::Paths::File::FILEID_FDS );

		if (hasHeader || PrepareFile( 4, Managers::Paths::File::FILEID_FDS_RAW ))
		{
			if (IsFileUnique())
			{
				Entry entry( Entry::FDS | compressed );

				entry.pRom = (u16) ((buffer.Size() - (hasHeader ? 16 : 0)) / Nes::Core::SIZE_1K);
				entry.wRam = 32;
				entry.bits.ntsc = TRUE;

				AddEntry( entry );
			}

			return TYPE_PROCESSED; 
		}

		return TYPE_INVALID;
	}

	Launcher::List::Files::Inserter::Type Launcher::List::Files::Inserter::ParseNsf() 
	{ 
		if (PrepareFile( 128, Managers::Paths::File::FILEID_NSF ))
		{
			if (IsFileUnique())
			{
				enum
				{
					NSF_CHIP_FDS  = 0x4,
					NSF_CHIP_MMC5 = 0x8
				};

            #pragma pack(push,1)

				struct Header
				{
					u8   pad1[14];
					char name[32];
					u8   pad2[32];
					char maker[32];
					u8   pad3[12];
					u8   mode;
					u8   chip;
					u8   pad4[4];
				};

            #pragma pack(pop)

				NST_COMPILE_ASSERT( sizeof(Header) == 128 );

				Header& header = reinterpret_cast<Header&>( buffer.Front() );

				Entry entry( Entry::NSF | compressed );

				entry.pRom = (u16) ((buffer.Size() - sizeof(Header)) / Nes::Core::SIZE_1K);

				switch (header.mode & 0x3)
				{
					case 0x0: entry.bits.ntsc = true;  entry.bits.pal = false; break;
					case 0x1: entry.bits.ntsc = false; entry.bits.pal = true;  break;
					default:  entry.bits.ntsc = true;  entry.bits.pal = true;  break;
				}

				switch (header.chip)
				{
					case NSF_CHIP_MMC5: entry.wRam = 8+1;  break;
					case NSF_CHIP_FDS:  entry.wRam = 8+32; break;
					default:            entry.wRam = 8;    break;
				}

				header.pad2[0] = '\0'; // in case string is not terminated
				entry.name = (strings << header.name);

				header.pad3[0] = '\0'; // in case string is not terminated
				entry.maker = (strings << header.maker);

				AddEntry( entry );
			}

			return TYPE_PROCESSED; 
		}

		return TYPE_INVALID;
	}

	Launcher::List::Files::Inserter::Type Launcher::List::Files::Inserter::ParseIps() 
	{ 
		if (PrepareFile( 5, Managers::Paths::File::FILEID_IPS ))
		{
			if (IsFileUnique())
			{
				Entry entry( Entry::IPS | compressed );
				AddEntry( entry );

				return TYPE_PROCESSED;
			}
		}

		return TYPE_INVALID;
	}

	Launcher::List::Files::Inserter::Type Launcher::List::Files::Inserter::ParseNsp() 
	{ 
		if (PrepareFile())
		{
			if (IsFileUnique())
			{
				Entry entry( Entry::NSP | compressed );
				AddEntry( entry );

				return TYPE_PROCESSED;
			}
		}

		return TYPE_INVALID;
	}

	Launcher::List::Files::Inserter::Type Launcher::List::Files::Inserter::ParseArchive() 
	{ 
		compressed = Entry::ARCHIVE;

		Io::File file;

		try
		{
			file.Open( path.string, Io::File::READ|Io::File::EXISTING );
		}
		catch (Io::File::Exception)
		{
			return TYPE_PROCESSED;
		}

		const Io::Archive archive( file );
		const uint length = path.string.Length();

		for (uint i=0; i < archive.NumFiles(); ++i)
		{
			NST_VERIFY( archive[i].GetName().Length() );

			// will look like: "archive.zip <image.nes>"
			path.string << " <" << archive[i].GetName();

			Parser const parser = GetParser();

			if (parser && parser != &Inserter::ParseArchive)
			{
				path.string << '>';
				buffer.Resize( archive[i].Size() );

				if (archive[i].Uncompress( buffer.Ptr() ))
					(*this.*parser)();
			}

			path.string.ShrinkTo( length );
		}

		return TYPE_PROCESSED;
	}

	Launcher::List::Files::Inserter::Type Launcher::List::Files::Inserter::ParseAny() 
	{
		const ibool notCompressed = buffer.Empty();

		if 
		(
			(!include[Include::NES] || ParseNes() == TYPE_INVALID) &&
			(!include[Include::UNF] || ParseUnf() == TYPE_INVALID) &&
			(!include[Include::FDS] || ParseFds() == TYPE_INVALID) &&
			(!include[Include::NSF] || ParseNsf() == TYPE_INVALID) &&
			(!include[Include::IPS] || ParseIps() == TYPE_INVALID) &&
			( include[Include::ARCHIVE] && notCompressed)
		)
			ParseArchive();

		return TYPE_PROCESSED;
	}

	class Launcher::List::Files::Searcher : public Inserter
	{
	public:

		Searcher
		(
			Strings&,
			Entries&,
			const Settings&,
			const Nes::Cartridge::Database&
		);

		~Searcher();

		void Search();

	private:

		struct Handlers;

		enum Abort
		{
			ABORT_SEARCH
		};

		typedef std::set<HeapString> SearchedPaths;

		ibool OnInitDialog (Param&);
		ibool OnCmdAbort   (Param&);

		ibool IsPathUnique();
		void  Start(System::Thread::Interrupt);
		void  Search(System::Thread::Interrupt);
		void  ReadPath(tstring const,System::Thread::Interrupt);

		const Settings::Folders& folders;
		SearchedPaths searchedPaths;
		Dialog dialog;
		System::Thread thread;
	};

	struct Launcher::List::Files::Searcher::Handlers
	{
		static const MsgHandler::Entry<Searcher> messages[];
		static const MsgHandler::Entry<Searcher> commands[];
	};

	const MsgHandler::Entry<Launcher::List::Files::Searcher> 
	Launcher::List::Files::Searcher::Handlers::messages[] =
	{
		{ WM_INITDIALOG, &Searcher::OnInitDialog }
	};

	const MsgHandler::Entry<Launcher::List::Files::Searcher> 
	Launcher::List::Files::Searcher::Handlers::commands[] =
	{
		{ IDC_LAUNCHER_FILESEARCH_ABORT, &Searcher::OnCmdAbort }
	};

	Launcher::List::Files::Searcher::Searcher
	(
		Strings& v,
		Entries& e,
		const Settings& s,
		const Nes::Cartridge::Database& r
	)
	: 
	Inserter (v,e,s.include,r),
	folders  (s.folders), 
	dialog   (IDD_LAUNCHER_FILESEARCH,this,Handlers::messages,Handlers::commands)
	{}

	Launcher::List::Files::Searcher::~Searcher()
	{
	}

	void Launcher::List::Files::Searcher::Start(System::Thread::Interrupt interrupt)
	{
		if (interrupt.Demanding())
			interrupt.Acknowledge();

		try
		{
			for (Settings::Folders::const_iterator it(folders.begin()); it != folders.end(); ++it)
			{
				if (it->path.Length())
				{
					path.string = it->path;

					if (IsPathUnique())
					{
						path.incSubDir = it->incSubDir;
						path.reference = PATH_NOT_ADDED;
						Search( interrupt );
					}
				}
			}
		}
		catch (Stop)
		{
		}
		catch (...)
		{
			dialog.Close();
			return;
		}

		dialog.Close();

		saveStrings = strings;
		saveEntries = entries;
	}

	ibool Launcher::List::Files::Searcher::OnInitDialog(Param&)
	{
		thread.Create( this, &Searcher::Start, dialog, System::Thread::START );
		return TRUE;
	}

	ibool Launcher::List::Files::Searcher::OnCmdAbort(Param& param)
	{
		if (param.Button().IsClicked())
			dialog.Close();

		return TRUE;
	}

	void Launcher::List::Files::Searcher::Search()
	{
		if (folders.size() && (include.Word() & Include::FILES))
		{
			const Generic mainWindow( Application::Instance::GetMainWindow() );
			const ibool enabled = !mainWindow.Disable();

			dialog.Open();

			mainWindow.Enable( enabled );
		}
	}

	void Launcher::List::Files::Searcher::Search(System::Thread::Interrupt interrupt)
	{
		NST_ASSERT( path.string.Length() );

		struct FileFinder
		{
			WIN32_FIND_DATA data;
			HANDLE const handle;

			FileFinder(tstring path)
			: handle(::FindFirstFile( path, &data )) {}

			~FileFinder()
			{
				if (handle != INVALID_HANDLE_VALUE)
					::FindClose( handle );
			}
		};

		path.string.File() = _T("*.*");

		FileFinder findFile( path.string.Ptr() );

		path.string.File().Clear();

		if (findFile.handle != INVALID_HANDLE_VALUE)
		{
			do 
			{	
				if (interrupt.DemandingTermination())
					throw ABORT_SEARCH;

				if (!(findFile.data.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM)))
				{
					if (findFile.data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
						ReadPath( findFile.data.cFileName, interrupt );
					else
						ReadFile( findFile.data.cFileName, dialog );
				}
			} 
			while (::FindNextFile( findFile.handle, &findFile.data ));
		}
	}

	ibool Launcher::List::Files::Searcher::IsPathUnique()
	{
		NST_ASSERT( path.string.Length() );

		SearchedPaths::iterator it( searchedPaths.lower_bound( path.string ) );

		if (it == searchedPaths.end() || *it != path.string)
		{
			searchedPaths.insert( it, path.string );
			return TRUE;
		}

		return FALSE;
	}

	void Launcher::List::Files::Searcher::ReadPath(tstring const subDir,System::Thread::Interrupt interrupt)
	{
		NST_ASSERT( subDir );

		if (path.incSubDir && *subDir && *subDir != '.')
		{
			path.string.Directory() += subDir;

			if (IsPathUnique())
			{
				const uint reference = path.reference;
				path.reference = PATH_NOT_ADDED;
				Search( interrupt );
				path.reference = reference;
			}

			path.string.Directory() -= 1;
		}
	}

	Launcher::List::Files::Files(const Nes::Cartridge::Database& imageDatabase)
	: dirty(FALSE)
	{
		const Path fileName( Application::Instance::GetPath(_T("launcher.nsd")) );

		try
		{
			const Io::File file( fileName, Io::File::COLLECT );

			Header header;
			file >> header;

			NST_VERIFY
			(
				header.id == Header::ID &&
				header.version == Header::VERSION &&
				header.numEntries && header.numEntries <= Header::MAX_ENTRIES &&
				header.numStrings && header.stringSize
			);

			if 
			(
				header.id != Header::ID ||
				header.version != Header::VERSION ||
				!header.numEntries || header.numEntries > Header::MAX_ENTRIES ||
				!header.numStrings || !header.stringSize ||
				!strings.Import( file, header.stringSize, header.flags & Header::FLAGS_UTF16 ) ||
				strings.Count() != header.numStrings
			)
				throw ERR_CORRUPT_DATA;

			entries.Resize( header.numEntries );

			for (uint i=0; i < header.numEntries; ++i)
			{
				Entry entry;

				file >> entry.file
					 >> entry.path
					 >> entry.type;

				switch (entry.type & Entry::ALL)
				{
					case Entry::UNF:
				
						file >> entry.name
							 >> entry.maker;
				
					case Entry::NES:
				
						file >> entry.pRom
							 >> entry.cRom 
							 >> entry.wRam 
							 >> entry.mapper
						 	 >> entry.flags;
				
						entry.dBaseEntry = imageDatabase.FindEntry( file.Read<u32>() );
						break;
				
					case Entry::NSF:
				
						file >> entry.name
							 >> entry.maker
							 >> entry.pRom
							 >> entry.wRam 
							 >> entry.flags; 
				
						entry.flags &= Entry::FLAGS_NTSC_PAL;
						break;
				
					case Entry::FDS:
				
						file >> entry.pRom;
				
						entry.wRam = 32;
						entry.bits.ntsc = TRUE;
				
					case Entry::IPS:
					case Entry::NSP:
						break;

					default: throw ERR_CORRUPT_DATA;
				}

				NST_VERIFY
				(
					entry.file  < header.stringSize &&
					entry.path  < header.stringSize &&
					entry.name  < header.stringSize &&
					entry.maker < header.stringSize
				);

				if 
				(
					entry.file  >= header.stringSize ||
					entry.path  >= header.stringSize ||
					entry.name  >= header.stringSize ||
					entry.maker >= header.stringSize
				)
					throw ERR_CORRUPT_DATA;

				entries[i] = entry;
			}

			return;		
		}
		catch (Io::File::Exception id)
		{
			if (id == Io::File::ERR_NOT_FOUND)
			{
				Io::Log() << "Launcher: database file \"launcher.nsd\" not present\r\n";
				return;			   
			}
		}
		catch (Exception)
		{
		}
		catch (...)
		{
		}

		dirty = TRUE;

		Clear();
		User::Warn( IDS_INVALID_LAUNCHERFILE );
	}

	Launcher::List::Files::~Files()
	{
	}

	void Launcher::List::Files::Save(const Nes::Cartridge::Database& imageDatabase)
	{
		if (dirty)
		{
			Io::Log log;

			const Path fileName( Application::Instance::GetPath(_T("launcher.nsd")) );

			NST_ASSERT( bool(entries.Size()) <= bool(strings.Size()) );

			Defrag();

			if (entries.Size())
			{
				try
				{
					const Io::File file( fileName, Io::File::DUMP );

					{
						Header header;

						header.id = Header::ID;
						header.version = Header::VERSION;
						header.numEntries = entries.Size();
						header.numStrings = strings.Count();
						header.stringSize = strings.Size();
						header.flags = (strings.IsUTF16() ? Header::FLAGS_UTF16 : 0);

						file << header;
					}

					strings.Export( file );

					Collection::Buffer buffer;
					buffer.Reserve( entries.Size() * sizeof(Entry) );

					for (Entries::ConstIterator it=entries.Begin(), end=entries.End(); it != end; ++it)
					{
						NST_ASSERT( it->type );

						buffer << it->file 
							   << it->path 
							   << it->type;

						switch (it->type & Entry::ALL)
						{
							case Entry::NSF:
						
								NST_ASSERT( !(it->flags & ~Entry::FLAGS_NTSC_PAL) );
						
								buffer << it->name
									   << it->maker
									   << it->pRom
									   << it->wRam
									   << it->flags;
						
								break;
						
							case Entry::FDS:
						
								buffer << it->pRom;
								break;
						
							case Entry::UNF:
						
								buffer << it->name
									   << it->maker;
						
							case Entry::NES:
						
								buffer << it->pRom
									   << it->cRom
									   << it->wRam
									   << it->mapper
									   << it->flags
									   << (u32) (it->dBaseEntry ? imageDatabase.GetCrc(it->dBaseEntry) : 0);
						
								break;
						}
					}

					file.Stream() << buffer;

					log << "Launcher: database saved to \"launcher.nsd\"\r\n";
				}
				catch (...)
				{
					User::Warn( IDS_LAUNCHERFILE_ERR_SAVE );
				}
			}
			else if (Io::File::FileExist( fileName.Ptr() ))
			{
				if (Io::File::Delete( fileName.Ptr() ))
					log << "Launcher: empty database, deleted \"launcher.nsd\"\r\n";
				else
					log << "Launcher: warning, couldn't delete \"launcher.nsd\"!\r\n";
			}
		}
	}

	void Launcher::List::Files::Defrag()
	{
		typedef Collection::Map<GenericString,uint> References;

		References references;

		if (entries.Size())
		{
			Entries tmp;
			tmp.Reserve( entries.Size() );

			for (Entries::ConstIterator it=entries.Begin(), end=entries.End(); it != end; ++it)
			{
				if (it->type)
				{
					if ( it->file  ) references( strings[ it->file  ] );
					if ( it->path  ) references( strings[ it->path  ] );
					if ( it->name  ) references( strings[ it->name  ] );
					if ( it->maker ) references( strings[ it->maker ] );

					tmp.PushBack( *it );
				}
			}

			if (entries.Size() != tmp.Size())
				entries = tmp;
		}

		if (entries.Size())
		{
			Strings tmp( strings.Size() );

			for (References::Iterator it=references.Begin(); it != references.End(); ++it)
				it->value = (tmp << it->key);

			for (Entries::Iterator it=entries.Begin(); it != entries.End(); ++it)
			{
				if ( it->file  ) it->file  = references.Locate( strings[ it->file  ] );
				if ( it->path  ) it->path  = references.Locate( strings[ it->path  ] );
				if ( it->name  ) it->name  = references.Locate( strings[ it->name  ] );
				if ( it->maker ) it->maker = references.Locate( strings[ it->maker ] );
			}

			strings = tmp;
		}
		else
		{
			Clear();
		}
	}

	ibool Launcher::List::Files::Insert(const Nes::Cartridge::Database& imageDatabase,const GenericString fileName)
	{
		return dirty |=
		(
			fileName.Length() &&
			Inserter( strings, entries, Paths::Settings::Include(true), imageDatabase ).Add( fileName )
		);
	}

	ibool Launcher::List::Files::ShouldDefrag() const
	{
		uint garbage = 0;

		for (uint i=0; i < entries.Size(); ++i)
		{
			garbage += (entries[i].type == 0);

			if (garbage > GARBAGE_THRESHOLD)
				return TRUE;
		}

		return FALSE;
	}

	void Launcher::List::Files::Clear()
	{	
		if (entries.Size())
			dirty = TRUE;

		entries.Destroy();
		strings.Clear();
	}

	void Launcher::List::Files::Refresh
	(
		const Paths::Settings& settings,
		const Nes::Cartridge::Database& imageDatabase
	)
	{
		dirty = TRUE;
		Searcher( strings, entries, settings, imageDatabase ).Search();
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("t", on)
    #endif

	tstring Launcher::List::Files::Entry::GetName(const Strings& strings,const Nes::Cartridge::Database* db) const 
	{ 
		if (name || !dBaseEntry || !db)
			return GetName( strings );
		else
			return _T("-");
	}

	uint Launcher::List::Files::Entry::GetSystem() const
	{
		if (bits.vs)
		{
			return SYSTEM_VS;
		}
		else if (bits.ntsc && bits.pal)
		{
			return SYSTEM_NTSC_PAL;
		}
		else if (bits.ntsc)
		{
			return SYSTEM_NTSC;
		}
		else if (bits.pal)
		{
			return SYSTEM_PAL;
		}

		return SYSTEM_UNKNOWN;
	}

	uint Launcher::List::Files::Entry::GetMirroring(const Nes::Cartridge::Database* db) const
	{
		if (dBaseEntry && db)
		{
			uint m = db->GetMirroring( dBaseEntry );

			return 
			(
				m == Nes::Cartridge::MIRROR_HORIZONTAL ? MIRROR_HORIZONTAL : 
				m == Nes::Cartridge::MIRROR_VERTICAL   ? MIRROR_VERTICAL : 
				m == Nes::Cartridge::MIRROR_FOURSCREEN ? MIRROR_FOURSCREEN :
				m == Nes::Cartridge::MIRROR_ZERO       ? MIRROR_ZERO : 
				m == Nes::Cartridge::MIRROR_ONE        ? MIRROR_ONE : 
				                                         MIRROR_CONTROLLED
			);
		}

		return bits.mirroring;
	}

	uint Launcher::List::Files::Entry::GetSystem(const Nes::Cartridge::Database* db) const
	{
		if (dBaseEntry && db)
		{
			Nes::Cartridge::System s = db->GetSystem( dBaseEntry );

			return 
			(
				s == Nes::Cartridge::SYSTEM_VS       ? SYSTEM_VS : 
				s == Nes::Cartridge::SYSTEM_PC10     ? SYSTEM_PC10 :
				s == Nes::Cartridge::SYSTEM_PAL      ? SYSTEM_PAL :
				s == Nes::Cartridge::SYSTEM_NTSC_PAL ? SYSTEM_NTSC_PAL :
				                                       SYSTEM_NTSC
			);
		}

		return GetSystem();
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("", on)
    #endif
}

