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

#ifndef PDXLIBRARY_H
#error Do not include PdxType.h directly!
#endif

#ifndef PDXTYPES_H
#define PDXTYPES_H

//////////////////////////////////////////////////////////////////////////////////////////////
// Basic types, try to avoid type name collision with the windows header
//////////////////////////////////////////////////////////////////////////////////////////////

#if defined(_INC_WINDOWS) || defined(PDX_WINDOWS_H_INCLUDED)

 #define PDX_CHAR_DEFINED
 #define PDX_UCHAR_DEFINED
 #define PDX_SHORT_DEFINED
 #define PDX_USHORT_DEFINED
 #define PDX_LONG_DEFINED
 #define PDX_ULONG_DEFINED
 #define PDX_INT_DEFINED
 #define PDX_UINT_DEFINED
 #define PDX_FLOAT_DEFINED
 #define PDX_VOID_DEFINED
 #define PDX_BOOL_DEFINED
 #define PDX_BYTE_DEFINED
 #define PDX_WORD_DEFINED
 #define PDX_DWORD_DEFINED

#endif

#ifndef PDX_CHAR_DEFINED
#define PDX_CHAR_DEFINED
typedef char CHAR;
#endif

#ifndef PDX_UCHAR_DEFINED
#define PDX_UCHAR_DEFINED
typedef unsigned char UCHAR;
#endif

#ifndef PDX_SHORT_DEFINED
#define PDX_SHORT_DEFINED
typedef short SHORT;
#endif

#ifndef PDX_USHORT_DEFINED
#define PDX_USHORT_DEFINED
typedef unsigned short USHORT;
#endif

#ifndef PDX_LONG_DEFINED
#define PDX_LONG_DEFINED
typedef long LONG;
#endif

#ifndef PDX_ULONG_DEFINED
#define PDX_ULONG_DEFINED
typedef unsigned long ULONG;
#endif

#ifndef PDX_INT_DEFINED
#define PDX_INT_DEFINED
typedef int INT;
#endif

#ifndef PDX_UINT_DEFINED
#define PDX_UINT_DEFINED
typedef unsigned int UINT;
#endif

#ifndef PDX_TSIZE_DEFINED
#define PDX_TSIZE_DEFINED
typedef size_t TSIZE;
#endif

#ifndef PDX_FLOAT_DEFINED
#define PDX_FLOAT_DEFINED
typedef float FLOAT;
#endif

#ifndef PDX_DOUBLE_DEFINED
#define PDX_DOUBLE_DEFINED
typedef double DOUBLE;
#endif

#ifndef PDX_LONG_DOUBLE_DEFINED
#define PDX_LONG_DOUBLE_DEFINED
typedef long double LONGDOUBLE;
#endif

#ifndef PDX_VOID_DEFINED
#define PDX_VOID_DEFINED
typedef void VOID;
#endif
 			
#ifndef PDX_BYTE_DEFINED
#define PDX_BYTE_DEFINED
typedef UCHAR BYTE;
#endif

#ifndef PDX_WORD_DEFINED
#define PDX_WORD_DEFINED
typedef USHORT WORD;
#endif

#ifndef PDX_DWORD_DEFINED
#define PDX_DWORD_DEFINED
typedef ULONG DWORD;
#endif

#ifndef PDX_BOOL_DEFINED
#define PDX_BOOL_DEFINED
typedef INT BOOL;
#undef  FALSE
#define FALSE 0
#undef  TRUE
#define TRUE 1
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Bit size for different types
//////////////////////////////////////////////////////////////////////////////////////////////

#ifndef UCHAR_BIT
#define UCHAR_BIT PDX_SIZE_BITS(UCHAR)
#endif

#ifndef USHORT_BIT     
#define USHORT_BIT PDX_SIZE_BITS(USHORT)
#endif

#ifndef SHORT_BIT      
#define SHORT_BIT PDX_SIZE_BITS(SHORT)
#endif

#ifndef ULONG_BIT      
#define ULONG_BIT PDX_SIZE_BITS(ULONG)
#endif

#ifndef LONG_BIT       
#define LONG_BIT PDX_SIZE_BITS(LONG)
#endif

