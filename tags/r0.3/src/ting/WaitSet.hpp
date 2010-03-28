/* The MIT License:

Copyright (c) 2009 Ivan Gagis

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

// ting 0.3
// Homepage: http://code.google.com/p/ting
// Author: Ivan Gagis <igagis@gmail.com>

// File description:
//	Wait set

#pragma once

#include "types.hpp"
#include "debug.hpp"
#include "Exc.hpp"
#include "Array.hpp"

#include <vector>



#if defined(__WIN32__) || defined(WIN32)
#ifndef __WIN32__
#define __WIN32__
#endif
#ifndef WIN32
#define WIN32
#endif

#include <windows.h>

#else //assume *nix
#include <sys/epoll.h>

#endif



namespace ting{



class Waitable{
	friend class WaitSet;

	bool isAdded;



public:
	enum EReadinessFlags{
		NOT_READY = 0,  // bin: 00000000
		READ = 1,       // bin: 00000001
		WRITE = 2       // bin: 00000010
	};
	
protected:
	u32 readinessFlags;

	inline Waitable() :
			isAdded(false),
			readinessFlags(NOT_READY)
	{}



	//TODO: write doxygen comments
	inline Waitable(const Waitable& w) :
			isAdded(false),
			readinessFlags(NOT_READY)//Treat copied Waitable as NOT_READY
	{
		//cannot copy from waitable which is adde to WaitSet
		if(w.isAdded)
			throw ting::Exc("Waitable::Waitable(copy): cannot copy Waitable which is added to WaitSet");

		const_cast<Waitable&>(w).ClearAllReadinessFlags();
	}



	//TODO: write doxygen comments
	inline Waitable& operator=(const Waitable& w){
		//cannot copy because this Waitable is added to WaitSet
		if(this->isAdded)
			throw ting::Exc("Waitable::Waitable(copy): cannot copy while this Waitable is added to WaitSet");

		//cannot copy from waitable which is adde to WaitSet
		if(w.isAdded)
			throw ting::Exc("Waitable::Waitable(copy): cannot copy Waitable which is added to WaitSet");
		
		ASSERT(!this->isAdded)

		//Clear readiness flags on copying.
		//Will need to wait for readiness again, using the WaitSet.
		this->ClearAllReadinessFlags();
		const_cast<Waitable&>(w).ClearAllReadinessFlags();
		return *this;
	}



	inline void SetCanReadFlag(){
		this->readinessFlags |= READ;
	}

	inline void ClearCanReadFlag(){
		this->readinessFlags &= (~READ);
	}

	inline void SetCanWriteFlag(){
		this->readinessFlags |= WRITE;
	}

	inline void ClearCanWriteFlag(){
		this->readinessFlags &= (~WRITE);
	}

	inline void ClearAllReadinessFlags(){
		this->readinessFlags = NOT_READY;
	}

public:
	virtual ~Waitable(){
		ASSERT(!this->isAdded)
	}

	inline bool CanRead()const{
		return (this->readinessFlags & READ) != 0;
	}

	inline bool CanWrite()const{
		return (this->readinessFlags & WRITE) != 0;
	}

#ifdef __WIN32__
protected:
	virtual HANDLE GetHandle() = 0;

	virtual void SetWaitingEvents(u32 flagsToWaitFor){}

	//returns true if signalled
	virtual bool CheckSignalled() throw(ting::Exc){
		return this->readinessFlags != 0;
	}



#else //assume *nix
protected:
	virtual int GetHandle() = 0;
#endif
};//~class Waitable






class WaitSet{
	uint numWaitables;//number of Waitables added
//	typedef std::vector<Waitable*> T_WaitablesList;
//	typedef T_WaitablesList::iterator T_WaitablesIter;
//	T_WaitablesList waitables;

#if defined(__WIN32__)
	Array<Waitable*> waitables;
	Array<HANDLE> handles; //used to pass array of HANDLEs to WaitForMultipleObjectsEx()

#else //assume *nix
	int epollSet;

	Array<epoll_event> revents;//used for getting the result from epoll_wait()
#endif

public:

	WaitSet(u32 maxSize) :
			numWaitables(0)
#if defined(__WIN32__)
			,waitables(maxSize)
			,handles(maxSize)
	{
		ASSERT_INFO(maxSize <= MAXIMUM_WAIT_OBJECTS, "maxSize should be less than " << MAXIMUM_WAIT_OBJECTS)
		if(maxSize > MAXIMUM_WAIT_OBJECTS)
			throw ting::Exc("WaitSet::WaitSet(): requested WaitSet size is too big");
	}

#else //assume *nix
			,revents(maxSize)
	{
		ASSERT(int(maxSize) > 0)
		this->epollSet = epoll_create(int(maxSize));
		if(this->epollSet < 0){
			throw ting::Exc("WaitSet::WaitSet(): epoll_create() failed");
		}
	}
#endif



	~WaitSet(){
		//assert the wait set is empty
		ASSERT(this->numWaitables == 0)
#if defined(__WIN32__)
		//do nothing
#else //assume *nix
		close(this->epollSet);
#endif
	}



	inline void Add(Waitable* w, u32 flagsToWaitFor){
//		TRACE(<< "WaitSet::Add(): enter" << std::endl)
		ASSERT(w)
		
		ASSERT(!w->isAdded)
		ASSERT(flagsToWaitFor != 0)//wait for at least something

#if defined(__WIN32__)
		ASSERT(this->numWaitables <= this->handles.Size())
		if(this->numWaitables == this->handles.Size())
			throw ting::Exc("WaitSet::Add(): wait set is full");

		//NOTE: Setting wait flags may throw an exception, so do that before
		//adding object to the array and incrementing number of added objects.
		w->SetWaitingEvents(flagsToWaitFor);

		this->handles[this->numWaitables] = w->GetHandle();
		this->waitables[this->numWaitables] = w;

#else //assume *nix
		epoll_event e;
		e.data.fd = w->GetHandle();
		e.data.ptr = w;
		e.events = (flagsToWaitFor & Waitable::READ ? (EPOLLIN | EPOLLPRI) : 0) |
				(flagsToWaitFor & Waitable::WRITE ? EPOLLOUT : 0);
		int res = epoll_ctl(
				this->epollSet,
				EPOLL_CTL_ADD,
				w->GetHandle(),
				&e
			);
		if(res < 0)
			throw ting::Exc("WaitSet::Add(): epoll_ctl() failed");
#endif

		++this->numWaitables;

		w->isAdded = true;
//		TRACE(<< "WaitSet::Add(): exit" << std::endl)
	}



	inline void Change(Waitable* w, u32 flagsToWaitFor){
		ASSERT(w)

		ASSERT(w->isAdded)
		ASSERT(flagsToWaitFor != 0)//wait for at least something

#if defined(__WIN32__)
		//check if the Waitable object is added to this wait set
		{
			uint i;
			for(i = 0; i < this->numWaitables; ++i){
				if(this->waitables[i] == w)
					break;
			}
			ASSERT(i <= this->numWaitables)
			if(i == this->numWaitables)
				throw ting::Exc("WaitSet::Change(): the Waitable is not added to this wait set");
		}

		//set new wait flags
		w->SetWaitingEvents(flagsToWaitFor);

#else //assume *nix
		epoll_event e;
		e.data.fd = w->GetHandle();
		e.data.ptr = w;
		e.events = (flagsToWaitFor & Waitable::READ ? (EPOLLIN | EPOLLPRI) : 0) |
				(flagsToWaitFor & Waitable::WRITE ? EPOLLOUT : 0);
		int res = epoll_ctl(
				this->epollSet,
				EPOLL_CTL_MOD,
				w->GetHandle(),
				&e
			);
		if(res < 0)
			throw ting::Exc("WaitSet::Change(): epoll_ctl() failed");
#endif
	}



	inline void Remove(Waitable* w){
		ASSERT(w)

		ASSERT(w->isAdded)

#if defined(__WIN32__)
		//remove object from array
		{
			uint i;
			for(i = 0; i < this->numWaitables; ++i){
				if(this->waitables[i] == w)
					break;
			}
			ASSERT(i <= this->numWaitables)
			if(i == this->numWaitables)
				throw ting::Exc("WaitSet::Change(): the Waitable is not added to this wait set");

			ting::uint numObjects = this->numWaitables - 1;//decrease number of objects before shifting the object handles in the array
			//shift object handles in the array
			for(; i < numObjects; ++i){
				this->handles[i] = this->handles[i + 1];
				this->waitables[i] = this->waitables[i + 1];
			}
		}

		//clear wait flags (disassociate socket and Windows event)
		w->SetWaitingEvents(0);

#else //assume *nix
		int res = epoll_ctl(
				this->epollSet,
				EPOLL_CTL_DEL,
				w->GetHandle(),
				0
			);
		if(res < 0)
			throw ting::Exc("WaitSet::Remove(): epoll_ctl() failed");
#endif

		--this->numWaitables;

		w->isAdded = false;
//		TRACE(<< "WaitSet::Remove(): completed successfuly" << std::endl)
	}



	inline uint Wait(){
		return this->Wait(true, 0);
	}



	inline uint Wait(u32 timeout){
		return this->Wait(false, timeout);
	}



private:
	uint Wait(bool waitInfinitly, u32 timeout){
#if defined(__WIN32__)
		if(timeout == INFINITE)
			timeout -= 1;

		DWORD waitTimeout = waitInfinitly ? (INFINITE) : DWORD(timeout);
		ASSERT(waitTimeout >= 0)
		DWORD res = WaitForMultipleObjectsEx(
				this->numWaitables,
				this->handles.Begin(),
				FALSE, //do not wait for all objects, wait for at least one
				waitTimeout,
				FALSE
			);

		ASSERT(res != WAIT_IO_COMPLETION)//it is impossible becaus we supplied FALSE as last parameter to WaitForMultipleObjectsEx()

		if(res == WAIT_FAILED)
			throw ting::Exc("WaitSet::Wait(): WaitForMultipleObjectsEx() failed");

		if(res == WAIT_TIMEOUT)
			return 0;

		//check for activities
		uint numEvents = 0;
		for(uint i = 0; i < this->numWaitables; ++i){
			if(this->waitables[i]->CheckSignalled()){
				++numEvents;
			}
		}
		
		return numEvents;

#else //assume *nix
		ASSERT(int(timeout) >= 0)
		int epollTimeout = waitInfinitly ? (-1) : int(timeout);

//		TRACE(<< "going to epoll_wait() with timeout = " << epollTimeout << std::endl)

		int res = epoll_wait(
				this->epollSet,
				this->revents.Begin(),
				this->revents.Size(),
				epollTimeout
			);

//		TRACE(<< "epoll_wait() returned " << res << std::endl)

		if(res < 0)
			throw ting::Exc("WaitSet::Wait(): epoll_wait() failed");

		ASSERT(uint(res) <= this->revents.Size())

		for(
				epoll_event *e = this->revents.Begin();
				e < this->revents.Begin() + res;
				++e
			)
		{
			Waitable* w = static_cast<Waitable*>(e->data.ptr);
			ASSERT(w)
			if((e->events & (EPOLLIN | EPOLLPRI)) != 0){
				w->SetCanReadFlag();
			}
			if((e->events & EPOLLOUT) != 0){
				w->SetCanWriteFlag();
			}
		}

		ASSERT(res >= 0)//NOTE: 'res' can be zero, if no events happened in the specified timeout
		return uint(res);
#endif
	}
};//~class WaitSet



}//~namespace ting