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
#include "../paradox/PdxMap.h"
#include "NstTypes.h"
#include "NstNsp.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSP::SetContext(const IO::NSP::CONTEXT& c) 
{ 
	context = c; 
	ValidateContext();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSP::GetContext(IO::NSP::CONTEXT& c) const 
{ 
	c = context; 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSP::ValidateContext()
{
	{
		typedef PDXMAP<IO::NSP::CONTEXT::GENIECODE,PDXSTRING> TESTED;

		TESTED tested;
		tested.Reserve( context.GenieCodes.Size() );

		for (UINT i=0; i < context.GenieCodes.Size(); ++i)
		{
			const IO::NSP::CONTEXT::GENIECODE& genie = context.GenieCodes[i];

			if ((genie.code.Length() == 6 || genie.code.Length() == 8) && tested.Find( genie.code ) == tested.End())
				tested.Insert( genie.code, genie );
		}

		if (context.GenieCodes.Size() != tested.Size())
		{
			context.GenieCodes.Clear();
			context.GenieCodes.Reserve( tested.Size() );

			for (TESTED::CONSTITERATOR i=tested.Begin(); i != tested.End(); ++i)
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

VOID NSP::RemoveSpace(PDXSTRING& string)
{
	for (UINT i=0; i < string.Length(); ++i)
	{
		if (string[i] == ' ')
		{
			string.Resize(i);
			return;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT NSP::ParseRestType(PDXFILE& file,const CHAR* const type)
{
	const INT length = strlen(type);
	CHAR* const buffer = new CHAR[length];

	if (!file.Read( buffer, buffer + length ))
	{
		delete [] buffer;
		file.Seek( PDXFILE::CURRENT, -length );
		return PDX_FAILURE;
	}

	for (UINT i=0; i < length; ++i)
	{
		if (tolower(buffer[i]) != type[i])
		{
			delete [] buffer;
			file.Seek( PDXFILE::CURRENT, -length );
			return PDX_FAILURE;
		}
	}

	delete [] buffer;

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT NSP::Load(PDXFILE& file)
{
	context.Reset();

	if (!file.IsOpen() || file.IsEmpty())
		return PDX_FAILURE;

	while (!file.Eof())
	{
		CHAR c;

		if (file.Read(c))
		{
			switch (tolower(c))
			{
				case '-':
			
					PDX_TRY(ParseType( file ));
					break;

				case '/': 

					PDX_TRY(ParseRestType( file, "/" ));
					ParseLineEnd( file );
					break;

				case ' ':
					break;

				case '\r':
				case '\n':

					PDX_TRY(ParseLineEnd( file ));
					break;

				default:

					return PDX_FAILURE;
			}
		}
		else
		{
			return PDX_FAILURE;
		}
	}

	ValidateContext();

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT NSP::ParseLineEnd(PDXFILE& file)
{
	while (!file.Eof())
	{
		CHAR c;

		if (!file.Read(c))
			return PDX_FAILURE;

		static const CHAR end[2][2] = 
		{
			{'\r','\n'},
			{'\n','\r'}
		};

		for (UINT i=0; i < 2; ++i)
		{
			if (c == end[i][0]) 
			{
				if (file.Read(c) && c != end[i][1])
					file.Seek( PDXFILE::CURRENT, -1 );

				return PDX_OK;
			}
		}
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT NSP::ParseType(PDXFILE& file)
{
	CHAR c;

	if (file.Read(c))
	{
		switch (tolower(c))
		{
			case 'r': 
				
				if (PDX_SUCCEEDED(ParseRestType( file, "om " )))
					return ParseChoice( file, context.ImageFile );

				break;

			case 's': 

				if (PDX_SUCCEEDED(ParseRestType( file, "tateslot" )))
					return ParseStateSlot( file );

				if (PDX_SUCCEEDED(ParseRestType( file, "av " )))
					return ParseChoice( file, context.SaveFile );

				if (PDX_SUCCEEDED(ParseRestType( file, "ave " )))
					return ParseChoice( file, context.SaveFile );

				if (PDX_SUCCEEDED(ParseRestType( file, "ate " )))
					return ParseChoice( file, context.StateFile );

				break;

			case 'i': 

				if (PDX_SUCCEEDED(ParseRestType( file, "mage " )))
					return ParseChoice( file, context.ImageFile );

				if (PDX_SUCCEEDED(ParseRestType( file, "ps " )))
					return ParseChoice( file, context.IpsFile );

				break;

			case 'm': 
				
				if (PDX_SUCCEEDED(ParseRestType( file, "ode " )))
					return ParseMode( file );

				if (PDX_SUCCEEDED(ParseRestType( file, "ovie " )))
					return ParseChoice( file, context.MovieFile );

				break;
			
			case 'c':

				if (PDX_SUCCEEDED(ParseRestType( file, "heat " )))
					return ParseGenie( file );

				break;

			case 'g': 
				
				if (PDX_SUCCEEDED(ParseRestType( file, "enie " )))
					return ParseGenie( file );

				break;

			case 'p': 
		
				if (PDX_SUCCEEDED(ParseRestType( file, "ort" )))
					return ParsePort( file );

				if (PDX_SUCCEEDED(ParseRestType( file, "alette " )))
					return ParseChoice( file, context.PaletteFile );

				break;
		}
	}

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT NSP::ParseChoice(PDXFILE& file,PDXSTRING& choice)
{
	const TSIZE length = choice.Length();

	CHAR c[2];
	c[1] = '\0';

	while (!file.Eof())
	{
		if (!file.Read(c[0]))
			return PDX_FAILURE;

		if (c[0] == ' ' && choice.Length() == length)
			continue;

		if (c[0] == '/' && (file.Readable(1) && file.Peek<CHAR>() == '/'))
			return ParseLineEnd(file);

		if (c[0] == '\r' || c[0] == '\n')
		{
			file.Seek( PDXFILE::CURRENT, -1 );
			return ParseLineEnd(file);
		}

		choice += c;
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT NSP::ParseMode(PDXFILE& file)
{
	PDXSTRING mode;

	if (PDX_SUCCEEDED(ParseChoice( file, mode )))
	{
		RemoveSpace( mode );

		if (mode == "ntsc") { context.pal = 0; return PDX_OK; }
		if (mode == "pal" ) { context.pal = 1; return PDX_OK; }
	}

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT NSP::ParseStateSlot(PDXFILE& file)
{
	CHAR c;

	if (file.Read(c))
	{
		UINT index;

		switch (c)
		{
    		case '1': index = 0; break;
       		case '2': index = 1; break;
     		case '3': index = 2; break;
     		case '4': index = 3; break;
     		case '5': index = 4; break;
			case '6': index = 5; break;
			case '7': index = 6; break;
			case '8': index = 7; break;
			case '9': index = 8; break;
     		default: return PDX_FAILURE;
		}

		if (file.Read(c) && c == ' ')
			return ParseChoice( file, context.StateSlots[index] );
	}

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT NSP::ParseGenie(PDXFILE& file)
{
	PDXSTRING genie;

	if (PDX_SUCCEEDED(ParseChoice( file, genie )))
	{
		context.GenieCodes.Grow();
		context.GenieCodes.Back().code = genie;

		for (UINT i=0; i < genie.Length(); ++i)
		{
			if (genie[i] == ' ')
			{
				context.GenieCodes.Back().code.Resize(i);

				if (++i < genie.Length())
					context.GenieCodes.Back().comment = genie.At(i);

				break;
			}
		}

		return PDX_OK;
	}

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT NSP::ParsePort(PDXFILE& file)
{
	CHAR c;

	if (file.Read(c))
	{
		UINT index;

		switch (c)
		{
     		case '1': index = 0; break;
			case '2': index = 1; break;
			case '3': index = 2; break;
			case '4': index = 3; break;
			case '5': index = 4; break;
			default: return PDX_FAILURE;
		}

		if (file.Read(c) && c == ' ')
		{
			PDXSTRING port;

			if (PDX_SUCCEEDED(ParseChoice( file, port )))
			{
				RemoveSpace( port );

				if (port == "unconnected") { context.controllers[index] = CONTROLLER_UNCONNECTED; return PDX_OK; }
				if (port == "pad1"       ) { context.controllers[index] = CONTROLLER_PAD1;        return PDX_OK; }
				if (port == "pad2"       ) { context.controllers[index] = CONTROLLER_PAD2;        return PDX_OK; }
				if (port == "pad3"       ) { context.controllers[index] = CONTROLLER_PAD3;        return PDX_OK; }
				if (port == "pad4"       ) { context.controllers[index] = CONTROLLER_PAD4;        return PDX_OK; }
				if (port == "zapper"     ) { context.controllers[index] = CONTROLLER_ZAPPER;      return PDX_OK; }
				if (port == "paddle"     ) { context.controllers[index] = CONTROLLER_PADDLE;      return PDX_OK; }
				if (port == "powerpad"   ) { context.controllers[index] = CONTROLLER_POWERPAD;    return PDX_OK; }
				if (port == "keyboard"   ) { context.controllers[index] = CONTROLLER_KEYBOARD;    return PDX_OK; }
			}
		}
	}

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT NSP::Save(PDXFILE& file) const
{
	PDXSTRING string = 
	(
	    "//\r\n"
	    "// Nestopia generated game configuration file. Version 1.04\r\n"
		"//\r\n"
		"// Syntax:\r\n"
		"//\r\n"
		"//  -ROM / -IMAGE <file>\r\n"
		"//  -SAV / -SAVE  <file>\r\n"
		"//  -STATE <file>\r\n"
		"//  -MOVIE <file>\r\n"
		"//  -STATESLOT1..STATESLOT9 <file>\r\n"
		"//  -IPS <file>\r\n"
		"//  -PALETTE <file>\r\n"
		"//  -MODE <ntsc/pal>\r\n"
		"//  -PORT1..PORT5 <unconnected,pad1,pad2,pad3,pad4,zapper,paddle,powerpad,keyboard>\r\n"
		"//  -GENIE / -CHEAT <code> <comment> (last is optional)\r\n"
		"//\r\n"
		"// Example:\r\n"
		"//\r\n"
		"//  -IMAGE mammamia.nes // C style comment\r\n"
		"//  -MODE ntsc\r\n"
		"//  -PORT1 pad1\r\n"
		"//  -PORT2 unconnected\r\n"
		"//  -CHEAT SXIOPO infinite plumbers\r\n"
		"//\r\n"
		"\r\n"
	);

	if (context.ImageFile.Length())
	{
		string << "-IMAGE ";
		string << context.ImageFile;
		string << "\r\n";
	}

	if (context.IpsFile.Length())
	{
		string << "-IPS ";
		string << context.IpsFile;
		string << "\r\n";
	}

	if (context.SaveFile.Length())
	{
		string << "-SAVE ";
		string << context.SaveFile;
		string << "\r\n";
	}

	for (UINT i=0; i < 9; ++i)
	{
		if (context.StateSlots[i].Length())
		{
			string << "-STATESLOT";
			string << (i+1);
			string << " ";
			string << context.StateSlots[i];
			string << "\r\n";
		}
	}

	if (context.MovieFile.Length())
	{
		string << "-MOVIE ";
		string << context.MovieFile;
		string << "\r\n";
	}

	if (context.PaletteFile.Length())
	{
		string << "-PALETTE ";
		string << context.PaletteFile;
		string << "\r\n";
	}

	if (context.pal != -1)
	{
		string << "-MODE ";
		string << (context.pal ? "pal\r\n" : "ntsc\r\n");
	}

	for (UINT i=0; i < 5; ++i)
	{
		string << "-PORT";
		string << (i+1);
		string << " ";

		switch (context.controllers[i])
		{
			case CONTROLLER_PAD1:        string << "pad1\r\n";        break;
       		case CONTROLLER_PAD2:        string << "pad2\r\n";        break;
       		case CONTROLLER_PAD3:        string << "pad3\r\n";        break;
       		case CONTROLLER_PAD4:        string << "pad4\r\n";        break;
       		case CONTROLLER_ZAPPER:      string << "zapper\r\n";      break;
       		case CONTROLLER_PADDLE:      string << "paddle\r\n";      break;
       		case CONTROLLER_POWERPAD:    string << "powerpad\r\n";    break;
			case CONTROLLER_KEYBOARD:    string << "keyboard\r\n";    break;
			default:                     string << "unconnected\r\n"; break;
		}
	}

	for (UINT i=0; i < context.GenieCodes.Size(); ++i)
	{
		const IO::NSP::CONTEXT::GENIECODE& genie = context.GenieCodes[i];

		if (genie.code.Length())
		{
			string << "-GENIE ";
			string << genie.code;

			if (genie.comment.Length())
			{
				string << " ";
				string << genie.comment;
			}

			string << "\r\n";
		}
	}

	file.Write( string.Begin(), string.End() );

	return PDX_OK;
}

NES_NAMESPACE_END