#ifndef UINT_BIT       
#define UINT_BIT PDX_SIZE_BITS(UINT)
#endif

#ifndef INT_BIT        
#define INT_BIT PDX_SIZE_BITS(INT)
#endif

#ifndef TSIZE_BIT      
#define TSIZE_BIT PDX_SIZE_BITS(TSIZE)
#endif

#ifndef FLOAT_BIT      
#define FLOAT_BIT PDX_SIZE_BITS(FLOAT)
#endif

#ifndef DOUBLE_BIT
#define DOUBLE_BIT PDX_SIZE_BITS(DOUBLE)
#endif

#ifndef LONGDOUBLE_BIT
#define LONGDOUBLE_BIT PDX_SIZE_BITS(LONGDOUBLE)
#endif

#ifndef BYTE_BIT
#define BYTE_BIT PDX_SIZE_BITS(BYTE)
#endif

#ifndef WORD_BIT
#define WORD_BIT PDX_SIZE_BITS(WORD)
#endif

#ifndef DWORD_BIT
#define DWORD_BIT PDX_SIZE_BITS(DWORD)
#endif

#ifndef	I8_BIT
#define I8_BIT 8
#endif

#ifndef	U8_BIT
#define U8_BIT 8
#endif

#ifndef	I16_BIT
#define I16_BIT 16
#endif

#ifndef	U16_BIT
#define U16_BIT 16
#endif

#ifndef	I32_BIT
#define I32_BIT 32
#endif

#ifndef	U32_BIT
#define U32_BIT 32
#endif

#ifndef	F32_BIT
#define F32_BIT 32
#endif

#ifndef	F64_BIT
#define F64_BIT 64
#endif

#ifndef F128_BIT
#define F128_BIT 128
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Integral type limits
//////////////////////////////////////////////////////////////////////////////////////////////
			  
#ifndef UCHAR_MIN
#define UCHAR_MIN 0U
#endif

#ifndef USHORT_MIN
#define USHORT_MIN 0U
#endif

#ifndef USHORT_MAX
#define USHORT_MAX USHRT_MAX
#endif

#ifndef SHORT_MIN
#define SHORT_MIN SHRT_MIN
#endif

#ifndef SHORT_MAX
#define SHORT_MAX SHRT_MAX
#endif

#ifndef ULONG_MIN
#define ULONG_MIN 0UL
#endif

#ifndef UINT_MIN
#define UINT_MIN 0U
#endif

#ifndef TSIZE_MAX
#define TSIZE_MAX ~TSIZE(0)
#endif

#ifndef TSIZE_MIN
#define TSIZE_MIN TSIZE(0)
#endif

#ifndef I8_MIN
#define I8_MIN (-128)        
#endif

#ifndef I8_MAX
#define I8_MAX 127
#endif

#ifndef U8_MIN
#define U8_MIN 0U
#endif

#ifndef U8_MAX
#define U8_MAX 255U        
#endif

#ifndef I16_MIN
#define I16_MIN (-32768)      
#endif

#ifndef I16_MAX
#define I16_MAX 32767       
#endif

#ifndef U16_MIN
#define U16_MIN 0U
#endif

#ifndef U16_MAX
#define U16_MAX 65535U      
#endif

#ifndef I32_MIN
#define I32_MIN (-2147483648L) 
#endif

#ifndef I32_MAX
#define I32_MAX 2147483647L
#endif

#ifndef U32_MIN
#define U32_MIN 0UL
#endif

#ifndef U32_MAX
#define U32_MAX 4294967295UL
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Real type limits
//////////////////////////////////////////////////////////////////////////////////////////////

#ifndef FLOAT_MIN
#define FLOAT_MIN FLT_MIN
#endif

#ifndef FLOAT_MAX
#define FLOAT_MAX FLT_MAX
#endif

#ifndef FLOAT_DIG
#define FLOAT_DIG FLT_DIG
#endif

#ifndef DOUBLE_MIN
#define DOUBLE_MIN DBL_MIN
#endif

#ifndef DOUBLE_MAX
#define DOUBLE_MAX DBL_MAX
#endif

#ifndef DOUBLE_DIG
#define DOUBLE_DIG DBL_DIG
#endif

