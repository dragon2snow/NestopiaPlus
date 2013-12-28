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

#include "NstIoNsp.hpp"
#include "../core/api/NstApiCheats.hpp"
#include "../core/api/NstApiMachine.hpp"

namespace Nestopia
{
	using Io::Nsp::Context;

	Context::Context()
	: mode(UNKNOWN)
	{
		for (uint i=0; i < NUM_CONTROLLER_PORTS; ++i)
			controllers[i] = UNKNOWN;
	}

	Context::~Context()
	{
	}

	void Context::Reset()
	{
		image.Clear();
		ips.Clear();
		save.Clear();
		state.Clear();
		movie.Clear();
		
		for (uint i=0; i < NUM_CONTROLLER_PORTS; ++i)
			controllers[i] = UNKNOWN;

		mode = UNKNOWN;

		cheats.clear();
	}

	NST_COMPILE_ASSERT
	(
		Nes::Input::UNCONNECTED	    ==  0 &&
		Nes::Input::PAD1		    ==  1 &&
		Nes::Input::PAD2		    ==  2 &&
		Nes::Input::PAD3		    ==  3 &&
		Nes::Input::PAD4		    ==  4 &&
		Nes::Input::ZAPPER		    ==  5 &&
		Nes::Input::PADDLE		    ==  6 &&
		Nes::Input::POWERPAD	    ==  7 &&
		Nes::Input::KEYBOARD	    ==  8 &&
		Nes::Input::OEKAKIDSTABLET  ==  9 &&
		Nes::Input::HYPERSHOT	    == 10 &&
		Nes::Input::CRAZYCLIMBER    == 11 &&
		Nes::Input::MAHJONG		    == 12 &&
		Nes::Input::EXCITINGBOXING  == 13 &&
		Nes::Input::TOPRIDER	    == 14 &&
		Nes::Input::POKKUNMOGURAA   == 15 &&
		Nes::Input::NUM_CONTROLLERS == 15
	);

	using Io::Nsp::File;

	cstring const File::controllerNames[] =
	{
		"unconnected",
		"pad1",
		"pad2",
		"pad3",
		"pad4",
		"zapper",
		"paddle",
		"powerpad",
		"keyboard",
		"oekakidstablet",
		"hypershot",
		"crazyclimber",
		"mahjong",
		"excitingboxing",
		"toprider",
		"pokkunmoguraa"
	};

	void File::Save(Output& output,const Context& context) const
	{
		NST_COMPILE_ASSERT
		( 
			Context::NUM_CONTROLLER_PORTS == 5 &&
			Context::NUM_CONTROLLERS == 15
		);

		output << "//\r\n"
				  "// Generated Nestopia Script File. Version 1.22\r\n"
				  "//\r\n"
				  "// Syntax:\r\n"
				  "//\r\n"
				  "//  -ROM / -IMAGE <filename>\r\n"
				  "//  -SAV / -SAVE  <filename>\r\n"
				  "//  -STATE <filename>\r\n"
				  "//  -MOVIE <filename>\r\n"
				  "//  -IPS <filename>\r\n"
				  "//  -MODE <ntsc/pal>\r\n"
				  "//  -PORT1..PORT5 <unconnected,pad1,pad2,pad3,pad4,zapper,paddle,powerpad,\r\n"
				  "//                 keyboard,oekakidstablet,hypershot,crazyclimber,mahjong,\r\n"
				  "//                 excitingboxing,toprider,pokkunmoguraa>\r\n"
				  "//\r\n"
				  "//  -GENIE <code> <on,off> (description)\r\n"
				  "//  -CHEAT <address> <value> (compare value) <on,off> (description)\r\n"
				  "//\r\n"
				  "// The values in parenthesis are optional.\r\n"
				  "// Only one argument per line is allowed.\r\n"
				  "//\r\n"
				  "// Example:\r\n"
				  "//\r\n"
				  "//  -IMAGE C:\\games\\bringbacknestacos.nes // C style comment\r\n"
				  "//  -MODE ntsc\r\n"
				  "//  -PORT1 pad1\r\n"
				  "//  -PORT2 unconnected\r\n"
				  "//  -GENIE SXIOPO on infinite plumbers\r\n"
				  "//\r\n\r\n";
				  
		if (context.image.Size())
			output << "-IMAGE " << context.image << "\r\n";

		if (context.ips.Size())
			output << "-IPS " << context.ips << "\r\n";

		if (context.save.Size())
			output << "-SAVE " << context.save << "\r\n";

		if (context.movie.Size())
			output << "-MOVIE " << context.movie << "\r\n";

		if (context.mode != Context::UNKNOWN)
			output << "-MODE " << (context.mode == Nes::Machine::PAL ? "pal\r\n" : "ntsc\r\n");

		for (uint i=0; i < Context::NUM_CONTROLLER_PORTS; ++i)
		{
			if (context.controllers[i] != Context::UNKNOWN)
				output << "-PORT" << (i+1) << ' ' << controllerNames[context.controllers[i]] << "\r\n";
		}

		for (Context::Cheats::const_iterator it(context.cheats.begin()); it != context.cheats.end(); ++it)
		{
			output << "-CHEAT " 
				   << String::Hex( (u16) it->address )
				   << ' '
				   << String::Hex( (u8) it->value );

			if (it->useCompare)
				output << ' ' << String::Hex( (u8) it->compare );
				   
		    output << (it->enabled ? " on" : " off");

			if (it->desc.Size())
			{
				output << ' ' << it->desc;
			}
			else if (it->address >= 0x8000U)
			{
				char characters[9];

				if (NES_SUCCEEDED(Nes::Cheats::GameGenieEncode( Nes::Cheats::Code(it->address,it->value,it->compare,it->useCompare), characters )))
					output << " (" << cstring(characters) << ')';
			}

			output << "\r\n";
		}
	}

