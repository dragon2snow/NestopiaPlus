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

#ifndef PDXSTRING_H
#define PDXSTRING_H

#include <cstdlib>
#include <cctype>
#include "PdxArray.h"
#include "PdxUtilities.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// String class
//////////////////////////////////////////////////////////////////////////////////////////////

class PDXSTRING
{
public:

	typedef CHAR                  VALUE;
	typedef PDXARRAY<CHAR>        BUFFER;
	typedef BUFFER::ITERATOR      ITERATOR;
	typedef BUFFER::CONSTITERATOR CONSTITERATOR;

	enum
	{
		DEC = 10,
		HEX = 16,
		DIG = 5
	};

	PDX_DEFAULT_CONSTRUCTOR(PDXSTRING)

	PDXSTRING(const PDXSTRING& s)
	: buffer(s.buffer) {}

	PDXSTRING(CONSTITERATOR s)
	: buffer(s,&s[strlen(s)+1]) 
	{ PDX_ASSERT(buffer.Back() == '\0'); }

	PDXSTRING(CONSTITERATOR s,const TSIZE l)
	: buffer(s,&s[l]) { Terminate(); }

	PDXSTRING(CONSTITERATOR begin,CONSTITERATOR end)
	: buffer(begin,end) { Terminate(); }

	VOID Set(const PDXSTRING& s)
	{ buffer = s.buffer; }

	VOID Set(CONSTITERATOR s)
	{ PDX_ASSERT(s); Set(s,&s[strlen(s)]); }

	inline VOID Set(CONSTITERATOR s,const TSIZE l)
	{ Set(s,&s[l]); }

	VOID InsertBack(const CHAR);

	PDXSTRING& Append(CONSTITERATOR s)
	{ PDX_ASSERT(s); return Append(s,&s[strlen(s)]); }

	inline PDXSTRING& Append(CONSTITERATOR s,const TSIZE l)
	{ return Append(s,&s[l]); }

	PDXSTRING& operator = (const PDXSTRING& s)
	{ buffer = s.buffer; return *this; }

	inline PDXSTRING& operator = (CONSTITERATOR s)
	{ Set(s); return *this; }

	inline PDXSTRING& operator += (const PDXSTRING& s)
	{ return Append(s); }

	inline PDXSTRING& operator += (CONSTITERATOR s)
	{ return Append(s); }

	inline PDXSTRING operator + (const PDXSTRING& s) const
	{ return PDXSTRING(*this).Append(s); }

	inline PDXSTRING operator + (CONSTITERATOR s) const
	{ return PDXSTRING(*this).Append(s); }

	inline friend PDXSTRING operator + (PDXSTRING::CONSTITERATOR a,const PDXSTRING& b)
	{ return PDXSTRING(a).Append(b); }

	inline VOID Insert(const TSIZE p,const PDXSTRING& s)
	{ Insert(buffer.At(p),s); }

	VOID Insert(const TSIZE p,CONSTITERATOR s)
	{ Insert(buffer.At(p),s,&s[strlen(s)]); }

	inline VOID Insert(const TSIZE p,CONSTITERATOR s,const TSIZE l)
	{ Insert(buffer.At(p),s,&s[l]); }

	inline VOID Insert(const TSIZE p,CONSTITERATOR begin,CONSTITERATOR end)
	{ Insert(buffer.At(p),begin,end); }

	VOID Insert(ITERATOR p,CONSTITERATOR s)
	{ Insert(p,s,&s[strlen(s)]); }

	inline VOID Insert(ITERATOR p,CONSTITERATOR s,const TSIZE l)
	{ Insert(p,s,&s[l]); }

	inline PDXSTRING& operator << (const PDXSTRING& s)
	{ return Append(s); }

	inline PDXSTRING& operator << (CONSTITERATOR s)
	{ return Append(s); }

	PDXSTRING& Append (const PDXSTRING&);
	PDXSTRING& Append (CONSTITERATOR,CONSTITERATOR);