#ifndef LONGDOUBLE_MIN
#define LONGDOUBLE_MIN LDBL_MIN
#endif

#ifndef LONGDOUBLE_MAX
#define LONGDOUBLE_MAX LDBL_MAX
#endif

#ifndef LONGDOUBLE_DIG
#define LONGDOUBLE_DIG LDBL_DIG
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Real min/max
//////////////////////////////////////////////////////////////////////////////////////////////

#ifndef F32_MIN
#define F32_MIN														   \
(																	   \
    (sizeof(FLOAT)      * CHAR_BIT == 32) ? FLOAT_MIN :				   \
    (sizeof(DOUBLE)     * CHAR_BIT == 32) ? DOUBLE_MIN :			   \
	(sizeof(LONGDOUBLE) * CHAR_BIT == 32) ? LONGDOUBLE_MIN : FLOAT_MIN \
)
#endif

#ifndef F32_MAX
#define F32_MAX														   \
(																	   \
    (sizeof(FLOAT)      * CHAR_BIT == 32) ? FLOAT_MAX :				   \
    (sizeof(DOUBLE)     * CHAR_BIT == 32) ? DOUBLE_MAX :			   \
	(sizeof(LONGDOUBLE) * CHAR_BIT == 32) ? LONGDOUBLE_MAX : FLOAT_MAX \
)
#endif

#ifndef F64_MIN
#define F64_MIN														    \
(																	    \
    (sizeof(FLOAT)      * CHAR_BIT == 64) ? FLOAT_MIN :				    \
    (sizeof(DOUBLE)     * CHAR_BIT == 64) ? DOUBLE_MIN :			    \
    (sizeof(LONGDOUBLE) * CHAR_BIT == 64) ? LONGDOUBLE_MIN : DOUBLE_MIN \
)
#endif

#ifndef F64_MAX
#define F64_MAX														    \
(																        \
    (sizeof(FLOAT)      * CHAR_BIT == 64) ? FLOAT_MAX :				    \
    (sizeof(DOUBLE)     * CHAR_BIT == 64) ? DOUBLE_MAX :			  	\
    (sizeof(LONGDOUBLE) * CHAR_BIT == 64) ? LONGDOUBLE_MAX : DOUBLE_MAX \
)
#endif

#ifndef F128_MIN
#define F128_MIN															 \
(																			 \
    (sizeof(FLOAT)      * CHAR_BIT == 128) ? FLOAT_MIN :					 \
    (sizeof(DOUBLE)     * CHAR_BIT == 128) ? DOUBLE_MIN :					 \
    (sizeof(LONGDOUBLE) * CHAR_BIT == 128) ? LONGDOUBLE_MIN : LONGDOUBLE_MIN \
)
#endif

#ifndef F128_MAX
#define F128_MAX                                                             \
(																			 \
    (sizeof(FLOAT)      * CHAR_BIT == 128) ? FLOAT_MAX :					 \
    (sizeof(DOUBLE)     * CHAR_BIT == 128) ? DOUBLE_MAX :					 \
    (sizeof(LONGDOUBLE) * CHAR_BIT == 128) ? LONGDOUBLE_MAX : LONGDOUBLE_MAX \
)
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Number counts
//////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CHAR_NUM
#define CHAR_NUM (1+CHAR_MAX)
#endif

#ifndef UCHAR_NUM
#define UCHAR_NUM  (1U+UCHAR_MAX)
#endif

#ifndef SHORT_NUM
#define SHORT_NUM  (1L+SHORT_MAX)
#endif 

#ifndef USHORT_NUM
#define USHORT_NUM (1UL+USHORT_MAX)
#endif

#ifndef INT_NUM
#define INT_NUM (1UL+INT_MAX)
#endif

#ifndef UINT_NUM 
#define UINT_NUM (1UL+UINT_MAX)
#endif

#ifndef I8_NUM
#define I8_NUM (1+I8_MAX)      
#endif

#ifndef U8_NUM
#define U8_NUM (1U+U8_MAX)
#endif

#ifndef I16_NUM
#define I16_NUM (1L+I16_MAX)    
#endif  

#ifndef U16_NUM
#define U16_NUM (1UL+U16_MAX)
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Detect sized types
//////////////////////////////////////////////////////////////////////////////////////////////

