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

#ifndef PDXMEMORY_H
#define PDXMEMORY_H

#include "PdXLibrary.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// Turn on the memory debugger if we're in debug mode
//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#undef  PDX_MEMORY_DEBUG
#define PDX_MEMORY_DEBUG
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Allocation / Deallocation routines
//////////////////////////////////////////////////////////////////////////////////////////////

namespace PDX
{
	VOID* PDX_STDCALL Allocate(const TSIZE);
	VOID PDX_STDCALL Free(VOID* const);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Enumerators used for the overloaded new operators
//////////////////////////////////////////////////////////////////////////////////////////////

enum PDX_ALIGNMENT_ENUM{PDX_DEFAULT_ALIGNMENT=16};
enum PDX_PLACEMENT_NEW_ENUM{PDX_PLACEMENT_NEW=53};

//////////////////////////////////////////////////////////////////////////////////////////////
// Memory debugging functions
//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef PDX_MEMORY_DEBUG

 namespace PDX
 {
	 namespace MEMORYDEBUGGER 
	 {
		 VOID* PDX_STDCALL Initialize(VOID* const,const ULONG);
		 VOID* PDX_STDCALL BasePointer(VOID* const);
		 ULONG PDX_STDCALL Size(const VOID* const);
		 ULONG PDX_STDCALL RequiredSize(const ULONG);
		 BOOL  PDX_STDCALL IsValid(const VOID* const);
		 BOOL  PDX_STDCALL IsAligned(const VOID* const);
		 BOOL  PDX_STDCALL IsOverlapping(const VOID* const,const ULONG,const VOID* const,const ULONG);

		 enum {ALIGNMENT=PDX_DEFAULT_ALIGNMENT};
		 enum {SIZEOFFSET=ALIGNMENT+sizeof(ULONG)};
	 }
 }

  #ifdef _DEBUG
  #define PDX_MEMORY_DEBUG_ASSERT(exp,msg) PDX_ASSERT_MSG(exp,msg)
  #elif defined(PDX_FUNCTION_MACRO_SUPPORTED)
  #define PDX_MEMORY_DEBUG_ASSERT(exp,msg) { if (!(exp)) PDX::AssertMessage(#exp,msg,__FILE__,__FUNCTION__,__LINE__); } PDX_FORCE_SEMICOLON 
  #else
  #define PDX_MEMORY_DEBUG_ASSERT(exp,msg) { if (!(exp)) PDX::AssertMessage(#exp,msg,__FILE__,NULL,__LINE__); } PDX_FORCE_SEMICOLON
  #endif 

 #define PDX_MEMORY_DEBUG_ASSERT_COPYSAFE(dst,src,size,num)    PDX_MEMORY_DEBUG_ASSERT(((num)==0) || ((dst != NULL) && (src != NULL) && (dst) != (src) && (size >= 16 || !PDX::MEMORYDEBUGGER::IsOverlapping(dst,(num)*size,src,(num)*size))),"Memory debug assertion, error in pointer data! Memory can't be copied!")
 #define PDX_MEMORY_DEBUG_ASSERT_MOVESAFE(dst,src,size,num)    PDX_MEMORY_DEBUG_ASSERT(((num)==0) || ((dst != NULL) && (src != NULL) && (dst) != (src)),"Memory debug assertion, error in pointer data! Memory can't be moved!")
 #define PDX_MEMORY_DEBUG_ASSERT_COMPARESAFE(dst,src,size,num) PDX_MEMORY_DEBUG_ASSERT_COPYSAFE(dst,src,size,num)
 #define PDX_MEMORY_DEBUG_ASSERT_ALIGNED(type)                 PDX_MEMORY_DEBUG_ASSERT(PDX::MEMORYDEBUGGER::IsAligned(type),"Memory debug assertion, pointer is not properly aligned!")
 #define PDX_MEMORY_DEBUG_ASSERT_VALIDBASEPOINTER(ptr)         PDX_MEMORY_DEBUG_ASSERT(PDX::MEMORYDEBUGGER::IsValid(ptr),"Memory debug assertion, memory in pointer has been trashed!")

#else

