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

#if defined(__GNUG__) && (defined(__i386__) || defined(__x86_64__))
//gcc atomic stuff available
#define M_GCC_ATOMIC_BUILTINS_ARE_AVAILABLE

//#elif defined(__GNUG__) && defined(__arm__) //TODO: support something for ARM

#elif defined(WIN32)
#define M_WIN32_INTERLOCKED_FUNCTIONS_ARE_AVAILABLE
#include <windows.h>

#elif defined(__APPLE__)
#define M_APPLE_CORESERVICES_ATOMIC_FUNCTIONS_ARE_AVAILABLE
#include <libkern/OSAtomic.h>

#else
//no native atomic operations support detected, will be using plain mutex
#include "Thread.hpp"
#endif



namespace ting{
namespace atomic{

/**
 * @brief Atomic signed 32bit integer.
 */
//On most architectures, atomic operations require that the value to be naturally aligned (4 bytes = sizeof(int)).
#ifndef M_DOXYGEN_DONT_EXTRACT //for doxygen
M_DECLARE_ALIGNED_MSVC(4)
#endif
class S32{

#if defined(M_GCC_ATOMIC_BUILTINS_ARE_AVAILABLE)
	//gcc atomic stuff available
#elif defined(M_WIN32_INTERLOCKED_FUNCTIONS_ARE_AVAILABLE)
	//Win32 Interlocked* stuff available
#elif defined(M_APPLE_CORESERVICES_ATOMIC_FUNCTIONS_ARE_AVAILABLE)
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
#if defined(M_GCC_ATOMIC_BUILTINS_ARE_AVAILABLE)
		//gcc atomic stuff available
		return __sync_fetch_and_add(&this->v, value);
		
#elif defined(M_WIN32_INTERLOCKED_FUNCTIONS_ARE_AVAILABLE)
		ASSERT(sizeof(LONG) == sizeof(this->v))
		return InterlockedExchangeAdd(&this->v, value);

#elif defined(M_APPLE_CORESERVICES_ATOMIC_FUNCTIONS_ARE_AVAILABLE)
		return (OSAtomicAdd32(value, &this->v) - value);
		
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
	
	inline ting::s32 FetchCompareAndExchange(ting::s32 compareTo, ting::s32 exchangeBy){
#if defined(M_GCC_ATOMIC_BUILTINS_ARE_AVAILABLE)
		//gcc atomic stuff available
		return __sync_val_compare_and_swap(&this->v, compareTo, exchangeBy);
		
#elif defined(M_WIN32_INTERLOCKED_FUNCTIONS_ARE_AVAILABLE)
		ASSERT(sizeof(LONG) == sizeof(this->v))
		return InterlockedCompareExchange(&this->v, exchangeBy, compareTo);

#elif defined(M_APPLE_CORESERVICES_ATOMIC_FUNCTIONS_ARE_AVAILABLE)
		if(OSAtomicCompareAndSwap32(compareTo, exchangeBy, &this->v)){
			return oldValue;
		}else{
			return this->v;//v is volatile, should be ok
		}
		
#else
		//no native atomic operations support detected, will be using plain mutex
		{
			ting::Mutex::Guard mutexGuard(this->mutex);
			if(this->v == compareTo){
				this->v = exchangeBy;
				return compareTo;
			}else{
				return this->v;
			}
		}
#endif
	}
} M_DECLARE_ALIGNED(sizeof(int)); //On most architectures, atomic operations require that the value to be naturally aligned.

}//~namespace atomic
}//~namespace ting