	VOID Set    (CONSTITERATOR,CONSTITERATOR);
	VOID Insert (ITERATOR,const PDXSTRING&);
	VOID Insert (ITERATOR,CONSTITERATOR,CONSTITERATOR);

	inline friend BOOL operator <  (CONSTITERATOR    a,const PDXSTRING& b) { return !(b < a) && !(b == a); }
	inline friend BOOL operator >  (const PDXSTRING& a,const PDXSTRING& b) { return b < a;                 }
	inline friend BOOL operator >  (const PDXSTRING& a,CONSTITERATOR    b) { return b < a;                 }
	inline friend BOOL operator >  (CONSTITERATOR    a,const PDXSTRING& b) { return b < a;                 }
	inline friend BOOL operator <= (const PDXSTRING& a,const PDXSTRING& b) { return (a < b) || (b == a);   }	
	inline friend BOOL operator <= (CONSTITERATOR    a,const PDXSTRING& b) { return !(b < a) || (b == a);  }	
	inline friend BOOL operator <= (const PDXSTRING& a,CONSTITERATOR    b) { return (a < b) || (a == b);   }
	inline friend BOOL operator >= (const PDXSTRING& a,const PDXSTRING& b) { return (a > b) || (b == a);   }
	inline friend BOOL operator >= (CONSTITERATOR    a,const PDXSTRING& b) { return (b < a) || (b == a);   }
	inline friend BOOL operator >= (const PDXSTRING& a,CONSTITERATOR    b) { return (a > b) || (a == b);   }
	inline friend BOOL operator == (CONSTITERATOR    a,const PDXSTRING& b) { return b == a;                }
	inline friend BOOL operator != (const PDXSTRING& a,const PDXSTRING& b) { return !(a == b);             }
	inline friend BOOL operator != (CONSTITERATOR    a,const PDXSTRING& b) { return !(b == a);             }
	inline friend BOOL operator != (const PDXSTRING& a,CONSTITERATOR    b) { return !(a == b);             }

	BOOL operator <	 (CONSTITERATOR)    const;
	BOOL operator == (const PDXSTRING&) const;
	BOOL operator == (CONSTITERATOR)    const;

	inline BOOL operator < (const PDXSTRING& a) const
	{ return (*this) < a.String(); }

	BOOL ExactEqual (const PDXSTRING&) const;
	BOOL ExactEqual (CONSTITERATOR)    const;

	VOID SetLowerCase();
	VOID SetUpperCase();

    #define PDX_AT_DECLARATION(name)                                                           \
																							   \
	static ITERATOR name(CONSTITERATOR a,CONSTITERATOR b,ITERATOR c,CONSTITERATOR d)		   \
	{ return &c[PDX_PP_MERGE(Pos,name)(a,b,c,d)]; }											   \
																							   \
	static CONSTITERATOR name(CONSTITERATOR a,CONSTITERATOR b,CONSTITERATOR c,CONSTITERATOR d) \
	{ return &c[PDX_PP_MERGE(Pos,name)(a,b,c,d)]; }											   \
																							   \
	ITERATOR name(CONSTITERATOR a,CONSTITERATOR b)											   \
	{ return At(PDX_PP_MERGE(Pos,name)(a,b,Begin(),End())); }								   \
																							   \
	ITERATOR name(CONSTITERATOR a)          												   \
	{ return At(PDX_PP_MERGE(Pos,name)(a,&a[strlen(a)],Begin(),End())); }					   \
																							   \
	CONSTITERATOR name(CONSTITERATOR a,CONSTITERATOR b) const								   \
	{ return At(PDX_PP_MERGE(Pos,name)(a,b,Begin(),End())); }								   \
																							   \
	CONSTITERATOR name(CONSTITERATOR a) const												   \
	{ return At(PDX_PP_MERGE(Pos,name)(a,&a[strlen(a)],Begin(),End())); }
	
	PDX_AT_DECLARATION(AtFirstOf)
	PDX_AT_DECLARATION(AtLastOf)																   

    #undef PDX_AT_DECLARATION

	VOID Replace(ITERATOR,CONSTITERATOR,CONSTITERATOR,CONSTITERATOR);
	
