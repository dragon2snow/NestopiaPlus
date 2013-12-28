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
#include "NstIoNsp.hpp"
#include "NstApplicationInstance.hpp"
#include "../core/api/NstApiCheats.hpp"
#include "../core/api/NstApiMachine.hpp"

namespace Nestopia
{
	NST_COMPILE_ASSERT
	(
		Nes::Input::UNCONNECTED	      ==  0 &&
		Nes::Input::PAD1		      ==  1 &&
		Nes::Input::PAD2		      ==  2 &&
		Nes::Input::PAD3		      ==  3 &&
		Nes::Input::PAD4		      ==  4 &&
		Nes::Input::ZAPPER		      ==  5 &&
		Nes::Input::PADDLE		      ==  6 &&
		Nes::Input::POWERPAD	      ==  7 &&
		Nes::Input::MOUSE	          ==  8 &&
		Nes::Input::FAMILYTRAINER     ==  9 &&
		Nes::Input::FAMILYKEYBOARD    == 10 &&
		Nes::Input::SUBORKEYBOARD     == 11 &&
		Nes::Input::DOREMIKKOKEYBOARD == 12 &&
		Nes::Input::OEKAKIDSTABLET    == 13 &&
		Nes::Input::HYPERSHOT	      == 14 &&
		Nes::Input::CRAZYCLIMBER      == 15 &&
		Nes::Input::MAHJONG		      == 16 &&
		Nes::Input::EXCITINGBOXING    == 17 &&
		Nes::Input::TOPRIDER	      == 18 &&
		Nes::Input::POKKUNMOGURAA     == 19 &&
		Nes::Input::PARTYTAP          == 20 &&
		Nes::Input::NUM_CONTROLLERS   == 20
	);

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
		tape.Clear();
		palette.Clear();
		
		for (uint i=0; i < NUM_CONTROLLER_PORTS; ++i)
			controllers[i] = UNKNOWN;

		mode = UNKNOWN;

