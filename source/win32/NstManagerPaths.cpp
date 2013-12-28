////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
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

#include "NstIoFile.hpp"
#include "NstIoArchive.hpp"
#include "NstIoIps.hpp"
#include "NstIoScreen.hpp"
#include "NstApplicationConfiguration.hpp"
#include "NstWindowUser.hpp"
#include "NstResourceString.hpp"
#include "NstWindowMenu.hpp"
#include "NstDialogBrowse.hpp"
#include "NstDialogPaths.hpp"
#include "NstManagerPaths.hpp"
#include "NstManagerPathsFilter.hpp"

namespace Nestopia
{
	using namespace Managers;

	cstring const Paths::recentDirCfgNames[NUM_RECENT_DIRS] =
	{
		"files last path images",
		"files last path scripts"
	};

	Paths::Paths(Emulator& e,const Configuration& cfg,Window::Menu& m)
	:
	emulator ( e ),
	menu     ( m ),
	dialog   ( new Window::Paths(cfg) )
	{
		m.Commands().Add( IDM_OPTIONS_PATHS, this, &Paths::OnMenu );
		emulator.Events().Add( this, &Paths::OnEmuEvent );

		for (uint i=0; i < NUM_RECENT_DIRS; ++i)
			recentDirs[i] = cfg[recentDirCfgNames[i]];

		UpdateSettings();
	}

	Paths::~Paths()
	{
		emulator.Events().Remove( this );
	}

	void Paths::Save(Configuration& cfg) const
	{
		dialog->Save( cfg );

		for (uint i=0; i < NUM_RECENT_DIRS; ++i)
		{
			if (recentDirs[i].DirectoryExists())
				cfg[recentDirCfgNames[i]].Quote() = GenericString(recentDirs[i]);
		}
	}

	void Paths::UpdateSettings()
	{
		emulator.WriteProtectCartridge
		(
			dialog->GetSetting(Window::Paths::READONLY_CARTRIDGE)
		);
	}

	void Paths::OnMenu(uint)
	{
		dialog->Open();
		UpdateSettings();
	}

	void Paths::OnEmuEvent(Emulator::Event event)
	{
		switch (event)
		{
			case Emulator::EVENT_NETPLAY_MODE_ON:
			case Emulator::EVENT_NETPLAY_MODE_OFF:

				menu[IDM_OPTIONS_PATHS].Enable( event == Emulator::EVENT_NETPLAY_MODE_OFF );
				break;
		}
	}

	ibool Paths::SaveSlotExportingEnabled() const
	{
		return dialog->GetSetting(Window::Paths::AUTO_EXPORT_STATE_SLOTS);
	}

	ibool Paths::SaveSlotImportingEnabled() const
	{
		return dialog->GetSetting(Window::Paths::AUTO_IMPORT_STATE_SLOTS);
	}

	ibool Paths::UseStateCompression() const
	{
		return dialog->GetSetting(Window::Paths::COMPRESS_STATES);
	}

	ibool Paths::LocateFile(Path& path,const File::Types types) const
	{
		NST_ASSERT( path.File().Length() );

		struct Lut
		{
			uint type;
			tchar extension[4];
		};

		static const Lut lut[17] =
		{
			{ File::INES,              _T( "nes" ) },
			{ File::UNIF,              _T( "unf" ) },
			{ File::FDS,               _T( "fds" ) },
			{ File::NSF,               _T( "nsf" ) },
			{ File::BATTERY,           _T( "sav" ) },
			{ File::TAPE,              _T( "tp"  ) },
			{ File::STATE|File::SLOTS, _T( "nst" ) },
			{ File::IPS,               _T( "ips" ) },
			{ File::MOVIE,             _T( "nsv" ) },
			{ File::SCRIPT,            _T( "nsp" ) },
			{ File::ROM,               _T( "rom" ) },
			{ File::PALETTE,           _T( "pal" ) },
			{ File::WAVE,              _T( "wav" ) },
			{ File::AVI,               _T( "avi" ) },
			{ File::ARCHIVE,           _T( "zip" ) },
			{ File::ARCHIVE,           _T( "rar" ) },
			{ File::ARCHIVE,           _T( "7z"  ) }
		};

		for (uint i=0; i < 17; ++i)
		{
			if (types( lut[i].type ))
			{
				path.Directory() = GetDefaultDirectory( lut[i].type );
				path.Extension() = lut[i].extension;

				if (path.FileExists())
					return true;
			}
		}

		return false;
	}