	VOID Replace(ITERATOR a,CONSTITERATOR b,CONSTITERATOR c)
	{ Replace(a,b,c,&c[strlen(c)]); }

	VOID Replace(ITERATOR a,CONSTITERATOR b)
	{ Replace(a,&a[strlen(a)],b,&b[strlen(b)]); }

	PDXSTRING& GetFilePath      (PDXSTRING&) const;
	PDXSTRING& GetFileName      (PDXSTRING&) const;
	PDXSTRING& GetFileExtension (PDXSTRING&) const;

	inline PDXSTRING GetFilePath() const
	{
		PDXSTRING tmp;
		GetFilePath(tmp);
		return tmp;
	}

	inline PDXSTRING GetFileName() const
	{
		PDXSTRING tmp;
		GetFileName(tmp);
		return tmp;
	}

	inline PDXSTRING GetFileExtension() const
	{
		PDXSTRING tmp;
		GetFileExtension(tmp);
		return tmp;
	}

	BOOL ReplaceFileExtension (CONSTITERATOR);
	BOOL ReplaceFilePath      (const PDXSTRING&);
	BOOL IsFileExtension      (const CHAR*,const TSIZE=0) const;

	PDXSTRING(const INT   v,const UINT r=DEC) { Append( v, r ); }
	PDXSTRING(const UINT  v,const UINT r=DEC) { Append(	v, r ); }
	PDXSTRING(const LONG  v,const UINT r=DEC) { Append(	v, r ); }
	PDXSTRING(const ULONG v,const UINT r=DEC) { Append(	v, r ); }
	PDXSTRING(const FLOAT v,const UINT r=DIG) { Append(	v, r ); }

	VOID Set(const INT   v,const UINT r=DEC) { Clear(); Append(	v, r ); }
	VOID Set(const UINT  v,const UINT r=DEC) { Clear(); Append(	v, r ); }
	VOID Set(const LONG  v,const UINT r=DEC) { Clear(); Append(	v, r ); }
	VOID Set(const ULONG v,const UINT r=DEC) { Clear(); Append(	v, r ); }
	VOID Set(const FLOAT v,const UINT r=DIG) { Clear(); Append(	v, r ); }

	PDXSTRING& Append(const INT,  const UINT=DEC);
	PDXSTRING& Append(const UINT, const UINT=DEC);
	PDXSTRING& Append(const FLOAT,const UINT=DIG);

	PDXSTRING& Append(const LONG  v,const UINT r=DEC) { return Append( INT(v),  r ); }
	PDXSTRING& Append(const ULONG v,const UINT r=DEC) { return Append( UINT(v), r ); }

	VOID Insert(ITERATOR a,const INT   b,const UINT c=DEC) { const PDXSTRING d(b,c); Insert(a,d); }
	VOID Insert(ITERATOR a,const UINT  b,const UINT c=DEC) { const PDXSTRING d(b,c); Insert(a,d); }
	VOID Insert(ITERATOR a,const LONG  b,const UINT c=DEC) { const PDXSTRING d(b,c); Insert(a,d); }
	VOID Insert(ITERATOR a,const ULONG b,const UINT c=DEC) { const PDXSTRING d(b,c); Insert(a,d); }
	VOID Insert(ITERATOR a,const FLOAT b,const UINT c=DIG) { const PDXSTRING d(b,c); Insert(a,d); }

	VOID Insert(const TSIZE a,const INT   b,const UINT c=DEC) { const PDXSTRING d(b,c); Insert(At(a),d); }
	VOID Insert(const TSIZE a,const UINT  b,const UINT c=DEC) { const PDXSTRING d(b,c); Insert(At(a),d); }
	VOID Insert(const TSIZE a,const LONG  b,const UINT c=DEC) { const PDXSTRING d(b,c); Insert(At(a),d); }
	VOID Insert(const TSIZE a,const ULONG b,const UINT c=DEC) { const PDXSTRING d(b,c); Insert(At(a),d); }
	VOID Insert(const TSIZE a,const FLOAT b,const UINT c=DIG) { const PDXSTRING d(b,c); Insert(At(a),d); }