#define PDX_M_SIGNED   0
#define PDX_M_UNSIGNED 1
#define PDX_M_REAL	   2

template<unsigned int I> struct PDXTYPESIZEINDEX {};
template<unsigned int N,unsigned int> struct PDXTYPESIZE {};

template<> struct PDXTYPESIZEINDEX<0>  { typedef unsigned long  TYPE; enum {SUPPORTED=0}; };
template<> struct PDXTYPESIZEINDEX<1>  { typedef signed char    TYPE; enum {SUPPORTED=1}; };
template<> struct PDXTYPESIZEINDEX<2>  { typedef signed short   TYPE; enum {SUPPORTED=1}; };
template<> struct PDXTYPESIZEINDEX<3>  { typedef signed long    TYPE; enum {SUPPORTED=1}; };
template<> struct PDXTYPESIZEINDEX<4>  { typedef signed int     TYPE; enum {SUPPORTED=1}; };
template<> struct PDXTYPESIZEINDEX<5>  { typedef unsigned char  TYPE; enum {SUPPORTED=1}; };
template<> struct PDXTYPESIZEINDEX<6>  { typedef unsigned short TYPE; enum {SUPPORTED=1}; };
template<> struct PDXTYPESIZEINDEX<7>  { typedef unsigned long  TYPE; enum {SUPPORTED=1}; };
template<> struct PDXTYPESIZEINDEX<8>  { typedef unsigned int   TYPE; enum {SUPPORTED=1}; };
template<> struct PDXTYPESIZEINDEX<9>  { typedef float          TYPE; enum {SUPPORTED=1}; };
template<> struct PDXTYPESIZEINDEX<10> { typedef double         TYPE; enum {SUPPORTED=1}; };
template<> struct PDXTYPESIZEINDEX<11> { typedef long double    TYPE; enum {SUPPORTED=1}; };
template<> struct PDXTYPESIZEINDEX<12> { typedef signed char    TYPE; enum {SUPPORTED=0}; };
template<> struct PDXTYPESIZEINDEX<13> { typedef signed short   TYPE; enum {SUPPORTED=0}; };
template<> struct PDXTYPESIZEINDEX<14> { typedef signed long    TYPE; enum {SUPPORTED=0}; };
template<> struct PDXTYPESIZEINDEX<15> { typedef signed int     TYPE; enum {SUPPORTED=0}; };
template<> struct PDXTYPESIZEINDEX<16> { typedef unsigned char  TYPE; enum {SUPPORTED=0}; };
template<> struct PDXTYPESIZEINDEX<17> { typedef unsigned short TYPE; enum {SUPPORTED=0}; };
template<> struct PDXTYPESIZEINDEX<18> { typedef unsigned long  TYPE; enum {SUPPORTED=0}; };
template<> struct PDXTYPESIZEINDEX<19> { typedef unsigned int   TYPE; enum {SUPPORTED=0}; };
template<> struct PDXTYPESIZEINDEX<20> { typedef float          TYPE; enum {SUPPORTED=0}; };
template<> struct PDXTYPESIZEINDEX<21> { typedef double         TYPE; enum {SUPPORTED=0}; };
template<> struct PDXTYPESIZEINDEX<22> { typedef long double    TYPE; enum {SUPPORTED=0}; };

#define PDX_DECLARE_SIZETYPE_SIGNED(s,b)		   		   \
								     			       	   \
template<> struct PDXTYPESIZE<s,b>  	       			   \
{					       			       				   \
	enum 		     			       					   \
	{						       		                   \
		I = sizeof( signed char  ) * CHAR_BIT == s ? 1 :   \
     	    sizeof( signed short ) * CHAR_BIT == s ? 2 :   \
			sizeof( signed long  ) * CHAR_BIT == s ? 3 :   \
			sizeof( signed int   ) * CHAR_BIT == s ? 4 :   \
			sizeof( signed char  ) * CHAR_BIT >  s ? 12	:  \
            sizeof( signed short ) * CHAR_BIT >  s ? 13 :  \
			sizeof( signed long  ) * CHAR_BIT >  s ? 14 :  \
			sizeof( signed int   ) * CHAR_BIT >  s ? 15 :  \
			0											   \
	};													   \
														   \
    typedef PDXTYPESIZEINDEX<I>::TYPE TYPE;				   \
	enum {SUPPORTED=PDXTYPESIZEINDEX<I>::SUPPORTED};	   \
};

