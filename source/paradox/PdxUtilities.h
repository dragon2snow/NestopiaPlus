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

#ifndef PDXUTILITIES_H
#define PDXUTILITIES_H

#include <cmath>
#include "PDXLibrary.h"
#include "PDXPair.h"

namespace PDX
{
	template<class A,class R=VOID> struct Unary 
	{
		typedef A FIRST;
		typedef R RESULT;
	};

	template<class A,class B,class R=VOID> struct Binary 
	{
		typedef A FIRST;
		typedef B SECOND;
		typedef R RESULT;
	};

	template<class A,class B=A,class R=BOOL> struct True : public Binary<A,B,R>
	{
		inline R operator () (const A&)          const { return TRUE; }
		inline R operator () (const A&,const B&) const { return TRUE; }
	};

	template<class A,class B=A,class R=BOOL> struct False : public Binary<A,B,R>
	{
		inline R operator () (const A&)          const { return FALSE; }
		inline R operator () (const A&,const B&) const { return FALSE; }
	};

	template<class A> struct Not : public Unary<A,BOOL>
	{
		inline BOOL operator() (const A& a) const
		{
			return (!a);
		}
	};

	template<class A,class B=A> struct And : public Binary<A,B,BOOL>
	{
		inline BOOL operator() (const A& a,const B& b) const
		{
			return (a && b);
		}
	};

	template<class A,class B=A> struct Or : public Binary<A,B,BOOL>
	{
		inline BOOL operator() (const A& a,const B& b) const
		{
			return (!a);
		}
	};

	template<class A=INT,class B=A> struct Equal : public Binary<A,B,BOOL>
	{
		inline BOOL operator() (const A& a,const B& b) const
		{
			return (a == b);
		}

		template<class T,class U> inline BOOL operator() (const T& a,const U& b) const
		{
			return (a == b);
		}
	};

	template<class A,class B=A> struct NotEqual : public Binary<A,B,BOOL>
	{
		inline BOOL operator() (const A& a,const B& b) const
		{
			return !(a == b);
		}
	};

	template<class A=INT,class B=A> struct Less : public Binary<A,B,BOOL>
	{
		inline BOOL operator() (const A& a,const B& b) const
		{
			return (a < b);
		}

		template<class T,class U> inline BOOL operator() (const T& a,const U& b) const
		{
			return (a < b);
		}
	};

	template<class A,class B=A> struct LessOrEqual : public Binary<A,B,BOOL>
	{
		inline BOOL operator() (const A& a,const B& b) const
		{
			return (a <= b);
		}
	};

	template<class A,class B=A> struct Greater : public Binary<A,B,BOOL>
	{
		inline BOOL operator() (const A& a,const B& b) const
		{
			return (b < a);
		}
	};

	template<class A,class B=A> struct GreaterOrEqual : public Binary<A,B,BOOL>
	{
		inline BOOL operator() (const A& a,const B& b) const
		{
			return (b <= a);
		}
	};

	template<class F,class R=F::RESULT> struct Dereference : public Unary<F,R>
	{
		template<class A> inline R operator() (A& a)                           const { return F()(*a);    }
		template<class A> inline R operator() (const A& a)                     const { return F()(*a);    }
		template<class A,class B> inline R operator () (A& a,B& b)             const { return F()(*a,*b); }
		template<class A,class B> inline R operator () (const A& a,const B& b) const { return F()(*a,*b); }
	};

	template<class F,class R=VOID> class DereferenceFunctionClass
	{
	public:

		inline DereferenceFunctionClass(F f)
		: function(f) {}

		template<class A> inline R operator() (A& a)                           const { return function(*a);    }
		template<class A> inline R operator() (const A& a)                     const { return function(*a);    }
		template<class A,class B> inline R operator () (A& a,B& b)             const { return function(*a,*b); }
		template<class A,class B> inline R operator () (const A& a,const B& b) const { return function(*a,*b); }

	private:

		F function;
	};

	template<class F> inline DereferenceFunctionClass<F> DereferenceFunction(F f)
	{
		return DereferenceFunctionClass<F>(f);
	}

