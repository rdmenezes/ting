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

// ting 0.4
// Homepage: http://code.google.com/p/ting
// Author: Ivan Gagis <igagis@gmail.com>

/**
 * @file types.hpp
 * @brief General types definitions.
 */

#pragma once

#include "debug.hpp"

namespace ting{

typedef unsigned char u8;    STATIC_ASSERT(sizeof(u8) == 1)

STATIC_ASSERT(u8(-1) == 0xff)//assert that byte consists of exactly 8 bits, e.g. some systems have 10 bits per byte!!!

typedef signed char s8;
typedef unsigned short int u16;
typedef signed short int s16;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long long int u64;
typedef long long int s64;


#ifndef M_DOC_DONT_EXTRACT //for doxygen

STATIC_ASSERT(sizeof(s8) == 1)
STATIC_ASSERT(sizeof(u16) == 2)
STATIC_ASSERT(sizeof(s16) == 2)
STATIC_ASSERT(sizeof(u32) == 4)
STATIC_ASSERT(sizeof(s32) == 4)
STATIC_ASSERT(sizeof(u64) == 8)
STATIC_ASSERT(u64(-1) == 0xffffffffffffffffLL)

#endif //~M_DOC_DONT_EXTRACT //for doxygen


typedef unsigned int uint;
typedef u8 byte;

}//~namespace ting
