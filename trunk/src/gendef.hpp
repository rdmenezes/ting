/* The MIT License:

Copyright (c) 2008 Ivan Gagis

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE. */

// ting 0.2
// Author: Ivan Gagis <igagis@gmail.com>

// File description:
//	General definitions

#ifndef M_GENDEF_HPP
#define M_GENDEF_HPP

#ifdef _MSC_VER //If Microsoft C++ compiler
#pragma warning(disable:4290)
#endif

#include "debug.hpp" //debugging facilities

#include "types.hpp"
#include "Exc.hpp"

namespace ting{

template <class T_Type> inline void Exchange( T_Type &a, T_Type &b){
	T_Type tmp = a;
	a = b;
	b = tmp;
};

//quick exchange two unsigned 32bit integers
template <> inline void Exchange<u32>(u32 &x, u32 &y){
//	TRACE(<<"Exchange<u32>(): invoked"<<std::endl)
	//NOTE: Do not make y^=x^=y^=x;
	//Some compilers (e.g. gcc4.1) may generate incorrect code
	y ^= x;
	x ^= y;
	y ^= x;
};

//quick exchange two floats
template <> inline void Exchange<float>(float &x, float &y){
//	TRACE(<<"Exchange<float>(): invoked"<<std::endl)
	STATIC_ASSERT(sizeof(float) == sizeof(u32))
	Exchange<u32>(reinterpret_cast<u32&>(x), reinterpret_cast<u32&>(y));
};

};//~namespace ting

#endif //~once
