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

#include "config.hpp"
#include "debug.hpp"
#include "types.hpp"
#include "utils.hpp"



#if M_CPU == M_CPU_X86 || \
		M_CPU == M_CPU_X86_64 || \
		M_CPU == M_CPU_ARM

#elif M_OS == M_OS_WIN32
	#include <windows.h>

#elif M_OS == M_OS_MACOSX
	#include <libkern/OSAtomic.h>

#else
	//no native atomic operations support detected, will be using plain mutex
	#include "Thread.hpp"
#endif



namespace ting{
namespace atomic{



/**
 * @brief Atomic flag.
 * Atomic flag is a bool-like value whose set and clear operations are atomic.
 */
//On most architectures, atomic operations require that the value to be naturally aligned (4 bytes = sizeof(int)).
#ifndef M_DOXYGEN_DONT_EXTRACT //for doxygen

//make sure theat we align by int size when using MSVC compiler.
STATIC_ASSERT(sizeof(int) == 4)

M_DECLARE_ALIGNED_MSVC(4)
#endif
class Flag{
#if M_CPU == M_CPU_X86 || \
		M_CPU == M_CPU_X86_64 || \
		M_CPU == M_CPU_ARM
	
	volatile int flag;
#elif M_OS == M_OS_WIN32
	volatile LONG flag;
#elif M_OS == M_OS_MACOSX
	volatile int flag;
#else //unknown cpu architecture, unknown OS, will be using plain mutex
	ting::Mutex mutex;
	bool flag;
#endif

public:
	/**
	 * @brief Constructor.
	 * @param initialValue - initial value of the flag.
     */
	inline Flag(bool initialValue = false){
#if M_CPU == M_CPU_X86 || \
		M_CPU == M_CPU_X86_64 || \
		M_CPU == M_CPU_ARM || \
		M_OS == M_OS_WIN32
	
		this->Set(initialValue);
#elif M_OS == M_OS_MACOSX
		this->flag = initialValue;
#else
		this->Set(initialValue);
#endif
	}
	
	
	
	/**
	 * @brief Gets the current flag value.
	 * Note, that the returned value may be not actual, since the flag value can
	 * be changed in parallel. It does not set any memory barrier.
	 * @return current flag value.
	 */
	inline bool Get()const{
#if M_COMPILER == M_COMPILER_MSVC
		return this->flag == 0 ? false : true;
#else
		return this->flag;
#endif
	}
	
	
	/**
	 * @brief Set the flag value.
	 * Sets the flag to the new value and returns its previous value as atomic operation.
	 * @param value - the flag value to set.
	 * @return old flag value.
     */
	inline bool Set(bool value = true){
#if M_CPU == M_CPU_X86 || M_CPU == M_CPU_X86_64
		int old;
	#if M_COMPILER == M_COMPILER_MSVC
		__asm{
			mov ebx, this
			xor eax, eax
			mov al, value
			lock xchg eax, [ebx].flag
			mov [old], eax
		}
		return old == 0 ? false : true; // this ternary ? : stuff is to avoid compiler warning
	#else
		__asm__ __volatile__(
				"lock; xchgl %0, %1"
						: "=r"(old), "=m"(this->flag)
						: "0"(int(value)), "m"(this->flag)
						: "memory"
			);
		return old;
	#endif

#elif M_CPU == M_CPU_ARM
		int old;
		__asm__ __volatile__(
				"swp %0, %2, [%3]"
						: "=&r"(old), "=&r"(&this->flag)
						: "r"(value), "1"(&this->flag)
						: "memory"
			);
		return old;
#elif M_OS == M_OS_WIN32
		return InterlockedExchange(&this->flag, value) == 0 ? false : true;
#elif M_OS == M_OS_MACOSX
		if(value){
			return !OSAtomicCompareAndSwap32(!value, value, &this->flag);
		}else{
			return OSAtomicCompareAndSwap32(!value, value, &this->flag);
		}
#else //unknown cpu architecture, unknown OS, will be using plain mutex
		{
			ting::Mutex::Guard mutexGuard(this->mutex);
			bool old = this->flag;
			this->flag = value;
			return old;
		}
#endif
	}
	
	
	
	/**
	 * @brief Clear flag.
	 * Basically, it is equivalent to Flag::Set(false), but on some architectures
	 * its implementation can be faster.
     */
	inline void Clear(){
#if M_CPU == M_CPU_X86 || M_CPU == M_CPU_X86_64 || M_CPU == M_CPU_ARM
		this->Set(false);
#elif M_OS == M_OS_WIN32
		InterlockedExchange(&this->flag, 0);
#elif M_OS == M_OS_MACOSX
		OSAtomicCompareAndSwap32(true, false, &this->flag);
#else //unknown cpu architecture, unkown OS, will be using plain mutex
		{
			//still need to lock the mutex to generate the memory barrier.
			ting::Mutex::Guard mutexGuard(this->mutex);
			this->flag = false;
		}
#endif
	}
} M_DECLARE_ALIGNED(sizeof(int)); //On most architectures, atomic operations require that the value to be naturally aligned.



/**
 * @brief Spinlock class.
 * Spinlock is the same thing as mutex, except that when trying to lock a locked
 * spinlock it enters a busy-loop until the spinlock is unlocked by previous locker so
 * it can be locked. But, in return, spinlock is much more lightweight than mutex,
 * because it does not do any syscalls.
 * Use spinlock to synchronize only very short and rare operations. In other cases, use mutex.
 */
//On most architectures, atomic operations require that the value to be naturally aligned (4 bytes = sizeof(int)).
#ifndef M_DOXYGEN_DONT_EXTRACT //for doxygen

//make sure theat we align by int size when using MSVC compiler.
STATIC_ASSERT(sizeof(int) == 4)

M_DECLARE_ALIGNED_MSVC(4)
#endif
class SpinLock{
#if M_CPU == M_CPU_X86 || M_CPU == M_CPU_X86_64 || M_CPU == M_CPU_ARM || \
		M_OS == M_OS_WIN32
		
