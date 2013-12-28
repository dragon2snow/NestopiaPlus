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
#include "NstIoLog.hpp"
#include "NstApplicationException.hpp"
#include "NstApplicationInstance.hpp"

#ifdef __INTEL_COMPILER
#pragma warning( disable : 111 279 )
#endif

namespace Nestopia
{
	using Application::Configuration;

	const String::Heap Configuration::nullString;

	void Configuration::Value::YesNoProxy::operator = (ibool i)
	{
		string.Assign( i ? "yes" : "no", i ? 3 : 2 );
	}

	void Configuration::Value::OnOffProxy::operator = (ibool i)
	{
		string.Assign( i ? "on" : "off", i ? 2 : 3 );
	}

	void Configuration::Value::QuoteProxy::operator = (const String::Anything& input)
	{
		string.Reserve( 2 + input.Size() );
		string = '\"';
		string << input << '\"';
	}

	ibool Configuration::ConstValue::operator == (State state) const
	{
		NST_ASSERT( state < 4 );

		static const char yesNoOnOff[4][4] = 
		{
			"yes","no","on","off"
		};

		return string == (cstring) yesNoOnOff[state];	
	}

	Configuration::Configuration(const String::Generic cmdLine)
	: save(FALSE)
	{
		items.Reserve( HINTED_SIZE );

		try
		{
			String::Heap buffer;

			try
			{
				Io::File( Instance::GetPath("nestopia.cfg"), Io::File::COLLECT ).Text() >> buffer;
			}
			catch (Io::File::Exception)
			{
				buffer.Clear();
			}

			if (buffer.Size())
			{
				try
				{
					Parse( buffer, buffer.Size() );
				}
				catch (Exception)
				{
					Reset( FALSE );
					Application::Exception( IDS_CFG_WARN_CORRUPT, Application::Exception::WARNING ).Issue();
				}
			}

			if (cmdLine.Size())
			{
				try
				{
					Parse( cmdLine, cmdLine.Size(), &startupFile );
				}
				catch (Exception)
				{
					Application::Exception( IDS_CMDLINE_ERR, Application::Exception::WARNING ).Issue();
				}
			}
		}
		catch (...)
		{
			Reset( FALSE );
			Application::Exception( "Configuration::Load() error!", Application::Exception::UNSTABLE ).Issue();
		}
	}

	Configuration::~Configuration()
	{
		if (save)
		{
			try
			{
				const Io::File file( Instance::GetPath("nestopia.cfg"), Io::File::DUMP );

				static const char header1[] =
				(
					"/////////////////////////////////////////////////////////////////////////////\r\n"
					"//\r\n"
					"// Nestopia Configuration File. Version "
				);

				static const char header2[] =
				(
					"\r\n"
					"//\r\n"
					"/////////////////////////////////////////////////////////////////////////////\r\n"
					"\r\n"
				);

				file.Text() << header1 << Instance::GetVersion() << header2;

				for (Items::ConstIterator it(items.Begin()); it != items.End(); ++it)
					file.Text() << '-' << it->key << " : " << it->value << "\r\n";
			}
			catch (Io::File::Exception)
			{
				// I/O failure
			}
		}

		Reset( FALSE );
	}

	void Configuration::Reset(const ibool notify)
	{
		for (Items::Iterator it(items.Begin()); it != items.End(); ++it)
		{
			if (notify && !it->key.referenced)
				Io::Log() << "Configuration: warning, invalid/unused parameter: \"" << it->key << "\"\r\n";

			it->key.Command::~Command();
			it->value.Heap::~Heap();
		}

		items.Destroy();
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("t", on)
    #endif

	void Configuration::Parse(cstring string,uint length,String::Heap* file)
	{
		class CommandLine
		{
			const String::Stream<false> stream;
			cstring it;

			void Skip(int ch)
			{
				while (*it == ch)
					++it;
			}

			void Parse(cstring (&range)[2],int breaker)
			{
				for (range[0] = it; *it != breaker; ++it)
					if (!*it) throw ERR_PARSING;

				for (range[1] = it++; range[1][-1] == ' '; --range[1]);
			}

			ibool ParseQuoted(cstring (&range)[2])
			{
				if (*it == '\"') 
				{
					++it;
					Skip(' ');
					Parse( range, '\"' );

					return TRUE;
				}

				return FALSE;
			}

			void Parse(cstring (&range)[2])
			{
				if (!ParseQuoted( range ))
				{
					range[0] = it;

					while (*it && *it != '\r' && *it != '-' && (it[0] != '/' || it[1] != '/'))
					{
						if (*it++ == '\"') 
						{
							Skip(' ');

							while (*it != '\"')
								if (!*it++) throw ERR_PARSING;

							range[1] = ++it;
							return;
						}
					}

					range[1] = it;

					while (range[1] != range[0] && range[1][-1] == ' ')
						--range[1];
				}
			}

		public:

			CommandLine(cstring string,uint length,String::Heap* file)
			: stream(string,length), it(stream) 
			{
				if (file)
				{
					Skip(' ');

					cstring range[2];

					if (ParseQuoted( range ) && range[0] != range[1])
						file->Assign( range[0], range[1] - range[0] );
				}
			}

			void operator >> (Items& items)
			{
				NST_ASSERT( *(it-1) == '-' );

				cstring ranges[2][2];

				Skip(' ');
				Parse( ranges[0], ':' );
				Skip(' ');
				Parse( ranges[1] );

				if (ranges[0][0] == ranges[0][1])
					throw ERR_PARSING;

				if (ranges[1][0] != ranges[1][1])
					items( stream(ranges[0][0],ranges[0][1]) ).Assign( ranges[1][0], ranges[1][1] - ranges[1][0] );
			}

			operator ibool ()
			{
				for (;;)
				{
					if (*it == '\r')
					{
						it += 2;
					}
					else if (it[0] == '/' && it[1] == '/')
					{
						for (it += 2; *it && *it != '\r'; ++it);
					}
					else if (*it == ' ')
					{
						++it;
					}
					else
					{
						break;
					}
				}

				if (*it == '-')		
				{
					++it;
					return TRUE;
				}

				if (*it)
					throw ERR_PARSING;

				return FALSE;
			}
		};

		for (CommandLine stream(string,length,file); stream; stream >> items);
	}

	Configuration::Value Configuration::operator [] (cstring const name)
	{
		return items( name );
	}

	Configuration::ConstValue Configuration::operator [] (cstring const name) const throw()
	{
		if (const Items::Entry* entry = items.Find( name ))
		{
			entry->key.referenced = TRUE;
			return entry->value;
		}

		return nullString;
	}

    #ifdef NST_PRAGMA_OPTIMIZE										   
    #pragma optimize("", on)
    #endif
}