	template<class F,class R> inline DereferenceFunctionClass<F,R> DereferenceFunction(F f,const R&)
	{
		return DereferenceFunctionClass<F,R>(f);
	}

	template<class F,class R=VOID> class DereferenceMemberFunctionClass
	{
	public:

		inline DereferenceMemberFunctionClass(F f)
		: function(f) {}

		template<class A> inline R operator() (A& a) const                           { return ((*a).*function)();  }
		template<class A> inline R operator() (const A& a) const                     { return ((*a).*function)();  }
		template<class A,class B> inline R operator () (A& a,B& b) const             { return ((*a).*function)(b); }
		template<class A,class B> inline R operator () (const A& a,const B& b) const { return ((*a).*function)(b); }

	private:

		F function;
	};

	template<class F> inline DereferenceMemberFunctionClass<F> DereferenceMemberFunction(F f)
	{
		return DereferenceMemberFunctionClass<F>(f);
	}

	template<class F,class R> inline DereferenceMemberFunctionClass<F,R> DereferenceMemberFunction(F f,const R&)
	{
		return DereferenceMemberFunctionClass<F,R>(f);
	}

	template<class A,class B=A> struct New : public Binary<A,B,A>
	{
		A* operator() () const           { return PDX_NEW A;    }
		A* operator() (const B& b) const { return PDX_NEW A(b); }
	};

	template<class A> struct Delete : public Unary<A>
	{
		VOID operator() (A*& a) const       { PDX_DELETE a; }
		VOID operator() (A* const& a) const { PDX_DELETE a; }
	};

	template<class A,class B=UINT> struct Release : public Unary<A,B>
	{
		inline RESULT operator() (A& a) const 
		{ 
			return a.Release(); 
		}
	};

	template<class T> 
	inline const T& Min(const T& a,const T& b) 
	{ 
		return (a < b) ? a : b; 
	}

	template<class T> 
	inline const T& Max(const T& a,const T& b) 
	{ 
		return (b < a) ? a : b; 
	}

	template<class T,class U> 
	inline T Min(const T a,const U b) 
	{ 
		return (a < b) ? a : T(b); 
	}

	template<class T,class U> 
	inline T Max(const T a,const U b) 
	{ 
		return (b < a) ? a : T(b); 
	}
	
    template<class T> 
	inline const T& Min(const T& a,const T& b,const T& c) 
	{ 
		return Min(Min(a,b),c); 
	}
    
	template<class T> 
	inline const T& Max(const T& a,const T& b,const T& c) 
	{ 
		return Max(Max(a,b),c); 
    }
	
	template<class T> 
	inline const T& Min(const T& a,const T& b,const T& c,const T& d) 
	{ 
    	return Min(Min(a,b),Min(c,d)); 
	}
	
	template<class T> 
	inline const T& Max(const T& a,const T& b,const T& c,const T& d) 
	{ 
		return Max(Max(a,b),Max(c,d)); 
	}
    
	template<class T> 
	inline VOID Swap(T& a,T& b)
	{
		CHAR c[sizeof(T)];
		memcpy(&c,&a,sizeof(T));
		memcpy(&a,&b,sizeof(T));
		memcpy(&b,&c,sizeof(T));
	}

	template<class T> 
	inline VOID Swap(T*& a,T*& b) 
	{ 
		T* const c=a; a=b; b=c; 
	}