	class File::Parser
	{
	public:

		Parser(cstring,cstring const);

	private:

		void SkipSpace()
		{
			while (*it && *it == ' ')
				++it;
		}

		cstring it;
		cstring buffer;

	public:

		~Parser()
		{
			delete [] buffer;
		}

		ibool Check() const
		{
			if (*it)
			{
				if (*it == '-' || (it[0] == '/' && it[1] == '/') || *it == ' ' || *it == '\n')
					return TRUE;

				throw ERR_SYNTAX;
			}

			return FALSE;
		}

		void NextLine()
		{
			while (*it && *it++ != '\n');
		}

		ibool CheckType()
		{
			SkipSpace();

			if (*it == '-')
			{
				++it;
				return TRUE;
			}

			return FALSE;
		}

		void ReadType(cstring (&range)[2])
		{
			SkipSpace();

			range[0] = it;

			while (*it && *it != ' ')
				++it;

			range[1] = it;

			if (range[0] == range[1])
				throw ERR_SYNTAX;
		}

		void ReadValue(cstring (&range)[2])
		{
			SkipSpace();

			range[0] = it;

			while (*it && *it != '\n' && (it[0] != '/' || it[1] != '/'))
				++it;

			range[1] = it;

			while (range[1] > range[0] && range[1][-1] == ' ')
				--range[1];

			if (range[0] == range[1])
				throw ERR_SYNTAX;
		}
	};

	File::Parser::Parser(cstring begin,cstring const end)
	{
		char* output = new char [(end - begin) + 1];
		buffer = output, it = output;

		while (begin != end)
		{
			const int character = *begin++;

			if
			(
				character != '\t' &&
				character != '\v' &&
				character != '\b' &&
				character != '\f' &&
				character != '\a' &&
				character != '\r' &&
				character != '\0'
			)
				*output++ = (char) character;
		}

		*output = '\0';
	}

	ibool File::Match(cstring const type,cstring (&values)[2])
	{
		return String::Compare( type, values[0], values[1] - values[0] ) == 0;
	}

	void File::Skip(cstring& in,cstring const end)
	{
		cstring it = in;

		while (it < end && *it != ' ')
			++it;

		while (it < end && *it == ' ')
			++it;

		in = it;
	}

	ibool File::ParseFile(cstring const type,cstring (&values)[2][2],Context::Path& file)
	{
		if (Match( type, values[0] ))
		{
			if (values[1][1] - values[1][0])
			{
				file.Assign( values[1][0], values[1][1] - values[1][0] );
				return TRUE;
			}

			throw ERR_SYNTAX;
		}

		return FALSE;
	}

	ibool File::ParsePort(cstring const type,cstring (&values)[2][2],uint& controller)
	{
		if (Match( type, values[0] ))
		{
			for (uint i=0; i < NST_COUNT(controllerNames); ++i)
			{
				if (Match( controllerNames[i], values[1] ))
				{
					controller = i;
					return TRUE;
				}
			}

			throw ERR_SYNTAX;
		}

		return FALSE;
	}

	ibool File::ParseMode(cstring const type,cstring (&values)[2][2],uint& mode)
	{
		if (Match( type, values[0] ))
		{
			if (Match( "ntsc", values[1] ))
			{
				mode = Nes::Machine::NTSC;
				return TRUE;
			}
			else if (Match( "pal", values[1] ))
			{
				mode = Nes::Machine::PAL;
				return TRUE;
			}

			throw ERR_SYNTAX;
		}

		return FALSE;
	}