		cheats.clear();
	}

	using Io::Nsp::File;

	tstring const File::controllerNames[] =
	{
		_T( "unconnected"		),
		_T( "pad1"				),
		_T( "pad2"				),
		_T( "pad3"				),
		_T( "pad4"				),
		_T( "zapper"			),
		_T( "paddle"			),
		_T( "powerpad"			),
		_T( "mouse"				),
		_T( "familytrainer" 	),
		_T( "familykeyboard"	),
		_T( "suborkeyboard"	    ),
		_T( "doremikkokeyboard" ),
		_T( "oekakidstablet"	),
		_T( "hypershot"			),
		_T( "crazyclimber"		),
		_T( "mahjong"			),
		_T( "excitingboxing"	),
		_T( "toprider"			),
		_T( "pokkunmoguraa"		),
		_T( "partytap"		    )
	};

	void File::Save(Output& output,const Context& context) const
	{
		NST_COMPILE_ASSERT( Context::NUM_CONTROLLER_PORTS == 5 && Context::NUM_CONTROLLERS == 20 );

		output << "//\r\n"
       			  "// Generated Nestopia Script File. Version " << Application::Instance::GetVersion() << "\r\n"
				  "//\r\n"
				  "// Syntax:\r\n"
				  "//\r\n"
				  "//  -ROM / -IMAGE <filename>\r\n"
				  "//  -SAVE <filename>\r\n"
				  "//  -TAPE <filename>\r\n"
				  "//  -STATE <filename>\r\n"
				  "//  -MOVIE <filename>\r\n"
				  "//  -IPS <filename>\r\n"
				  "//  -PALETTE <filename>\r\n"
				  "//  -MODE <ntsc/pal>\r\n"
				  "//  -PORT1..PORT5 <unconnected,pad1,pad2,pad3,pad4,zapper,paddle,powerpad,mouse,\r\n"
				  "//                 familytrainer,familykeyboard,suborkeyboard,doremikkokeyboard,\r\n"
				  "//                 oekakidstablet,hypershot,crazyclimber,mahjong,excitingboxing,\r\n"
				  "//                 toprider,pokkunmoguraa,partytap>\r\n"
				  "//\r\n"
				  "//  -GENIE <code> <on,off> (description)\r\n"
				  "//  -CHEAT <address> <value> (compare value) <on,off> (description)\r\n"
				  "//\r\n"
				  "// The values in parenthesis are optional.\r\n"
				  "// Only one argument per line is allowed.\r\n"
				  "//\r\n"
				  "// Example:\r\n"
				  "//\r\n"
				  "//  -IMAGE C:\\games\\monkey island 3 (a).nes // C style comment\r\n"
				  "//  -MODE ntsc\r\n"
				  "//  -PORT1 pad1\r\n"
				  "//  -PORT2 unconnected\r\n"
				  "//  -GENIE SXIOPO on infinite plumbers\r\n"
				  "//\r\n\r\n";
				  
		if (context.image.Length())
			output << "-IMAGE " << context.image << "\r\n";

		if (context.ips.Length())
			output << "-IPS " << context.ips << "\r\n";

		if (context.save.Length())
			output << "-SAVE " << context.save << "\r\n";

		if (context.movie.Length())
			output << "-MOVIE " << context.movie << "\r\n";

		if (context.tape.Length())
			output << "-TAPE " << context.tape << "\r\n";

		if (context.palette.Length())
			output << "-PALETTE " << context.palette << "\r\n";

		if (context.mode != Context::UNKNOWN)
			output << "-MODE " << (context.mode == Nes::Machine::PAL ? "pal\r\n" : "ntsc\r\n");

		for (uint i=0; i < Context::NUM_CONTROLLER_PORTS; ++i)
		{
			if (context.controllers[i] != Context::UNKNOWN)
				output << "-PORT" << (i+1) << ' ' << controllerNames[context.controllers[i]] << "\r\n";
		}

		for (Context::Cheats::const_iterator it(context.cheats.begin()), end(context.cheats.end()); it != end; ++it)
		{
			output << "-CHEAT " 
				   << HexString( (u16) it->address )
				   << ' '
				   << HexString( (u8) it->value );

			if (it->useCompare)
				output << ' ' << HexString( (u8) it->compare );
				   
		    output << (it->enabled ? " on" : " off");

			if (it->desc.Length())
			{
				output << ' ' << it->desc;
			}
			else if (it->address >= 0x8000U)
			{
				char characters[9];

				if (NES_SUCCEEDED(Nes::Cheats::GameGenieEncode( Nes::Cheats::Code(it->address,it->value,it->compare,it->useCompare), characters )))
					output << " (" << characters << ')';
			}

			output << "\r\n";
		}
	}

	class File::Parser
	{
	public:

		Parser(tstring,tstring const);

	private:

		void SkipSpace()
		{
			while (*it && *it == ' ')
				++it;
		}

		tstring it;
		tstring buffer;

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

		void ReadType(tstring (&range)[2])
		{
			SkipSpace();

			range[0] = it;

			while (*it && *it != ' ')
				++it;

			range[1] = it;

			if (range[0] == range[1])
				throw ERR_SYNTAX;
		}

		void ReadValue(tstring (&range)[2])
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

	File::Parser::Parser(tstring begin,tstring const end)
	{
		tchar* output = new tchar [(end - begin) + 1];
		buffer = output, it = output;

		while (begin != end)
		{
			const uint c = (uint) *begin++;

			if (c > 31 || c == '\n')
				*output++ = (tchar) c;
		}

		*output = '\0';
	}

	ibool File::Match(tstring const type,tstring (&values)[2])
	{
		return ::_tcsnicmp( type, values[0], values[1] - values[0] ) == 0;
	}

	void File::Skip(tstring& in,tstring const end)
	{
		tstring it = in;

		while (it < end && *it != ' ')
			++it;

		while (it < end && *it == ' ')
			++it;

		in = it;
	}

	ibool File::ParseFile(tstring const type,tstring (&values)[2][2],Path& file)
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

	ibool File::ParsePort(tstring const type,tstring (&values)[2][2],uint& controller)
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

	ibool File::ParseMode(tstring const type,tstring (&values)[2][2],uint& mode)
	{
		if (Match( type, values[0] ))
		{
			if (Match( _T("ntsc"), values[1] ))
			{
				mode = Nes::Machine::NTSC;
				return TRUE;
			}
			else if (Match( _T("pal"), values[1] ))
			{
				mode = Nes::Machine::PAL;
				return TRUE;
			}

			throw ERR_SYNTAX;
		}

		return FALSE;
	}

	ibool File::ParseGenie(tstring const type,tstring (&values)[2][2],Context::Cheats& cheats,const bool shortcut)
	{
		if (shortcut || Match( type, values[0] ))
		{
			cheats.push_back( Context::Cheat() );
			Context::Cheat& cheat = cheats.back();

			tstring it = values[1][0];
			tstring const end = values[1][1];

			{
				Nes::Cheats::Code code;

				if (NES_FAILED(Nes::Cheats::GameGenieDecode( String::Stack<8,char>(it,NST_MIN(8,end-it)).Ptr(), code )))
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

	ibool File::ParseCheat(tstring const type,tstring (&values)[2][2],Context::Cheats& cheats)
	{
		if (Match( type, values[0] ))
		{
			{
				const int first = values[1][0][0];

				if ((first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z'))
					return ParseGenie( type, values, cheats, true );
			}

			const HeapString string( values[1][0], values[1][1] - values[1][0] );

			cheats.push_back( Context::Cheat() );
			Context::Cheat& cheat = cheats.back();

			String::Stack<3,tchar> state;
			String::Stack<255,tchar> desc;

			int address=INT_MAX, value=INT_MAX, compare=INT_MAX;
			int count = ::_stscanf( string.Ptr(), _T("%x %x %x %3s %255[^\0]"), &address, &value, &compare, state.Ptr(), desc.Ptr() );

			if (count > 2 && compare >= 0x00 && compare <= 0xFF)
			{
				cheat.useCompare = true;
				cheat.compare = compare;
			}
			else if (count == 2)
			{
				cheat.useCompare = false;
				cheat.compare = 0x00;
				count = ::_stscanf( string.Ptr(), _T("%*s %*s %3s %255[^\0]"), state.Ptr(), desc.Ptr() );
			}
			else
			{
				throw ERR_SYNTAX;
			}
				
			if (address < 0x0000 || address > 0xFFFF || value < 0x00 || value > 0xFF)
				throw ERR_SYNTAX;

			cheat.address = address;
			cheat.value = value;
			
			state.Validate();
			cheat.enabled = (state != _T("off"));
		
			desc.Validate();
			cheat.desc = desc;
			cheat.desc.Trim();

			return TRUE;
		}

		return FALSE;
	}

	void File::Load(const Input input,Context& context) const
	{
		HeapString buffer;
		Nestopia::Io::File::ParseText( input.Ptr(), input.Length(), buffer );

		for (Parser parser( buffer.Ptr(), buffer.Ptr() + buffer.Length() ); parser.Check(); parser.NextLine())
		{
			if (parser.CheckType())
			{
				tstring values[2][2];
		
				parser.ReadType( values[0] );
				parser.ReadValue( values[1] );
		
				if 
				(
					!ParseCheat( _T( "CHEAT "   ), values, context.cheats         ) &&
					!ParseGenie( _T( "GENIE "   ), values, context.cheats         ) &&
					!ParsePort(  _T( "PORT1 "   ), values, context.controllers[0] ) &&
					!ParsePort(  _T( "PORT2 "   ), values, context.controllers[1] ) &&
					!ParsePort(  _T( "PORT3 "   ), values, context.controllers[2] ) &&
					!ParsePort(  _T( "PORT4 "   ), values, context.controllers[3] ) &&
					!ParsePort(  _T( "PORT5 "   ), values, context.controllers[4] ) &&
					!ParseFile(  _T( "ROM "     ), values, context.image          ) &&
					!ParseFile(  _T( "IMAGE "   ), values, context.image          ) &&
					!ParseFile(  _T( "SAV "     ), values, context.save           ) &&
					!ParseFile(  _T( "SAVE "    ), values, context.save           ) &&
					!ParseMode(  _T( "MODE "    ), values, context.mode           ) &&
					!ParseFile(  _T( "STATE "   ), values, context.state          ) &&
					!ParseFile(  _T( "MOVIE "   ), values, context.movie          ) &&
					!ParseFile(  _T( "IPS "     ), values, context.ips            ) &&
					!ParseFile(  _T( "TAPE "    ), values, context.tape           ) &&
					!ParseFile(  _T( "PALETTE " ), values, context.palette        ) 
				)
					throw ERR_SYNTAX;
			}
		}
	}
}