	ibool Paths::FindFile(Path& path) const
	{
		NST_ASSERT( path.File().Length() );

		path.Directory() = Application::Instance::GetExePath().Directory();

		if (path.FileExists())
			return true;

		for (uint i=0; i < Window::Paths::NUM_DIRS; ++i)
		{
			path.Directory() = dialog->GetDirectory( (Window::Paths::Type) i );

			if (path.FileExists())
				return true;
		}

		for (uint i=0; i < NUM_RECENT_DIRS; ++i)
		{
			path.Directory() = recentDirs[i];

			if (path.FileExists())
				return true;
		}

		path.Directory().Clear();

		return false;
	}

	void Paths::UpdateRecentDirectory(const Path& path,const File::Types types) const
	{
		NST_ASSERT( path.Length() );

		if (types( File::IMAGE|File::SCRIPT ))
			recentDirs[ types(File::IMAGE) ? RECENT_DIR_IMAGE : RECENT_DIR_SCRIPT ] = path.Directory();
	}

	Path Paths::GetScreenShotPath() const
	{
		Path path;

		path.Directory() = dialog->GetDirectory( Window::Paths::DIR_SCREENSHOT );
		path.File() = emulator.GetImagePath().Target().File();
		path.Extension().Clear();

		const uint offset = path.Length() + 1;
		path << _T("_xx.") << dialog->GetScreenShotExtension();

		for (uint i=1; i < 100; ++i)
		{
			path[offset+0] = (tchar) ('0' + (i / 10));
			path[offset+1] = (tchar) ('0' + (i % 10));

			if (!path.FileExists())
				break;
		}

		return path;
	}

	tstring Paths::GetDefaultExtension(const File::Types types)
	{
		return
		(
			types( File::INES              ) ? _T( "nes" ) :
			types( File::UNIF              ) ? _T( "unf" ) :
			types( File::FDS               ) ? _T( "fds" ) :
			types( File::NSF               ) ? _T( "nsf" ) :
			types( File::BATTERY           ) ? _T( "sav" ) :
			types( File::TAPE              ) ? _T( "tp"  ) :
			types( File::STATE|File::SLOTS ) ? _T( "nst" ) :
			types( File::IPS               ) ? _T( "ips" ) :
			types( File::MOVIE             ) ? _T( "nsv" ) :
			types( File::SCRIPT            ) ? _T( "nsp" ) :
			types( File::ROM               ) ? _T( "rom" ) :
			types( File::PALETTE           ) ? _T( "pal" ) :
			types( File::WAVE              ) ? _T( "wav" ) :
			types( File::AVI               ) ? _T( "avi" ) :
			types( File::ARCHIVE           ) ? _T( "zip" ) :
                                               _T( ""    )
		);
	}

	const GenericString Paths::GetDefaultDirectory(const File::Types types) const
	{
		Window::Paths::Type type;

		if (types( File::IMAGE|File::ROM ))
		{
			if (dialog->GetSetting(Window::Paths::USE_LAST_IMAGE_DIR) && recentDirs[RECENT_DIR_IMAGE].DirectoryExists())
				return recentDirs[RECENT_DIR_IMAGE];

			type = Window::Paths::DIR_IMAGE;
		}
		else if (types( File::SCRIPT ))
		{
			if (dialog->GetSetting(Window::Paths::USE_LAST_SCRIPT_DIR) && recentDirs[RECENT_DIR_SCRIPT].DirectoryExists())
				return recentDirs[RECENT_DIR_SCRIPT];

			type = Window::Paths::DIR_SCRIPT;
		}
		else if (types( File::STATE|File::SLOTS|File::MOVIE ))
		{
			type = Window::Paths::DIR_STATE;
		}
		else if (types( File::BATTERY|File::TAPE ))
		{
			type = Window::Paths::DIR_SAVE;
		}
		else if (types( File::IPS ))
		{
			type = Window::Paths::DIR_IPS;
		}
		else
		{
			return Application::Instance::GetExePath().Directory();
		}

		return dialog->GetDirectory( type );
	}