	template<> inline VOID Swap< INT        >( INT&        a, INT&        b) { const INT    c=a; a=b; b=c; }
	template<> inline VOID Swap< UINT       >( UINT&       a, UINT&       b) { const UINT   c=a; a=b; b=c; }
	template<> inline VOID Swap< CHAR       >( CHAR&       a, CHAR&       b) { const CHAR   c=a; a=b; b=c; }
	template<> inline VOID Swap< UCHAR      >( UCHAR&      a, UCHAR&      b) { const UCHAR  c=a; a=b; b=c; }
	template<> inline VOID Swap< SHORT      >( SHORT&      a, SHORT&      b) { const SHORT  c=a; a=b; b=c; }
	template<> inline VOID Swap< USHORT     >( USHORT&     a, USHORT&     b) { const USHORT c=a; a=b; b=c; }
	template<> inline VOID Swap< LONG       >( LONG&       a, LONG&       b) { const LONG   c=a; a=b; b=c; }
	template<> inline VOID Swap< ULONG      >( ULONG&      a, ULONG&      b) { const ULONG  c=a; a=b; b=c; }
	template<> inline VOID Swap< FLOAT      >( FLOAT&      a, FLOAT&      b) { const FLOAT  c=a; a=b; b=c; }
	template<> inline VOID Swap< DOUBLE     >( DOUBLE&     a, DOUBLE&     b) { const DOUBLE c=a; a=b; b=c; }
	template<> inline VOID Swap< LONGDOUBLE >( LONGDOUBLE& a, LONGDOUBLE& b) { const DOUBLE c=a; a=b; b=c; }
	
    template<class T> 
	inline T Clamp(T t,const T minvalue=0,const T maxvalue=1) 
	{ 
		return (t < minvalue) ? minvalue : (maxvalue < t) ? maxvalue : t; 
	}
    
	template<class T> 
	inline T ClampLow(const T t,const T minvalue=0)
	{ 
		return (t < minvalue) ? minvalue : t;                   
	}
	
	template<class T> 
	inline T ClampHigh(const T t,const T maxvalue=1)       
    { 
		return (maxvalue < t) ? maxvalue : t;                   
	}
	
	template<class T> 
	inline T Abs(const T t)
	{
		return (t < 0) ? -t : t; 
    }

	template<> inline CHAR   Abs<CHAR>   (const CHAR   t) { return abs(INT(t)); }
	template<> inline SHORT  Abs<SHORT>  (const SHORT  t) { return abs(INT(t)); }
	template<> inline LONG   Abs<LONG>   (const LONG   t) { return labs(t);     }
	template<> inline INT    Abs<INT>    (const INT    t) { return abs(t);      } 
	template<> inline FLOAT  Abs<FLOAT>  (const FLOAT  t) { return fabsf(t);    }
	template<> inline DOUBLE Abs<DOUBLE> (const DOUBLE t) { return fabs(t);     }
	
	template<class T> inline T AbsMin(const T a,const T b) { return Min(Abs(a),Abs(b)); }
	template<class T> inline T AbsMax(const T a,const T b) { return Max(Abs(a),Abs(b)); }
    
	template<class T,class U> 
	T Pow(T a,const U& b)
	{
		for (U i(0); i < b; ++i)
    		a *= a;
	
		return a;
	}
	
	inline FLOAT Round(FLOAT f,const INT factor)
	{
    	FLOAT r = FLOAT(INT(f * factor)) / factor;
		
		f -= r;
	
     	if (f * factor > 0.5f)
     		r += 1.f / factor;

     	return r;
	}
	
	inline FLOAT Fraction(const FLOAT f)
    {
		FLOAT i;
		return modff(f,&i);
    }

	inline DOUBLE Fraction(const DOUBLE d)
	{
		DOUBLE i;
		return modf(d,&i);
	}

    template<class T,class U,class V> 
	inline BOOL InRange(const T& value,const U& low,const V& high)
	{
		return value >= low && value <= high;
	}
    
	template<class A,class B,class C,class D> 
	BOOL EqualRange(A first1,const B last1,C first2,const D last2)
    {
		for (; first1 != last1 && first2 != last2; ++first1, ++first2)
			if (!((*first1) == (*first2)))
				return FALSE;

		return (first1 == last1 && first2 == last2);
	}
    
	template<class A,class B,class C,class D> 
	inline BOOL NotEqualRange(A a,const B b,C c,const D d)
	{
		return !EqualRange(a,b,c,d);
	}
	
	template<class T> 
	inline BOOL AlmostEqual(const T& a,const T& b,const FLOAT epsilon=1.e-4f)
    {
		return Abs(a-b) < epsilon * (Abs(a) + 1);
	}

	template<class T> 
	inline BOOL AlmostZero(const T& t,const FLOAT epsilon=1.e-4f)
	{
		return (t < epsilon) && (t > -epsilon);
    }
	
