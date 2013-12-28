////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2007 Martin Freij
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
#include "NstWindowUser.hpp"
#include "NstApplicationInstance.hpp"

namespace Nestopia
{
	namespace Application
	{
		void Configuration::Value::YesNoProxy::operator = (bool i)
		{
			string.Assign( i ? "yes" : "no", i ? 3 : 2 );
		}

		void Configuration::Value::OnOffProxy::operator = (bool i)
		{
			string.Assign( i ? "on" : "off", i ? 2 : 3 );
		}

		void Configuration::Value::QuoteProxy::operator = (const GenericString& input)
		{
			string.Clear();
			string.Reserve( 2 + input.Length() );
			string << '\"' << input << '\"';
		}

		bool Configuration::ConstValue::operator == (State state) const
		{
			NST_ASSERT( state < 4 );

			static const tchar yesNoOnOff[4][4] =
			{
				_T("yes"), _T("no"), _T("on"), _T("off")
			};

			return GenericString::operator == (yesNoOnOff[state]);
		}

		Configuration::Configuration()
		: save(false)
		{
			items.Reserve( HINTED_SIZE );

			try
			{
				{
					HeapString buffer;

					try
					{
						Io::File( Instance::GetExePath(_T("nestopia.cfg")), Io::File::COLLECT ).ReadText( buffer );
					}
					catch (Io::File::Exception id)
					{
						if (id != Io::File::ERR_NOT_FOUND)
							Window::User::Warn( _T("Configuration file load error! Default settings will be used!") );

						buffer.Clear();
					}

					if (buffer.Length())
					{
						try
						{
							Parse( buffer.Ptr(), buffer.Length() );
						}
						catch (Error)
						{
							Reset( false );
							Window::User::Warn( _T("Corrupt configuration file! Default settings will be used!") );
						}
					}
				}

				if (tstring ptr = ::GetCommandLine())
				{
					for (uint quote=0; (*ptr > ' ') || (*ptr && quote); ++ptr)
					{
						if (*ptr == '\"')
							quote ^= 1;
					}

					while (*ptr && *ptr <= ' ')
						++ptr;

					if (*ptr)
					{
						if (*ptr != '-')
						{
							tstring const offset = ptr;

							for (uint quote=0; (*ptr > ' ') || (*ptr && quote); ++ptr)
							{
								if (*ptr == '\"')
									quote ^= 1;
							}

							startupFile.Assign( offset, ptr - offset );
							startupFile.Remove( '\"' );
							startupFile.Trim();

							// Win98/ME/2k fix
							if (startupFile.Length())
								startupFile = Instance::GetLongPath( startupFile.Ptr() );
						}

						if (const uint length = _tcslen(ptr))
						{
							try
							{
								Parse( ptr, length );
							}
							catch (Error)
							{
								Window::User::Warn( _T("Command line parser error!") );
							}
						}
					}
				}
			}
			catch (...)
			{
				Reset( false );
				Window::User::Fail( _T("Generic configuration loading error!") );
			}
		}

		Configuration::~Configuration()
		{
			if (save)
			{
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

				HeapString buffer;
				buffer << header1 << Instance::GetVersion() << header2;

				for (Items::ConstIterator it(items.Begin()), end(items.End()); it != end; ++it)
					buffer << '-' << it->key << " : " << it->value << "\r\n";

				try
				{
					Io::File( Instance::GetExePath(_T("nestopia.cfg")), Io::File::DUMP|Io::File::WRITE_THROUGH ).WriteText( buffer.Ptr(), buffer.Length() );
				}
				catch (Io::File::Exception)
				{
					Window::User::Warn( _T("Couldn't save the configuration!") );
				}
			}

			Reset( false );
		}

		void Configuration::Reset(const bool notify)
		{
			for (Items::Iterator it(items.Begin()), end(items.End()); it != end; ++it)
			{
				if (notify && !it->key.referenced)
					Io::Log() << "Configuration: warning, unused/invalid parameter: \"" << it->key << "\"\r\n";

				it->key.Command::~Command();
				it->value.HeapString::~HeapString();
			}

			items.Destroy();
		}

		void Configuration::Parse(tstring string,uint length)
		{
			class CommandLine
			{
				class Stream : public HeapString
				{
				public:

					Stream(const tchar* NST_RESTRICT src,const uint length)
					{
						if (length)
						{
							Reserve( length * 2 );

							Type* dst = Ptr();
							const Type* const end = src + length;

							do
							{
								const Type ch = *src++;

								if (ch > 31)
								{
									*dst++ = ch;
								}
								else if (ch == '\n')
								{
									*dst++ = '\r';
									*dst++ = '\n';
								}
								else if (ch == '\t')
								{
									*dst++ = ' ';
								}
							}
							while (src != end);

							ShrinkTo( dst - Ptr() );
						}
					}

					const String::Heap<char> operator () (tstring begin,tstring end) const
					{
						return String::Heap<Type>::operator () (begin-Ptr(),end-begin);
					}
				};

				const Stream stream;
				tstring it;

				void Skip(int ch)
				{
					while (*it == ch)
						++it;
				}

				void Parse(tstring (&range)[2],int breaker)
				{
					for (range[0] = it; *it != breaker; ++it)
						if (!*it) throw ERR_PARSING;

					for (range[1] = it++; range[1][-1] == ' '; --range[1]);
				}

				bool ParseQuoted(tstring (&range)[2])
				{
					if (*it == '\"')
					{
						++it;
						Skip(' ');
						Parse( range, '\"' );

						return true;
					}

					return false;
				}

				void Parse(tstring (&range)[2])
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

				CommandLine(tstring string,uint length)
				: stream(string,length), it(stream.Ptr()) {}

				void operator >> (Items& items)
				{
					NST_ASSERT( *(it-1) == '-' );

					tstring ranges[2][2];

					Skip(' ');
					Parse( ranges[0], ':' );
					Skip(' ');
					Parse( ranges[1] );

					if (ranges[0][0] == ranges[0][1])
						throw ERR_PARSING;

					if (ranges[1][0] != ranges[1][1])
						items( stream(ranges[0][0],ranges[0][1]) ).Assign( ranges[1][0], ranges[1][1] - ranges[1][0] );
				}

				operator bool ()
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
						return true;
					}

					if (*it)
						throw ERR_PARSING;

					return false;
				}
			};

			for (CommandLine stream(string,length); stream; stream >> items);
		}

		Configuration::Value Configuration::operator [] (const String::Generic<char> name)
		{
			return items( name );
		}

		const Configuration::ConstValue Configuration::operator [] (const String::Generic<char> name) const
		{
			GenericString match;

			if (const Items::Entry* entry = items.Find( name ))
			{
				entry->key.referenced = true;
				match = entry->value;
			}

			return match;
		}
	}
}