	ibool Paths::CheckFile(Path& path,const File::Types types,const Alert alert,const uint title) const
	{
		NST_ASSERT( types.Word() );

		try
		{
			if (LoadFromFile( path, NULL, types ))
				return true;
		}
		catch (int ids)
		{
			if (alert == NOISY)
			{
				Window::User::Fail( ids, title );
			}
			else if (alert == STICKY)
			{
				Io::Screen() << Resource::String(title) << ' ' << Resource::String(ids);
			}
		}

		path.Clear();
		return false;
	}

	Path Paths::BrowseLoad(const File::Types types,const GenericString dir,const Checking checking) const
	{
		NST_ASSERT( types.Word() );

		Path path
		(
			Window::Browser::OpenFile
			(
				Filter( types ).Ptr(),
				dir.Length() ? dir : GetDefaultDirectory( types ),
				GetDefaultExtension( types )
			)
		);

		if (path.Length())
		{
			UpdateRecentDirectory( path, types );

			if (checking == CHECK_FILE)
				CheckFile( path, types, NOISY );
		}

		return path;
	}

	Path Paths::BrowseSave(const File::Types types,const Method method,const GenericString initPath) const
	{
		NST_ASSERT( types.Word() );

		Path path( initPath );

		if (path.Directory().Empty())
			path.Directory() = GetDefaultDirectory( types );

		if (method == SUGGEST && path.File().Empty())
			path.File() = emulator.GetImagePath().Target().File();

		path.Extension() = GetDefaultExtension( types );
		path = Window::Browser::SaveFile( Filter(types).Ptr(), path );

		if (path.Length())
			UpdateRecentDirectory( path, types );

		return path;
	}

	void Paths::FixFile(const File::Type type,Path& path) const
	{
		if (path.File().Length())
		{
			if (path.Directory().Empty())
				path.Directory() = GetDefaultDirectory( type );

			if (path.Extension().Empty())
				path.Extension() = GetDefaultExtension( type );
		}
		else
		{
			path.Clear();
		}
	}

	Path Paths::GetSavePath(const Path& image,const File::Type type) const
	{
		NST_ASSERT( image.Length() );

		Path save;

		if (type & File::GAME)
		{
			GenericString dir, ext;

			if (type & File::CARTRIDGE)
			{
				dir = dialog->GetDirectory( Window::Paths::DIR_SAVE );
				ext = _T("sav");
			}
			else
			{
				dir = dialog->GetDirectory( Window::Paths::DIR_IPS );
				ext = _T("ips");
			}

			save.Set( dir, image.Target().File(), ext );
		}

		return save;
	}

	Path Paths::GetIpsPath(const Path& image,const File::Type type) const
	{
		Path ips;

		if ((type & File::INES) && dialog->GetSetting( Window::Paths::IPS_AUTO_PATCH ))
			ips.Set( dialog->GetDirectory( Window::Paths::DIR_IPS ), image.Target().File(), _T("ips") );

		return ips;
	}

	const Path& Paths::GetSamplesPath() const
	{
		return dialog->GetDirectory( Window::Paths::DIR_SAMPLES );
	}

	Paths::File::Type Paths::Load
	(
		File& file,
		const File::Types types,
		const GenericString path,
		const Alert alert
	)   const
	{
		if (path.Length())
		{
			file.name = path;

			if (file.name.Directory().Empty())
				file.name.Directory() = GetDefaultDirectory( types );
		}
		else
		{
			file.name = BrowseLoad( types );
		}

		if (file.name.Empty() || (alert == QUIETLY && !file.name.FileExists()))
			return File::NONE;

		try
		{
			Application::Instance::Waiter wait;
			file.type = LoadFromFile( file.name, &file.data, types );
		}
		catch (int ids)
		{
			if (alert == NOISY)
			{
				Window::User::Fail( ids, IDS_TITLE_ERROR );
			}
			else if (alert == STICKY)
			{
				Io::Screen() << Resource::String(IDS_TITLE_ERROR) << ' ' << Resource::String(ids);
			}

			file.type = File::NONE;
		}

		if (file.type == File::NONE)
		{
			file.name.Clear();
			file.data.Clear();
		}

		return file.type;
	}

