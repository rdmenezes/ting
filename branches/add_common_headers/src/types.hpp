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

// General types definitions
// Version 2
//
// Author: Ivan Gagis <igagis@gmail.com>

#ifndef M_TYPES_HPP
#define M_TYPES_HPP

#include "debug.hpp"

namespace ting{

typedef unsigned char u8;    STATIC_ASSERT(sizeof(u8) == 1)

STATIC_ASSERT(u8(-1) == 0xff)//assert that byte consists of exactly 8 bits, e.g. some systems have 10 bits per byte!!!

typedef signed char s8;    STATIC_ASSERT(sizeof(s8) == 1)
typedef unsigned short int u16;    STATIC_ASSERT(sizeof(u16) == 2)
typedef signed short int s16;    STATIC_ASSERT(sizeof(s16) == 2)
typedef unsigned int u32;    STATIC_ASSERT(sizeof(u32) == 4)
typedef signed int s32;    STATIC_ASSERT(sizeof(s32) == 4)
typedef unsigned long long int u64;    STATIC_ASSERT(sizeof(u64) == 8 && u64(-1) == 0xffffffffffffffffLL)
typedef long long int s64;    STATIC_ASSERT( sizeof(u64) == 8)

typedef unsigned int uint;
typedef u8 byte;

}//~namespace ting

#endif //~once