	inline PDXSTRING& operator = (const INT   v) { Set(v); return *this; }
	inline PDXSTRING& operator = (const UINT  v) { Set(v); return *this; }
	inline PDXSTRING& operator = (const LONG  v) { Set(v); return *this; }
	inline PDXSTRING& operator = (const ULONG v) { Set(v); return *this; }
	inline PDXSTRING& operator = (const FLOAT v) { Set(v); return *this; }

	inline PDXSTRING& operator += (const INT   v) { Append(v); return *this; }
	inline PDXSTRING& operator += (const UINT  v) { Append(v); return *this; }
	inline PDXSTRING& operator += (const LONG  v) { Append(v); return *this; }
	inline PDXSTRING& operator += (const ULONG v) { Append(v); return *this; }
	inline PDXSTRING& operator += (const FLOAT v) { Append(v); return *this; }

	inline PDXSTRING& operator << (const INT   v) { Append(v); return *this; }
	inline PDXSTRING& operator << (const UINT  v) { Append(v); return *this; }
	inline PDXSTRING& operator << (const LONG  v) { Append(v); return *this; }
	inline PDXSTRING& operator << (const ULONG v) { Append(v); return *this; }
	inline PDXSTRING& operator << (const FLOAT v) { Append(v); return *this; }

	inline PDXSTRING operator + (const INT   v) const { return PDXSTRING(*this).Append(v); }
	inline PDXSTRING operator + (const UINT  v) const { return PDXSTRING(*this).Append(v); }
	inline PDXSTRING operator + (const LONG  v) const { return PDXSTRING(*this).Append(v); }
	inline PDXSTRING operator + (const ULONG v) const { return PDXSTRING(*this).Append(v); }
	inline PDXSTRING operator + (const FLOAT v) const { return PDXSTRING(*this).Append(v); }

	inline friend PDXSTRING operator + (const INT   v,const PDXSTRING& s) { return PDXSTRING(v).Append(s); }
	inline friend PDXSTRING operator + (const UINT  v,const PDXSTRING& s) { return PDXSTRING(v).Append(s); }
	inline friend PDXSTRING operator + (const LONG  v,const PDXSTRING& s) { return PDXSTRING(v).Append(s); }
	inline friend PDXSTRING operator + (const ULONG v,const PDXSTRING& s) { return PDXSTRING(v).Append(s); }
	inline friend PDXSTRING operator + (const FLOAT v,const PDXSTRING& s) { return PDXSTRING(v).Append(s); }

	VOID Resize (const TSIZE,const CHAR=' ');
	VOID Grow   (const TSIZE,const CHAR=' ');
	
	VOID EraseBack();
	VOID RemoveSpaces();
	VOID RemoveQuotes();

	ULONG ToUlong() const;
	PDXSTRING Quoted() const;

	VOID Validate();

	inline PDXSTRING SubString(CONSTITERATOR a,CONSTITERATOR b) const
	{ return PDXSTRING(a,b); }

	inline PDXSTRING operator() (CONSTITERATOR a,CONSTITERATOR b) const
	{ return PDXSTRING(a,b); }

	inline PDXSTRING SubString(const TSIZE a,const TSIZE b) const
	{ return PDXSTRING(At(a),At(b)); }

	inline PDXSTRING operator() (const TSIZE a,const TSIZE b) const
	{ return PDXSTRING(At(a),At(b)); }

	inline TSIZE NumLines() const
	{ return buffer.Size() ? PDX::Count(buffer.Begin(),buffer.End(),'\n') : 0; }

	inline TSIZE Size() const
	{ return PDX::Max(0,LONG(buffer.Size()) - 1); }

	inline TSIZE Length() const
	{ return PDX::Max(0,LONG(buffer.Size()) - 1); }

	inline TSIZE Capacity() const
	{ return buffer.Capacity(); }

	inline const CHAR* String() const
	{ 
		PDX_ASSERT(nill == '\0');
		return buffer.Size() ? buffer.Begin() : &nill;  
	}

