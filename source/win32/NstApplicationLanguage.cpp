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

#include <algorithm>
#include "NstApplicationException.hpp"
#include "NstApplicationLanguage.hpp"

namespace Nestopia
{
	namespace Application
	{
		ibool Instance::Language::Resource::Load(tstring const p)
		{
			if (!Dll::Load( p ))
				return false;

			path = p;

			return true;
		}

		void Instance::Language::Load(const Configuration& cfg)
		{
			if (!resource.Loaded())
				Load( cfg["language"].Ptr() );
		}

		void Instance::Language::Save(Configuration& cfg) const
		{
			if (resource.path.Length())
				cfg["language"].Quote() = resource.path;
		}

		void Instance::Language::Load(tstring const path)
		{
			if (!path || !*path)
			{
				Paths paths;
				EnumerateResources( paths );

				if (paths.empty())
					throw Exception(_T("Language plugin file missing!"));

				Paths::const_iterator path(std::find( paths.begin(), paths.end(), Instance::GetExePath(_T("english.nlg")) ));

				if (path == paths.end())
					path = paths.begin();

				if (!resource.Load( path->Ptr() ))
					throw Exception(_T("Failed to load language plugin file!"));
			}
			else if (!resource.Load( path ))
			{
				Load( NULL );
			}
		}

		void Instance::Language::UpdateResource(tstring const update)
		{
			NST_ASSERT( update );
			resource.path = update;
		}

		void Instance::Language::EnumerateResources(Paths& paths) const
		{
			struct FileFinder
			{
				WIN32_FIND_DATA data;
				HANDLE const handle;

				FileFinder(tstring const path)
				: handle(::FindFirstFile( path, &data )) {}

				~FileFinder()
				{
					if (handle != INVALID_HANDLE_VALUE)
						::FindClose( handle );
				}
			};

			Path path( Instance::GetExePath(_T("language\\*.*")) );

			FileFinder findFile( path.Ptr() );

			if (findFile.handle != INVALID_HANDLE_VALUE)
			{
				do
				{
					if (!(findFile.data.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM|FILE_ATTRIBUTE_DIRECTORY)))
					{
						path.File() = findFile.data.cFileName;

						if (path.Extension() == _T("nlg"))
							paths.push_back( path );
					}
				}
				while (::FindNextFile( findFile.handle, &findFile.data ));
			}
		}
	}
}
