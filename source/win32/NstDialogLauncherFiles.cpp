////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2007 Martin Freij
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
#include "NstIoLog.hpp"
#include "NstManagerPaths.hpp"
#include "NstDialogLauncher.hpp"
#include "../core/NstCrc32.hpp"
#include "../core/api/NstApiCartridge.hpp"

namespace Nestopia
{
	namespace Window
	{
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
		type       (t),
		attributes (0)
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

			bool Add(const GenericString);

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

			const Include include;
			uint compressed;

			struct
			{
				Path string;
				bool incSubDir;
				uint reference;
			}   path;

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

			typedef Collection::Set<uint> FileChecksums;
			typedef Collection::Buffer Buffer;
			typedef Type (Inserter::*Parser)();

			Parser GetParser() const;

			inline uint Crc(const uint,const uint) const;

			bool UniqueFile();
			bool PrepareFile(const uint=1,const uint=0);
			Type ParseAny();
			Type ParseNes();
			Type ParseUnf();
			Type ParseFds();
			Type ParseNsf();
			Type ParseIps();
			Type ParseNsp();
			Type ParseArchive();
			void AddEntry(const Entry&);

			Buffer buffer;
			FileChecksums parsedFiles;
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
		include       ( i ),
		saveStrings   ( v ),
		saveEntries   ( e ),
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

			if (entries.Size() == HEADER_MAX_ENTRIES)
				throw STOP_SEARCH;
		}

		bool Launcher::List::Files::Inserter::Add(const GenericString fileName)
		{
			NST_ASSERT( fileName.Length() && fileName.Length() <= _MAX_PATH );

			if (entries.Size() == HEADER_MAX_ENTRIES)
				return false;

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
					return false;
				}
				catch (Stop)
				{
					// reached file count limit
				}

