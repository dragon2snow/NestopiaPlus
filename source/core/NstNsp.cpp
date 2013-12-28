////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003 Martin Freij
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

#include "../paradox/PdxFile.h"
#include "NstTypes.h"
#include "NstNsp.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSP::SetContext(const CONTEXT& c) 
{ 
	context = c; 
	ValidateContext();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSP::GetContext(CONTEXT& c) const 
{ 
	c = context; 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSP::ValidateContext()
{
	{
		typedef PDXMAP<CONTEXT::GENIECODE,PDXSTRING> TESTED;

		TESTED tested;
		tested.Reserve( context.GenieCodes.Size() );

		for (TSIZE i=0; i < context.GenieCodes.Size(); ++i)
		{
			const CONTEXT::GENIECODE& genie = context.GenieCodes[i];

			if ((genie.code.Length() == 6 || genie.code.Length() == 8) && tested.Find( genie.code ) == tested.End())
				tested.Insert( genie.code, genie );
		}

		if (context.GenieCodes.Size() != tested.Size())
		{
			context.GenieCodes.Clear();
			context.GenieCodes.Reserve( tested.Size() );

			for (TESTED::CONSTITERATOR i(tested.Begin()); i != tested.End(); ++i)
				context.GenieCodes.InsertBack( (*i).Second() );
		}
	}

	for (UINT i=0; i < 5; ++i)
	{
		switch (context.controllers[i])
		{
   			case CONTROLLER_PAD1:    
   			case CONTROLLER_PAD2:    
   			case CONTROLLER_PAD3:    
   			case CONTROLLER_PAD4:    
   			case CONTROLLER_ZAPPER:  
  			case CONTROLLER_PADDLE:  
   			case CONTROLLER_POWERPAD:
   			case CONTROLLER_KEYBOARD:
     			continue;

     		default:     

				context.controllers[i] = CONTROLLER_UNCONNECTED;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSP::PreProcess(PDXSTRING& string,PDXFILE& file)
{
	string.Buffer().Resize( file.Size() + 1 );
	CHAR* iterator = string.Buffer().Begin();

	while (!file.Eof())
	{
		const CHAR ch = file.Read<CHAR>();

		if (ch != '\a' && ch != '\t' && ch != '\f' && ch != '\0')
			*iterator++ = ch;
	}

	*iterator++ = '\0';
	string.Buffer().Resize( iterator - string.Buffer().Begin() );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL NSP::Parse(TREE& tree,BATCH& batch,PDXSTRING& buffer)
{
	PDXSTRING command;

	for (const CHAR* offset = buffer.String(); *offset != '\0'; )
	{
		if (*offset == '/' && offset[1] == '/')
		{
			for (offset += 2; *offset != '\r' && *offset != '\n' && *offset != '\0'; ++offset);

			if (*offset++ == '\0')
				break;

			if (*offset == '\n' || *offset == '\r')
				++offset;
		}
		else if (*offset == '-')
		{
			for (++offset; *offset == ' '; ++offset);

			if (*offset == '\0')
				return FALSE;

			const CHAR* end;

			for (end=offset+1; *end != ' ' && *end != '\0'; ++end);

			if (*end == '\0')
				return FALSE;

			command.Set( offset, end );

			for (offset=end+1; *offset == ' '; ++offset);

			if (*offset == '\0')
				return FALSE;

			for (end=offset+1; ; ++end)
			{
				if
				(
					*end == '\r' || 
					*end == '\n' ||
					(*end == '/' && end[1] == '/') ||
					*end == '\0'
				)
     				break;
			}

			const CHAR* last;

			for (last=end-1; *last == ' '; --last);

			PDXSTRING* value;

			if (command == "GENIE" || command == "CHEAT")
			{
				batch.Grow();
				value = &batch.Back();
			}
			else
			{
				value = &tree[command];
			}

			value->Set( offset, last + 1 );

			for (offset=end; *offset != '\r' && *offset != '\n' && *offset != '\0'; ++offset);

			if (*offset++ == '\0')
				break;

			if (*offset == '\n' || *offset == '\r')
				++offset;
		}
		else
		{
			++offset;
		}
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

const PDXSTRING* NSP::FindItem(const TREE& tree,const CHAR* const item)
{
	TREE::CONSTITERATOR iterator( tree.Find(item) );
	return (iterator != tree.End() && (*iterator).Second().Length() ? &(*iterator).Second() : NULL);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSP::ParseFileName(const PDXSTRING* const item,PDXSTRING& type)
{
	if (item)
	{
		type = *item;
		type.RemoveQuotes();
		type.RemoveSpaces();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSP::ParsePort(const PDXSTRING* const item,CONTROLLERTYPE& type)
{
	if (item)
	{
		     if (*item == "pad1"       ) type = CONTROLLER_PAD1;
		else if (*item == "pad2"       ) type = CONTROLLER_PAD2;
		else if (*item == "pad3"       ) type = CONTROLLER_PAD3;
		else if (*item == "pad4"       ) type = CONTROLLER_PAD4;
		else if (*item == "zapper"     ) type = CONTROLLER_ZAPPER;
		else if (*item == "paddle"     ) type = CONTROLLER_PADDLE;
		else if (*item == "powerpad"   ) type = CONTROLLER_POWERPAD;
		else if (*item == "keyboard"   ) type = CONTROLLER_KEYBOARD;
		else if (*item == "unconnected") type = CONTROLLER_UNCONNECTED;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSP::ParseMode(const PDXSTRING* const item,INT& pal)
{
	if (item)
	{
		     if (*item == "pal")  pal = TRUE;
		else if (*item == "ntsc") pal = FALSE;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSP::ParseGenie(const BATCH& batch)
{
	for (TSIZE i=0; i < batch.Size(); ++i)
	{
		context.GenieCodes.Grow();
		context.GenieCodes.Back().code = batch[i];

		for (TSIZE j=0; j < batch[i].Length(); ++j)
		{
			if (batch[i][j] == ' ')
			{
				context.GenieCodes.Back().code.Resize(j);

				PDXSTRING::CONSTITERATOR offset = batch[i].At(j+1);

				while (*offset == ' ')
					++offset;

				context.GenieCodes.Back().enabled = !
				(
					(offset[0] == 'O' || offset[0] == 'o') &&
					(offset[1] == 'F' || offset[1] == 'f') &&
					(offset[2] == 'F' || offset[2] == 'f') &&
					(offset[3] == ' ' || offset[3] == '\0')
				);

				while (*offset != ' ' && *offset != '\0')
					++offset;

				while (*offset == ' ')
					++offset;

				if (*offset != '\0')
					context.GenieCodes.Back().comment = offset;

				break;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT NSP::Load(PDXFILE& file)
{
	context.Reset();

	if (!file.IsOpen() || file.IsEmpty())
		return PDX_FAILURE;

	PDXSTRING buffer;
	PreProcess( buffer, file );

	TREE tree;
	BATCH batch;

	if (!Parse( tree, batch, buffer ))
		return PDX_FAILURE;

	const PDXSTRING* item;
	
	if (!(item = FindItem( tree, "IMAGE" )))
		item = FindItem( tree, "ROM" );

	ParseFileName( item, context.ImageFile );
		
	if (!(item = FindItem( tree, "SAVE" )))
		item = FindItem( tree, "SAV" );

	ParseFileName( item, context.SaveFile );
		
	ParseFileName( FindItem( tree, "STATE"      ), context.StateFile     );
	ParseFileName( FindItem( tree, "MOVIE"      ), context.MovieFile     );
	ParseFileName( FindItem( tree, "STATESLOT1" ), context.StateSlots[0] );
	ParseFileName( FindItem( tree, "STATESLOT2" ), context.StateSlots[1] );
	ParseFileName( FindItem( tree, "STATESLOT3" ), context.StateSlots[2] );
	ParseFileName( FindItem( tree, "STATESLOT4" ), context.StateSlots[3] );
	ParseFileName( FindItem( tree, "STATESLOT5" ), context.StateSlots[4] );
	ParseFileName( FindItem( tree, "STATESLOT6" ), context.StateSlots[5] );
	ParseFileName( FindItem( tree, "STATESLOT7" ), context.StateSlots[6] );
	ParseFileName( FindItem( tree, "STATESLOT8" ), context.StateSlots[7] );
	ParseFileName( FindItem( tree, "STATESLOT9" ), context.StateSlots[8] );
	ParseFileName( FindItem( tree, "IPS"        ), context.IpsFile       );
	ParseFileName( FindItem( tree, "PALETTE"    ), context.PaletteFile   );

	ParsePort( FindItem( tree, "PORT1" ), context.controllers[0] );
	ParsePort( FindItem( tree, "PORT2" ), context.controllers[1] );
	ParsePort( FindItem( tree, "PORT3" ), context.controllers[2] );
	ParsePort( FindItem( tree, "PORT4" ), context.controllers[3] );
	ParsePort( FindItem( tree, "PORT5" ), context.controllers[4] );

	ParseMode( FindItem( tree, "MODE" ), context.pal );

	ParseGenie( batch );

	ValidateContext();

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT NSP::Save(PDXFILE& file,const SAVEOPTION SaveOption) const
{
	PDXSTRING string = 
	(
	    "//\r\n"
	    "// Generated Nestopia Script File. Version 1.07\r\n"
		"//\r\n"
		"// Syntax:\r\n"
		"//\r\n"
		"//  -ROM / -IMAGE <filename>\r\n"
		"//  -SAV / -SAVE  <filename>\r\n"
		"//  -STATE <filename>\r\n"
		"//  -MOVIE <filename>\r\n"
		"//  -STATESLOT1..STATESLOT9 <filename>\r\n"
		"//  -IPS <filename>\r\n"
		"//  -PALETTE <filename>\r\n"
		"//  -MODE <ntsc/pal>\r\n"
		"//  -PORT1..PORT5 <unconnected,pad1,pad2,pad3,pad4,zapper,paddle,powerpad,keyboard>\r\n"
		"//  -GENIE / -CHEAT <code> <on,off> <comment> (last is optional)\r\n"
		"//\r\n"
		"// only one command per line is allowed.\r\n"
		"//\r\n"
		"// Example:\r\n"
		"//\r\n"
		"//  -IMAGE C:\\Games\\mammamia.nes // C style comment\r\n"
		"//  -MODE ntsc\r\n"
		"//  -PORT1 pad1\r\n"
		"//  -PORT2 unconnected\r\n"
		"//  -CHEAT SXIOPO on infinite plumbers\r\n"
		"//\r\n"
		"\r\n"								  
	);										  

	if (SaveOption == SAVE_ALL)
	{
		if (context.ImageFile.Length())
			string << "-IMAGE " << context.ImageFile << "\r\n";

		if (context.IpsFile.Length())
			string << "-IPS " << context.IpsFile << "\r\n";

		if (context.SaveFile.Length())
			string << "-SAVE " << context.SaveFile << "\r\n";

		for (UINT i=0; i < 9; ++i)
		{
			if (context.StateSlots[i].Length())
				string << "-STATESLOT" << (i+1) << context.StateSlots[i] << "\r\n";
		}

		if (context.MovieFile.Length())
			string << "-MOVIE " << context.MovieFile << "\r\n";

		if (context.PaletteFile.Length())
			string << "-PALETTE " << context.PaletteFile << "\r\n";

		if (context.pal != -1)
			string << "-MODE " << (context.pal ? "pal\r\n" : "ntsc\r\n");

		for (UINT i=0; i < 5; ++i)
		{
			string << "-PORT" << (i+1);

			switch (context.controllers[i])
			{
				case CONTROLLER_PAD1:        string << " pad1\r\n";        break;
				case CONTROLLER_PAD2:        string << " pad2\r\n";        break;
				case CONTROLLER_PAD3:        string << " pad3\r\n";        break;
				case CONTROLLER_PAD4:        string << " pad4\r\n";        break;
				case CONTROLLER_ZAPPER:      string << " zapper\r\n";      break;
				case CONTROLLER_PADDLE:      string << " paddle\r\n";      break;
				case CONTROLLER_POWERPAD:    string << " powerpad\r\n";    break;
				case CONTROLLER_KEYBOARD:    string << " keyboard\r\n";    break;
				default:                     string << " unconnected\r\n"; break;
			}
		}
	}

	for (TSIZE i=0; i < context.GenieCodes.Size(); ++i)
	{
		const CONTEXT::GENIECODE& genie = context.GenieCodes[i];

		if (genie.code.Length())
		{
			string << "-GENIE " << genie.code << (genie.enabled ? " on" : " off");

			if (genie.comment.Length())
				string << " " << genie.comment;
					  
			string << "\r\n";
		}
	}

	file.Write( string.Begin(), string.End() );

	return PDX_OK;
}

NES_NAMESPACE_END
