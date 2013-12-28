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

#ifndef NST_DIALOG_NETPLAY_H
#define NST_DIALOG_NETPLAY_H

#pragma once

#include "NstWindowDialog.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Netplay
		{
		public:

			Netplay(const Configuration&,Managers::Emulator&,const Managers::Paths&);
			~Netplay();

			void Save(Configuration&,ibool) const;

			class Chat
			{
			public:

				Chat();

				void Close();
				void Open();

			private:

				struct Handlers;

				ibool OnInit    (Param&);
				ibool OnCommand (Param&);

				Dialog dialog;
				String::Heap<char> text;
			};

		private:

			struct Handlers;

			enum 
			{
				LAUNCH = 0xB00B
			};

			struct Games : Collection::Set<Path>
			{
				Games();
				~Games();

				Iterator Add(const Path&);
				void Erase(uint);

				enum
				{
					LIMIT = 64
				};

				enum State
				{
					CLEAN,
					DIRTY,
					UNINITIALIZED
				};

				State state;
			};

			struct Settings
			{
				ibool fullscreen;
			};

			void LoadFile();
			void SaveFile() const;
			void Add(Path);

			ibool OnInitDialog (Param&);
			ibool OnAdd        (Param&);
			ibool OnRemove     (Param&); 
			ibool OnClear      (Param&);  
			ibool OnDefault    (Param&);
			ibool OnCancel     (Param&); 
			ibool OnLaunch     (Param&);
			ibool OnFullscreen (Param&);
			ibool OnDropFiles  (Param&);

			void OnKeyDown     (const NMHDR&);
			void OnItemChanged (const NMHDR&);
			void OnInsertItem  (const NMHDR&);
			void OnDeleteItem  (const NMHDR&);

			Dialog dialog;
			Settings settings;
			const Managers::Paths& paths;
			Managers::Emulator& emulator;
			const Control::NotificationHandler notifications;
			Games games;

		public:

			ibool Open()
			{
				return dialog.Open() == LAUNCH;
			}

			ibool ShouldGoFullscreen() const
			{
				return settings.fullscreen;
			}

			uint GetNumGames() const
			{
				return games.Size();
			}

			GenericString GetGame(const uint i) const
			{
   				return games[i].Target().File();
			}

			tstring GetPath(tstring const game) const
			{
				for (uint i=0; i < games.Size(); ++i)
				{
					if (GetGame(i) == game)
						return games[i].Ptr();
				}

				return NULL;
			}
		};
	}
}

#endif
