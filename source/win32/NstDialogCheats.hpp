////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
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

#ifndef NST_DIALOG_CHEATS_H
#define NST_DIALOG_CHEATS_H

#pragma once

#include "NstWindowDialog.hpp"
#include "../core/api/NstApiCheats.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Cheats
		{
		public:

			Cheats(Managers::Emulator&,const Configuration&,const Managers::Paths&);
			~Cheats();

			typedef Nes::Cheats::Code Mem;

			void Save(Configuration&) const;
			uint ClearTemporaryCodes();

			enum
			{
				MAX_CODES = 0xFFFF
			};

		private:

			struct Handlers;

			class CodeDialog : public Dialog
			{
				HWND listView;

			public:

				template<typename T,typename U,typename V>
				inline CodeDialog(T*,const U&,const V&);

				inline void Open(HWND);
				inline HWND GetListView() const;
			};

			class List
			{
			public:

				List(CodeDialog&,const Managers::Paths&);
				~List();

				void Add(const Mem&,Generic::Stream);
				void Import(const Path&);
				void Export(const Path&) const;
				void InitDialog(Dialog&,uint);

			private:

				enum
				{
					ADD,
					REMOVE,
					IMPORT,
					EXPORT,
					CLEAR,
					NUM_CONTROLS
				};

				struct Code
				{
					explicit Code(const Mem&);

					void CheckDesc();

					bool enabled;
					Mem mem;
					HeapString desc;

					bool operator == (uint address) const
					{
						return mem.address == address;
					}
				};

				struct Codes : Collection::Vector<Code>
				{
					~Codes();

					void Load(const Configuration&);
					void Save(Configuration&) const;
					void Clear();
					Code& Add(const Mem&);
				};

				class ListView : public Control::ListView
				{
				public:

					ListView(List* const,Dialog&,uint);
					void Init(Control::ListView);

					Control::Generic controls[NUM_CONTROLS];

				private:

					Control::NotificationHandler notificationHandler;
				};

				void AddToDialog(const Code&) const;
				static bool Import(Codes&,const Path&);

				ibool OnCmdAdd    (Param&);
				ibool OnCmdRemove (Param&);
				ibool OnCmdExport (Param&);
				ibool OnCmdImport (Param&);
				ibool OnCmdClear  (Param&);

				void OnKeyDown     (const NMHDR&);
				void OnItemChanged (const NMHDR&);
				void OnInsertItem  (const NMHDR&);
				void OnDeleteItem  (const NMHDR&);

				Codes codes;
				CodeDialog& codeDialog;
				const Managers::Paths& paths;
				ListView* listView;

			public:

				void Clear()
				{
					codes.Clear();
				}

				void Load(const Configuration& cfg)
				{
					codes.Load( cfg );
				}

				void Save(Configuration& cfg) const
				{
					codes.Save( cfg );
				}

				uint Size() const
				{
					return codes.Size();
				}

				const Code& operator [] (uint i) const
				{
					return codes[i];
				}

				HWND GetHandle() const
				{
					return listView->GetHandle();
				}
			};

			struct Searcher
			{
				Searcher();

				enum
				{
					NO_FILTER = 0xFFFF
				};

				ushort filter;
				uchar a;
				uchar b;
				bool hex;
				uchar ram[Nes::Cheats::RAM_SIZE];
			};

			void AddSearchEntry(Control::ListView,uint) const;
			void UpdateSearchList() const;
			void UpdateHexView(bool);
			void UpdateInput() const;

			void OnCodeItemChanged (const NMHDR&);

			ibool OnInitMainDialog    (Param&);
			ibool OnInitCodeDialog    (Param&);
			ibool OnDestroyCodeDialog (Param&);
			ibool OnDropFiles         (Param&);
			ibool OnCodeCmdReset      (Param&);
			ibool OnCodeCmdHex        (Param&);
			ibool OnCodeCmdSubmit     (Param&);
			ibool OnCodeCmdValidate   (Param&);
			ibool OnCodeCmdType       (Param&);
			ibool OnCodeSearchType    (Param&);

			uint GetSearchValue (uint) const;
			void SetSearchValue (uint,uint) const;
			bool GetRawCode     (Mem&) const;
			void SetRawCode     (const Mem&) const;
			bool GetGenieCode   (Mem&) const;
			bool GetRockyCode   (Mem&) const;

			Dialog mainDialog;
			CodeDialog codeDialog;
			List staticList;
			List tempList;
			Managers::Emulator& emulator;
			Searcher searcher;
			Control::NotificationHandler searcherNotificationHandler;

		public:

			void Open()
			{
				mainDialog.Open();
			}

			enum
			{
				STATIC_CODES,
				TEMP_CODES,
				NUM_CODE_TYPES
			};

			void ResetRamSearch()
			{
				searcher.filter = Searcher::NO_FILTER;
				searcher.a = searcher.b = 0x00;
			}

			uint GetNumCodes(uint type) const
			{
				return (type ? tempList : staticList).Size();
			}

			bool CodeEnabled(uint type,uint i) const
			{
				return (type ? tempList : staticList)[i].enabled;
			}

			const Mem& GetCode(uint type,uint i) const
			{
				return (type ? tempList : staticList)[i].mem;
			}
		};
	}
}

#endif