	ibool Paths::Save
	(
		const void* const data,
		const uint size,
		const File::Type type,
		Path path,
		const Alert alert
	)   const
	{
		if (path.Directory().Empty())
			path.Directory() = GetDefaultDirectory( type );

		try
		{
			const Io::File file( path, Io::File::DUMP );

			if (type == File::SCRIPT)
				file.WriteText( static_cast<const tchar*>(data), size );
			else
				file.Write( data, size );

			return true;
		}
		catch (Io::File::Exception ids)
		{
			if (alert == NOISY)
			{
				Window::User::Fail( ids, IDS_TITLE_ERROR );
			}
			else if (alert == STICKY)
			{
				Io::Screen() << Resource::String(IDS_TITLE_ERROR) << ' ' << Resource::String(ids);
			}

			return false;
		}
	}

	Paths::File::Type Paths::LoadFromFile(Path& path,File::Data* const data,const File::Types types)
	{
		NST_ASSERT( path.Length() );

		const GenericString fileInArchive( path.FileInArchive() );
		GenericString filePath( path );

		if (fileInArchive.Length())
			filePath = path.Archive();

		try
		{
			Io::File file( filePath, Io::File::READ|Io::File::EXISTING );

			const File::Type type = CheckFile( types, file.Peek<u32>(), path.Extension().Id() );

			if (type == File::NONE)
				throw IDS_FILE_ERR_INVALID;

			if (type == File::ARCHIVE)
				return LoadFromArchive( Io::Archive(file), path, data, fileInArchive, types );

			if (fileInArchive.Length())
				throw IDS_FILE_ERR_INVALID;

			if (data)
				file.Stream() >> *data;

			return type;
		}
		catch (Io::File::Exception id)
		{
			throw int(id);
		}
	}

	Paths::File::Type Paths::LoadFromArchive
	(
		const Io::Archive& archive,
		Path& path,
		File::Data* const data,
		const GenericString& fileInArchive,
		const File::Types types
	)
	{
		uint index;

		if (fileInArchive.Length())
		{
			index = archive.Find( fileInArchive );
		}
		else
		{
			uint count = 0;
			GenericString filter[22];

			if (types( File::INES    )) filter[count++] = _T( "nes" );
			if (types( File::FDS     )) filter[count++] = _T( "fds" );
			if (types( File::NSF     )) filter[count++] = _T( "nsf" );
			if (types( File::BATTERY )) filter[count++] = _T( "sav" );
			if (types( File::TAPE    )) filter[count++] = _T( "tp"  );
			if (types( File::STATE   )) filter[count++] = _T( "nst" );
			if (types( File::SCRIPT  )) filter[count++] = _T( "nsp" );
			if (types( File::MOVIE   )) filter[count++] = _T( "nsv" );
			if (types( File::IPS     )) filter[count++] = _T( "ips" );
			if (types( File::ROM     )) filter[count++] = _T( "rom" );
			if (types( File::PALETTE )) filter[count++] = _T( "pal" );
			if (types( File::WAVE    )) filter[count++] = _T( "wav" );
			if (types( File::AVI     )) filter[count++] = _T( "avi" );

			if (types( File::UNIF ))
			{
				filter[count++] = _T( "unf"  );
				filter[count++] = _T( "unif" );
			}

			if (types( File::SLOTS ))
			{
				filter[count++] = _T( "ns1" );
				filter[count++] = _T( "ns2" );
				filter[count++] = _T( "ns3" );
				filter[count++] = _T( "ns4" );
				filter[count++] = _T( "ns5" );
				filter[count++] = _T( "ns6" );
				filter[count++] = _T( "ns7" );
				filter[count++] = _T( "ns8" );
				filter[count++] = _T( "ns9" );
			}

			// If more than one file in the archive let the user choose which to load,
			// non-valid files will be filtered out by named extension detection

			index = archive.UserSelect( filter, count );
		}

		switch (index)
		{
			case Io::Archive::NO_FILES:
				throw IDS_FILE_ERR_NOTHING_IN_ARCHIVE;

			case Io::Archive::NO_SELECTION:
				return File::NONE;
		}

		File::Type type = File::ARCHIVE;

		if (data)
		{
			data->Resize( archive[index].Size() );

			if (data->Size() < 4 || !archive[index].Uncompress( data->Ptr() ))
				throw IDS_FILE_ERR_INVALID;

			type = CheckFile
			(
				types,
				reinterpret_cast<const u32&>(data->Front()),
				archive[index].GetName().Extension().Id()
			);

			if (type == File::NONE || type == File::ARCHIVE)
				throw IDS_FILE_ERR_INVALID;
		}

		if (fileInArchive.Empty())
			path << _T(" <") << archive[index].GetName() << '>';

		return type;
	}