	atomic::Flag flag;
	
#elif M_OS == M_OS_MACOSX
	volatile OSSpinLock sl;
#else //unknown cpu architecture, unknown OS, will be using plain mutex
	ting::Mutex mutex;
#endif


public:
	/**
	 * @brief Constructor.
	 * Creates an initially unlocked spinlock.
     */
	inline SpinLock(){
#if M_CPU == M_CPU_X86 || M_CPU == M_CPU_X86_64 || M_CPU == M_CPU_ARM || \
		M_OS == M_OS_WIN32
		
		//initially unlocked.
#elif M_OS == M_OS_MACOSX
		this->sl = 0; // 0 means unlocked state
#else //unknown cpu architecture, unknown OS, will be using plain mutex
		//no need to initialize mutex, it is unlocked initially itself.
#endif
	}
	
	
	
	/**
	 * @brief Lock the spinlock.
     */
	inline void Lock(){
#if M_CPU == M_CPU_X86 || M_CPU == M_CPU_X86_64 || M_CPU == M_CPU_ARM || \
		M_OS == M_OS_WIN32
		
		while(this->flag.Set(true)){
			while(this->flag.Get()){}
		}
#elif M_OS == M_OS_MACOSX
		OSSpinLockLock(&this->sl);
#else //unknown cpu architecture, unknown OS, will be using plain mutex
		this->mutex.Lock();
#endif
	}
	
	
	
	/**
	 * @brief Unlock the spinlock.
     */
	inline void Unlock(){
#if M_CPU == M_CPU_X86 || M_CPU == M_CPU_X86_64 || M_CPU == M_CPU_ARM || \
		M_OS == M_OS_WIN32
		
		this->flag.Clear();
#elif M_OS == M_OS_MACOSX
		OSSpinLockUnlock(&this->sl);
#else //unknown cpu architecture, unknown OS, will be using plain mutex
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

#if M_CPU == M_CPU_X86 || M_CPU == M_CPU_X86_64 || \
		M_OS == M_OS_WIN32 || M_OS == M_OS_MACOSX
		
	//no additional variables required
#elif M_CPU == M_CPU_ARM
	atomic::SpinLock spinLock;
#else //unknown cpu architecture, will be using plain mutex
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
#if M_CPU == M_CPU_X86 || M_CPU == M_CPU_X86_64
		
		{
			ting::s32 old;
	#if M_COMPILER == M_COMPILER_MSVC
			__asm{
				mov ebx, this
				mov eax, [value]
				lock xadd [ebx].v, eax
				mov [old], eax
			}
	#else
			__asm__ __volatile__ (
					"lock; xaddl %0, %1"
							: "=r"(old), "=m"(this->v)
							: "0"(value), "m"(this->v)
							: "memory"
				);
	#endif
			return old;
		}
		
#elif M_CPU == M_CPU_ARM
		this->spinLock.Lock();
		ting::s32 old = this->v;
		this->v += value;
		this->spinLock.Unlock();
		return old;
#elif M_OS == M_OS_WIN32
		ASSERT(sizeof(LONG) == sizeof(this->v))
		return InterlockedExchangeAdd(reinterpret_cast<volatile LONG*>(&this->v), LONG(value));

#elif M_OS == M_OS_MACOSX
		return (OSAtomicAdd32(value, &this->v) - value);
		
#else
		//no native atomic operations support detected, will be using plain mutex
		{
			ting::Mutex::Guard mutexGuard(this->mutex);
			ting::s32 old = this->v;
			this->v += value;
			return old;
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
#if M_CPU == M_CPU_X86 || M_CPU == M_CPU_X86_64
		
		ting::s32 old;
	#if M_COMPILER == M_COMPILER_MSVC
		__asm{
			mov ebx, this
			mov eax, [compareTo]
			mov edx, [exchangeBy]
			lock cmpxchg [ebx].v, edx
			mov [old], eax
		}
	#else
		__asm__ __volatile__(
				"lock; cmpxchgl %3, %2"
						: "=m"(this->v), "=a"(old)
						: "m"(this->v), "r"(exchangeBy), "a"(compareTo)
						: "memory"
			);
	#endif
		return old;
		
#elif M_CPU == M_CPU_ARM
		this->spinLock.Lock();
		ting::s32 old = this->v;
		if(old == compareTo){
			this->v = exchangeBy;
		}
		this->spinLock.Unlock();
		return old;
#elif M_OS == M_OS_WIN32
		ASSERT(sizeof(LONG) == sizeof(this->v))
		return InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(&this->v), exchangeBy, compareTo);

#elif M_OS == M_OS_MACOSX
		for(;;){
			//No memory barrier is needed, since we don't care when the old
			//value will be read because we are checking anyway if it is equal
			//to 'compareTo' before returning.
			if(OSAtomicCompareAndSwap32(compareTo, exchangeBy, &this->v)){
				return compareTo; //operation succeeded, return previous value.
			}else{
				//The following scenario is still possible:
				// - The value of this->v was not equal to 'compareTo'. Thus, the atomic
				//   operation returned false.
				// - Right after the real value of this->v, in parallel,
				//   has changed to something which is equal to 'compareTo'.
				// - Thus, we need to check if we are returning the value which is equal to 'compareTo'
				//   and prevent it by repeating the procedure again.
				volatile ting::s32 old = this->v;
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