	template<class T> 
	inline BOOL AlmostOne(const T& t,const FLOAT epsilon=1.e-4f)
	{
    	return (t < (1.f+epsilon)) && (t > (1.f-epsilon));
	}
	
	template<class I,class J,class F> 
	VOID ForEach(I begin,const J end,F function)
	{
		PDX_ASSERT(ITERATOR::Valid(begin,end));

		for (; begin != end; ++begin)
			function(*begin);
	}
    
	template<class I,class J,class F> 
	VOID ForEachIn(I begin,const J end,F function)
	{
		PDX_ASSERT(ITERATOR::Valid(begin,end));

		for (; begin != end; ++begin)
			((*begin).*function)();
	}
    
	template<class I,class J,class F,class P> 
	VOID ForEachIn(I begin,const J end,F function,P& parameter)
	{
		PDX_ASSERT(ITERATOR::Valid(begin,end));

		for (; begin != end; ++begin)
			((*begin).*function)(parameter);
    }
	
    template<class I,class J,class V> 
	VOID Fill(I begin,const J end,const V& value)
	{
		PDX_ASSERT(ITERATOR::Valid(begin,end));

		for (; begin != end; ++begin)
			(*begin) = value;
	}

    #define PDX_FILL_(t,f)						\
												\
	template<class V>							\
	VOID Fill(t begin,f end,const V& value)		\
	{											\
		PDX_ASSERT(ITERATOR::Valid(begin,end));	\
		memset( begin, value, end - begin );	\
	}											

	PDX_FILL_(CHAR*,CHAR*)
	PDX_FILL_(CHAR*,const CHAR*)
	PDX_FILL_(UCHAR*,UCHAR*)
	PDX_FILL_(UCHAR*,const UCHAR*)

    #undef PDX_FILL_

	template<class I,class J,class F> 
	I Transform(I begin,const J end,F function)
	{
		PDX_ASSERT(ITERATOR::Valid(begin,end));

		for (TSIZE i=0; begin != end; ++begin, ++i)
			*begin = function(i);

		return begin;
	}

	template<class I,class J,class K,class L,class F> 
	I Transform(I begin,const J end,K opbegin,const L opend,F function)
	{
		PDX_ASSERT(ITERATOR::Valid(begin,end) && ITERATOR::Valid(opbegin,opend));

		for (; begin != end && opbegin != opend; ++begin, ++opbegin)
			(*begin) = function(*opbegin);

		return begin;
	}
	
	namespace HIDDEN
	{
     	template<class T> 
		inline Equal<T> GetEqual(const T&)
		{
			return Equal<T>();
		}

		template<class T> 
		inline Less<T> GetLess(const T&)
		{
			return Less<T>();
		}
	}

    template<class I,class J,class V,class P> 
	I Find(I begin,const J end,const V& value,P match)
	{
		PDX_ASSERT(ITERATOR::Valid(begin,end));

		for (; begin != end; ++begin)
			if (match(*begin,value))
				return begin;

		return begin;
	}

    template<class I,class J,class V> 
	inline I Find(I begin,const J end,const V& value)
	{
		return Find(begin,end,value,HIDDEN::GetEqual(*begin));
	}
	
    template<class I,class J,class V,class F,class P> 
	I FindIf(I begin,const J end,const V& value,F function,P match)
	{
		PDX_ASSERT(ITERATOR::Valid(begin,end));

		for (; begin != end; ++begin)
			if (match(function(*begin),value))
				return begin;

		return begin;
	}

    template<class I,class J,class V,class F> 
	inline I FindIf(I begin,const J end,const V& value,F function)
	{
		return Find(begin,end,value,HIDDEN::GetEqual(*begin));
	}
    	
	template<class I,class J,class V,class M> 
	TSIZE Count(I begin,const J end,const V& value,M match)
	{
		PDX_ASSERT(ITERATOR::Valid(begin,end));

		TSIZE count=0;

		for (; begin != end; ++begin)
			count += PDX_TO_BOOL(match(*begin,value));

		return count;
	}

