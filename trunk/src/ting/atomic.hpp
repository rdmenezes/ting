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

//TODO: don't use OS specific atomic ops, implement atomics for different architectures instead

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



//On most architectures, atomic operations require that the value to be naturally aligned (4 bytes = sizeof(int)).
#ifndef M_DOXYGEN_DONT_EXTRACT //for doxygen

//make sure theat we align by int size when using MSVC compiler.
STATIC_ASSERT(sizeof(int) == 4)

M_DECLARE_ALIGNED_MSVC(4)
#endif
class SpinLock{
#if defined(M_GCC_ATOMIC_BUILTINS_ARE_AVAILABLE)
	volatile int sl;
#elif defined(M_WIN32_INTERLOCKED_FUNCTIONS_ARE_AVAILABLE)
	volatile LONG sl;
#elif defined(M_APPLE_CORESERVICES_ATOMIC_FUNCTIONS_ARE_AVAILABLE)
	volatile OSSpinLock sl;
#else //no native atomic operations support detected, will be using plain mutex
	ting::Mutex mutex;
#endif

public:
	inline SpinLock(){
#if defined(M_GCC_ATOMIC_BUILTINS_ARE_AVAILABLE)
		this->sl = 0; // 0 means unlocked state
#elif defined(M_WIN32_INTERLOCKED_FUNCTIONS_ARE_AVAILABLE)
		this->sl = 0; // 0 means unlocked state
#elif defined(M_APPLE_CORESERVICES_ATOMIC_FUNCTIONS_ARE_AVAILABLE)
		this->sl = 0; // 0 means unlocked state
#else
		//no need to initialize mutex, it is unlocked initially itself.
#endif
	}
	
	
	inline ~SpinLock(){}
	
	
	inline void Lock(){
#if defined(M_GCC_ATOMIC_BUILTINS_ARE_AVAILABLE)
		while(__sync_lock_test_and_set(&this->sl, 1)){ //__sync_lock_test_and_set() generates acquire memory barrier, i.e. references after it cannot go before, but not vice versa.
			while(this->sl != 0){}
		}
#elif defined(M_WIN32_INTERLOCKED_FUNCTIONS_ARE_AVAILABLE)
		while(InterlockedExchange(&this->sl, 1) != 0){ //InterlockedExchange() generates full memory barrier
			while(this->sl != 0){}
		}
#elif defined(M_APPLE_CORESERVICES_ATOMIC_FUNCTIONS_ARE_AVAILABLE)
		OSSpinLockLock(&this->sl);
#else //no native atomic operations support detected, will be using plain mutex
		this->mutex.Lock();
#endif
	}
	
	
	inline void Unlock(){
#if defined(M_GCC_ATOMIC_BUILTINS_ARE_AVAILABLE)
		__sync_lock_release(&this->sl); //__sync_lock_release() generates release memory barrier, i.e. references before it cannot go after, but not vice versa.
#elif defined(M_WIN32_INTERLOCKED_FUNCTIONS_ARE_AVAILABLE)
		InterlockedExchange(&this->sl, 0); //InterlockedExchange() generates full memory barrier
#elif defined(M_APPLE_CORESERVICES_ATOMIC_FUNCTIONS_ARE_AVAILABLE)
		OSSpinLockUnlock(&this->sl);
#else //no native atomic operations support detected, will be using plain mutex
		this->mutex.Unlock();
#endif
	}
} M_DECLARE_ALIGNED(sizeof(int)); //On most architectures, atomic operations require that the value to be naturally aligned.



/**
 * @brief Atomic signed 32bit integer.
 */
//On most architectures, atomic operations require that the value to be naturally aligned (4 bytes = sizeof(int)).
#ifndef M_DOXYGEN_DONT_EXTRACT //for doxygen

//make sure theat we align by int size when using MSVC compiler.
STATIC_ASSERT(sizeof(int) == 4)

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
	
	/**
	 * @brief Atomic compare and exchange operation
	 * Compares the current value to the 'compareTo' value and if they are equal
	 * it will store the 'exchangeBy' value to the current value.
     * @param compareTo - the value to compare the current value to.
     * @param exchangeBy - the value to store as the the current value in case the comparison will result in equals.
	 *                     Otherwise, the current value will remain untouched.
     * @return old current value.
     */
	inline ting::s32 CompareAndExchange(ting::s32 compareTo, ting::s32 exchangeBy){
#if defined(M_GCC_ATOMIC_BUILTINS_ARE_AVAILABLE)
		//gcc atomic stuff available
		return __sync_val_compare_and_swap(&this->v, compareTo, exchangeBy);
		
#elif defined(M_WIN32_INTERLOCKED_FUNCTIONS_ARE_AVAILABLE)
		ASSERT(sizeof(LONG) == sizeof(this->v))
		return InterlockedCompareExchange(&this->v, exchangeBy, compareTo);

#elif defined(M_APPLE_CORESERVICES_ATOMIC_FUNCTIONS_ARE_AVAILABLE)
		for(;;){
			ting::s32 old = this->v;
			if(OSAtomicCompareAndSwap32Barrier(compareTo, exchangeBy, &this->v)){ //memory barrier since we are reading this->v before atomic operation.
				//operation succeeded, return previous value.
				return compareTo;
			}else{
				//The following scenario is still possible:
				// - The value of this->v was equal to 'compareTo'.
				// - After saving the old value and before issuing the atomic operation,
				//   the real value of this->v, in parallel, has changed to something which is
				//   not equal to 'compareTo'.
				// - Thus, the atomic operation fails, i.e. returns false.
				// - But saved old value is equal to 'compareTo', we can't return it, since it will
				//   indicate that atomic operation succeeded. Therefore, saved old value should
				//   not be equal to 'compareTo' here. If it is, then try to do the procedure again.
				if(old == compareTo){
					continue;
				}
				
				return old;
			}
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
