/* The MIT License:

Copyright (c) 2008-2011 Ivan Gagis <igagis@gmail.com>

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

// Homepage: http://code.google.com/p/ting



#include "Thread.hpp"



using namespace ting;



Mutex::Mutex(){
	M_MUTEX_TRACE(<< "Mutex::Mutex(): invoked " << this << std::endl)
#ifdef WIN32
	InitializeCriticalSection(&this->m);
#elif defined(__SYMBIAN32__)
	if(this->m.CreateLocal() != KErrNone){
		throw ting::Exc("Mutex::Mutex(): failed creating mutex (CreateLocal() failed)");
	}
#elif defined(__linux__) || defined(__APPLE__)
	if(pthread_mutex_init(&this->m, NULL) != 0){
		throw ting::Exc("Mutex::Mutex(): failed creating mutex (pthread_mutex_init() failed)");
	}
#else
#error "unknown system"
#endif
}



Mutex::~Mutex(){
	M_MUTEX_TRACE(<< "Mutex::~Mutex(): invoked " << this << std::endl)
#ifdef WIN32
	DeleteCriticalSection(&this->m);
#elif defined(__SYMBIAN32__)
	this->m.Close();
#elif defined(__linux__) || defined(__APPLE__)
	int ret = pthread_mutex_destroy(&this->m);
	if(ret != 0){
		std::stringstream ss;
		ss << "Mutex::~Mutex(): pthread_mutex_destroy() failed"
				<< " error code = " << ret << ": " << strerror(ret) << ".";
		if(ret == EBUSY){
			ss << " You are trying to destroy locked mutex.";
		}
		ASSERT_INFO_ALWAYS(false, ss.str())
	}
#else
#error "unknown system"
#endif
}
