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

#ifndef PDXTYPEINFO_H
#define PDXTYPEINFO_H

#ifndef PDXLIBRARY_H
#error Do not include PdxTypeInfo.h directly!
#endif

typedef ULONG PDXTYPEID;

//////////////////////////////////////////////////////////////////////////////////////////////
// Default identifier class
//////////////////////////////////////////////////////////////////////////////////////////////

template<class TYPE=PDX::DUMMY> class PDXTYPE
{
public:

	enum { ID       = 0     };
	enum { INTEGRAL = FALSE };
	enum { POD      = FALSE };
	enum { SIGNED   = FALSE };
	enum { POINTER  = FALSE };

private:

	PDX_DEFAULT_CONSTRUCTOR(PDXTYPE)
};

//////////////////////////////////////////////////////////////////////////////////////////////
// Default identifier class for pointers
//////////////////////////////////////////////////////////////////////////////////////////////

#define PDX_DECLARE_TYPE_POINTER(t) 		  \
											  \
template<class T> class PDXTYPE<t>			  \
{											  \
public:										  \
										 	  \
	enum { ID       = 0                    }; \
	enum { INTEGRAL = TRUE                 }; \
	enum { SIZE     = sizeof(t)            }; \
	enum { BITS     = sizeof(t) * CHAR_BIT }; \
	enum { POD      = TRUE                 }; \
	enum { SIGNED   = FALSE                }; \
	enum { POINTER  = TRUE                 }; \
											  \
private:									  \
											  \
	PDX_DEFAULT_CONSTRUCTOR(PDXTYPE)		  \
};

PDX_DECLARE_TYPE_POINTER(T*)
PDX_DECLARE_TYPE_POINTER(T* const)
PDX_DECLARE_TYPE_POINTER(const T*)
PDX_DECLARE_TYPE_POINTER(const T* const)

#undef PDX_DECLARE_TYPE_POINTER

//////////////////////////////////////////////////////////////////////////////////////////////
// class declarator macro for integrals
//////////////////////////////////////////////////////////////////////////////////////////////

#define PDX_DECLARE_IDENTIFIER_CLASS_INT(type,sgn,name,id)  \
														    \
template<> class PDXTYPE<type>			         		    \
{														    \
public:													    \
														    \
    typedef type TYPE;									    \
														    \
    enum { ID       = id                      };		    \
    enum { SIGNED   = sgn                     }; 		    \
	enum { REAL     = FALSE                   }; 		    \
	enum { SIZE     = sizeof(type)            }; 		    \
	enum { BITS     = sizeof(type) * CHAR_BIT };		    \
    enum { INTEGRAL = TRUE                    };		    \
	enum { POD      = TRUE                    };		    \
	enum { POINTER  = FALSE                   };		    \
	static const TYPE MIN;                                  \
    static const TYPE MAX;		                            \
    static const CHAR NAME[];                         	    \
														    \
private:												    \
														    \
	PDX_DEFAULT_CONSTRUCTOR(PDXTYPE)					    \
};																	   

//////////////////////////////////////////////////////////////////////////////////////////////
// declaration of integral type classes
//////////////////////////////////////////////////////////////////////////////////////////////

PDX_DECLARE_IDENTIFIER_CLASS_INT( UCHAR,  FALSE, U, 1)
PDX_DECLARE_IDENTIFIER_CLASS_INT( CHAR,   TRUE,  I, 2)
PDX_DECLARE_IDENTIFIER_CLASS_INT( USHORT, FALSE, U, 3)
PDX_DECLARE_IDENTIFIER_CLASS_INT( SHORT,  TRUE,  I, 4)
PDX_DECLARE_IDENTIFIER_CLASS_INT( ULONG,  FALSE, U, 5)
PDX_DECLARE_IDENTIFIER_CLASS_INT( LONG,   TRUE,  I, 6) 
PDX_DECLARE_IDENTIFIER_CLASS_INT( UINT,   FALSE, U, 7)
PDX_DECLARE_IDENTIFIER_CLASS_INT( INT,    TRUE,  I, 8)

PDX_DECLARE_IDENTIFIER_CLASS_INT( const UCHAR,  FALSE, U, 1)
PDX_DECLARE_IDENTIFIER_CLASS_INT( const CHAR,   TRUE,  I, 2)
PDX_DECLARE_IDENTIFIER_CLASS_INT( const USHORT, FALSE, U, 3)
PDX_DECLARE_IDENTIFIER_CLASS_INT( const SHORT,  TRUE,  I, 4)
PDX_DECLARE_IDENTIFIER_CLASS_INT( const ULONG,  FALSE, U, 5)
PDX_DECLARE_IDENTIFIER_CLASS_INT( const LONG,   TRUE,  I, 6) 
PDX_DECLARE_IDENTIFIER_CLASS_INT( const UINT,   FALSE, U, 7)
PDX_DECLARE_IDENTIFIER_CLASS_INT( const INT,    TRUE,  I, 8)	

#ifdef PDX_I64_SUPPORT

 PDX_DECLARE_IDENTIFIER_CLASS_INT( U64, FALSE, U, 9)
 PDX_DECLARE_IDENTIFIER_CLASS_INT( I64, TRUE,  I, 10)

 PDX_DECLARE_IDENTIFIER_CLASS_INT( const U64, FALSE, U, 9)
 PDX_DECLARE_IDENTIFIER_CLASS_INT( const I64, TRUE,  I, 10)

#endif

#undef PDX_DECLARE_IDENTIFIER_CLASS_INT