	Paths::File::Type Paths::CheckFile(const File::Types types,const uint fileId,const uint extensionId)
	{
		File::Type type = File::NONE;

		switch (fileId)
		{
			case File::FILEID_INES:    if (types( File::INES              )) type = File::INES;    break;
			case File::FILEID_UNIF:    if (types( File::UNIF              )) type = File::UNIF;    break;
			case File::FILEID_FDS:
			case File::FILEID_FDS_RAW: if (types( File::FDS               )) type = File::FDS;     break;
			case File::FILEID_NSF:     if (types( File::NSF               )) type = File::NSF;     break;
			case File::FILEID_IPS:     if (types( File::IPS               )) type = File::IPS;     break;
			case File::FILEID_NSV:     if (types( File::MOVIE             )) type = File::MOVIE;   break;
			case File::FILEID_NST:     if (types( File::STATE|File::SLOTS )) type = File::STATE;   break;
			case File::FILEID_ZIP:
			case File::FILEID_7Z:
			case File::FILEID_RAR:     if (types( File::ARCHIVE           )) type = File::ARCHIVE; break;

			default:

				switch (extensionId)
				{
					// raw or text file, must check the file extension

					case NST_FOURCC('n','s','p','\0'):  if (types( File::SCRIPT  )) type = File::SCRIPT;  break;
					case NST_FOURCC('s','a','v','\0'):  if (types( File::BATTERY )) type = File::BATTERY; break;
					case NST_FOURCC('t','p','\0','\0'): if (types( File::TAPE    )) type = File::TAPE;    break;
					case NST_FOURCC('r','o','m','\0'):  if (types( File::ROM     )) type = File::ROM;     break;
					case NST_FOURCC('p','a','l','\0'):  if (types( File::PALETTE )) type = File::PALETTE; break;
					case NST_FOURCC('w','a','v','\0'):  if (types( File::WAVE    )) type = File::WAVE;    break;
					case NST_FOURCC('a','v','i','\0'):  if (types( File::AVI     )) type = File::AVI;     break;

					case NST_FOURCC('n','e','s','\0'):
					case NST_FOURCC('u','n','f','\0'):
					case NST_FOURCC('u','n','i', 'f'):
					case NST_FOURCC('f','d','s','\0'):
					case NST_FOURCC('n','s','f','\0'):
					case NST_FOURCC('i','p','s','\0'):
					case NST_FOURCC('n','s','v','\0'):
					case NST_FOURCC('n','s','t','\0'):
					case NST_FOURCC('n','s','0','\0'):
					case NST_FOURCC('n','s','1','\0'):
					case NST_FOURCC('n','s','2','\0'):
					case NST_FOURCC('n','s','3','\0'):
					case NST_FOURCC('n','s','4','\0'):
					case NST_FOURCC('n','s','5','\0'):
					case NST_FOURCC('n','s','6','\0'):
					case NST_FOURCC('n','s','7','\0'):
					case NST_FOURCC('n','s','8','\0'):
					case NST_FOURCC('n','s','9','\0'):
					case NST_FOURCC('z','i','p','\0'):
					case NST_FOURCC('r','a','r','\0'):
					case NST_FOURCC('7','z','\0','\0'):

						// either corrupt data or wrong extension, bail..

						break;

					default:

						// extension is unknown, but the file may still be valid,
						// allow it to pass if only one file type was selected

						if (types(~uint(File::ARCHIVE|File::IMAGE)) == File::ROM)
						{
							type = File::ROM;
						}
						else
						{
							switch (types(~uint(File::ARCHIVE)))
							{
								case File::SCRIPT:  type = File::SCRIPT;  break;
								case File::BATTERY: type = File::BATTERY; break;
								case File::TAPE:    type = File::TAPE;    break;
								case File::PALETTE: type = File::PALETTE; break;
							}
						}
				}
		}

		return type;
	}
}
