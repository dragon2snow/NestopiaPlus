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

#ifndef NST_API_USER_H
#define NST_API_USER_H

#include <vector>
#include <string>
#include "NstApi.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#if NST_ICC >= 810
#pragma warning( push )
#pragma warning( disable : 304 444 )
#elif NST_MSVC >= 1200
#pragma warning( push )
#pragma warning( disable : 4512 )
#endif

namespace Nes
{
	namespace Api
	{
		class User : public Base
		{
			struct LogCaller;
			struct EventCaller;
			struct InputCaller;
			struct QuestionCaller;
			struct FileIoCaller;

		public:

			template<typename T>
			User(T& e)
			: Base(e) {}

			enum Question
			{
				QUESTION_NST_PRG_CRC_FAIL_CONTINUE = 1,
				QUESTION_NSV_PRG_CRC_FAIL_CONTINUE
			};

			enum Answer
			{
				ANSWER_NO,
				ANSWER_YES,
				ANSWER_DEFAULT
			};

			enum Event
			{
				EVENT_CPU_JAM = 1,
				EVENT_DISPLAY_TIMER,
				EVENT_TAPE_PLAYING,
				EVENT_TAPE_RECORDING,
				EVENT_TAPE_STOPPED,
				EVENT_NONSTANDARD_DISK
			};

			enum File
			{
				FILE_LOAD_BATTERY = 1,
				FILE_SAVE_BATTERY,
				FILE_SAVE_FDS,
				FILE_LOAD_EEPROM,
				FILE_SAVE_EEPROM,
				FILE_LOAD_TAPE,
				FILE_SAVE_TAPE,
				FILE_LOAD_TURBOFILE,
				FILE_SAVE_TURBOFILE
			};

			enum Input
			{
				INPUT_CHOOSE_MAPPER = 1
			};

			typedef std::string String;
			typedef std::vector<uchar> FileData;

			typedef void   ( NST_CALLBACK *LogCallback      ) (UserData,const char*,ulong);
			typedef void   ( NST_CALLBACK *EventCallback    ) (UserData,Event,const void*);
			typedef void   ( NST_CALLBACK *InputCallback    ) (UserData,Input,const char*,String&);
			typedef Answer ( NST_CALLBACK *QuestionCallback ) (UserData,Question);
			typedef void   ( NST_CALLBACK *FileIoCallback   ) (UserData,File,FileData&);

			static LogCaller logCallback;
			static EventCaller eventCallback;
			static InputCaller inputCallback;
			static QuestionCaller questionCallback;
			static FileIoCaller fileIoCallback;
		};

		struct User::LogCaller : Core::UserCallback<User::LogCallback>
		{
			void operator () (const char* text,ulong length) const
			{
				if (function)
					function( userdata, text, length );
			}

			template<ulong N>
			void operator () (const char (&c)[N]) const
			{
				(*this)( c, N-1 );
			}
		};

		struct User::EventCaller : Core::UserCallback<User::EventCallback>
		{
			void operator () (Event event,const void* data=0) const
			{
				if (function)
					function( userdata, event, data );
			}
		};

		struct User::InputCaller : Core::UserCallback<User::InputCallback>
		{
			void operator () (Input what,const char* info,String& answer) const
			{
				if (function)
					function( userdata, what, info, answer );
			}
		};

		struct User::QuestionCaller : Core::UserCallback<User::QuestionCallback>
		{
			Answer operator () (Question question) const
			{
				return function ? function( userdata, question ) : ANSWER_DEFAULT;
			}
		};

		struct User::FileIoCaller : Core::UserCallback<User::FileIoCallback>
		{
			void operator () (File file,FileData& data) const
			{
				if (function)
					function( userdata, file, data );
			}
		};
	}
}

#if NST_MSVC >= 1200 || NST_ICC >= 810
#pragma warning( pop )
#endif

#endif