//////////////////////////////////////////////////////////////////////////////////////////////
// class declarator macro for reals
//////////////////////////////////////////////////////////////////////////////////////////////

#define PDX_DECLARE_IDENTIFIER_CLASS_REAL(type,id)  \
												    \
template<> class PDXTYPE<type>			            \
{												    \
public:											    \
												    \
    typedef type TYPE;							    \
												    \
	enum { ID       = id                        };  \
	enum { SIGNED   = TRUE                      };  \
	enum { REAL     = TRUE                      };  \
	enum { SIZE     = sizeof(type)              };  \
	enum { BITS     = sizeof(type) * CHAR_BIT   };  \
    enum { INTEGRAL = TRUE                      };  \
    enum { POD      = TRUE                      };  \
	enum { POINTER  = FALSE                     };  \
	static const TYPE MAX;                          \
    static const TYPE MIN;                          \
    static const CHAR NAME[];                       \
												    \
private:										    \
												    \
	PDX_DEFAULT_CONSTRUCTOR(PDXTYPE)			    \
};

//////////////////////////////////////////////////////////////////////////////////////////////
// declaration of real type classes
//////////////////////////////////////////////////////////////////////////////////////////////

PDX_DECLARE_IDENTIFIER_CLASS_REAL( FLOAT,            11 )
PDX_DECLARE_IDENTIFIER_CLASS_REAL( DOUBLE,           12 )
PDX_DECLARE_IDENTIFIER_CLASS_REAL( LONGDOUBLE,       13 )
PDX_DECLARE_IDENTIFIER_CLASS_REAL( const FLOAT,      11 )
PDX_DECLARE_IDENTIFIER_CLASS_REAL( const DOUBLE,     12 )
PDX_DECLARE_IDENTIFIER_CLASS_REAL( const LONGDOUBLE, 13 )

#undef PDX_DECLARE_IDENTIFIER_CLASS_REAL

//////////////////////////////////////////////////////////////////////////////////////////////
// Store the type ID's
//////////////////////////////////////////////////////////////////////////////////////////////
											    
enum { PDX_CHAR_ID       = PDXTYPE<CHAR>::ID	   };
enum { PDX_UCHAR_ID      = PDXTYPE<UCHAR>::ID	   };
enum { PDX_SHORT_ID      = PDXTYPE<SHORT>::ID	   }; 
enum { PDX_USHORT_ID     = PDXTYPE<USHORT>::ID	   };
enum { PDX_LONG_ID       = PDXTYPE<LONG>::ID	   };
enum { PDX_ULONG_ID      = PDXTYPE<ULONG>::ID	   };
enum { PDX_INT_ID        = PDXTYPE<INT>::ID	       };
enum { PDX_UINT_ID       = PDXTYPE<UINT>::ID	   };
enum { PDX_FLOAT_ID      = PDXTYPE<FLOAT>::ID	   };
enum { PDX_DOUBLE_ID     = PDXTYPE<DOUBLE>::ID     };
enum { PDX_LONGDOUBLE_ID = PDXTYPE<LONGDOUBLE>::ID };
	   
#ifdef PDX_I64_SUPPORT
enum { PDX_I64_ID = PDXTYPE<I64>::ID };
enum { PDX_U64_ID = PDXTYPE<U64>::ID };
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Clean macros
//////////////////////////////////////////////////////////////////////////////////////////////

#define PDX_ID_OF(x)		PDXTYPE<x>::ID
#define PDX_IS_POINTER(x)   PDXTYPE<x>::POINTER
#define PDX_IS_POD(x)	    PDXTYPE<x>::POD
#define PDX_IS_INTEGRAL(x)  PDXTYPE<x>::INTEGRAL
#define PDX_IS_REAL(x)	    PDXTYPE<x>::REAL
#define PDX_IS_SIGNED(x)    PDXTYPE<x>::SIGNED
#define PDX_IS_UNSIGNED(x) !PDXTYPE<x>::SIGNED
#define PDX_MIN_OF(x)		PDXTYPE<x>::MIN
#define PDX_MAX_OF(x)		PDXTYPE<x>::MAX

//////////////////////////////////////////////////////////////////////////////////////////////
// Generate a random ID
//////////////////////////////////////////////////////////////////////////////////////////////

#define PDX_RANDOM_ID(type) PDX_PSEUDO_RANDOM(20+(sizeof(type)*sizeof(#type)))

//////////////////////////////////////////////////////////////////////////////////////////////
// Table class macro
//////////////////////////////////////////////////////////////////////////////////////////////

#define PDX_SET_TYPE_INFO(type,p)                                       \
											                            \
enum {PDX_PP_MERGE(PDX_PP_MERGE(PDX_,type),_ID) = PDX_RANDOM_ID(type)}; \
											                            \
template<> class PDXTYPE<type>			                                \
{											                            \
public:										                            \
											                            \
	enum { ID       = PDX_PP_MERGE(PDX_PP_MERGE(PDX_,type),_ID) };      \
	enum { SIZE     = sizeof(type)            };                        \
	enum { BITS     = sizeof(type) * CHAR_BIT };                        \
	enum { INTEGRAL = FALSE                   };                        \
	enum { POD      = p                       };                        \
    enum { SIGNED   = FALSE                   };                        \
    enum { POINTER  = FALSE                   };                        \
												                        \
    static inline const CHAR* Name()			                        \
	{											                        \
		return #type;							                        \
	}                                                                   \
												                        \
private:										                        \
												                        \
	PDX_DEFAULT_CONSTRUCTOR(PDXTYPE)			                        \
};

#endif

