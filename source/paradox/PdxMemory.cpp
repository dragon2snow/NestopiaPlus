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

#include <cstdlib>

#ifdef _WIN32

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

#endif

#include "PdxMemory.h"

#define PDX_DEAD_END_ 0x0BADBABEUL

//////////////////////////////////////////////////////////////////////////////////////////////
// msvc _aligned_malloc() extension
//////////////////////////////////////////////////////////////////////////////////////////////

#if defined(__INTEL_COMPILER)
#undef PDX_ALIGNED_MALLOC_SUPPORT
#define PDX_ALIGNED_MALLOC_SUPPORT
#else
#ifdef _MSC_VER
#if _MSC_VER >= 1200
#undef PDX_ALIGNED_MALLOC_SUPPORT
#define PDX_ALIGNED_MALLOC_SUPPORT
#endif
#endif
#endif

#ifdef PDX_ALIGNED_MALLOC_SUPPORT
#include <malloc.h>
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// General allocator routine
//////////////////////////////////////////////////////////////////////////////////////////////

namespace PDX
{

VOID* PDX_STDCALL Allocate(const TSIZE size)
{
	if (!size) return NULL;

   #ifdef PDX_MEMORY_DEBUG    		

    #ifdef PDX_ALIGNED_MALLOC_SUPPORT
	VOID* const ptr = _aligned_malloc(PDX::MEMORYDEBUGGER::RequiredSize(size),PDX_DEFAULT_ALIGNMENT);	
    #else
	VOID* const ptr = (VOID*) new CHAR[PDX::MEMORYDEBUGGER::RequiredSize(size)];
    #endif

	PDX_MEMORY_DEBUG_ASSERT(ptr,"Failed to allocate memory!");	

   #else

    #ifdef PDX_ALIGNED_MALLOC_SUPPORT
	VOID* const ptr = _aligned_malloc(size,PDX_DEFAULT_ALIGNMENT);
    #else
	VOID* const ptr = (VOID*) new CHAR[size];
    #endif

   #endif

	if (!ptr)
	{
        #ifdef _WIN32
		MessageBox(NULL,"Out of memory!","Error!",MB_OK|MB_ICONERROR);	
        #endif		
        #ifdef _DEBUG
		PDX_HALT;
        #else
		exit(EXIT_FAILURE);
        #endif
	}

    #ifdef PDX_MEMORY_DEBUG    		
	return PDX::MEMORYDEBUGGER::Initialize(ptr,size);
	#else
	return ptr;
	#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////
// General free routine, must be used in conjunction with PDX::Allocate
//////////////////////////////////////////////////////////////////////////////////////////////

VOID PDX_STDCALL Free(VOID* const p)
{
  #ifdef PDX_MEMORY_DEBUG

	if (p) 
	{
		PDX_MEMORY_DEBUG_ASSERT_VALIDBASEPOINTER(p);

        #ifdef PDX_ALIGNED_MALLOC_SUPPORT
		_aligned_free(PDX::MEMORYDEBUGGER::BasePointer(p));
        #else
		delete [] ((CHAR*)PDX::MEMORYDEBUGGER::BasePointer(p));
        #endif
	}

  #else

    #ifdef PDX_ALIGNED_MALLOC_SUPPORT
	_aligned_free(p);
    #else
	delete [] ((CHAR*)p);
    #endif

  #endif
}

}

//////////////////////////////////////////////////////////////////////////////////////////////
// Memory tracing routines for debugging
//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef PDX_MEMORY_DEBUG

namespace PDX
{
	namespace MEMORYDEBUGGER
	{
		VOID* PDX_STDCALL Initialize(VOID* const verybase,const ULONG size)
		{
			if (verybase)
			{
				CHAR* const ptr = PDX_CAST(CHAR*,verybase);

				PDX_CAST_REF(ULONG,ptr[0]) = size;
				PDX_CAST_REF(ULONG,ptr[ALIGNMENT+size]) = PDX_DEAD_END_;

				return &ptr[ALIGNMENT];
			}

			return NULL;
		}

		VOID* PDX_STDCALL BasePointer(VOID* const ptr)
		{
			if (ptr)
			{
				CHAR* const base = PDX_CAST(CHAR*,ptr);
				return (base - ALIGNMENT);
			}

			return NULL;
		}

		BOOL PDX_STDCALL IsValid(const VOID* const base)
		{
			if (base)
			{
				const CHAR* const ptr = PDX_CAST(const CHAR*,base);
				return PDX_CAST_REF(const ULONG,ptr[Size(base)]) == PDX_DEAD_END_;
			}

			return FALSE;
		}

		ULONG PDX_STDCALL Size(const VOID* const base)
		{
			if (base)
			{
				const CHAR* const ptr = PDX_CAST(const CHAR*,base);
				return PDX_CAST_REF(const ULONG,ptr[-INT(ALIGNMENT)]);
			}

			return 0;
		}

		BOOL PDX_STDCALL IsAligned(const VOID* const ptr)
		{
			return PDX_CAST_REF(const ULONG,ptr) % ALIGNMENT == 0;
		}

		ULONG PDX_STDCALL RequiredSize(const ULONG size)
		{
			return size + SIZEOFFSET;
		}

		BOOL PDX_STDCALL IsOverlapping(const VOID* const mem1,const ULONG size1,const VOID* const mem2,const ULONG size2)
		{
			const ULONG offset1 = PDX_CAST_REF(const ULONG,mem1);
			const ULONG offset2 = PDX_CAST_REF(const ULONG,mem2);

			if (offset1 < offset2) return (offset1+size1) > offset2;	
			if (offset2 < offset1) return (offset2+size2) > offset1;

			return TRUE;
		}
	}
}

#endif