	template<class I,class J,class V> 
	inline TSIZE Count(I begin,const J end,const V& value)
	{
		return Count(begin,end,value,HIDDEN::GetEqual(*begin));
	}

	template<class I,class J,class V>
	VOID Set(I begin,const J end,const V& value)
	{
		PDX_ASSERT(ITERATOR::Valid(begin,end));

		for (; begin != end; ++begin)
			*begin = value;
	}

	template<class I,class J,class K>
	VOID Copy(I to,J from,const K end)
	{
		PDX_ASSERT(ITERATOR::Valid(from,end));

		for (; from != end; ++to, ++from)
			*to = *from;
	}

    template<class I,class J,class P> 
	I FindMin(I begin,const J end,P predicate)
	{
		PDX_ASSERT(ITERATOR::Valid(begin,end));

		I MinIt(begin);

		if (begin != end)
		{
     		for (++begin; begin != end; ++begin)
     			if (predicate(*begin,*MinIt))
    				MinIt = begin;
		}

		return MinIt;
	}

    template<class I,class J> 
	inline I FindMin(I begin,const J end)
	{
		return FindMin(begin,end,HIDDEN::GetLess(*begin));
	}
	
    template<class I,class J,class P> 
	I FindMax(I begin,const J end,P predicate)
	{
		PDX_ASSERT(ITERATOR::Valid(begin,end));

		I MaxIt(begin);

		if (begin != end)
		{
     		for (++begin; begin != end; ++begin)
     			if (predicate(*MaxIt,*begin))
    				MaxIt = begin;
		}

		return MaxIt;
	}

    template<class I,class J> 
	inline I FindMax(I begin,const J end)
	{
		return FindMax(begin,end,HIDDEN::GetLess(*begin));
	}

	template<class I,class J,class P>
	PDXPAIR<I,I> FindMinMax(I begin,const J end,P predicate)
	{
		PDX_ASSERT(ITERATOR::Valid(begin,end));

		PDXPAIR<I,I> MinMaxIt(begin,begin);

		if (begin != end)
		{
			for (++begin; begin != end; ++begin)
			{
				if (predicate(*begin,*MinMaxIt.First()))
					MinMaxIt.First() = begin;

				if (predicate(*MinMaxIt.Second(),*begin))
					MinMaxIt.Second() = begin;
			}
		}

		return MinMaxIt;
	}

	template<class I,class J> 
	inline PDXPAIR<I,I> FindMinMax(I begin,const J end)
	{
		return FindMinMax(begin,end,HIDDEN::GetLess(*begin));
	}

    template<class I,class J,class V,class P,class M> 
	static I BinarySearch(I begin,const J end,const V& value,P predicate,M match)
	{
		PDX_ASSERT(ITERATOR::Valid(begin,end));

		LONG last = (end - begin) - 1;
	
		if (last == -1)
			return I(NULL);

    	LONG first = 0;
		LONG middle;

       	while (first <= last)
		{
			middle = (first+last) / 2;
	
       		if (predicate(begin[middle],value))
			{
	   			first = middle + 1;
			}
       		else if (predicate(value,begin[middle]))
			{
	   			last = middle - 1;
			}
       		else
			{
				break;
			}
    	}

		return match(begin[middle],value) ? I(&begin[middle]) : I(NULL);
	}

    template<class I,class J,class V,class P> 
	inline I BinarySearch(I begin,const J end,const V& value,P predicate)
	{
		return BinarySearch(begin,end,value,predicate,HIDDEN::GetEqual(*begin));
	}

    template<class I,class J,class V> 
	inline I BinarySearch(I begin,const J end,const V& value)
	{
		return BinarySearch(begin,end,value,HIDDEN::GetLess(*begin),HIDDEN::GetEqual(*begin));
	}
	
	template<class I,class J> 
	inline VOID Sort(I begin,const J end)
	{
    	PDXQUICKSORT::Sort(begin,end);
	}

	template<class I,class J,class P> 
	inline VOID Sort(I begin,const J end,P predicate)
	{
    	PDXQUICKSORT::Sort(begin,end,predicate);
	}
}

#endif
