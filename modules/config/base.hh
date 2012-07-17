//------------------------------------------------------------------------------
// This file is part of the OpenStructure project <www.openstructure.org>
//
// Copyright (C) 2008-2011 by the OpenStructure authors
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 3.0 of the License, or (at your option)
// any later version.
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//------------------------------------------------------------------------------
#ifndef OST_BASE_HH
#define OST_BASE_HH

#include <string>
#include <cmath>
#include <math.h>
#include <complex>
#include <ost/config.hh>
#include <ost/dllexport.hh>

#ifndef uint
typedef unsigned int uint;
#endif

#ifndef uchar
  typedef unsigned char uchar;
#endif

#ifndef ushort
  typedef unsigned short int ushort;
#endif


#if OST_DOUBLE_PRECISION
  typedef double Real;
#else
  typedef float Real;
#endif

#ifdef Complex
  // undef Complex from xorg X.h to avoid clash with our definition
  #undef Complex
#endif
typedef std::complex<Real> Complex;
typedef unsigned short Word;



#ifndef round_function
#define round_function
#ifndef round
inline Real round( Real d )
{
  return floor(d+Real(0.5));
}
#endif
#endif

#ifndef rint_function
#define rint_function
#ifndef rint
inline Real rint(Real d)
{
  return floor(d+Real(0.5));
}
#endif
#endif

#if _MSC_VER
#pragma warning(disable:4251)
#pragma warning(disable:4275)
#pragma warning(disable:4244)
#pragma warning(disable:4231)
#pragma warning(disable:4305)
#pragma warning(disable:4351) // turn off "new behavior ... will be default initialized" warning 

#ifndef log2_function
#define log2_function
#ifndef log2
   //C99 defines log2, C90 does not and MSVC is C90:(
  // Calculates log2 of number.  
inline double log2( double n )  
  {  
      // log(n)/log(2) is log2.  
      return log( double(n) ) / log( 2.0 );  
  }
#endif
#endif

# ifdef FindAtom
#  undef FindAtom
# endif
# ifdef AddAtom
#  undef AddAtom
# endif

#endif


typedef std::string String;


#endif