	ibool File::ParseGenie(cstring const type,cstring (&values)[2][2],Context::Cheats& cheats,const bool shortcut)
	{
		if (shortcut || Match( type, values[0] ))
		{
			cheats.push_back( Context::Cheat() );
			Context::Cheat& cheat = cheats.back();

			cstring it = values[1][0];
			cstring const end = values[1][1];

			{
				Nes::Cheats::Code code;

				if (NES_FAILED(Nes::Cheats::GameGenieDecode( it, code )))
					throw ERR_SYNTAX;

				cheat.address = code.address;
				cheat.value = code.value;
				cheat.compare = code.compare;
				cheat.useCompare = code.useCompare;
			}

			Skip( it, end );

			cheat.enabled =	!
			(
				(it[0] == 'O' || it[0] == 'o') &&
				(it[1] == 'F' || it[1] == 'f') &&
				(it[2] == 'F' || it[2] == 'f')
			);

			Skip( it, end );

			if (it < end)
				cheat.desc.Assign( it, end - it );

			return TRUE;
		}

		return FALSE;
	}

	ibool File::ParseCheat(cstring const type,cstring (&values)[2][2],Context::Cheats& cheats)
	{
		if (Match( type, values[0] ))
		{
			{
				const int first = values[1][0][0];

				if ((first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z'))
					return ParseGenie( type, values, cheats, true );
			}

			const String::Heap string( values[1][0], values[1][1] - values[1][0] );

			cheats.push_back( Context::Cheat() );
			Context::Cheat& cheat = cheats.back();

			char state[4];
			state[0] = '\0';

			char desc[256];
			desc[0] = '\0';

			int address=INT_MAX, value=INT_MAX, compare=INT_MAX;
			int count = std::sscanf( string, "%x %x %x %3s %255[^\0]", &address, &value, &compare, state, desc );

			if (count > 2 && compare >= 0x00 && compare <= 0xFF)
			{
				cheat.useCompare = true;
				cheat.compare = (u8) compare;
			}
			else if (count == 2)
			{
				cheat.useCompare = false;
				cheat.compare = 0x00;
				count = std::sscanf( string, "%*s %*s %3s %255[^\0]", state, desc );
			}
			else
			{
				throw ERR_SYNTAX;
			}
				
			if (address < 0x0000 || address > 0xFFFF || value < 0x00 || value > 0xFF)
				throw ERR_SYNTAX;

			cheat.address = (u16) address;
			cheat.value = (u8) value;
			
			cheat.enabled = 
			(
			   	(state[0] != 'o' && state[0] != 'O') || 
				(state[1] != 'f' && state[1] != 'F') || 
				(state[2] != 'f' && state[2] != 'F') || 
				(state[3] != '\0')
			);
		
			if (*desc)
			{
				cheat.desc = (cstring) desc;
				cheat.desc.Trim();
			}

			return TRUE;
		}

		return FALSE;
	}

	void File::Load(const Input input,Context& context) const
	{
		for (Parser parser( input, input + input.Size() ); parser.Check(); parser.NextLine())
		{
			if (parser.CheckType())
			{
				cstring values[2][2];
		
				parser.ReadType( values[0] );
				parser.ReadValue( values[1] );
		
				if 
				(
					!ParseCheat( "CHEAT ",  values, context.cheats         ) &&
					!ParseGenie( "GENIE ",  values, context.cheats         ) &&
					!ParsePort( "PORT1 ",   values, context.controllers[0] ) &&
					!ParsePort( "PORT2 ",   values, context.controllers[1] ) &&
					!ParsePort( "PORT3 ",   values, context.controllers[2] ) &&
					!ParsePort( "PORT4 ",   values, context.controllers[3] ) &&
					!ParsePort( "PORT5 ",   values, context.controllers[4] ) &&
					!ParseFile( "ROM ",     values, context.image          ) &&
					!ParseFile( "IMAGE ",   values, context.image          ) &&
					!ParseFile( "SAV ",     values, context.save           ) &&
					!ParseFile( "SAVE ",    values, context.save           ) &&
					!ParseMode( "MODE ",    values, context.mode           ) &&
					!ParseFile( "STATE ",   values, context.state          ) &&
					!ParseFile( "MOVIE ",   values, context.movie          ) &&
					!ParseFile( "IPS ",     values, context.ips            )
				)
					throw ERR_SYNTAX;
			}
		}
	}
}
