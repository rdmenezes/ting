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

/**
 * @file Timer.hpp
 * @author Ivan Gagis <igagis@gmail.com>
 * @author Jose Luis Hidalgo <joseluis.hidalgo@gmail.com> - Mac OS X port
 * @brief Timer library.
 */

#pragma once

//#define M_ENABLE_TIMER_TRACE
#ifdef M_ENABLE_TIMER_TRACE
#define M_TIMER_TRACE(x) TRACE(<<"[Timer]" x)
#else
#define M_TIMER_TRACE(x)
#endif

#ifdef _MSC_VER //If Microsoft C++ compiler
#pragma warning(disable:4290) //WARNING: C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
#endif

//  ==System dependent headers inclusion==
#if defined(__WIN32__) || defined(WIN32)
#ifndef __WIN32__
#define __WIN32__
#endif

#include <windows.h>

#elif defined(__APPLE__)

#include<sys/time.h>

#elif defined(__linux__)

#include <ctime>

#else
#error "Unknown OS"
#endif
//~ ==System dependent headers inclusion==

#include <vector>
#include <map>
#include <algorithm>

#include "debug.hpp" //debugging facilities
#include "types.hpp"
#include "Singleton.hpp"
#include "Thread.hpp"
#include "math.hpp"
#include "Signal.hpp"



namespace ting{



//forward declarations
class Timer;



//function prototypes
inline ting::u32 GetTicks();



const unsigned DMaxTicks = 0xffffffff;// TODO: change to ting::u32(-1) after testing



class TimerLib : public Singleton<TimerLib>{
	friend class ting::Timer;

	class TimerThread : public ting::Thread{
	public:
		ting::Inited<bool, false> quitFlag;

		ting::Mutex mutex;
		ting::Semaphore sema;

        //map requires key uniquiness, but in our case the key is a stop ticks,
        //so, use multimap to allow similar keys.
		typedef std::multimap<ting::u64, Timer*> T_TimerList;
		typedef T_TimerList::iterator T_TimerIter;
		T_TimerList timers;



		ting::Inited<ting::u64, 0> ticks;
		ting::Inited<bool, false> incTicks;//flag indicates that high dword of ticks needs increment

        //This function should be called at least once in 16 days.
        //This should be achieved by having a repeating timer set to 16 days, which will do nothing but
        //calling this function.
        inline ting::u64 GetTicks();



		TimerThread(){
			ASSERT(!this->quitFlag)
		}

		~TimerThread(){
			//at the time of TimerLib destroying there should be no active timers
			ASSERT(this->timers.size() == 0)
		}

		inline void AddTimer(Timer* timer, u32 timeout);

		inline bool RemoveTimer(Timer* timer);

		inline void SetQuitFlagAndSignalSemaphore(){
			this->quitFlag = true;
			this->sema.Signal();
		}

		//override (inline is just to make possible method definition in header file)
		inline void Run();

	private:
//		inline void UpdateTimer(Timer* timer, u32 newTimeout);

	} thread;

public:
	TimerLib(){
		this->thread.Start();

        //TODO: add timer for half of the max ticks
	}

	~TimerLib(){
        ASSERT(this->thread.timers.size() == 0)
		this->thread.SetQuitFlagAndSignalSemaphore();
		this->thread.Join();
	}
};



class Timer{
	friend class TimerLib::TimerThread;

    ting::Inited<bool, false> isRunning;//true if timer has been started and has not stopped yet

private:
	ting::TimerLib::TimerThread::T_TimerIter i;//if timer is running, this is the iterator into the map of timers

public:

	ting::Signal1<Timer&> expired;

	inline Timer(){
		ASSERT(!this->isRunning)
	}

	virtual ~Timer(){
		ASSERT(TimerLib::IsCreated())
		this->Stop();

		ASSERT(!this->isRunning)
	}

    /**
     * @brief Start timer.
     * After calling this method one can be sure that the timer state has been
     * switched to running. This means that if you call Stop() after that and it
     * returns false then this will mean that the timer has expired rather than not started.
     * @param millisec - timer timeout in milliseconds.
     */
    //TODO: add note about calling Start from expired signal handler.
	inline void Start(ting::u32 millisec){
		ASSERT_INFO(TimerLib::IsCreated(), "Timer library is not initialized, you need to create TimerLib singletone object first")

        TimerLib::Inst().thread.AddTimer(this, millisec);
	}

