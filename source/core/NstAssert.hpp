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

#ifndef NST_TYPES_H
#error Do not include NstAssert.hpp directly!
#endif
		 
#ifndef NDEBUG

 #ifdef _WIN32

  #ifndef NST_HALT
   #ifdef _MSC_VER
    #if (_MSC_VER >= 1300)
     #define NST_HALT __debugbreak()
    #else
     #define NST_HALT __asm {int 3} NST_NOP
    #endif
   #else
    #include <cstdlib>
    #define NST_HALT std::abort()
   #endif
  #endif

  #ifndef NST_FUNCTION_NAME
   #if defined(_MSC_VER) && (_MSC_VER >= 1300)
    #define NST_FUNCTION_NAME __FUNCTION__
   #else
    #define NST_FUNCTION_NAME 0
   #endif
  #endif

  namespace Nes	
  {
	  namespace Assertion
	  {
		  NST_NO_INLINE uint NST_CALL Issue
		  (
			  cstring,
			  cstring,
			  cstring,
			  cstring,
			  int
		  );
	  }
  }

  #define NST_DEBUG_MSG(msg_)           										          \
  {																				          \
	  static bool ignore_ = false;										         	      \
																			         	  \
	  if (!ignore_)         											          	      \
	  {																		      	      \
    	  switch (Nes::Assertion::Issue(0,msg_,__FILE__,NST_FUNCTION_NAME,__LINE__))      \
		  {												       	       					  \
    		  case 0: NST_HALT; break;				     					     	      \
			  case 1: ignore_ = true; break;							               	  \
		  }																			      \
	  }																				      \
  }																					      \
  NST_NOP

  #define NST_ASSERT_MSG(expr_,msg_)											          \
  {																				          \
	  static bool ignore_ = false;										         	      \
																			         	  \
	  if (!ignore_ && !(expr_))											          	      \
	  {																		      	      \
    	  switch (Nes::Assertion::Issue(#expr_,msg_,__FILE__,NST_FUNCTION_NAME,__LINE__)) \
		  {												       	       					  \
    		  case 0: NST_HALT; break;				     					     	      \
			  case 1: ignore_ = true; break;							               	  \
		  }																			      \
	  }																				      \
  }																					      \
  NST_NOP

  #define NST_VERIFY_MSG(expr_,msg_) NST_ASSERT_MSG(expr_,msg_)

 #else

  #include <cassert>
  #define NST_DEBUG_MSG(msg_) NST_NOP
  #define NST_ASSERT_MSG(expr_,msg_) assert( expr_ )
  #define NST_VERIFY_MSG(expr_,msg_) NST_NOP

 #endif

#else						
								
 #define NST_DEBUG_MSG(msg_) NST_NOP
 #define NST_ASSERT_MSG(expr_,msg_) NST_ASSUME(expr_)
 #define NST_VERIFY_MSG(expr_,msg_) NST_NOP

#endif

#define NST_ASSERT(expr_) NST_ASSERT_MSG(expr_,0)
#define NST_VERIFY(expr_) NST_VERIFY_MSG(expr_,0)

#define NST_COMPILE_ASSERT(expr_) typedef char Nestopia_compile_time_assertion_at_line_##__LINE__[(expr_) ? 1 : -1]