	inline const CHAR* String(const TSIZE i) const
	{ 
		PDX_ASSERT(nill == '\0');
		return buffer.Size() ? buffer.At(i) : &nill; 
	}

	inline const CHAR& operator [] (const TSIZE i) const
	{ 
		PDX_ASSERT(nill == '\0');
		return buffer.Size() ? buffer[i] : nill; 
	}

	inline CHAR& operator [] (const TSIZE i)
	{ 
		PDX_ASSERT(nill == '\0');
		return buffer.Size() ? buffer[i] : nill; 
	}
  
	inline CHAR& Front()
	{ 
		PDX_ASSERT(buffer.Size() >= 2); 
		return buffer.Front(); 
	}

	inline const CHAR& Front() const
	{ 
		PDX_ASSERT(buffer.Size() >= 2); 
		return buffer.Front(); 
	}

	inline CHAR& Back()
	{ 
		PDX_ASSERT(buffer.Size() >= 2); 
		return buffer[buffer.Size()-2]; 
	}

	inline const CHAR& Back() const
	{ 
		PDX_ASSERT(buffer.Size() >= 2); 
		return buffer[buffer.Size()-2]; 
	}

	inline ITERATOR Begin()
	{ 
		PDX_ASSERT(nill == '\0');
		return buffer.Size() ? buffer.Begin() : &nill; 
	}

	inline CONSTITERATOR Begin() const
	{ 
		PDX_ASSERT(nill == '\0');
		return buffer.Size() ? buffer.Begin() : &nill; 
	}

	inline ITERATOR At(const TSIZE i)
	{ 
		PDX_ASSERT(nill == '\0');
		return buffer.Size() ? buffer.At(i) : &nill; 
	}

	inline CONSTITERATOR At(const TSIZE i) const
	{ 
		PDX_ASSERT(nill == '\0');
		return buffer.Size() ? buffer.At(i) : &nill;
	}

	inline ITERATOR End()
	{ 
		PDX_ASSERT(nill == '\0');
		return buffer.Size() ? buffer.End()-1 : &nill; 
	}

	inline CONSTITERATOR End() const
	{ 
		PDX_ASSERT(nill == '\0');
		return buffer.Size() ? buffer.End()-1 : &nill; 
	}

	inline BOOL IsEmpty() const
	{ return Length() == 0; }

	inline VOID Reserve(const TSIZE size)
	{ buffer.Reserve(size+1); }

	inline VOID Defrag()
	{ buffer.Defrag(); }

	inline VOID Clear()
	{ buffer.Clear(); }

	inline VOID Destroy()
	{ buffer.Destroy(); }

	inline const BUFFER& Buffer() const
	{ return buffer; }

	inline BUFFER& Buffer()
	{ return buffer; }

	static inline CHAR ToLower(const CHAR c) { return tolower(c); }
	static inline CHAR ToUpper(const CHAR c) { return toupper(c); }
	static inline CHAR IsLower(const CHAR c) { return islower(c); }
	static inline CHAR IsUpper(const CHAR c) { return isupper(c); }

private:

	static TSIZE PosAtFirstOf (CONSTITERATOR,CONSTITERATOR,CONSTITERATOR,CONSTITERATOR);
	static TSIZE PosAtLastOf  (CONSTITERATOR,CONSTITERATOR,CONSTITERATOR,CONSTITERATOR);

	VOID Terminate()
	{
		PDX_ASSERT(nill == '\0');

		if (buffer.Back() != '\0')
			buffer.InsertBack('\0');
	}

	enum
	{
		STRING_INT_SIZE   = 48,
		STRING_FLOAT_SIZE = 48
	};

	struct CONVERTER
	{
		enum {SIZE=1+CHAR_MAX};

		CONVERTER()
		{
			for (UINT i=0; i < SIZE; ++i)
			{
				lower[i] = tolower(i);
				upper[i] = toupper(i);
			}
		}

		CHAR lower[SIZE];
		CHAR upper[SIZE];
	};

	BUFFER buffer;

	static CHAR nill;
	static const CONVERTER converter;
};

#endif