#define PDX_DECLARE_SIZETYPE_UNSIGNED(s,b)	      		    \
								     			       	    \
template<> struct PDXTYPESIZE<s,b>             			    \
{					       			       			        \
	enum 		     			       				        \
	{					       	     	                    \
		I = sizeof( unsigned char  ) * CHAR_BIT == s ? 5 :  \
     	    sizeof( unsigned short ) * CHAR_BIT == s ? 6 :  \
			sizeof( unsigned long  ) * CHAR_BIT == s ? 7 :  \
			sizeof( unsigned int   ) * CHAR_BIT == s ? 8 :  \
			sizeof( unsigned char  ) * CHAR_BIT >  s ? 16 : \
            sizeof( unsigned short ) * CHAR_BIT >  s ? 17 :	\
            sizeof( unsigned long  ) * CHAR_BIT >  s ? 18 :	\
            sizeof( unsigned int   ) * CHAR_BIT >  s ? 19 :	\
			0												\
	};														\
															\
    typedef PDXTYPESIZEINDEX<I>::TYPE TYPE;					\
	enum {SUPPORTED=PDXTYPESIZEINDEX<I>::SUPPORTED};		\
};														   

#define PDX_DECLARE_SIZETYPE_REAL(s,b)		   			 \
								     			       	 \
template<> struct PDXTYPESIZE<s,b>  	       			 \
{					       			       				 \
	enum 		     			       					 \
	{						       		                 \
		I = sizeof( float       ) * CHAR_BIT == s ?  9 : \
			sizeof( double      ) * CHAR_BIT == s ? 10 : \
			sizeof( long double ) * CHAR_BIT == s ? 11 : \
			sizeof( float       ) * CHAR_BIT >  s ? 20 : \
            sizeof( double      ) * CHAR_BIT >  s ? 21 : \
            sizeof( long double ) * CHAR_BIT >  s ? 22 : \
			0											 \
	};													 \
														 \
    typedef PDXTYPESIZEINDEX<I>::TYPE TYPE;				 \
	enum {SUPPORTED=PDXTYPESIZEINDEX<I>::SUPPORTED};	 \
};

PDX_DECLARE_SIZETYPE_SIGNED   ( 8,  PDX_M_SIGNED   )
PDX_DECLARE_SIZETYPE_UNSIGNED ( 8,  PDX_M_UNSIGNED )
PDX_DECLARE_SIZETYPE_SIGNED   ( 16, PDX_M_SIGNED   )
PDX_DECLARE_SIZETYPE_UNSIGNED ( 16, PDX_M_UNSIGNED )
PDX_DECLARE_SIZETYPE_SIGNED   ( 32, PDX_M_SIGNED   )
PDX_DECLARE_SIZETYPE_UNSIGNED ( 32, PDX_M_UNSIGNED )
PDX_DECLARE_SIZETYPE_REAL     ( 32, PDX_M_REAL     )
PDX_DECLARE_SIZETYPE_REAL     ( 64, PDX_M_REAL     )

#undef PDX_DECLARE_SIZETYPE_SIGNED
#undef PDX_DECLARE_SIZETYPE_UNSIGNED
#undef PDX_DECLARE_SIZETYPE_REAL

typedef PDXTYPESIZE< 8,  PDX_M_SIGNED   >::TYPE I8;
typedef PDXTYPESIZE< 8,  PDX_M_UNSIGNED >::TYPE U8;
typedef PDXTYPESIZE< 16, PDX_M_SIGNED   >::TYPE I16;
typedef PDXTYPESIZE< 16, PDX_M_UNSIGNED >::TYPE U16;
typedef PDXTYPESIZE< 32, PDX_M_SIGNED   >::TYPE I32;
typedef PDXTYPESIZE< 32, PDX_M_UNSIGNED >::TYPE U32;
typedef PDXTYPESIZE< 32, PDX_M_REAL     >::TYPE F32;
typedef PDXTYPESIZE< 64, PDX_M_REAL     >::TYPE F64;

