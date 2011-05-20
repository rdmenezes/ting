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



/**
 * @brief General purpose timer.
 * This is a class representing a timer. Its accuracy is not expected to be high,
 * approximately it is tens of milliseconds, i.e. 0.01 second.
 * Before using the timers it is necessary to initialize the timer library, see
 * description of ting::TimerLib class for details.
 */
class Timer{
	friend class TimerLib;

	//This constant is for testing purposes.
	//Should be set to ting::u32(-1) in release.
	inline static ting::u32 DMaxTicks(){
		return ting::u32(-1);
	}
	
	ting::Inited<bool, false> isRunning;//true if timer has been started and has not stopped yet

private:
	typedef std::multimap<ting::u64, Timer*> T_TimerList;
	typedef T_TimerList::iterator T_TimerIter;

	T_TimerIter i;//if timer is running, this is the iterator into the map of timers

public:

	/**
	 * @brief Timer expiration handler.
	 * This method is called when timer expires.
	 * Note, that the method is called from a separate thread, so user should
	 * do all the necessary synchronization when implementing this method.
	 * Also, note that expired methods from different timers are called sequentially,
	 * that means that, for example, if two timers have expired simultaneously then
	 * the expired method of the first timer is called first, and only after it returns
	 * the expired method of the second timer is called.
	 * That means, that one should handle the timer expiration as fast as possible to
	 * avoid inaccuracy of other timers which have expired at the same time, since
	 * the longer your expired handler method is executed, the latter expired method of those other timers will be called.
	 * Do not do any heavy calculations of logics in the expired handler method. Do just
	 * quick initiation of the action which should be taken on timer expiration,
	 * for example, post a message to the message queue of another thread to be handled by that another thread.
	 */
	virtual void OnExpired() = 0;

	/**
	 * @brief Constructor for new Timer instance.
	 * The newly created timer is initially not running.
	 */
	inline Timer(){
		ASSERT(!this->isRunning)
	}

	virtual ~Timer();

	/**
	 * @brief Start timer.
	 * After calling this method one can be sure that the timer state has been
	 * switched to running. This means that if you call Stop() after that and it
	 * returns false then this will mean that the timer has expired rather than not started.
	 * It is allowed to call the Start() method from within the handler of the timer expired signal.
	 * If the timer is already running (i.e. it was already started before and has not expired yet)
	 * the ting::Exc exception will be thrown.
	 * @param millisec - timer timeout in milliseconds.
	 */
	inline void Start(ting::u32 millisec);

	/**
	 * @brief Stop the timer.
	 * Stops the timer if it was started before. In case it was not started
	 * or it has already expired this method does nothing.
	 * @return true if timer was running and was stopped.
	 * @return false if timer was not running already when the Stop() method was called. I.e.
	 *         the timer has expired already or was not started.
	 */
	inline bool Stop();
};



/**
 * @brief Timer library singleton class.
 * This is a singleton class which represents timer library which allows using
 * timers (see ting::Timer class). Before using timers one needs to initialize
 * the timer library, this is done just by creating the singleton object of
 * the timer library class.
 */
class TimerLib : public Singleton<TimerLib>{
	friend class ting::Timer;

	class TimerThread : public ting::Thread{
	public:
		ting::Inited<volatile bool, false> quitFlag;

		ting::Mutex mutex;
		ting::Semaphore sema;

		//map requires key uniqueness, but in our case the key is a stop ticks,
		//so, use std::multimap to allow similar keys.
		Timer::T_TimerList timers;



		ting::Inited<ting::u64, 0> ticks;
		ting::Inited<bool, false> incTicks;//flag indicates that high dword of ticks needs increment

		//This function should be called at least once in 16 days (half of ting::u32(-1) milliseconds).
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

		inline void AddTimer_ts(Timer* timer, u32 timeout);

		inline bool RemoveTimer_ts(Timer* timer);

		inline void SetQuitFlagAndSignalSemaphore(){
			this->quitFlag = true;
			this->sema.Signal();
		}

		//override (inline is just to make possible method definition in header file)
		inline void Run();

	} thread;

