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

#ifdef PDX_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#ifndef PDXASSERT_H
#define PDXASSERT_H

#ifndef PDXLIBRARY_H
#error Do not include PdxAssert.h directly!
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Error, warning and question messages
//////////////////////////////////////////////////////////////////////////////////////////////

namespace PDX
{
	PDXRESULT PDX_STDCALL Error(const CHAR* const);

 #ifdef _WIN32

	VOID PDX_STDCALL Warning(const CHAR* const);
	BOOL PDX_STDCALL Ask(const CHAR* const,const CHAR* const);

 #endif
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Declaration of the assertion function used by the macros
//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
namespace PDX
{
	PDX_NO_INLINE INT PDX_STDCALL AssertMessage
	(
     	const CHAR* const,
		const CHAR* const,
		const CHAR* const,
		const CHAR* const,
		const INT
	);
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Break-into-the-debugger macros
//////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(PDX_HALT) && defined(PDX_X86)
#define PDX_HALT __asm {int 3}
#endif

#ifndef PDX_HALT
#define PDX_HALT exit(EXIT_FAILURE)
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Detect if the 'function string' is supported
//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER									 
#if _MSC_VER >= 1300							 
#define PDX_FUNCTION_MACRO_SUPPORTED
#endif											 
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Assertion macros
//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

 #ifdef _WIN32

   #ifdef PDX_FUNCTION_MACRO_SUPPORTED

    #define PDX_ASSERT_MSG(expr,msg)						     	              \
	{														       	              \
		static bool ignore = false;							       	              \
															     	              \
		if (!ignore && (!(expr)))												  \
		{																		  \
    		switch (PDX::AssertMessage(#expr,msg,__FILE__,__FUNCTION__,__LINE__)) \
	       	{																  	  \
	          	case 0: PDX_HALT; break;										  \
	          	case 1: ignore = true; break;									  \
          	}																	  \
       	}																		  \
   	}																			  \
   	PDX_FORCE_SEMICOLON
		
   #else

    #define PDX_ASSERT_MSG(expr,msg)							     	        \
	{														       	            \
		static bool ignore = false;							       	            \
															     	            \
		if (!ignore && (!(expr)))							       	            \
	    {																		\
     		switch (PDX::AssertMessage(#expr,msg,__FILE__,NULL,__LINE__))		\
			{																	\
		     	case 0: PDX_HALT; break;										\
		     	case 1: ignore = true; break;									\
	     	}																	\
     	}																		\
   	}																			\
   	PDX_FORCE_SEMICOLON
   
   #endif
				
    #define PDX_ASSERT(expr)              PDX_ASSERT_MSG(expr,NULL)						
    #define PDX_ALIGN_ASSERT(t,a)         PDX_ASSERT_MSG((t) >= a) && ((t) % a == 0),"value is not properly aligned!")
    #define PDX_ALIGN_ADDRESS_ASSERT(t,a) PDX_ASSERT_MSG(((ULONG)(t)) % a == 0,"address is not properly aligned!")

   #ifdef PDX_FUNCTION_MACRO_SUPPORTED

    #define PDX_BREAK_MSG(msg)							         	             \
	{												            	             \
		static bool ignore = false;					        		             \
											           				             \
		if (!ignore)											                 \
		{																	     \
     		switch (PDX::AssertMessage(NULL,msg,__FILE__,__FUNCTION__,__LINE__)) \
	     	{																     \
	          	case 0: PDX_HALT; break;									     \
	          	case 1: ignore = true; break;								     \
          	}																     \
       	}																	     \
   	}																		     \
   	PDX_FORCE_SEMICOLON 

   #else

    #define PDX_BREAK_MSG(msg)							         	           \
	{												            	           \
		static bool ignore = false;					        		           \
											           				           \
		if (!ignore)											               \
		{																	   \
     		switch (PDX::AssertMessage(NULL,msg,__FILE__,NULL,__LINE__))	   \
	     	{																   \
	          	case 0: PDX_HALT; break;									   \
	          	case 1: ignore = true; break;								   \
          	}																   \
       	}																	   \
   	}																		   \
   	PDX_FORCE_SEMICOLON

   #endif

#else

 #include <cassert>

  #define PDX_ASSERT_MSG(expr,msg) assert(expr && msg)
  #define PDX_BREAK_MSG(msg)	   assert(0 && msg)

 #endif

 #define PDX_BREAK PDX_BREAK_MSG(NULL)

#else						
								
 #define PDX_ASSERT(expr) PDX_FORCE_SEMICOLON
 #define PDX_ASSERT_MSG(expr,msg) PDX_FORCE_SEMICOLON
 #define PDX_ALIGN_ASSERT(type,align) PDX_FORCE_SEMICOLON
 #define PDX_ALIGN_ADDRESS_ASSERT(addr,align) PDX_FORCE_SEMICOLON
 #define PDX_BREAK_MSG(msg) PDX_HALT
 #define PDX_BREAK PDX_HALT

#endif

#ifdef _DEBUG
#define PDX_DEBUG_BREAK_MSG(msg) PDX_BREAK_MSG(msg)
#else
#define PDX_DEBUG_BREAK_MSG(msg) PDX_FORCE_SEMICOLON
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Compile-time assertion
//////////////////////////////////////////////////////////////////////////////////////////////

#define PDX_COMPILE_ASSERT(expr) \
typedef char PDX_PP_MERGE(pdx_compile_time_assertion_at_line_,__LINE__)[(expr) ? 1 : -1]

#endif