#define PDX_I8_SUPPORT  PDXTYPESIZE< 8,  PDX_M_SIGNED   >::SUPPORTED
#define PDX_U8_SUPPORT  PDXTYPESIZE< 8,  PDX_M_UNSIGNED >::SUPPORTED
#define PDX_I16_SUPPORT PDXTYPESIZE< 16, PDX_M_SIGNED   >::SUPPORTED
#define PDX_U16_SUPPORT PDXTYPESIZE< 16, PDX_M_UNSIGNED >::SUPPORTED
#define PDX_I32_SUPPORT PDXTYPESIZE< 32, PDX_M_SIGNED   >::SUPPORTED
#define PDX_U32_SUPPORT PDXTYPESIZE< 32, PDX_M_UNSIGNED >::SUPPORTED
#define PDX_F32_SUPPORT PDXTYPESIZE< 32, PDX_M_REAL     >::SUPPORTED
#define PDX_F64_SUPPORT PDXTYPESIZE< 64, PDX_M_REAL     >::SUPPORTED

#undef PDX_M_SIGNED
#undef PDX_M_UNSIGNED
#undef PDX_M_REAL

//////////////////////////////////////////////////////////////////////////////////////////////
// 64 bit integer
//////////////////////////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER) && defined(_INTEGRAL_MAX_BITS)
#if (_INTEGRAL_MAX_BITS >= 64)
   
  #ifndef PDX_I64_DEFINED
  #define PDX_I64_DEFINED
  typedef signed __int64 I64;
  #endif

  #ifndef PDX_U64_DEFINED
  #define PDX_U64_DEFINED
  typedef unsigned __int64 U64;
  #endif

  #ifndef PDX_QWORD_DEFINED
  #define PDX_QWORD_DEFINED
  typedef unsigned __int64 QWORD;
  #endif

  #ifndef I64_MIN
  #define I64_MIN _I64_MIN
  #endif

  #ifndef I64_MAX
  #define I64_MAX _I64_MAX
  #endif

  #ifndef U64_MIN
  #define U64_MIN  0ui64
  #endif

  #ifndef U64_MAX
  #define U64_MAX _UI64_MAX  
  #endif

  #ifndef I64_BIT
  #define I64_BIT 64
  #endif

  #ifndef U64_BIT
  #define U64_BIT 64
  #endif

  #ifndef QWORD_BIT
  #define QWORD_BIT U64_BIT
  #endif

  #define PDX_I64_SUPPORT 1
  #define PDX_U64_SUPPORT 1

#endif
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Calling conventions
//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER 
#if    _MSC_VER >= 800

 #undef  PDX_CDECL
 #define PDX_CDECL __cdecl
 
 #undef  PDX_STDCALL
 #define PDX_STDCALL __stdcall
 
 #undef  PDX_FASTCALL
 #define PDX_FASTCALL __fastcall
 
 #undef  PDX_NAKED
 #define PDX_NAKED __declspec(naked)

#endif
#endif

#if !defined(PDX_CDECL) && defined(_CDECL_SUPPORTED)
#define PDX_CDECL __cdecl
#endif

#if !defined(PDX_STDCALL) && defined(_STDCALL_SUPPORTED)
#define PDX_STDCALL __stdcall
#endif

#if !defined(PDX_FASTCALL) && defined(_FASTCALL_SUPPORTED)
#define PDX_FASTCALL __fastcall
#endif

#ifndef PDX_CDECL
#define PDX_CDECL
#endif

#ifndef PDX_STDCALL
#define PDX_STDCALL
#endif

#ifndef PDX_FASTCALL
#define PDX_FASTCALL
#endif

#ifndef PDX_NAKED
#define PDX_NAKED
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Force correct for-loop scoping 
//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#if _MSC_VER <= 1200 
#define PDX_FOR_FORCE_NONE_GLOBAL_SCOPE
#endif
#endif

#ifdef __INTEL_COMPILER
#if __INTEL_COMPILER >= 600
#undef PDX_FOR_FORCE_NONE_GLOBAL_SCOPE
#endif
#endif

