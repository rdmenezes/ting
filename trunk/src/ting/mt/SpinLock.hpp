/* The MIT License:

Copyright (c) 2014 Ivan Gagis <igagis@gmail.com>

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

// Home page: http://ting.googlecode.com



/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include <atomic>
#include <thread>

#if M_OS == M_OS_WINDOWS && M_COMPILER == M_COMPILER_GCC
#	include "../windows.hpp"
#endif


namespace ting{
namespace mt{

class SpinLock{
	std::atomic_flag flag
#if M_OS == M_OS_WINDOWS
	;
#else
	= ATOMIC_FLAG_INIT;//initialize to false
#endif
	
public:
	SpinLock(){
		this->unlock();//NOTE: initialization of flag does not work for some reason, so set to unlocked state initially.
	}
	
	
	SpinLock(const SpinLock&) = delete;
	SpinLock& operator=(const SpinLock&) = delete;
	
	~SpinLock()NOEXCEPT{}

	/**
	 * @brief Lock the spinlock.
	 * If the lock cannot be acquired immediately it will enter a busy loop
	 * reading the lock value until it becomes 'unlocked' and then try to lock again.
	 * Each cycle of the busy loop it will yield the thread.
	 * Right after acquiring the lock the memory barrier is set.
	 */
	void lock()NOEXCEPT{
		while(this->flag.test_and_set(std::memory_order_acquire)){
#if M_OS == M_OS_WINDOWS && M_COMPILER == M_COMPILER_GCC
			SleepEx(0, FALSE);
#else
			std::this_thread::yield();
#endif
		}
	}
	


	/**
	 * @brief Unlock the spinlock.
	 * Right before releasing the lock the memory barrier is set.
	 */
	void unlock()NOEXCEPT{
		this->flag.clear(std::memory_order_release);
	}
};

}
}
