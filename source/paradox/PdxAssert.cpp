//////////////////////////////////////////////////////////////////////////////////////////////
//
// Paradox Library - general purpose C++ utilities
//
// Copyright (C) 2003 Martin Freij
//
// This file is part of Paradox Library.
// 
// Paradox Library is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// Paradox Library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Paradox Library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32

#include <cstdio>

//////////////////////////////////////////////////////////////////////////////////////////////
// Anything to avoid needles windows stuff
//////////////////////////////////////////////////////////////////////////////////////////////

#undef  VC_EXTRALEAN
#define VC_EXTRALEAN
#undef  WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#undef  NOGDICAPMASKS     
#define NOGDICAPMASKS     
#undef  NOVIRTUALKEYCODES 
#define NOVIRTUALKEYCODES 
#undef  NOWINMESSAGES     
#define NOWINMESSAGES     
#undef  NOWINSTYLES       
#define NOWINSTYLES       
#undef  NOSYSMETRICS      
#define NOSYSMETRICS      
#undef  NOMENUS           
#define NOMENUS           
#undef  NOICONS           
#define NOICONS           
#undef  NOKEYSTATES       
#define NOKEYSTATES       
#undef  NOSYSCOMMANDS     
#define NOSYSCOMMANDS     
#undef  NORASTEROPS       
#define NORASTEROPS       
#undef  NOSHOWWINDOW      
#define NOSHOWWINDOW      
#undef  OEMRESOURCE       
#define OEMRESOURCE       
#undef  NOATOM            
#define NOATOM            
#undef  NOCLIPBOARD       
#define NOCLIPBOARD       
#undef  NOCOLOR           
#define NOCOLOR           
#undef  NOCTLMGR          
#define NOCTLMGR          
#undef  NODRAWTEXT        
#define NODRAWTEXT        
#undef  NOGDI             
#define NOGDI             
#undef  NOKERNEL          
#define NOKERNEL          
#undef  NONLS             
#define NONLS             
#undef  NOMEMMGR          
#define NOMEMMGR          
#undef  NOMETAFILE        
#define NOMETAFILE        
#undef  NOMINMAX          
#define NOMINMAX          
#undef  NOMSG             
#define NOMSG             
#undef  NOOPENFILE        
#define NOOPENFILE        
#undef  NOSCROLL          
#define NOSCROLL          
#undef  NOSERVICE         
#define NOSERVICE         
#undef  NOSOUND           
#define NOSOUND           
#undef  NOTEXTMETRIC      
#define NOTEXTMETRIC      
#undef  NOWH              
#define NOWH              
#undef  NOWINOFFSETS      
#define NOWINOFFSETS      
#undef  NOCOMM            
#define NOCOMM            
#undef  NOKANJI           
#define NOKANJI           
#undef  NOHELP            
#define NOHELP            
#undef  NOPROFILER        
#define NOPROFILER        
#undef  NODEFERWINDOWPOS  
#define NODEFERWINDOWPOS  
#undef  NOMCX             
#define NOMCX             

#include <Windows.h>
#include "PdxLibrary.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// The actual assertion function called by the macros (win32 version)
//////////////////////////////////////////////////////////////////////////////////////////////

namespace PDX
{
INT PDX_STDCALL AssertMessage(const CHAR* const expression,const CHAR* const msg,const CHAR* const file,const CHAR* const function,const INT line)
{
	const TSIZE length = 
	(
    	(msg ?        strlen(msg)        :  0) + 
		(expression ? strlen(expression) : 16) + 
		(file ?       strlen(file)       : 16) + 
		(function ?   strlen(function)   : 16) + 
		64 + 1
	);	

	CHAR* const buffer = new CHAR[length];

	if (!buffer)
	{
		MessageBox
		(
	     	NULL,
            #ifdef _DEBUG
			"Out of memory, application will now break into the debugger!","Paradox Assertion!",
            #else
			"Out of memory, application will now exit!","Paradox Assertion!",
            #endif
			MB_OK|MB_ICONERROR
		);
	
		return 0;
	}

	if (msg) 
	{
		sprintf
		(
	    	buffer,
			"%s, Expression: %s\n\n File: %s\n Function: %s\n Line: %i",
			msg,
			expression ? expression : "break point",
			file,
			function ? function : "unknown",
			line
		);  
	}
	else
	{
		sprintf
		(
	     	buffer,
			"Expression: %s\n\n File: %s\n Function: %s\n Line: %i",
			expression ? expression : "break point",
			file,
			function ? function : "unknown",
			line
		); 
	}

	const INT result = MessageBoxEx	         						 
	(																 
    	NULL,														 
		buffer,                                                       
		"Paradox Assertion!",										 
		MB_ABORTRETRYIGNORE,MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT) 
	);																 

	delete [] buffer;

    return result == IDABORT ? 0 : (result == IDIGNORE ? 1 : 2);
}
}

#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// error/warning/question messages
//////////////////////////////////////////////////////////////////////////////////////////////

namespace PDX
{
	PDXRESULT PDX_STDCALL Error(const CHAR* const text)
	{
        #ifdef _WIN32
		MessageBox(NULL,text,"Error!",MB_OK|MB_ICONERROR);
        #else
		assert(0 && text);
        #endif
		return PDX_FAILURE;
	}

 #ifdef _WIN32

	VOID PDX_STDCALL Warning(const CHAR* const text)
	{
		MessageBox(NULL,text,"Warning!",MB_OK|MB_ICONWARNING);
	}

	BOOL PDX_STDCALL Ask(const CHAR* const headline,const CHAR* const text)
	{
		return MessageBox(NULL,text,headline,MB_YESNO|MB_ICONQUESTION) == IDYES;
	}

 #endif
}




