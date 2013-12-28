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

#ifndef NST_DIALOG_LAUNCHER_H
#define NST_DIALOG_LAUNCHER_H

#pragma once

#include <vector>
#include "NstCollectionBitSet.hpp"
#include "NstWindowMenu.hpp"
#include "NstWindowStatusBar.hpp"
#include "NstWindowDialog.hpp"
#include "NstWindowParam.hpp"
#include "NstDialogFind.hpp"
#include "../core/api/NstApiCartridge.hpp"
#include <CommCtrl.h>

namespace Nes
{
	using namespace Api;
}

namespace Nestopia
{
	namespace Io
	{
		class File;
	}

	namespace Window
	{
		class Launcher
		{
		public:

			Launcher(const Nes::Cartridge::Database&,const Managers::Paths&,const Configuration&);
			~Launcher();

			void Save(Configuration&,ibool);
			void Open();
			void Close();

		private:

			struct Handlers;

			enum
			{
				STATUSBAR_SECOND_FIELD_WIDTH = 14,
				CMD_ENTER = 1
			};

			void UpdateItemCount(u32) const;
			void OnNoSelection() const;

			ibool OnInitDialog (Param&);
			ibool OnDropFiles  (Param&);
			ibool OnSize       (Param&);
			ibool OnCmdEnter   (Param&);
			ibool OnDestroy    (Param&);

			void OnCmdFileRun	            	(uint=0);
			void OnCmdFileRefresh	            (uint);
			void OnCmdViewShowGrids             (uint); 
			void OnCmdViewShowDatabaseCorrected (uint); 
			void OnCmdOptionsPaths              (uint); 
			void OnCmdOptionsColors	            (uint);

			void OnListGetDisplayInfo    (const NMHDR&);
			void OnListKeyDown		     (const NMHDR&);
			void OnListColumnClick       (const NMHDR&);
			void OnListItemActivate      (const NMHDR&);
			void OnListItemChanged       (const NMHDR&);
			void OnListInsertItem        (const NMHDR&);
			void OnListDeleteItem        (const NMHDR&);
			void OnListDeleteAllItems    (const NMHDR&);
			void OnTreeSelectionChanging (const NMHDR&);

			class List
			{
			public:

				List
				(
					Dialog&,
					Menu::CmdHandler&,
					const Managers::Paths&,
					const Configuration&,
					const Nes::Cartridge::Database&
				);

				~List();

				void operator = (const Control::ListView&);

				enum Updater
				{	 
					DONT_REPAINT,
					REPAINT
				};

				void  Add(tstring);
				void  Close();
				void  Save(Configuration&,ibool);
				void  Sort(uint=0);
				ibool CanRefresh() const;
				void  Refresh();
				void  Insert(const Window::Param&);
				void  SetColors(uint,uint,Updater=DONT_REPAINT) const;
				void  OnGetDisplayInfo(LPARAM);

				class Paths
				{
				public:

					explicit Paths(const Configuration&);
					~Paths();

					void Save(Configuration&) const;

					struct Settings
					{
						struct Folder
						{
							Path path;
							ibool incSubDir;
						};

						typedef std::vector<Folder> Folders;

						struct Include : Collection::BitSet
						{
							enum
							{
								NES,UNF,FDS,NSF,IPS,NSP,ARCHIVE,ANY,UNIQUE
							};

							enum						
							{
								TYPES = NES|UNF|FDS|NSF|IPS|NSP,
								FILES = TYPES|ARCHIVE
							};

							explicit Include(bool a=false)
							: Collection::BitSet( Nes::b10111111 | (a << 6) ) {}
						};

						Include include;
						Folders folders;
					};

				private:

					struct Handlers;

					enum 
					{
						LIMIT = 999
					};

					ibool OnInitDialog (Param&);
					ibool OnCmdAdd     (Param&);
					ibool OnCmdRemove  (Param&);
					ibool OnCmdClear   (Param&);
					ibool OnCmdCancel  (Param&);
					ibool OnCmdOk      (Param&);

					void OnKeyDown     (const NMHDR&);
					void OnItemChanged (const NMHDR&);
					void OnInsertItem  (const NMHDR&);
					void OnDeleteItem  (const NMHDR&);

					Settings settings;
					Dialog dialog;
					const Control::NotificationHandler notifications;

				public:

					void Open()
					{
						dialog.Open();
					}

					const Settings& GetSettings() const
					{
						return settings;
					}
				};

				class Files
				{
					class Inserter;
					class Searcher;

