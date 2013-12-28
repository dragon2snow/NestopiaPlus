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

#ifndef NST_DIALOG_CHEATS_H
#define NST_DIALOG_CHEATS_H

#pragma once

#include "NstWindowDialog.hpp"

namespace Nestopia
{
	namespace Io
	{
		namespace Nsp
		{
			struct Context;
		}
	}

	namespace Window
	{
		class Cheats
		{
		public:

			Cheats(const Configuration&,const Managers::Paths&);
			~Cheats();

			void Save(Configuration&) const;
			void Save(Io::Nsp::Context&) const;
			void Load(const Io::Nsp::Context&);
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

				ibool Add(ulong,Generic::Stream);
				void  Load(const Io::Nsp::Context&);
				void  Import(String::Generic=String::Generic());
				void  InitDialog(Dialog&,uint);

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
					explicit Code(ulong);

					void CheckDesc();

					const ulong packed;
					ibool enabled;
					String::Heap desc;

					operator ulong () const
					{
						return packed;
					}
				};
						   
				struct Codes : Collection::Vector<Code>
				{
					~Codes();

					void Load(const Configuration&);
					void Load(const Io::Nsp::Context&);
					void Save(Configuration&) const;
					uint Save(Io::Nsp::Context&) const;
					void Clear();
					ibool Add(ulong);
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

				uint Save(Io::Nsp::Context& context) const
				{
					return codes.Save( context );
				}

				uint Size() const
				{
					return codes.Size();
				}

				ulong operator [] (uint i) const
				{
					return codes[i].packed;
				}

				ibool IsEnabled(uint i) const
				{
					return codes[i].enabled;
				}

				HWND GetHandle() const
				{
					return listView->GetHandle();
				}
			};

			ibool OnInitMainDialog  (Param&);
			ibool OnInitCodeDialog  (Param&);
			ibool OnClose           (Param&);
			ibool OnDropFiles       (Param&);
			ibool OnCodeCmdSubmit	(Param&);
			ibool OnCodeCmdCancel	(Param&);
			ibool OnCodeCmdValidate (Param&);

			ibool GetPackedEncodedCode (ulong&) const;
			ibool GetPackedDecodedCode (ulong&) const;

			Dialog mainDialog;
			CodeDialog codeDialog;
			List staticList;
			List tempList;

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

			uint GetNumCodes(uint type) const
			{
				return (type ? tempList : staticList).Size();
			}

			ibool IsCodeEnabled(uint type,uint i) const
			{
				return (type ? tempList : staticList).IsEnabled(i);
			}

			ulong GetCode(uint type,uint i) const
			{
				return (type ? tempList : staticList)[i];
			}
		};
	}
}

#endif
