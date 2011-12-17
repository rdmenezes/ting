/* The MIT License:

Copyright (c) 2011 Ivan Gagis <igagis@gmail.com>

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

// Homepage: http://ting.googlecode.com



/**
 * @author Ivan Gagis <igagis@gmail.com>
 * @brief Environment configuration definitions.
 */

#pragma once



//====================================================|
//            Compiler definitions                    |
//                                                    |

#define M_COMPILER_UNKNOWN                            0
#define M_COMPILER_GCC                                1
#define M_COMPILER_MSVC                               2

#if defined(__GNUC__) || defined(__GNUG__)
	#define M_COMPILER M_COMPILER_GCC
#elif defined(_MSC_VER)
	#define M_COMPILER M_COMPILER_MSVC
#else
	#define M_COMPILER M_COMPILER_UNKNOWN
#endif



//====================================================|
//            CPU architecture definitions            |
//                                                    |

#define M_CPU_UNKNOWN                                 0
#define M_CPU_X86                                     1
#define M_CPU_X86_64                                  2
#define M_CPU_ARM                                     3

#if M_COMPILER == M_COMPILER_GCC
	#if defined(__i386__)
		#define M_CPU M_CPU_X86
	#elif defined(__x86_64__)
		#define M_CPU M_CPU_X86_64
	#elif defined(__arm__)
		#define M_CPU M_CPU_ARM
	#else
		#define M_CPU M_CPU_UNKNOWN
	#endif

#elif M_COMPILER == M_COMPILER_MSVC
	#if defined(_M_IX86)
		#define M_CPU M_CPU_X86
	#elif defined(_M_AMD64) || defined(_M_X64)
		#define M_CPU M_CPU_X86_64
	#else
		#define M_CPU M_CPU_UNKNOWN
	#endif
#else
	#define M_CPU M_CPU_UNKNOWN
#endif



//======================================|
//            OS definitions            |
//                                      |

#define M_OS_UNKNOWN                    0
#define M_OS_LINUX                      1
#define M_OS_WIN32                      2
#define M_OS_MACOSX                     3

#if defined(__linux__)
	#define M_OS M_OS_LINUX
#elif defined(WIN32)
	#define M_OS M_OS_WIN32
#elif defined(__APPLE__)
	#define M_OS M_OS_MACOSX
#else
	#define M_OS M_OS_UNKNOWN
#endif