				public:

					class Strings
					{
						HeapString container;

					public:

						explicit Strings(uint=0);

						typedef u32 Index;

						enum 
						{
							NONE = -1
						};

						int   Find(GenericString) const;
						void  Clear();
						uint  Count() const;
						ibool Import(const Io::File&,uint,ibool);
						void  Export(const Io::File&) const;

						template<typename T>
						Index operator << (const T& t)
						{
							uint pos = container.Length();
							container << t << '\0';
							return pos;
						}

						tstring operator [] (uint i) const
						{
							return container.Ptr() + i;
						}

						bool IsUTF16() const
						{
							return container.Wide();
						}

						uint Size() const
						{
							return container.Length() * (IsUTF16() ? 2 : 1);
						}
					};

					explicit Files(const Nes::Cartridge::Database&);
					~Files();

					void  Save(const Nes::Cartridge::Database&);
					void  Refresh(const Paths::Settings&,const Nes::Cartridge::Database&);
					ibool Insert(const Nes::Cartridge::Database&,GenericString); 
					void  Clear();
					ibool ShouldDefrag() const;
					void  Defrag();

					class Entry
					{
						friend class Files;
						friend class Inserter;
						friend class Searcher;

					public:

						enum Type
						{
							NES     = 0x01,
							UNF     = 0x02,
							FDS     = 0x04,
							NSF     = 0x08,
							IPS     = 0x10,
							NSP	    = 0x20,
							ARCHIVE = 0x40,
							ALL     = NES|UNF|FDS|NSF|IPS|NSP
						};

						enum
						{
							MIRROR_NONE,
							MIRROR_HORIZONTAL,
							MIRROR_VERTICAL,
							MIRROR_ZERO,
							MIRROR_ONE,      
							MIRROR_FOURSCREEN,
							MIRROR_CONTROLLED
						};

						enum
						{
							SYSTEM_UNKNOWN,
							SYSTEM_PC10,
							SYSTEM_VS,
							SYSTEM_PAL,
							SYSTEM_NTSC,
							SYSTEM_NTSC_PAL
						};

						tstring GetName(const Strings&,const Nes::Cartridge::Database*) const;
						uint GetSystem() const;
						uint GetMirroring(const Nes::Cartridge::Database*) const;
						uint GetSystem(const Nes::Cartridge::Database*) const;

					private:

						explicit Entry(uint=0);

						Strings::Index file;
						Strings::Index path;
						Strings::Index name;
						Strings::Index maker;

						Nes::Cartridge::Database::Entry dBaseEntry;

						u16 pRom;
						u16 cRom;
						u16 wRam;
						u8 mapper;
						u8 type;

						enum
						{
							FLAGS_NTSC_PAL = 0x40|0x80
						};

                    #pragma pack(push,1)

						struct Bits 
						{
							u8 mirroring : 3;
							u8 battery   : 1;
							u8 trainer   : 1;
							u8 vs        : 1;
							u8 pal       : 1;
							u8 ntsc      : 1;
						};

                    #pragma pack(pop)

						NST_COMPILE_ASSERT( sizeof(Bits) == 1 );

						union
						{
							u8 flags;
							Bits bits;
						};

					public:

						Type GetType() const
						{
							return (Type) type;
						}

						tstring GetPath(const Strings& strings) const
						{
							return strings[path];
						}

						tstring GetFile(const Strings& strings) const
						{
							return strings[file];
						}

						tstring GetName(const Strings& strings) const
						{
							return strings[name];
						}

						tstring GetMaker(const Strings& strings) const
						{
							return strings[maker];
						}

						uint GetPRom() const 
						{ 
							return pRom; 
						}

						uint GetCRom() const 
						{ 
							return cRom; 
						}

						uint GetWRam() const 
						{ 
							return wRam; 
						}

						uint GetPRom(const Nes::Cartridge::Database* db) const 
						{ 
							return dBaseEntry && db ? db->GetPRomSize( dBaseEntry ) / Nes::Core::SIZE_1K : pRom; 
						}

						uint GetCRom(const Nes::Cartridge::Database* db) const 
						{ 
							return dBaseEntry && db ? db->GetCRomSize( dBaseEntry ) / Nes::Core::SIZE_1K : cRom; 
						}

						uint GetWRam(const Nes::Cartridge::Database* db) const 
						{ 
							return dBaseEntry && db ? db->GetWRamSize( dBaseEntry ) / Nes::Core::SIZE_1K : wRam; 
						}