	class HalfMaxTicksTimer : public Timer{
	public:
		//override
		void OnExpired(){
			this->Start(Timer::DMaxTicks() / 2);
		}
	} halfMaxTicksTimer;

public:
	inline TimerLib(){
		this->thread.Start();

		//start timer for half of the max ticks
		this->halfMaxTicksTimer.OnExpired();
	}

	/**
	 * @brief Destructor.
	 * Note, that before destroying the timer library singleton object all the
	 * timers should be stopped. Otherwise, in debug mode it will result in assertion failure.
	 */
	~TimerLib(){
#ifdef DEBUG
		{
			ting::Mutex::Guard mutexGuard(this->thread.mutex);
			ASSERT(this->thread.timers.size() == 1) // 1 for half max ticks timer
		}
#endif
		this->thread.SetQuitFlagAndSignalSemaphore();
		this->thread.Join();
	}
};



inline Timer::~Timer(){
	ASSERT(TimerLib::IsCreated())
	this->Stop();

	ASSERT(!this->isRunning)
}



inline void Timer::Start(ting::u32 millisec){
	ASSERT_INFO(TimerLib::IsCreated(), "Timer library is not initialized, you need to create TimerLib singletone object first")

	TimerLib::Inst().thread.AddTimer_ts(this, millisec);
}



inline bool Timer::Stop(){
	ASSERT(TimerLib::IsCreated())
	return TimerLib::Inst().thread.RemoveTimer_ts(this);
}



inline bool TimerLib::TimerThread::RemoveTimer_ts(Timer* timer){
	ASSERT(timer)
	ting::Mutex::Guard mutexGuard(this->mutex);

	if(!timer->isRunning){
		return false;
	}

	//if isStarted flag is set then the timer will be stopped now, so
	//change the flag
	timer->isRunning = false;

	ASSERT(timer->i != this->timers.end())

	//if that was the first timer, signal the semaphore about timer deletion in order to recalculate the waiting time
	if(this->timers.begin() == timer->i){
		this->sema.Signal();
	}

	this->timers.erase(timer->i);

	//was running
	return true;
}



inline void TimerLib::TimerThread::AddTimer_ts(Timer* timer, u32 timeout){
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
	ting::u32 ticks = ting::GetTicks() % Timer::DMaxTicks();

	if(this->incTicks){
		if(ticks < Timer::DMaxTicks() / 2){
			this->incTicks = false;
			this->ticks += (ting::u64(Timer::DMaxTicks()) + 1); //update 64 bit ticks counter
		}
	}else{
		if(ticks > Timer::DMaxTicks() / 2){
			this->incTicks = true;
		}
	}

	return this->ticks + ting::u64(ticks);
}



//override
inline void TimerLib::TimerThread::Run(){
	M_TIMER_TRACE(<< "TimerLib::TimerThread::Run(): enter" << std::endl)

	while(!this->quitFlag){
		ting::u32 millis;

		while(true){
			std::vector<Timer*> expiredTimers;

			{
				ting::Mutex::Guard mutexGuard(this->mutex);

				ting::u64 ticks = this->GetTicks();

				for(Timer::T_TimerIter b = this->timers.begin(); b != this->timers.end(); b = this->timers.begin()){
					if(b->first > ticks){
						break;//~for
					}

					Timer *timer = b->second;
					//add the timer to list of expired timers
					ASSERT(timer)
					expiredTimers.push_back(timer);

					//Change the expired timer state to not running.
					//This should be done before the expired signal of the timer will be emitted.
					timer->isRunning = false;

					this->timers.erase(b);
				}

				if(expiredTimers.size() == 0){
					ASSERT(this->timers.size() > 0) //if we have no expired timers here, then at least one timer should be running (the half-max-ticks timer).

					//calculate new waiting time
					ASSERT(this->timers.begin()->first > ticks)
					ASSERT(this->timers.begin()->first - ticks <= ting::u64(ting::u32(-1)))
					millis = ting::u32(this->timers.begin()->first - ticks);

					//zero out the semaphore for optimization purposes
					while(this->sema.Wait(0)){}

					break;//~while(true)
				}
			}

			//emit expired signal for expired timers
			for(std::vector<Timer*>::iterator i = expiredTimers.begin(); i != expiredTimers.end(); ++i){
				ASSERT(*i)
				(*i)->OnExpired();
			}
		}

		this->sema.Wait(millis);
	}//~while(!this->quitFlag)

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