 #define PDX_MEMORY_DEBUG_ASSERT(exp,msg)
 #define PDX_MEMORY_DEBUG_ASSERT_COPYSAFE(dst,src,size,num)  	   
 #define PDX_MEMORY_DEBUG_ASSERT_MOVESAFE(dst,src,size,num)  	   
 #define PDX_MEMORY_DEBUG_ASSERT_COMPARESAFE(dst,src,size,num)	   
 #define PDX_MEMORY_DEBUG_ASSERT_ALIGNED(type)                    
 #define PDX_MEMORY_DEBUG_ASSERT_VALIDBASEPOINTER(ptr)            

#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Memory manipulation class
//////////////////////////////////////////////////////////////////////////////////////////////

class PDXMEMORY	: PDX_STATIC_CLASS
{
public:

	template<class T> 
	static inline VOID ClassCopy(T& dst,const T& src)
	{
		PDX_MEMORY_DEBUG_ASSERT_COPYSAFE(&dst,&src,sizeof(T),1);
		memcpy(&dst,&src,sizeof(T));
	}

	template<class T> 
	static inline VOID ClassZero(T& dst)
	{
		memset(&dst,0,sizeof(T));
	}

	template<class T> 
	static inline VOID Copy(T* const dst,const T* const src,const TSIZE num)
	{
		PDX_MEMORY_DEBUG_ASSERT_COPYSAFE(dst,src,sizeof(T),num);
		memcpy(dst,src,num*sizeof(T));
	}

	template<class T> 
	static inline VOID Set(T* const dst,const T& src,const TSIZE num)
	{
		PDX_MEMORY_DEBUG_ASSERT_COPYSAFE(dst,&src,sizeof(T),num);

      #ifdef PDX_X86

		if (sizeof(T) == sizeof(CHAR)) 
		{
			memset(dst,PDX_CAST_REF(const CHAR,src),sizeof(T) * num);
			return;
		}

      #endif

		for (TSIZE i=0; i < num; ++i)
			ClassCopy(dst[i],src);
	}

	template<class T> 
	static inline BOOL ClassCompare(const T& a,const T& b)
	{
		PDX_MEMORY_DEBUG_ASSERT_COMPARESAFE(&a,&b,sizeof(T),1);
		return memcmp(&a,&b,sizeof(T)) == 0;
	}

	template<class T> 
	static inline BOOL Compare(const T* const a,const T* const b,const TSIZE num)
	{
		PDX_MEMORY_DEBUG_ASSERT_COMPARESAFE(a,b,sizeof(T),num);
		return memcmp(a,b,num*sizeof(T)) == 0;
	}

	template<class T> 
	static inline VOID Move(T* const dst,const T* const src,const TSIZE num)
	{
		PDX_MEMORY_DEBUG_ASSERT_MOVESAFE(dst,src,sizeof(T),num);
		memmove(dst,src,num*sizeof(T));
	}