#ifdef PDX_FOR_FORCE_NONE_GLOBAL_SCOPE
#undef PDX_FOR_FORCE_NONE_GLOBAL_SCOPE
#define for if(false) {} else for
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Enable some compiler instrinsic
//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER

 #if _MSC_VER >= 1200

  #define PDX_SSE_SUPPORT

 #endif
 
 #if _MSC_VER >= 1300

  #define PDX_ALIGN_16   __declspec(align(16))
  #define PDX_ALIGN_8    __declspec(align(8))
  #define PDX_ALIGN_1    __declspec(align(1))
  #define PDX_ASSUME(x)  __assume(x)
  #define PDX_NO_INLINE  __declspec(noinline)
  
 #endif

#endif

#ifdef __INTEL_COMPILER

 #undef  PDX_SSE_SUPPORT
 #define PDX_SSE_SUPPORT
 
 #ifndef PDX_ALIGN_16
 #define PDX_ALIGN_16 __declspec(align(16))
 #define PDX_ALIGN_8  __declspec(align(8))
 #define PDX_ALIGN_1  __declspec(align(1))
 #endif

 #define PDX_CPU_DISPATCH_SUPPORT

 #define PDX_CPU_TARGET(cpu)               __declspec(cpu_specific(cpu))
 #define PDX_CPU_DISPATCH(cpu1,cpu2)       __declspec(cpu_dispatch(cpu1,cpu2))
 #define PDX_CPU_DISPATCH2(cpu1,cpu2)      __declspec(cpu_dispatch(cpu1,cpu2))
 #define PDX_CPU_DISPATCH3(cpu1,cpu2,cpu3) __declspec(cpu_dispatch(cpu1,cpu2,cpu3))
 
 #if __INTEL_COMPILER >= 600
 
  #ifndef PDX_ASSUME
  #define PDX_ASSUME(x) __assume(x)
  #define PDX_NO_INLINE __declspec(noinline)
  #endif

  #define PDX_RESTRICT __restrict

 #endif

#endif

#ifndef PDX_ALIGN_1
#define PDX_ALIGN_1
#endif

#ifndef PDX_ALIGN_8
#define PDX_ALIGN_8
#endif

#ifndef PDX_ALIGN_16
#define PDX_ALIGN_16
#endif

#ifndef PDX_NO_INLINE
#define PDX_NO_INLINE
#endif

#ifndef PDX_RESTRICT
#define PDX_RESTRICT
#endif

#ifndef PDX_ASSUME
#define PDX_ASSUME(x)
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// SSE support
//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef PDX_SSE
 #ifdef PDX_SSE_SUPPORT

  #include <xmmintrin.h>
  #include <emmintrin.h>

  typedef __m128i M128I;
  typedef __m128  M128;
  typedef __m64   M64;

 #else

  #ifdef PDX_PRAGMA_PACK_SUPPORT
  #pragma pack(push,1)
  #endif

  struct M128  { F32 f[4]; };
  struct M128I { I32 i[4]; };
  struct M64   { F32 f[2]; };

  #ifdef PDX_PRAGMA_PACK_SUPPORT
  #pragma pack(pop)
  #endif

 #endif
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Error checkings
//////////////////////////////////////////////////////////////////////////////////////////////

enum PDXRESULT
{
	PDX_OK,
	PDX_FAILURE,
	PDX_BUSY,
	PDX_OUT_OF_MEMORY,
	PDX_FILE_NOT_FOUND,
	PDX_NOT_UPDATED
};

#define PDX_FAILED(result)    ((result) != PDX_OK)
#define PDX_SUCCEEDED(result) ((result) == PDX_OK)

#define PDX_TRY(function)                               \
{						                                \
	const PDXRESULT result = (function);                \
	if (PDX_FAILED(result)) return result;              \
}	 												    \
PDX_FORCE_SEMICOLON

//////////////////////////////////////////////////////////////////////////////////////////////
// Macro ## merger and " stringiziers
//////////////////////////////////////////////////////////////////////////////////////////////

#define PDX_PP_MERGE(x,y)   PDX_PP_MERGE_1(x,y)
#define PDX_PP_MERGE_1(x,y) PDX_PP_MERGE_2(x,y)
#define PDX_PP_MERGE_2(x,y) x##y

