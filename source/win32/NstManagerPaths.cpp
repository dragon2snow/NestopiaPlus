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

#include "NstIoFile.hpp"
#include "NstIoZip.hpp"
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
			if (recentDirs[i].DirExist())
				cfg[recentDirCfgNames[i]].Quote() = recentDirs[i];
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
		NST_ASSERT( path.File().Size() );

		if (types( File::INES              )) { path.Directory() = GetDefaultDirectory( File::INES              ); path.Extension() = "nes"; if (path.FileExist()) return TRUE; }
     	if (types( File::UNIF              )) { path.Directory() = GetDefaultDirectory( File::UNIF              ); path.Extension() = "unf"; if (path.FileExist()) return TRUE; }
		if (types( File::FDS               )) { path.Directory() = GetDefaultDirectory( File::FDS               ); path.Extension() = "fds"; if (path.FileExist()) return TRUE; }
		if (types( File::NSF               )) { path.Directory() = GetDefaultDirectory( File::NSF               ); path.Extension() = "nsf"; if (path.FileExist()) return TRUE; }
		if (types( File::BATTERY           )) { path.Directory() = GetDefaultDirectory( File::BATTERY           ); path.Extension() = "sav"; if (path.FileExist()) return TRUE; }
		if (types( File::STATE|File::SLOTS )) { path.Directory() = GetDefaultDirectory( File::STATE|File::SLOTS ); path.Extension() = "nst"; if (path.FileExist()) return TRUE; }
		if (types( File::IPS               )) { path.Directory() = GetDefaultDirectory( File::IPS               ); path.Extension() = "ips"; if (path.FileExist()) return TRUE; }
		if (types( File::MOVIE             )) { path.Directory() = GetDefaultDirectory( File::MOVIE             ); path.Extension() = "nsv"; if (path.FileExist()) return TRUE; }
		if (types( File::SCRIPT            )) { path.Directory() = GetDefaultDirectory( File::SCRIPT            ); path.Extension() = "nsp"; if (path.FileExist()) return TRUE; }
		if (types( File::ROM               )) { path.Directory() = GetDefaultDirectory( File::ROM               ); path.Extension() = "rom"; if (path.FileExist()) return TRUE; }
		if (types( File::PALETTE           )) { path.Directory() = GetDefaultDirectory( File::PALETTE           ); path.Extension() = "pal"; if (path.FileExist()) return TRUE; }
		if (types( File::WAVE              )) { path.Directory() = GetDefaultDirectory( File::WAVE              ); path.Extension() = "wav"; if (path.FileExist()) return TRUE; }
		if (types( File::ARCHIVE           )) { path.Directory() = GetDefaultDirectory( File::ARCHIVE           ); path.Extension() = "zip"; if (path.FileExist()) return TRUE; }

		return FALSE;
	}

	ibool Paths::FindFile(Path& path) const
	{
		NST_ASSERT( path.File().Size() );

		path.Directory() = Application::Instance::GetPath().Directory();

		if (path.FileExist())
			return TRUE;

		for (uint i=0; i < Window::Paths::NUM_DIRS; ++i)
		{
			path.Directory() = dialog->GetDirectory( (Window::Paths::Type) i );

			if (path.FileExist())
				return TRUE;
		}

		for (uint i=0; i < NUM_RECENT_DIRS; ++i)
		{
			path.Directory() = recentDirs[i];

			if (path.FileExist())
				return TRUE;
		}

		path.Directory().Clear();

		return FALSE;
	}

	void Paths::UpdateRecentDirectory(const TmpPath& path,const File::Types types) const
	{
		NST_ASSERT( path.Size() );

		if (types( File::IMAGE|File::SCRIPT ))
			recentDirs[ types(File::IMAGE) ? RECENT_DIR_IMAGE : RECENT_DIR_SCRIPT ] = path.Directory();
	}

	Paths::TmpPath Paths::GetScreenShotPath() const
	{
		TmpPath path;

		path.Directory() = dialog->GetDirectory( Window::Paths::DIR_SCREENSHOT );
		path.File() = emulator.GetImagePath().Target().File();
		path.Extension().Clear();

		const uint offset = path.Size() + 1;
		path << "_xx." << dialog->GetScreenShotExtension();

		for (uint i=1; i < 100; ++i)
		{
			path[offset+0] = (char) ('0' + (i / 10));
			path[offset+1] = (char) ('0' + (i % 10));

			if (!path.FileExist())
				break;
		}

		return path;
	}

	cstring Paths::GetDefaultExtension(const File::Types types)
	{
		return
		(
			types( File::INES              ) ? "nes" :
			types( File::UNIF              ) ? "unf" :
			types( File::FDS               ) ? "fds" :
			types( File::NSF               ) ? "nsf" :
			types( File::BATTERY           ) ? "sav" :
			types( File::STATE|File::SLOTS ) ? "nst" :
			types( File::IPS               ) ? "ips" :
			types( File::MOVIE             ) ? "nsv" :
			types( File::SCRIPT            ) ? "nsp" :
			types( File::ROM               ) ? "rom" :
			types( File::PALETTE           ) ? "pal" :
			types( File::WAVE              ) ? "wav" :
			types( File::ARCHIVE           ) ? "zip" : ""
		);
	}

	const String::Generic Paths::GetDefaultDirectory(const File::Types types) const
	{
		Window::Paths::Type type;

		if (types( File::IMAGE|File::ROM ))
		{
			if (dialog->GetSetting(Window::Paths::USE_LAST_IMAGE_DIR) && recentDirs[RECENT_DIR_IMAGE].DirExist())
				return recentDirs[RECENT_DIR_IMAGE]; 

			type = Window::Paths::DIR_IMAGE;
		}
		else if (types( File::SCRIPT ))
		{
			if (dialog->GetSetting(Window::Paths::USE_LAST_SCRIPT_DIR) && recentDirs[RECENT_DIR_SCRIPT].DirExist())
				return recentDirs[RECENT_DIR_SCRIPT]; 
	
			type = Window::Paths::DIR_SCRIPT;
		}
		else if (types( File::STATE|File::SLOTS|File::MOVIE ))
		{
			type = Window::Paths::DIR_STATE;
		}
		else if (types( File::BATTERY ))
		{
			type = Window::Paths::DIR_DATA;
		}
		else if (types( File::IPS ))
		{
			type = Window::Paths::DIR_IPS;
		}
		else
		{
			return Application::Instance::GetPath().Directory();
		}

		return dialog->GetDirectory( type );
	}

	ibool Paths::CheckFile(Path& path,const File::Types types,const Alert alert,const uint title) const
	{
		NST_ASSERT( types.Word() );

		try
		{
			if (LoadFromFile( path, NULL, types ))
				return TRUE;
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
		return FALSE;
	}

	Paths::TmpPath Paths::BrowseLoad(const File::Types types,const String::Generic dir) const
	{
		NST_ASSERT( types.Word() );

		Path path
		(
			Window::Browser::OpenFile
			(
				Filter( types ),
				dir.Size() ? dir : GetDefaultDirectory( types ),
				GetDefaultExtension( types )
			)
		);

		if (path.Size())
		{
			UpdateRecentDirectory( path, types );
			CheckFile( path, types, NOISY );
		}

		return path;
	}

	Paths::TmpPath Paths::BrowseSave(const File::Types types,const String::Generic dir,const String::Generic name) const
	{
		NST_ASSERT( types.Word() );

		const TmpPath path
		(
			Window::Browser::SaveFile
			(
				Filter( types ),
				dir.Size() ? dir : GetDefaultDirectory( types ),
				GetDefaultExtension( types ),
				name
			)
		);

		if (path.Size())
			UpdateRecentDirectory( path, types );

		return path;
	}

	Paths::TmpPath Paths::GetSavePath(const Path& image,const File::Type type) const
	{
		NST_ASSERT( image.Size() );

		TmpPath save;

		if (type & File::GAME)
		{
			String::Generic dir, ext;

			if (type & File::CARTRIDGE)
			{
				dir = dialog->GetDirectory( Window::Paths::DIR_DATA );
				ext = "sav";
			}
			else
			{
				dir = dialog->GetDirectory( Window::Paths::DIR_IPS );
				ext = "ips";
			}

			save.Set( dir, image.Target().File(), ext );
		}

		return save;
	}

	Paths::TmpPath Paths::GetIpsPath(const Path& image,const File::Type type) const
	{
		TmpPath ips;

		if ((type & File::INES) && dialog->GetSetting( Window::Paths::IPS_AUTO_PATCH ))
			ips.Set( dialog->GetDirectory( Window::Paths::DIR_IPS ), image.Target().File(), "ips" );

		return ips;
	}

	Paths::File::Type Paths::Load
	(
       	File& file,
		const File::Types types,
		const String::Generic path,
		const Alert alert,
		const uint title
	)   const
	{
		if (path.Size())
		{
			file.name = path;

			if (file.name.Directory().Empty())
				file.name.Directory() = GetDefaultDirectory( types );
		}
		else
		{
			file.name = BrowseLoad( types );

			if (file.name.Empty())
				return File::NONE;
		}

		try
		{
			file.type = LoadFromFile( file.name, &file.data, types );
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
     	const Object::ConstRaw& data,
		const File::Type type,
		const String::Generic name,
		const Alert alert,
		const uint title
	)   const
	{
		TmpPath path;

		if (name.Size())
		{
			path = name;

			if (path.Directory().Empty())
				path.Directory() = GetDefaultDirectory( type );
		}
		else
		{
			path = BrowseSave( type );

			if (path.Empty())
				return FALSE;
		}

		try
		{
			Io::File( path, Io::File::DUMP ).Stream() << data;
		}
		catch (Io::File::Exception ids)
		{
			if (alert == NOISY)
			{
				Window::User::Fail( ids, title );
			}
			else if (alert == STICKY)
			{
				Io::Screen() << Resource::String(title) << ' ' << Resource::String(ids);
			}

			return FALSE;
		}

		return TRUE;
	}

	Paths::File::Type Paths::LoadFromFile(Path& path,File::Data* const data,const File::Types types)
	{
		NST_ASSERT( path.Size() );

		const String::Generic fileInArchive( path.FileInArchive() );
		String::Generic filePath( path );

		if (fileInArchive.Size())
			filePath = path.Archive();

		try
		{
			Io::File file( filePath, Io::File::READ|Io::File::EXISTING );

			const File::Type type = CheckFile( types, file.Peek<u32>(), path.Extension().Id() );

			if (type == File::NONE)
				throw IDS_FILE_ERR_INVALID;

			if (type == File::ARCHIVE)
				return LoadFromArchive( Io::Zip(file), path, data, fileInArchive, types );

			if (fileInArchive.Size())
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
       	const Io::Zip& archive,
		Path& path,
		File::Data* const data,
		const String::Generic& fileInArchive,
		const File::Types types
	)
	{
		uint index;

		if (fileInArchive.Size())
		{
			index = archive.Find( fileInArchive );
		}
		else
		{
			uint count = 0;
			String::Generic filter[22];

			if (types( File::INES    )) filter[count++] = "nes";
			if (types( File::FDS     )) filter[count++] = "fds";
			if (types( File::NSF     )) filter[count++] = "nsf";
			if (types( File::BATTERY )) filter[count++] = "sav";
			if (types( File::STATE   )) filter[count++] = "nst";
			if (types( File::SCRIPT  )) filter[count++] = "nsp";
			if (types( File::MOVIE   )) filter[count++] = "nsv";
			if (types( File::IPS     )) filter[count++] = "ips";
			if (types( File::ROM     )) filter[count++] = "rom";
			if (types( File::PALETTE )) filter[count++] = "pal";
			if (types( File::WAVE    )) filter[count++] = "wav";

			if (types( File::UNIF )) 
			{
				filter[count++] = "unf";
				filter[count++] = "unif";
			}

			if (types( File::SLOTS ))
			{
				filter[count++] = "ns1";
				filter[count++] = "ns2";
				filter[count++] = "ns3";
				filter[count++] = "ns4";
				filter[count++] = "ns5";
				filter[count++] = "ns6";
				filter[count++] = "ns7";
				filter[count++] = "ns8";
				filter[count++] = "ns9";
			}

			// If more than one file in the archive let the user choose which to load,
			// non-valid files will be filtered out by named extension detection

			index = archive.UserSelect( filter, count );
		}

		switch (index)
		{
			case Io::Zip::NO_FILES:
				throw IDS_FILE_ERR_NOTHING_IN_ARCHIVE;
		
			case Io::Zip::NO_SELECTION:
				return File::NONE;
		}

		File::Type type = File::ARCHIVE; 

		if (data)
		{
			data->Resize( archive[index].UncompressedSize() );

			if (data->Size() < 4 || !archive[index].Uncompress( *data ))
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
			path << " <" << archive[index].GetName() << '>'; 

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
			case File::FILEID_NSV:	   if (types( File::MOVIE             )) type = File::MOVIE;   break;
			case File::FILEID_NST:	   if (types( File::STATE|File::SLOTS )) type = File::STATE;   break;
			case File::FILEID_WAV:	   if (types( File::WAVE              )) type = File::WAVE;    break;		
			case File::FILEID_ZIP:	   if (types( File::ARCHIVE           )) type = File::ARCHIVE; break;
		
			default:
		
				switch (extensionId)
				{
					// raw or text file, need to investigate the file extension
		
					case NST_FOURCC('n','s','p','\0'): if (types( File::SCRIPT  )) type = File::SCRIPT;  break;
					case NST_FOURCC('s','a','v','\0'): if (types( File::BATTERY )) type = File::BATTERY; break;
					case NST_FOURCC('r','o','m','\0'): if (types( File::ROM     )) type = File::ROM;     break;
					case NST_FOURCC('p','a','l','\0'): if (types( File::PALETTE )) type = File::PALETTE; break;
					case NST_FOURCC('w','a','v','\0'): if (types( File::WAVE    )) type = File::WAVE;    break;
				
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
								case File::PALETTE: type = File::PALETTE; break;
							}
						}
				}
		}

		return type;
	}
}