	/**
	 * @brief Stop the timer.
	 * Stops the timer if it was started before. In case it was not started
	 * or it has already expired this method does nothing.
	 * @return true if timer was running and was stopped.
     * @return false if timer was not running already when the Stop() method was called. I.e.
     *         the timer has expired already or was not started.
	 */
	inline bool Stop(){
		ASSERT(TimerLib::IsCreated())
		return TimerLib::Inst().thread.RemoveTimer(this);
	}
};



inline bool TimerLib::TimerThread::RemoveTimer(Timer* timer){
	ASSERT(timer)
	ting::Mutex::Guard mutexGuard(this->mutex);

	if(!timer->isRunning){
		return false;
    }

	//if isStarted flag is set then the timer will be stopped now, so
	//change the flag
	timer->isRunning = false;

	ASSERT(timer->i != this->timers.end())

	this->timers.erase(timer->i);

	//was running
	return true;
}



inline void TimerLib::TimerThread::AddTimer(Timer* timer, u32 timeout){
	ASSERT(timer)
	ting::Mutex::Guard mutexGuard(this->mutex);

	if(timer->isRunning){
		throw ting::Exc("TimerLib::TimerThread::AddTimer(): timer is already running!");
    }

	timer->isRunning = true;

	ting::u64 stopTicks = this->GetTicks() + ting::u64(timeout);

    timer->i = this->timers.insert(
            std::pair<ting::u64, ting::Timer*>(stopTicks, timer)
        );
    
    ASSERT(timer->i != this->timers.end())
    ASSERT(timer->i->second)

    //signal the semaphore about new timer addition in order to recalculate the waiting time
	this->sema.Signal();
}



inline ting::u64 TimerLib::TimerThread::GetTicks(){
    ting::u32 ticks = ting::GetTicks() % DMaxTicks;

    if(this->incTicks){
        if(ticks < DMaxTicks / 2){
            this->incTicks = false;
            this->ticks += (ting::u64(DMaxTicks) + 1); //update 64 bit ticks counter
        }
    }else{
        if(ticks > DMaxTicks / 2){
            this->incTicks = true;
        }
    }

    return this->ticks + ting::u64(ticks);
}



//override
inline void TimerLib::TimerThread::Run(){
	M_TIMER_TRACE(<< "TimerLib::TimerThread::Run(): enter" << std::endl)
            
    ting::Mutex::Guard mutexGuard(this->mutex);
    
	while(!this->quitFlag){

        ting::u64 ticks = this->GetTicks();

        std::vector<Timer*> expiredTimers;
        
        for(T_TimerIter b = this->timers.begin(); b != this->timers.end(); b = this->timers.begin()){
            if(b->first <= ticks){
                Timer *timer = b->second;
                //add the timer to list of expired timers and change the timer state to not running
                ASSERT(timer)
                expiredTimers.push_back(timer);
                
                timer->isRunning = false;
                
                this->timers.erase(b);
                continue;
            }
            break;
        }
        
        //calculate new waiting time
        ASSERT(this->timers.begin()->first > ticks)
        ASSERT(this->timers.begin()->first - ticks <= ting::u64(ting::u32(-1)))
        ting::u32 millis = ting::u32(this->timers.begin()->first - ticks);
        
        //TODO: zero out the semaphore
        
		this->mutex.Unlock();
//
        //emit expired signal
        for(std::vector<Timer*>::iterator i = expiredTimers.begin(); i != expiredTimers.end(); ++i){
            (*i)->expired.Emit(*(*i));
        }
        
//		M_TIMER_TRACE(<< "TimerThread: waiting for " << millis << " ms" << std::endl)
        
        //TODO: recalculate new waiting time here getting ticks again???
        
        //It does not matter signaled or timed out
		this->sema.Wait(millis);
        
//		M_TIMER_TRACE(<< "TimerThread: signalled" << std::endl)
		
		this->mutex.Lock();
	}//~while

	M_TIMER_TRACE(<< "TimerLib::TimerThread::Run(): exit" << std::endl)
}//~Run()



/**
 * @brief Get constantly increasing millisecond ticks.
 * It is not guaranteed that the ticks counting started at the system start.
 * @return constantly increasing millisecond ticks.
 */
inline ting::u32 GetTicks(){
#ifdef __WIN32__
	static LARGE_INTEGER perfCounterFreq = {{0, 0}};
	if(perfCounterFreq.QuadPart == 0){
		if(QueryPerformanceFrequency(&perfCounterFreq) == FALSE){
			//looks like the system does not support high resolution tick counter
			return GetTickCount();
		}
	}
	LARGE_INTEGER ticks;
	if(QueryPerformanceCounter(&ticks) == FALSE){
		return GetTickCount();
	}

	return ting::u32((ticks.QuadPart * 1000) / perfCounterFreq.QuadPart);
#elif defined(__APPLE__)
	//Mac os X doesn't support clock_gettime
	timeval t;
	gettimeofday(&t, 0);
	return ting::u32(t.tv_sec * 1000 + (t.tv_usec / 1000));
#elif defined(__linux__)
	timespec ts;
	if(clock_gettime(CLOCK_MONOTONIC, &ts) == -1)
		throw ting::Exc("GetTicks(): clock_gettime() returned error");

	return u32(u32(ts.tv_sec) * 1000 + u32(ts.tv_nsec / 1000000));
#else
#error "Unsupported OS"
#endif
}



}//~namespace