#define PDX_PP_STRINGIZE(x)   PDX_PP_STRINGIZE_1(x)
#define PDX_PP_STRINGIZE_1(x) #x

//////////////////////////////////////////////////////////////////////////////////////////////
// misc.
//////////////////////////////////////////////////////////////////////////////////////////////

#define PDX_NONE (-1)
#define PDX_FORCE_SEMICOLON typedef char PDX_PP_MERGE(pdx_force_semicolon_at_,__LINE__)

//////////////////////////////////////////////////////////////////////////////////////////////
// at least 32 bit unsigned integer
//////////////////////////////////////////////////////////////////////////////////////////////

union PDXWORD
{
	struct { U8 l,h,l2,h2; } b;
	struct { U16 l,h; } w;
	UINT d;
};

//////////////////////////////////////////////////////////////////////////////////////////////
// Casting
//////////////////////////////////////////////////////////////////////////////////////////////

#define PDX_CAST(t,v)        reinterpret_cast<t>((v))
#define PDX_CAST_REF(t,v)    (*(reinterpret_cast<t*>(&(v)))) 
#define PDX_CAST_PTR(t,v)    reinterpret_cast<t*>(&(v))
#define PDX_TO_CONST(t,v)    PDX_CAST_REF(const t,(v))
#define PDX_STATIC_CAST(t,v) static_cast<t>((v))

//////////////////////////////////////////////////////////////////////////////////////////////
// byte/bit utilities
//////////////////////////////////////////////////////////////////////////////////////////////

#define PDX_IS_ALIGNED(s,a)         (((s) >= (a)) && ((s) % (a) == 0))
#define PDX_IS_ADDRESS_ALIGNED(m,a) (ULONG(m) % (a) == 0)

#ifdef PDX_X86
#define PDX_IS_POWER_OF_TWO(i) (((i)&((i)-1)) == 0)
#define PDX_ALIGN_UP(s,a)      (((s)+(a)-1) & ~((a)-1))
#endif

#define PDX_MIN(x,y) ((x) < (y) ? (x) : (y))
#define PDX_MAX(x,y) ((x) < (y) ? (y) : (x))

#define PDX_CLAMP(x,min_,max_) PDX_MAX(PDX_MIN(x,max_),min_)

#ifdef __INTEL_COMPILER
#define PDX_TO_BOOL(exp) BOOL(bool(exp))
#else
#define PDX_TO_BOOL(exp) ((exp) ? 1 : 0)
#endif

#define PDX_PSEUDO_RANDOM(n) (7919 * ((n) + (sizeof(__FILE__) * __LINE__)) + 997)

//////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////

template<class T>
static VOID PDXMemZero(T& t)
{ 
	memset( &t, 0x00, sizeof(t) ); 
}

template<class T>
static VOID PDXMemZero(T* const t,const TSIZE length)
{
	memset( t, 0x00, sizeof(*t) * length); 
}

template<class T> 
static VOID PDXMemCopy(T& dst,const T& src)
{
	memcpy( &dst, &src, sizeof(T) );
}

template<class T>
static BOOL PDXMemCompare(const T& a,const T& b)
{
	return !memcmp( &a, &b, sizeof(T) );
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Dummy class, used by several template classes
//////////////////////////////////////////////////////////////////////////////////////////////

namespace PDX
{
	class DUMMY {};
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Class utilities
//////////////////////////////////////////////////////////////////////////////////////////////

namespace PDX
{
	class NON_COPYABLE_CLASS
	{
	protected:

		inline NON_COPYABLE_CLASS() {}
		inline ~NON_COPYABLE_CLASS() {}

	private:

		NON_COPYABLE_CLASS(const NON_COPYABLE_CLASS&);
		VOID operator = (const NON_COPYABLE_CLASS&) const;
	};

	class STATIC_CLASS
	{
	private:

		STATIC_CLASS();
		~STATIC_CLASS();
	};
}

#define PDX_STATIC_CLASS private PDX::STATIC_CLASS
#define PDX_NON_COPYABLE_CLASS private PDX::NON_COPYABLE_CLASS

#define PDX_DEFAULT_CONSTRUCTOR(ClassName) inline ClassName() {} 

#endif


