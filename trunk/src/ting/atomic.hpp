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
 * @brief Atomic operations.
 */

#pragma once

#include "debug.hpp"
#include "types.hpp"
#include "utils.hpp"

#if defined(__GNUG__) && defined(__i386__)
//gcc atomic stuff available

#elif defined(WIN32)
#include <windows.h>

#elif defined(__APPLE__)
#include <CoreServices/CoreServices.h>

#else
//no native atomic operations support detected, will be using plain mutex
#include "Thread.hpp"
#endif

namespace ting{
namespace atomic{

/**
 * @brief Atomic signed 32bit integer.
 */
//Microsoft's Interlocked* functions require that the value to be aligned on 32bit boundary
M_DECLARE_ALIGNED_MSVC(4) class S32{

#if defined(__GNUG__) && defined(__i386__)
	//gcc atomic stuff available		
#elif defined(WIN32)
	//Win32 Interlocked* stuff available
#elif defined(__APPLE__)
	//Mac os *Atomic stuff available
#else
	//no native atomic operations support detected, will be using plain mutex
	ting::Mutex mutex;
#endif
	
	volatile ting::s32 v;
	
public:
	/**
	 * @brief Constructor.
     * @param initialValue - initial value to assign to this atomic variable.
     */
	inline S32(ting::s32 initialValue = 0) :
			v(initialValue)
	{}

	/**
	 * @brief Adds the value to this atomic variable and returns its initial value.
	 * @param value - the value to add to this atomic variable.
	 * @return initial value of this atomic variable.
	 */
	inline ting::s32 FetchAndAdd(ting::s32 value){
#if defined(__GNUG__) && defined(__i386__)
		//gcc atomic stuff available
		return __sync_fetch_and_add(&this->v, value);
		
#elif defined(WIN32)
		ASSERT(sizeof(LONG) == sizeof(this->v))
		return InterlockedExchangeAdd(&this->v, value);

#elif defined(__APPLE__)
		return AddAtomic(value, &this->v);
		
#else
		//no native atomic operations support detected, will be using plain mutex
		{
			ting::Mutex::Guard mutexGuard(this->mutex);
			ting::s32 ret = this->v;
			this->v += value;
			return ret;
		}
#endif
	}
};

}//~namespace atomic
}//~namespace ting