						uint GetMapper() const 
						{ 
							return mapper;         
						}

						ibool GetBattery() const 
						{ 
							return bits.battery;   
						}

						ibool GetTrainer() const 
						{ 
							return bits.trainer;   
						}

						uint GetMirroring() const 
						{ 
							return bits.mirroring; 
						}

						ibool GetBattery(const Nes::Cartridge::Database* db) const 
						{ 
							return dBaseEntry && db ? db->HasBattery( dBaseEntry ) : bits.battery; 
						}

						ibool GetTrainer(const Nes::Cartridge::Database* db) const 
						{
							return dBaseEntry && db ? db->HasTrainer( dBaseEntry ) : bits.trainer; 
						}

						uint GetMapper(const Nes::Cartridge::Database* db) const 
						{ 
							return dBaseEntry && db ? db->GetMapper( dBaseEntry ) : mapper;  
						}
					};

					typedef Collection::Vector<Entry> Entries;

				private:

					enum Exception
					{
						ERR_CORRUPT_DATA
					};

					enum
					{
						GARBAGE_THRESHOLD = 127
					};

               #pragma pack(push,1)

					struct Header
					{
						enum 
						{
							ID = NST_FOURCC('n','s','d',0),
							MAX_ENTRIES = 0xFFFFF,
							VERSION = 2,
							FLAGS_UTF16 = 0x1
						};

						u32 id;
						u32 version;
						u32 stringSize;
						u32 numStrings;
						u32 numEntries;
						u32 flags;
					};

               #pragma pack(pop)

					NST_COMPILE_ASSERT( sizeof(Header) == 24 );

					ibool dirty;
					Strings strings;
					Entries entries;

				public:

					uint Count() const
					{
						return entries.Size();
					}

					const Entry& operator [] (uint i) const
					{
						return entries[i];
					}

					void Disable(Entry* entry)
					{
						if (entry->type)
						{
							entry->type = 0;
							dirty = TRUE;
						}
					}

					const Strings& GetStrings() const
					{
						return strings;
					}
				};

				class Columns
				{
				public:

					explicit Columns(const Configuration&);
					~Columns();

					void Update(const uchar*);
					void Save(Configuration&) const;

					enum Type
					{
						TYPE_FILE,
						TYPE_SYSTEM,
						TYPE_MAPPER,
						TYPE_PROM,
						TYPE_CROM,
						TYPE_WRAM,
						TYPE_BATTERY,
						TYPE_TRAINER,
						TYPE_MIRRORING,
						TYPE_NAME,
						TYPE_MAKER,
						TYPE_FOLDER,
						NUM_TYPES,
						NUM_DEFAULT_SELECTED_TYPES = 9,
						NUM_DEFAULT_AVAILABLE_TYPES = NUM_TYPES - NUM_DEFAULT_SELECTED_TYPES
					};

				private:

					struct Handlers;

					enum 
					{
						GUI,
						CFG,
						NUM_STRING_STYLES
					};

					static cstring CfgName(uint);

					void Reset();
					void Add(uint,uint);
					void UpdateButtonRemove();
					void UpdateButtonAdd();

					ibool OnInitDialog   (Param&);
					ibool OnCmdSelected  (Param&);
					ibool OnCmdAvailable (Param&);
					ibool OnCmdAdd       (Param&);
					ibool OnCmdRemove    (Param&);
					ibool OnCmdDefault   (Param&);
					ibool OnCmdOk        (Param&);
					ibool OnCmdCancel    (Param&);

					typedef Collection::Vector<uchar> Types;

					Types available;
					Types selected;
					Dialog dialog;

					static tstring const strings[NUM_STRING_STYLES][NUM_TYPES]; 

				public:

					uint Count() const
					{ 
						return selected.Size(); 
					}

					Type GetType(uint i) const
					{ 
						return Type(selected[i]);
					}

					tstring GetString(uint i) const
					{ 
						return strings[GUI][selected[i]]; 
					}

					void Open()
					{
						dialog.Open();
					}
				};

			private:

				class Strings
				{
				public:

					tstring GetSize(u32);
					inline tstring GetMapper(uint) const;
					inline void Flush();

				private:

					struct SizeString : String::Stack<10+1,tchar>
					{
						explicit SizeString(u32 i)
						{
							(*this) << i << 'k';
						}
					};

					Collection::Map<u32,SizeString,true> sizes;

					static const tchar mappers[256][4];
				};

