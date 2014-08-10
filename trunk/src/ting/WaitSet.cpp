/* The MIT License:

Copyright (c) 2009-2013 Ivan Gagis <igagis@gmail.com>

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



#include "WaitSet.hpp"


#if M_OS == M_OS_MACOSX
#	include <sys/time.h>
#endif



using namespace ting;



#if M_OS == M_OS_MACOSX

void WaitSet::AddFilter(Waitable& w, int16_t filter){
	struct kevent e;

	EV_SET(&e, w.GetHandle(), filter, EV_ADD | EV_RECEIPT, 0, 0, (void*)&w);

	const timespec timeout = {0, 0}; //0 to make effect of polling, because passing NULL will cause to wait indefinitely.

	int res = kevent(this->queue, &e, 1, &e, 1, &timeout);
	if(res < 0){
		throw Exc("WaitSet::Add(): AddFilter(): kevent() failed");
	}
	
	ASSERT((e.flags & EV_ERROR) != 0) //EV_ERROR is always returned because of EV_RECEIPT, according to kevent() documentation.
	if(e.data != 0){//data should be 0 if added successfully
		TRACE(<< "WaitSet::Add(): e.data = " << e.data << std::endl)
		throw Exc("WaitSet::Add(): AddFilter(): kevent() failed to add filter");
	}
}



void WaitSet::RemoveFilter(Waitable& w, int16_t filter){
	struct kevent e;

	EV_SET(&e, w.GetHandle(), filter, EV_DELETE | EV_RECEIPT, 0, 0, 0);

	const timespec timeout = {0, 0}; //0 to make effect of polling, because passing NULL will cause to wait indefinitely.

	int res = kevent(this->queue, &e, 1, &e, 1, &timeout);
	if(res < 0){
		//ignore the failure
		TRACE(<< "WaitSet::Remove(): RemoveFilter(): kevent() failed" << std::endl);
	}
	
	ASSERT((e.flags & EV_ERROR) != 0) //EV_ERROR is always returned because of EV_RECEIPT, according to kevent() documentation.
}

#endif



void WaitSet::Add(Waitable& w, Waitable::EReadinessFlags flagsToWaitFor){
//		TRACE(<< "WaitSet::Add(): enter" << std::endl)
	ASSERT(!w.isAdded)

#if M_OS == M_OS_WINDOWS
	ASSERT(this->numWaitables <= this->handles.Size())
	if(this->numWaitables == this->handles.size()){
		throw Exc("WaitSet::Add(): wait set is full");
	}

	//NOTE: Setting wait flags may throw an exception, so do that before
	//adding object to the array and incrementing number of added objects.
	w.SetWaitingEvents(flagsToWaitFor);

	this->handles[this->numWaitables] = w.GetHandle();
	this->waitables[this->numWaitables] = &w;

#elif M_OS == M_OS_LINUX
	epoll_event e;
	e.data.fd = w.GetHandle();
	e.data.ptr = &w;
	e.events =
			(std::uint32_t(flagsToWaitFor) & Waitable::READ ? (EPOLLIN | EPOLLPRI) : 0)
			| (std::uint32_t(flagsToWaitFor) & Waitable::WRITE ? EPOLLOUT : 0)
			| (EPOLLERR);
	int res = epoll_ctl(
			this->epollSet,
			EPOLL_CTL_ADD,
			w.GetHandle(),
			&e
		);
	if(res < 0){
		TRACE(<< "WaitSet::Add(): epoll_ctl() failed. If you are adding socket, please check that is is opened before adding to WaitSet." << std::endl)
		throw Exc("WaitSet::Add(): epoll_ctl() failed");
	}
#elif M_OS == M_OS_MACOSX
	ASSERT(this->NumWaitables() <= revents.size() / 2)
	
	if((std::uint32_t(flagsToWaitFor) & Waitable::READ) != 0){
		this->AddFilter(w, EVFILT_READ);
	}
	if((std::uint32_t(flagsToWaitFor) & Waitable::WRITE) != 0){
		this->AddFilter(w, EVFILT_WRITE);
	}
#else
#	error "Unsupported OS"
#endif

	++this->numWaitables;

	w.isAdded = true;
//		TRACE(<< "WaitSet::Add(): exit" << std::endl)
}



void WaitSet::Change(Waitable& w, Waitable::EReadinessFlags flagsToWaitFor){
	ASSERT(w.isAdded)

#if M_OS == M_OS_WINDOWS
	//check if the Waitable object is added to this wait set
	{
		unsigned i;
		for(i = 0; i < this->numWaitables; ++i){
			if(this->waitables[i] == &w){
				break;
			}
		}
		ASSERT(i <= this->numWaitables)
		if(i == this->numWaitables){
			throw Exc("WaitSet::Change(): the Waitable is not added to this wait set");
		}
	}

	//set new wait flags
	w.SetWaitingEvents(flagsToWaitFor);

#elif M_OS == M_OS_LINUX
	epoll_event e;
	e.data.fd = w.GetHandle();
	e.data.ptr = &w;
	e.events =
			(std::uint32_t(flagsToWaitFor) & Waitable::READ ? (EPOLLIN | EPOLLPRI) : 0)
			| (std::uint32_t(flagsToWaitFor) & Waitable::WRITE ? EPOLLOUT : 0)
			| (EPOLLERR);
	int res = epoll_ctl(
			this->epollSet,
			EPOLL_CTL_MOD,
			w.GetHandle(),
			&e
		);
	if(res < 0){
		throw Exc("WaitSet::Change(): epoll_ctl() failed");
	}
#elif M_OS == M_OS_MACOSX
	if((std::uint32_t(flagsToWaitFor) & Waitable::READ) != 0){
		this->AddFilter(w, EVFILT_READ);
	}else{
		this->RemoveFilter(w, EVFILT_READ);
	}
	if((std::uint32_t(flagsToWaitFor) & Waitable::WRITE) != 0){
		this->AddFilter(w, EVFILT_WRITE);
	}else{
		this->RemoveFilter(w, EVFILT_WRITE);
	}
#else
#	error "Unsupported OS"
#endif
}



void WaitSet::Remove(Waitable& w)NOEXCEPT{
	ASSERT(w.isAdded)
	
	ASSERT(this->NumWaitables() != 0)

#if M_OS == M_OS_WINDOWS
	//remove object from array
	{
		unsigned i;
		for(i = 0; i < this->numWaitables; ++i){
			if(this->waitables[i] == &w){
				break;
			}
		}
		ASSERT(i <= this->numWaitables)
		ASSERT_INFO(i != this->numWaitables, "WaitSet::Remove(): Waitable is not added to wait set")

		unsigned numObjects = this->numWaitables - 1;//decrease number of objects before shifting the object handles in the array
		//shift object handles in the array
		for(; i < numObjects; ++i){
			this->handles[i] = this->handles[i + 1];
			this->waitables[i] = this->waitables[i + 1];
		}
	}

	//clear wait flags (disassociate socket and Windows event)
	w.SetWaitingEvents(0);

#elif M_OS == M_OS_LINUX
	int res = epoll_ctl(
			this->epollSet,
			EPOLL_CTL_DEL,
			w.GetHandle(),
			0
		);
	if(res < 0){
		ASSERT_INFO(false, "WaitSet::Remove(): epoll_ctl failed, probably the Waitable was not added to the wait set")
	}
#elif M_OS == M_OS_MACOSX	
	this->RemoveFilter(w, EVFILT_READ);
	this->RemoveFilter(w, EVFILT_WRITE);
#else
#	error "Unsupported OS"
#endif

	--this->numWaitables;

	w.isAdded = false;
//		TRACE(<< "WaitSet::Remove(): completed successfuly" << std::endl)
}



unsigned WaitSet::Wait(bool waitInfinitly, std::uint32_t timeout, ArrayAdaptor<Waitable*>* out_events){
	if(this->numWaitables == 0){
		throw Exc("WaitSet::Wait(): no Waitable objects were added to the WaitSet, can't perform Wait()");
	}

	if(out_events){
		if(out_events->size() < this->numWaitables){
			throw Exc("WaitSet::Wait(): passed out_events buffer is not large enough to hold all possible triggered objects");
		}
	}

#if M_OS == M_OS_WINDOWS
	DWORD waitTimeout = waitInfinitly ? (INFINITE) : DWORD(timeout);

	DWORD res = WaitForMultipleObjectsEx(
			this->numWaitables,
			&*this->handles.begin(),
			FALSE, //do not wait for all objects, wait for at least one
			waitTimeout,
			FALSE
		);

	ASSERT(res != WAIT_IO_COMPLETION)//it is impossible because we supplied FALSE as last parameter to WaitForMultipleObjectsEx()

	//we are not expecting abandoned mutexes
	ASSERT(res < WAIT_ABANDONED_0 || (WAIT_ABANDONED_0 + this->numWaitables) <= res)

	if(res == WAIT_FAILED){
		throw Exc("WaitSet::Wait(): WaitForMultipleObjectsEx() failed");
	}

	if(res == WAIT_TIMEOUT){
		return 0;
	}

	ASSERT(WAIT_OBJECT_0 <= res && res < (WAIT_OBJECT_0 + this->numWaitables ))

	//check for activities
	unsigned numEvents = 0;
	for(unsigned i = 0; i < this->numWaitables; ++i){
		if(this->waitables[i]->CheckSignaled()){
			if(out_events){
				ASSERT(numEvents < out_events->size())
				out_events->operator[](numEvents) = this->waitables[i];
			}
			++numEvents;
		}else{
			//NOTE: sometimes the event is reported as signaled, but no read/write events indicated.
			//      Don't know why it happens.
//			ASSERT_INFO(i != (res - WAIT_OBJECT_0), "i = " << i << " (res - WAIT_OBJECT_0) = " << (res - WAIT_OBJECT_0) << " waitflags = " << this->waitables[i]->readinessFlags)
		}
	}

	//NOTE: Sometimes the event is reported as signaled, but no actual activity is there.
	//      Don't know why.
//		ASSERT(numEvents > 0)

	return numEvents;

#elif M_OS == M_OS_LINUX
	ASSERT(int(timeout) >= 0)
	int epollTimeout = waitInfinitly ? (-1) : int(timeout);

//		TRACE(<< "going to epoll_wait() with timeout = " << epollTimeout << std::endl)

	int res;

	while(true){
		res = epoll_wait(
				this->epollSet,
				&*this->revents.begin(),
				this->revents.size(),
				epollTimeout
			);

//			TRACE(<< "epoll_wait() returned " << res << std::endl)

		if(res < 0){
			//if interrupted by signal, try waiting again.
			if(errno == EINTR){
				continue;
			}

			std::stringstream ss;
			ss << "WaitSet::Wait(): epoll_wait() failed, error code = " << errno << ": " << strerror(errno);
			throw Exc(ss.str().c_str());
		}
		break;
	};

	ASSERT(unsigned(res) <= this->revents.size())

	unsigned numEvents = 0;
	for(
			epoll_event *e = &*this->revents.begin();
			e < &*this->revents.begin() + res;
			++e
		)
	{
		Waitable* w = static_cast<Waitable*>(e->data.ptr);
		ASSERT(w)
		if((e->events & EPOLLERR) != 0){
			w->SetErrorFlag();
		}
		if((e->events & (EPOLLIN | EPOLLPRI)) != 0){
			w->SetCanReadFlag();
		}
		if((e->events & EPOLLOUT) != 0){
			w->SetCanWriteFlag();
		}
		ASSERT(w->CanRead() || w->CanWrite() || w->ErrorCondition())
		if(out_events){
			ASSERT(numEvents < out_events->size())
			out_events->operator[](numEvents) = w;
			++numEvents;
		}
	}

	ASSERT(res >= 0)//NOTE: 'res' can be zero, if no events happened in the specified timeout
	return unsigned(res);
#elif M_OS == M_OS_MACOSX
	struct timespec ts = {
		timeout / 1000, //seconds
		(timeout % 1000) * 1000000 //nanoseconds
	};

	//loop forever
	for(;;){
		int res = kevent(
				this->queue,
				0,
				0,
				this->revents.begin(),
				this->revents.size(),
				(waitInfinitly) ? 0 : &ts
			);

		if(res < 0){
			if(errno == EINTR){
				continue;
			}
			
			std::stringstream ss;
			ss << "WaitSet::Wait(): kevent() failed, error code = " << errno << ": " << strerror(errno);
			throw Exc(ss.str().c_str());
		}else if(res == 0){
			return 0; // timeout
		}else if(res > 0){
			unsigned out_i = 0;// index to out_events
			for(unsigned i = 0; i != unsigned(res); ++i){
				struct kevent &e = this->revents[i];
				Waitable *w = reinterpret_cast<Waitable*>(e.udata);
				if(e.filter == EVFILT_WRITE){
					w->SetCanWriteFlag();
				}else if(e.filter == EVFILT_READ){
					w->SetCanReadFlag();
				}
				
				if((e.flags & EV_ERROR) != 0){
					w->SetErrorFlag();
				}
				
				if(out_events){
					//check if Waitable is already added
					unsigned k = 0;
					for(; k != out_i; ++k){
						if(out_events->operator[](k) == w){
							break;
						}
					}
					if(k == out_i){
						ASSERT(out_i < out_events->size())
						out_events->operator[](out_i) = w;
						++out_i;
					}
				}
			}
			return unsigned(res);
		}
	}
#else
#	error "Unsupported OS"
#endif
}