	template<class T> 
	static inline VOID CopyUp(UCHAR*& ptr,const T& value,const TSIZE size=sizeof(T))
	{
		PDX_MEMORY_DEBUG_ASSERT(ptr,"Null pointer!");
		memcpy(ptr,&value,size);
		ptr += size;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////
// Overloaded new
//////////////////////////////////////////////////////////////////////////////////////////////

inline VOID* operator new (const TSIZE size,const PDX_ALIGNMENT_ENUM e)
{	
	PDX_MEMORY_DEBUG_ASSERT(e == PDX_DEFAULT_ALIGNMENT,"overloaded operator new() assertion, invalid input parameter!");
	return PDX::Allocate(size);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Overloaded placement new
//////////////////////////////////////////////////////////////////////////////////////////////

inline VOID* operator new (const TSIZE,VOID* const ptr,const PDX_PLACEMENT_NEW_ENUM e)
{
	PDX_MEMORY_DEBUG_ASSERT(ptr && e == PDX_PLACEMENT_NEW,"overloaded operator new() assertion, invalid input parameter(s)!");
	return ptr; 
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Destructor helper templates
//////////////////////////////////////////////////////////////////////////////////////////////

namespace PDX
{
	struct DESTRUCTOR_FROM_HELL
	{
		template<class T> 
		static inline VOID Destruct(T& p) 
		{ 
			PDX_COMPILE_ASSERT(sizeof(T));
			p.~T(); 
		}

		template<class T>
		inline VOID operator << (T*& p) const
		{ 
			PDX_COMPILE_ASSERT(sizeof(T));

			if (p)
			{
				p->~T();
				PDX::Free(p);
				p = NULL;		        
			}		
		}

		template<class T> 
		inline VOID operator << (T* const& p) const
		{
			PDX_COMPILE_ASSERT(sizeof(T));

			if (p)
			{
				p->~T();
				PDX::Free(p);
			}		
		}
	};
}

//////////////////////////////////////////////////////////////////////////////////////////////
// new and delete wrapped into nice macros
//////////////////////////////////////////////////////////////////////////////////////////////

#define PDX_NEW    new (PDX_DEFAULT_ALIGNMENT)
#define PDX_DELETE PDX::DESTRUCTOR_FROM_HELL() <<

//////////////////////////////////////////////////////////////////////////////////////////////
// placement new / delete wrapped into nice macros
//////////////////////////////////////////////////////////////////////////////////////////////

#define PDX_CONSTRUCT(p) new ((VOID*)(&(p)),PDX_PLACEMENT_NEW)
#define PDX_DESTRUCT(p)  PDX::DESTRUCTOR_FROM_HELL::Destruct(p)

//////////////////////////////////////////////////////////////////////////////////////////////
// Iterator routines used by several containers
//////////////////////////////////////////////////////////////////////////////////////////////

#define PDX_FUNCTION_ inline VOID
#define PDX_FUNCTION1_(t) template<class t> static inline VOID
#define PDX_FUNCTION2_(t,u) template<class t,class u> static inline VOID
#define PDX_FUNCTION3_(t,u,v) template<class t,class u,class v> static inline VOID

namespace PDX { namespace ITERATOR
{
	namespace
	{
		template<bool P> struct VALID : PDX_STATIC_CLASS
		{
			template<class I,class J> 
			static inline BOOL Valid(const I& a,const J& b)
			{ 
				return (a <= b); 
			}
		};

		template<> struct VALID<true> : PDX_STATIC_CLASS
		{
			static inline BOOL Valid(const VOID* const a,const VOID* const b)
			{ 
				return ((a && b) && (a <= b)) || (!a && !b); 
			}
		};
	}

	template<class I,class J,class K,class L> 
	inline BOOL InBound(const I a,const J b,const K c,const L d)
	{
		return (a >= c && a <= d && b >= c && b <= d);
	}

	template<class I,class J> 
	inline BOOL Valid(const I& a,const J& b)
	{ 
		return VALID<PDX_IS_POINTER(I)>::Valid(a,b);
	}

	template<class DUMMY,bool P> class ISPOD : PDX_STATIC_CLASS
	{
	private:

		template<class DUMMY,bool B> struct BASIC : PDX_STATIC_CLASS
		{
			PDX_FUNCTION2_(T,U) CopyConstruct(T& to,const U& from)
			{ PDX_CONSTRUCT(to) U(from); }

			PDX_FUNCTION2_(T,U) Copy(T& to,const U& from)
			{ to = from; }
		};

		template<class DUMMY> struct BASIC<DUMMY,false> : PDX_STATIC_CLASS
		{
			PDX_FUNCTION2_(T,U) CopyConstruct(T& to,const U& from)
			{ memcpy(&to,&from,sizeof(T)); }

			PDX_FUNCTION2_(T,U) Copy(T& to,const U& from)
			{ memcpy(&to,&from,sizeof(T)); }
		};

	public:

		PDX_FUNCTION1_(T) Construct(const T&) {}
		PDX_FUNCTION1_(T) Destruct(const T&) {}

        #define PDX_FASTCOPY_ bool(PDX_IS_INTEGRAL(T) || sizeof(T) != sizeof(U))

		PDX_FUNCTION2_(T,U) CopyConstruct(T& to,const U& from)
		{ BASIC<bool,PDX_FASTCOPY_>::CopyConstruct(to,from); }

		PDX_FUNCTION2_(T,U) Copy(T& to,const U& from)
		{ BASIC<bool,PDX_FASTCOPY_>::Copy(to,from); }

        #undef PDX_FASTCOPY_
	};

	template<class DUMMY> struct ISPOD<DUMMY,false> : PDX_STATIC_CLASS
	{
		PDX_FUNCTION1_(T) Construct(T& t) 
		{ PDX_CONSTRUCT(t) T; }

		PDX_FUNCTION1_(T) Destruct(T& t) 
		{ t.~T(); }

		PDX_FUNCTION2_(T,U) CopyConstruct(T& to,const U& from)
		{ PDX_CONSTRUCT(to) T(from); }

		PDX_FUNCTION2_(T,U) Copy(T& to,const U& from)
		{ to = from; }
	};

	template<class DUMMY,bool R> struct ISRANDOMACCESS : PDX_STATIC_CLASS
	{
		template<class DUMMY,bool P> class ISPOD : PDX_STATIC_CLASS
		{
		public:

            #define PDX_FASTFILL_1_  INT(sizeof(T) == sizeof(CHAR) ? 0 : 2)
            #define PDX_FASTFILL_2_  INT(sizeof(T) == sizeof(CHAR) ? 1 : 2)
            #define PDX_SAMEPOINTER_ bool(PDX_IS_POINTER(U) && sizeof(T) == sizeof(*begin))

			PDX_FUNCTION1_(T) Construct(const T* const,const T* const) {}
			PDX_FUNCTION1_(T) Destruct(const T* const,const T* const) {}    	

			PDX_FUNCTION1_(T) CopyConstruct(T* const begin,const T* const end,const T& value)
			{ FILL<bool,PDX_FASTFILL_1_>::FillConstruct(begin,end,value); }

			PDX_FUNCTION2_(T,U) CopyConstruct(T* const begin,const T* const end,const U& value)
			{ FILL<bool,PDX_FASTFILL_2_>::FillConstruct(begin,end,value); }
  
			PDX_FUNCTION3_(T,U,V) CopyConstructSequence(T* const out,U& begin,const V& end)
			{ COPY<bool,PDX_SAMEPOINTER_>::CopyConstructSequence(out,begin,end); }
  
			PDX_FUNCTION3_(T,U,V) CopySequence(T* const out,U& begin,const V& end)
			{ COPY<bool,PDX_SAMEPOINTER_>::CopyConstructSequence(out,begin,end); }

			PDX_FUNCTION1_(T) Fill(T* const begin,const T* const end,const T& value)
			{ FILL<bool,PDX_FASTFILL_1_>::Fill(begin,end,value); }
 
			PDX_FUNCTION2_(T,V) Fill(T* const begin,const T* const end,const V& value)
			{ FILL<bool,PDX_FASTFILL_2_>::Fill(begin,end,value); }
   
            #undef PDX_SAMEPOINTER_
            #undef PDX_FASTFILL_1_
            #undef PDX_FASTFILL_2_

			template<class DUMMY,INT> struct FILL : PDX_STATIC_CLASS
			{
				PDX_FUNCTION1_(T) Fill(T* const begin,const T* const end,const T& value)
				{ 
					PDX_COMPILE_ASSERT(sizeof(T) == sizeof(CHAR));
					PDX_ASSERT(begin <= end);
					memset(begin,PDX_CAST_REF(const CHAR,value),sizeof(T) * (end-begin)); 
				}

				PDX_FUNCTION1_(T) FillConstruct(T* const begin,const T* const end,const T& value)
				{ 
					PDX_COMPILE_ASSERT(sizeof(T) == sizeof(CHAR));
					PDX_ASSERT(begin <= end);
					memset((VOID*)begin,PDX_CAST_REF(const CHAR,value),sizeof(T) * (end-begin)); 
				}
			};

			template<class DUMMY> struct FILL<DUMMY,1> : PDX_STATIC_CLASS
			{
				PDX_FUNCTION2_(T,U) Fill(T* const begin,const T* const end,const U& value)
				{
					PDX_COMPILE_ASSERT(sizeof(T) == sizeof(CHAR));
					PDX_ASSERT(begin <= end);

					if (begin != end)
					{
						*begin = value;
						memset(&begin[1],PDX_CAST_REF(const CHAR,*begin),sizeof(T) * (end - &begin[1]));
					}
				}

				PDX_FUNCTION2_(T,U) FillConstruct(T* const begin,const T* const end,const U& value)
				{
					PDX_COMPILE_ASSERT(sizeof(T) == sizeof(CHAR));
					PDX_ASSERT(begin <= end);

					if (begin != end)
					{
						PDX_CONSTRUCT(*begin) T(value);
						memset((VOID*)&begin[1],PDX_CAST_REF(const CHAR,*begin),sizeof(T) * (end - &begin[1]));
					}
				}
			};

			template<class DUMMY> struct FILL<DUMMY,2> : PDX_STATIC_CLASS
			{
				PDX_FUNCTION2_(T,U) Fill(T* begin,const T* const end,const U& value)
				{
					PDX_ASSERT(begin <= end);

					while (begin != end)
						*begin++ = value;
				}

				PDX_FUNCTION2_(T,U) FillConstruct(T* begin,const T* const end,const U& value)
				{
					PDX_ASSERT(begin <= end);

					for (;begin != end; ++begin)
						PDX_CONSTRUCT(*begin) T(value);
				}
			};

		private:

			template<class DUMMY,bool P> struct COPY : PDX_STATIC_CLASS
			{
				PDX_FUNCTION1_(T) CopyConstructSequence(T* const out,const T* const begin,const T* const end)
				{ 
					PDX_ASSERT(out || begin == end);
					memcpy((VOID*)out,begin,sizeof(T) * (end-begin)); 
				}

				PDX_FUNCTION1_(T) CopySequence(T* const out,const T* const begin,const T* const end)
				{ 
					PDX_ASSERT(out || begin == end);
					memcpy(out,begin,sizeof(T) * (end-begin)); 
				}

				PDX_FUNCTION2_(T,U) CopyConstructSequence(T* out,const U* begin,const U* const end)
				{ 
					PDX_ASSERT(out || begin == end);

					for (; begin != end; ++begin, ++out)
						PDX_CONSTRUCT(*out) T(*begin);
				}

				PDX_FUNCTION2_(T,U) CopySequence(T* out,const U* begin,const U* const end)
				{ 
					PDX_ASSERT(out || begin == end);

					for (; begin != end; ++begin, ++out)
						*out = *begin;
				}
			};

			template<class DUMMY> struct COPY<DUMMY,false> : PDX_STATIC_CLASS
			{
				PDX_FUNCTION3_(T,U,V) CopyConstructSequence(T* out,U& begin,const V& end)
				{ 
					PDX_ASSERT(out || !(begin != end));

					for (; begin != end; ++begin, ++out)
						PDX_CONSTRUCT(*out) T(*begin);
				}

				PDX_FUNCTION3_(T,U,V) CopySequence(T* out,U& begin,const V& end)
				{ 
					PDX_ASSERT(out || !(begin != end));

					for (; begin != end; ++begin, ++out)
						*out = *begin;
				}
			};
		};

		template<class DUMMY> struct ISPOD<DUMMY,false> : PDX_STATIC_CLASS
		{
			PDX_FUNCTION1_(T) Construct(T* begin,const T* const end) 
			{
				PDX_ASSERT(begin <= end);

				for (;begin != end; ++begin)
					PDX_CONSTRUCT(*begin) T;
			}

			PDX_FUNCTION1_(T) Destruct(T* begin,const T* const end) 
			{
				PDX_ASSERT(begin <= end);

				for (;begin != end; ++begin)
					begin->~T();
			}    	

			PDX_FUNCTION2_(T,U) CopyConstruct(T* begin,const T* const end,const U& value)
			{ 
				PDX_ASSERT(begin <= end);

				for (;begin != end; ++begin)
					PDX_CONSTRUCT(*begin) T(value);
			}

			PDX_FUNCTION3_(T,U,V) CopyConstructSequence(T* out,U& begin,const V& end)
			{ 
				for (; begin != end; ++begin, ++out)
					PDX_CONSTRUCT(*out) T(*begin);
			}

			PDX_FUNCTION3_(T,U,V) CopySequence(T* out,U& begin,const V& end)
			{ 
				for (; begin != end; ++begin, ++out)
					*out = *begin;
			}

			PDX_FUNCTION2_(T,U) Fill(T* begin,const T* const end,const U& value)
			{ 
				PDX_ASSERT(begin <= end);

				for (;begin != end; ++begin)
					*begin = value;
			}
		};
	};

	template<class DUMMY> struct ISRANDOMACCESS<DUMMY,false> : PDX_STATIC_CLASS
	{
		template<class DUMMY,bool P> struct ISPOD : PDX_STATIC_CLASS
		{
			PDX_FUNCTION1_(T) Construct(const T&,const T&) {}
			PDX_FUNCTION1_(T) Destruct(const T&,const T&) {}    	

			PDX_FUNCTION2_(T,U) CopyConstruct(T& begin,const T& end,const U& value)
			{ 
				for (;begin != end; ++begin)
					PDX_CONSTRUCT(*begin) T(value);
			}

			PDX_FUNCTION3_(T,U,V) CopyConstructSequence(T& out,U& begin,const V& end)
			{ 
				for (;begin != end; ++begin, ++out)
					PDX_CONSTRUCT(*out) T(begin);
			}

			PDX_FUNCTION3_(T,U,V) CopySequence(T& out,U& begin,const V& end)
			{ 
				for (;begin != end; ++begin, ++out)
					*out = *begin;
			}

			PDX_FUNCTION2_(T,U) Fill(T& begin,const T& end,const U& value)
			{ 
				for (;begin != end; ++begin)
					*begin = value;
			}
		};

		template<class DUMMY> struct ISPOD<DUMMY,false>	: PDX_STATIC_CLASS
		{
			PDX_FUNCTION1_(T) Construct(T& begin,const T& end) 
			{
				for (;begin != end; ++begin)
					PDX_CONSTRUCT(*begin) T;
			}

			PDX_FUNCTION1_(T) Destruct(T& begin,const T& end) 
			{
				for (;begin != end; ++begin)
					(*begin).~T();
			}    	

			PDX_FUNCTION2_(T,U) CopyConstruct(T& begin,const T& end,const U& value)
			{ 
				for (;begin != end; ++begin)
					PDX_CONSTRUCT(*begin) T(value);
			}

			PDX_FUNCTION3_(T,U,V) CopyConstructSequence(T& out,U& begin,const V& end)
			{ 
				for (;begin != end; ++begin, ++out)
					PDX_CONSTRUCT(*out) T(*begin);
			}

			PDX_FUNCTION3_(T,U,V) CopySequence(T& out,U& begin,const V& end)
			{ 
				for (;begin != end; ++begin, ++out)
					*out = *begin;
			}

			PDX_FUNCTION2_(T,U) Fill(T& begin,const T& end,const U& value)
			{ 
				for (;begin != end; ++begin)
					*begin = value;
			}
		};
	};

	template<bool P,class T>
	PDX_FUNCTION_ Construct(T& t) 
	{ ISPOD<bool,P>::Construct(t); }

	template<bool P,class T>
	PDX_FUNCTION_ Destruct(T& t) 
	{ ISPOD<bool,P>::Destruct(t); }

	template<bool P,class T,class U>
	PDX_FUNCTION_ CopyConstruct(T& to,const U& from)
	{ ISPOD<bool,P>::CopyConstruct(to,from); }

	template<bool P,class T,class U>
	PDX_FUNCTION_ Copy(T& to,const U& from)
	{ ISPOD<bool,P>::Copy(to,from); }

	template<bool P,class T,class U>
	PDX_FUNCTION_ Construct(T begin,const U& end) 
	{ ISRANDOMACCESS<bool,PDX_IS_POINTER(T)>::ISPOD<bool,P>::Construct(begin,end); }

	template<bool P,class T,class U>
	PDX_FUNCTION_ Destruct(T begin,const U& end) 
	{ ISRANDOMACCESS<bool,PDX_IS_POINTER(T)>::ISPOD<bool,P>::Destruct(begin,end); }

	template<bool P,class T,class U>
	PDX_FUNCTION_ CopyConstruct(T begin,const T end,const U& value)
	{ ISRANDOMACCESS<bool,PDX_IS_POINTER(T)>::ISPOD<bool,P>::CopyConstruct(begin,end,value); }
  
	template<bool P,class T,class U,class V>
	PDX_FUNCTION_ CopyConstructSequence(T out,U begin,const V& end)
	{ ISRANDOMACCESS<bool,PDX_IS_POINTER(T)>::ISPOD<bool,P>::CopyConstructSequence(out,begin,end); }
  
	template<bool P,class T,class U,class V>
	PDX_FUNCTION_ CopySequence(T out,U begin,const V& end)
	{ ISRANDOMACCESS<bool,PDX_IS_POINTER(T)>::ISPOD<bool,P>::CopySequence(out,begin,end); }

	template<bool P,class T,class U,class V>
	PDX_FUNCTION_ Fill(T begin,const U& end,const V& value)
	{ ISRANDOMACCESS<bool,PDX_IS_POINTER(T)>::ISPOD<bool,P>::Fill(begin,end,value); }
  
	PDX_FUNCTION1_(T) MemCopy(T& to,const T& from)
	{ memcpy(&to,&from,sizeof(T)); }	 

	PDX_FUNCTION1_(T) MemCopy(T* const out,const T* const begin,const T* const end)
	{
		PDX_ASSERT(out || end == begin);
		memcpy((VOID*)out,begin,sizeof(T) * (end - begin)); 
	}

	PDX_FUNCTION1_(T) MemMove(T* const out,const T* const begin,const T* const end) 
	{ 
		PDX_ASSERT(out || end == begin);
		memmove((VOID*)out,begin,sizeof(T) * (end - begin)); 
	}

	PDX_FUNCTION1_(T) MemSet(T* const begin,const T* const end,const INT value)
	{ 
		PDX_ASSERT(begin <= end);
		memset((VOID*)begin,value,sizeof(T) * (end - begin));
	}
}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// No need for these anymore
//////////////////////////////////////////////////////////////////////////////////////////////

#undef PDX_FUNCTION_
#undef PDX_FUNCTION1_   
#undef PDX_FUNCTION2_ 
#undef PDX_FUNCTION3_

#endif