				enum 
				{
					STYLE = 
					(
						LVS_EX_FULLROWSELECT |
						LVS_EX_TWOCLICKACTIVATE |
						LVS_EX_HEADERDRAGDROP
					)
				};

				void  ReloadListColumns() const;
				void  UpdateColumnOrder();
				void  UpdateSortColumnOrder(uint);
				void  Redraw();
				int   Sorter(const void*,const void*);
				void  OnFind(GenericString,uint);
				ibool Optimize();

				void OnCmdEditFind         (uint); 
				void OnCmdEditInsert       (uint); 
				void OnCmdEditDelete       (uint);
				void OnCmdEditClear        (uint);
				void OnCmdViewAlignColumns (uint); 
				void OnCmdOptionsColumns   (uint); 

				Control::ListView ctrl;

				const Nes::Cartridge::Database imageDatabase;
				const Nes::Cartridge::Database* useImageDatabase;

				uchar order[Columns::NUM_TYPES];

				uint typeFilter;
				uint style;

				Finder finder;
				Paths paths;
				Files files;
				Columns columns;
				Strings strings;
				const Managers::Paths& pathManager;
				
			public:

				Generic GetWindow() const
				{
					return ctrl.GetWindow();
				}

				void OpenPathDialog()
				{
					paths.Open();
				}

				void Draw(uint type)
				{
					typeFilter = type;
					Redraw();
				}

				ibool DatabaseCorrectionEnabled() const
				{
					return useImageDatabase != NULL;
				}

				ibool ToggleDatabase()
				{
					useImageDatabase = (useImageDatabase ? NULL : &imageDatabase);
					ctrl.Repaint();
					return useImageDatabase != NULL;
				}

				ibool ToggleGrids()
				{
					style ^= LVS_EX_GRIDLINES;
					ctrl.StyleEx() = style;
					return style & LVS_EX_GRIDLINES;
				}

				ibool HitTest(const Point& point) const
				{
					return ctrl.HitTest( point.x, point.y );
				}

				uint GetStyle() const
				{
					return ctrl.StyleEx();
				}

				HWND GetHandle() const
				{
					return ctrl.GetHandle();
				}

				uint NumPaths() const
				{
					return paths.GetSettings().folders.size();
				}

				uint Size() const
				{
					return ctrl.Size();
				}

				const Files::Entry* operator [] (uint i) const
				{
					return static_cast<const Files::Entry*>(static_cast<const void*>( ctrl[i].Data() ));
				}

				const Files::Entry* GetSelection() const
				{
					int index = ctrl.Selection().GetIndex();
					return index >= 0 ? (*this)[index] : NULL;
				}

				const Files::Strings& GetStrings() const
				{
					return files.GetStrings();
				}
			};

			class Tree
			{
			public:

				Tree();

				void operator = (const Control::TreeView&);

				enum Updater {DONT_REPAINT,REPAINT};

				void SetColors(uint,uint,Updater=DONT_REPAINT) const;
				uint GetType(HTREEITEM) const;
				void Close();

			private:

				Control::TreeView ctrl;
				uint selection;
				const Control::TreeView::ImageList imageList;

			public:

				Generic GetWindow() const
				{
					return ctrl.GetWindow();
				}
			};

			class Colors
			{
			public:

				explicit Colors(const Configuration&);

				void Save(Configuration&) const;

			private:

				struct Handlers;

				enum
				{
					DEF_BACKGROUND_COLOR = RGB(0xFF,0xFF,0xFF),
					DEF_FOREGROUND_COLOR = RGB(0x00,0x00,0x00)
				};

				struct Type
				{
					inline Type(int,int,int,int);

					COLORREF color;
					const Rect rect;
				};

				void Paint(const Type&) const;
				void ChangeColor(COLORREF&);

				ibool OnPaint               (Param&);
				ibool OnCmdChangeBackground (Param&);
				ibool OnCmdChangeForeground (Param&);
				ibool OnCmdDefault          (Param&);
				ibool OnCmdOk               (Param&);

				Type background;
				Type foreground;

				COLORREF customColors[16];

				Dialog dialog;

			public:

				COLORREF GetBackgroundColor() const
				{
					return background.color;
				}

				COLORREF GetForegroundColor() const
				{
					return foreground.color;
				}

				void Open()
				{
					dialog.Open();
				}
			};

			Dialog dialog;
			Menu menu;
			Control::NotificationHandler listNotifications;
			Control::NotificationHandler treeNotifications;
			StatusBar statusBar;
			Tree tree;
			List list;
			Point margin;
			Colors colors;
		};
	}
}

#endif