				if (entries.Size())
				{
					saveEntries.PushBack( entries );
					saveStrings = strings;
					return true;
				}
			}

			return false;
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

		inline uint Launcher::List::Files::Inserter::Crc(const uint start,const uint length) const
		{
			NST_ASSERT( buffer.Size() );
			return Nes::Core::Crc32::Compute( &buffer[start], length );
		}

		bool Launcher::List::Files::Inserter::UniqueFile()
		{
			NST_ASSERT( buffer.Size() );
			return !include[Include::UNIQUE] || parsedFiles.Insert(Crc(0,buffer.Size()));
		}

		bool Launcher::List::Files::Inserter::PrepareFile(const uint minSize,const uint fileId)
		{
			NST_ASSERT( path.string.Length() && minSize && minSize >= bool(fileId) * 4U );

			if (buffer.Empty())
			{
				const Io::File file( path.string, Io::File::COLLECT );
				const uint size = file.Size();

				if (size >= minSize && (!fileId || fileId == file.Peek32()))
				{
					buffer.Resize( size );
					file.Read( buffer.Ptr(), size );
					return true;
				}

				return false;
			}

			return
			(
				buffer.Size() >= minSize &&
				(!fileId || fileId == NST_FOURCC(buffer[0],buffer[1],buffer[2],buffer[3]))
			);
		}

		Launcher::List::Files::Inserter::Type Launcher::List::Files::Inserter::ParseNes()
		{
			if (PrepareFile( 16, Managers::Paths::File::ID_INES ))
			{
				Nes::Api::Cartridge::Setup setup;

				if (UniqueFile())
				{
					Nes::Api::Cartridge::ReadNesHeader( setup, buffer.Ptr(), buffer.Size() );

					Entry entry( Entry::NES | compressed );

					entry.pRom = setup.prgRom / Nes::Core::SIZE_1K;
					entry.cRom = setup.chrRom / Nes::Core::SIZE_1K;
					entry.wRam = (setup.wrkRam + setup.wrkRamBacked) / Nes::Core::SIZE_1K;
					entry.mapper = setup.mapper;

					entry.attributes =
					(
						( setup.wrkRamBacked >= Nes::Core::SIZE_1K ? Entry::ATR_BATTERY : 0 ) |
						( setup.trainer                            ? Entry::ATR_TRAINER : 0 ) |
						( setup.system == Nes::SYSTEM_VS           ? Entry::ATR_VS      : 0 ) |
						(
							setup.region == Nes::REGION_BOTH ? Entry::ATR_NTSC_PAL :
							setup.region == Nes::REGION_PAL  ? Entry::ATR_PAL :
                                                               Entry::ATR_NTSC
						)
							|
						(
							setup.mirroring == Nes::Api::Cartridge::MIRROR_FOURSCREEN ? Entry::MIRROR_FOURSCREEN :
							setup.mirroring == Nes::Api::Cartridge::MIRROR_VERTICAL   ? Entry::MIRROR_VERTICAL   :
																						Entry::MIRROR_HORIZONTAL
						)
					);

					entry.dBaseEntry = imageDatabase.FindEntry( buffer.Ptr(), buffer.Size() );

					AddEntry( entry );
				}

				return TYPE_PROCESSED;
			}

			return TYPE_INVALID;
		}

		Launcher::List::Files::Inserter::Type Launcher::List::Files::Inserter::ParseUnf()
		{
			if (PrepareFile( 32, Managers::Paths::File::ID_UNIF ))
			{
				if (UniqueFile())
				{
					Entry entry( Entry::UNF | compressed );

					cstring const end = buffer.End() - 8;
					uint pRom = 0, cRom = 0;

					for (char* it = buffer.At(32); it <= end; it += 8 + NST_FOURCC(it[4],it[5],it[6],it[7]))
					{
						switch (NST_FOURCC(it[0],it[1],it[2],it[3]))
						{
							case NST_FOURCC('N','A','M','E'):

								// limit to 255 characters by looking at the first byte only
								if (it+1 < end && NST_FOURCC(it[4],it[5],it[6],it[7]) > 1)
								{
									// in case string is not terminated
									it[8-1 + NST_MIN( it[4], end-it )] = '\0';
									entry.name = strings.Import( it+8 );
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

								entry.pRom = (pRom += NST_FOURCC(it[4],it[5],it[6],it[7])) / Nes::Core::SIZE_1K;
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

								entry.cRom = (cRom += NST_FOURCC(it[4],it[5],it[6],it[7])) / Nes::Core::SIZE_1K;
								break;

							case NST_FOURCC('T','V','C','I'):

								if (it < end)
								{
									switch (it[8])
									{
										case 1:  entry.attributes |= Entry::ATR_PAL;      break;
										case 2:  entry.attributes |= Entry::ATR_NTSC_PAL; break;
										default: entry.attributes |= Entry::ATR_NTSC;     break;
									}
								}
								break;

							case NST_FOURCC('B','A','T','R'):

								entry.attributes |= Entry::ATR_BATTERY;
								break;

							case NST_FOURCC('M','I','R','R'):

								if (it < end)
								{
									entry.attributes &= ~uint(Entry::ATR_MIRRORING);

									switch (it[8])
									{
										case 0: entry.attributes |= Entry::MIRROR_HORIZONTAL; break;
										case 1: entry.attributes |= Entry::MIRROR_VERTICAL;   break;
										case 2: entry.attributes |= Entry::MIRROR_ZERO;       break;
										case 3: entry.attributes |= Entry::MIRROR_ONE;        break;
										case 4: entry.attributes |= Entry::MIRROR_FOURSCREEN; break;
										case 5: entry.attributes |= Entry::MIRROR_CONTROLLED; break;
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
			const bool hasHeader = PrepareFile( 16, Managers::Paths::File::ID_FDS );

			if (hasHeader || PrepareFile( 4, Managers::Paths::File::ID_FDS_RAW ))
			{
				if (UniqueFile())
				{
					Entry entry( Entry::FDS | compressed );

					const uint size = buffer.Size() - (hasHeader ? 16 : 0);
					entry.pRom = (size / Nes::Core::SIZE_1K) + (size % Nes::Core::SIZE_1K != 0);
					entry.wRam = 32;
					entry.attributes |= Entry::ATR_NTSC;

					AddEntry( entry );
				}

				return TYPE_PROCESSED;
			}

			return TYPE_INVALID;
		}

		Launcher::List::Files::Inserter::Type Launcher::List::Files::Inserter::ParseNsf()
		{
			if (PrepareFile( 128, Managers::Paths::File::ID_NSF ))
			{
				if (UniqueFile())
				{
					enum
					{
						NSF_CHIP_FDS  = 0x4,
						NSF_CHIP_MMC5 = 0x8
					};

					#pragma pack(push,1)

					struct Header
					{
						uchar pad1[14];
						char  name[32];
						uchar pad2[32];
						char  maker[32];
						uchar pad3[12];
						uchar mode;
						uchar chip;
						uchar pad4[4];
					};

					#pragma pack(pop)

					NST_COMPILE_ASSERT( sizeof(Header) == 128 );

					Header& header = reinterpret_cast<Header&>( buffer.Front() );

					Entry entry( Entry::NSF | compressed );

					const uint size = buffer.Size() - sizeof(Header);
					entry.pRom = (size / Nes::Core::SIZE_1K) + (size % Nes::Core::SIZE_1K != 0);

					switch (header.mode & 0x3U)
					{
						case 0x0: entry.attributes |= Entry::ATR_NTSC;     break;
						case 0x1: entry.attributes |= Entry::ATR_PAL;      break;
						default:  entry.attributes |= Entry::ATR_NTSC_PAL; break;
					}

					switch (header.chip)
					{
						case NSF_CHIP_MMC5: entry.wRam = 8+1;  break;
						case NSF_CHIP_FDS:  entry.wRam = 8+32; break;
						default:            entry.wRam = 8;    break;
					}

					header.pad2[0] = '\0'; // in case string is not terminated
					entry.name = strings.Import( header.name );

					header.pad3[0] = '\0'; // in case string is not terminated
					entry.maker = strings.Import( header.maker );

					AddEntry( entry );
				}

				return TYPE_PROCESSED;
			}

			return TYPE_INVALID;
		}

		Launcher::List::Files::Inserter::Type Launcher::List::Files::Inserter::ParseIps()
		{
			if (PrepareFile( 5, Managers::Paths::File::ID_IPS ))
			{
				if (UniqueFile())
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
				if (UniqueFile())
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
			const bool notCompressed = buffer.Empty();

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

		class Launcher::List::Files::Searcher : Inserter
		{
		public:

			Searcher
			(
				Strings&,
				Entries&,
				const Settings&,
				const Nes::Cartridge::Database&
			);

			void Search();

		private:

			enum Abort
			{
				ABORT_SEARCH
			};

			typedef std::set<HeapString> SearchedPaths;

			ibool OnInitDialog (Param&);

			bool UniquePath();
			void Start(System::Thread::Terminator);
			void Search(System::Thread::Terminator);
			void ReadPath(tstring const,System::Thread::Terminator);

			const Settings::Folders& folders;
			SearchedPaths searchedPaths;
			Dialog dialog;
			System::Thread thread;
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
		dialog   (IDD_LAUNCHER_SEARCH,WM_INITDIALOG,this,&Searcher::OnInitDialog)
		{}

		void Launcher::List::Files::Searcher::Start(System::Thread::Terminator terminator)
		{
			try
			{
				for (Settings::Folders::const_iterator it(folders.begin()), end(folders.end()); it != end; ++it)
				{
					if (it->path.Length())
					{
						path.string = it->path;

						if (UniquePath())
						{
							path.incSubDir = it->incSubDir;
							path.reference = PATH_NOT_ADDED;
							Search( terminator );
						}
					}
				}
			}
			catch (Stop)
			{
			}
			catch (...)
			{
				static_cast<Generic&>(dialog).Close();
				return;
			}

			static_cast<Generic&>(dialog).Close();

			saveStrings = strings;
			saveEntries = entries;
		}

		ibool Launcher::List::Files::Searcher::OnInitDialog(Param&)
		{
			thread.Start( System::Thread::Callback(this,&Searcher::Start) );
			return true;
		}

		void Launcher::List::Files::Searcher::Search()
		{
			if (folders.size() && (include.Word() & Include::FILES))
			{
				bool enabled = !Application::Instance::GetMainWindow().Enable( false );
				dialog.Open();
				Application::Instance::GetMainWindow().Enable( enabled );
			}
		}

		void Launcher::List::Files::Searcher::Search(System::Thread::Terminator terminate)
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
					if (terminate)
						throw ABORT_SEARCH;

					if (!(findFile.data.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM)))
					{
						if (findFile.data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
							ReadPath( findFile.data.cFileName, terminate );
						else
							ReadFile( findFile.data.cFileName, dialog );
					}
				}
				while (::FindNextFile( findFile.handle, &findFile.data ));
			}
		}

		bool Launcher::List::Files::Searcher::UniquePath()
		{
			NST_ASSERT( path.string.Length() );

			SearchedPaths::iterator it( searchedPaths.lower_bound( path.string ) );

			if (it == searchedPaths.end() || *it != path.string)
			{
				searchedPaths.insert( it, path.string );
				return true;
			}

			return false;
		}

		void Launcher::List::Files::Searcher::ReadPath(tstring const subDir,System::Thread::Terminator terminator)
		{
			NST_ASSERT( subDir );

			if (path.incSubDir && *subDir && *subDir != '.')
			{
				path.string.Directory() += subDir;

				if (UniquePath())
				{
					const uint reference = path.reference;
					path.reference = PATH_NOT_ADDED;
					Search( terminator );
					path.reference = reference;
				}

				path.string.Directory() -= 1;
			}
		}

		Launcher::List::Files::Files()
		:
		dirty  (false),
		loaded (!Application::Instance::GetExePath(_T("launcher.nsd")).FileExists())
		{
			if (loaded)
				Io::Log() << "Launcher: database file \"launcher.nsd\" not present\r\n";
		}

		void Launcher::List::Files::Load(const Nes::Cartridge::Database& imageDatabase)
		{
			if (loaded)
				return;

			loaded = true;

			try
			{
				const Io::File file( Application::Instance::GetExePath(_T("launcher.nsd")), Io::File::COLLECT );

				uint numEntries, numStrings, stringSize;

				if
				(
					file.Read32() != HEADER_ID ||
					file.Read32() != HEADER_VERSION ||
					0 == (stringSize=file.Read32()) ||
					0 == (numStrings=file.Read32()) ||
					0 == (numEntries=file.Read32()) ||
					numEntries > HEADER_MAX_ENTRIES ||
					!strings.Import( file, stringSize, file.Read32() & HEADER_FLAGS_UTF16 ) ||
					strings.Count() != numStrings
				)
					throw ERR_CORRUPT_DATA;

				entries.Resize( numEntries );

				for (uint i=0; i < numEntries; ++i)
				{
					Entry entry;

					entry.file = file.Read32();
					entry.path = file.Read32();
					entry.type = file.Read8();

					switch (entry.type & Entry::ALL)
					{
						case Entry::UNF:

							entry.name = file.Read32();
							entry.maker = file.Read32();

						case Entry::NES:

							entry.pRom = file.Read16();
							entry.cRom = file.Read16();
							entry.wRam = file.Read16();
							entry.mapper = file.Read16();
							entry.attributes = file.Read8();

							entry.dBaseEntry = imageDatabase.FindEntry( file.Read32() );
							break;

						case Entry::NSF:

							entry.name = file.Read32();
							entry.maker = file.Read32();
							entry.pRom = file.Read16();
							entry.wRam = file.Read16();
							entry.attributes = file.Read8();

							entry.attributes &= Entry::ATR_NTSC_PAL;
							break;

						case Entry::FDS:

							entry.pRom = file.Read16();
							entry.wRam = 32;
							entry.attributes |= Entry::ATR_NTSC;

						case Entry::IPS:
						case Entry::NSP:
							break;

						default: throw ERR_CORRUPT_DATA;
					}

					NST_VERIFY
					(
						entry.file  < stringSize &&
						entry.path  < stringSize &&
						entry.name  < stringSize &&
						entry.maker < stringSize
					);

					if
					(
						entry.file  >= stringSize ||
						entry.path  >= stringSize ||
						entry.name  >= stringSize ||
						entry.maker >= stringSize
					)
						throw ERR_CORRUPT_DATA;

					entries[i] = entry;
				}
			}
			catch (...)
			{
				dirty = true;

				Clear();
				User::Warn( IDS_INVALID_LAUNCHERFILE );
			}
		}

		void Launcher::List::Files::Save(const Nes::Cartridge::Database& imageDatabase)
		{
			if (dirty)
			{
				Io::Log log;

				const Path fileName( Application::Instance::GetExePath(_T("launcher.nsd")) );

				NST_ASSERT( bool(entries.Size()) <= bool(strings.Size()) );

				Defrag();

				if (entries.Size())
				{
					try
					{
						const Io::File file( fileName, Io::File::DUMP );

						file.Write32( HEADER_ID      );
						file.Write32( HEADER_VERSION );
						file.Write32( strings.Size()  );
						file.Write32( strings.Count() );
						file.Write32( entries.Size()  );
						file.Write32( strings.IsUTF16() ? HEADER_FLAGS_UTF16 : 0 );

						strings.Export( file );

						for (Entries::ConstIterator it(entries.Begin()), end(entries.End()); it != end; ++it)
						{
							NST_ASSERT( it->type );

							file.Write32( it->file );
							file.Write32( it->path );
							file.Write8( it->type );

							switch (it->type & Entry::ALL)
							{
								case Entry::NSF:

									NST_ASSERT( !(it->attributes & ~uint(Entry::ATR_NTSC_PAL)) );

									file.Write32( it->name );
									file.Write32( it->maker );
									file.Write16( it->pRom );
									file.Write16( it->wRam );
									file.Write8( it->attributes );
									break;

								case Entry::FDS:

									file.Write16( it->pRom );
									break;

								case Entry::UNF:

									file.Write32( it->name );
									file.Write32( it->maker );

								case Entry::NES:

									file.Write16( it->pRom );
									file.Write16( it->cRom );
									file.Write16( it->wRam );
									file.Write16( it->mapper );
									file.Write8( it->attributes );
									file.Write32( it->dBaseEntry ? imageDatabase.GetCrc(it->dBaseEntry) : 0 );
									break;
							}
						}

						log << "Launcher: database saved to \"launcher.nsd\"\r\n";
					}
					catch (...)
					{
						User::Warn( IDS_LAUNCHERFILE_ERR_SAVE );
					}
				}
				else if (fileName.FileExists())
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

				for (Entries::ConstIterator it(entries.Begin()), end(entries.End()); it != end; ++it)
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

				for (References::Iterator it(references.Begin()), end(references.End()); it != end; ++it)
					it->value = (tmp << it->key);

				for (Entries::Iterator it(entries.Begin()), end(entries.End()); it != end; ++it)
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

		bool Launcher::List::Files::Insert(const Nes::Cartridge::Database& imageDatabase,const GenericString fileName)
		{
			return dirty |=
			(
				fileName.Length() &&
				Inserter( strings, entries, Paths::Settings::Include(true), imageDatabase ).Add( fileName )
			);
		}

		bool Launcher::List::Files::ShouldDefrag() const
		{
			uint garbage = 0;

			for (Entries::ConstIterator it(entries.Begin()), end(entries.End()); it != end; ++it)
			{
				garbage += (it->type == 0);

				if (garbage > GARBAGE_THRESHOLD)
					return true;
			}

			return false;
		}

		void Launcher::List::Files::Clear()
		{
			if (entries.Size())
				dirty = true;

			entries.Destroy();
			strings.Clear();
		}

		void Launcher::List::Files::Refresh
		(
			const Paths::Settings& settings,
			const Nes::Cartridge::Database& imageDatabase
		)
		{
			dirty = true;
			Searcher( strings, entries, settings, imageDatabase ).Search();
		}

		#ifdef NST_MSVC_OPTIMIZE
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
			if (attributes & ATR_VS)
			{
				return SYSTEM_VS;
			}
			else if ((attributes & ATR_NTSC_PAL) == ATR_NTSC_PAL)
			{
				return SYSTEM_NTSC_PAL;
			}
			else if (attributes & ATR_NTSC)
			{
				return SYSTEM_NTSC;
			}
			else if (attributes & ATR_NTSC_PAL)
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

			return attributes & ATR_MIRRORING;
		}

		uint Launcher::List::Files::Entry::GetSystem(const Nes::Cartridge::Database* db) const
		{
			if (dBaseEntry && db)
			{
				switch (db->GetSystem( dBaseEntry ))
				{
					case Nes::SYSTEM_VS:
						return SYSTEM_VS;

					case Nes::SYSTEM_PC10:
						return SYSTEM_PC10;
				}

				switch (db->GetRegion( dBaseEntry ))
				{
					case Nes::REGION_BOTH:
						return SYSTEM_NTSC_PAL;

					case Nes::REGION_PAL:
						return SYSTEM_PAL;
				}

				return SYSTEM_NTSC;
			}
			else
			{
				return GetSystem();
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}

