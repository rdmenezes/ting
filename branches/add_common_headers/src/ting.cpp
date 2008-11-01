/* The MIT License:

Copyright (c) 2008 Ivan Gagis

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

// (c) Ivan Gagis
// e-mail: igagis@gmail.com
// Version: 1

// Description:
//          ting is a ThreadING library

#include "ting.hpp"

#include <cassert>

#define M_TING_ASSERT(x) assert(x);

//
//  Windows specific definitions
//
#if defined(__WIN32__) || defined(WIN32)
#ifndef __WIN32__
#define __WIN32__
#endif

#include <windows.h>
typedef HANDLE ThreadType;
typedef CRITICAL_SECTION MutexType;
typedef HANDLE SemaphoreType;

#elif defined(__SYMBIAN32__)
#include <string.h>
#include <e32std.h>
#include <hal.h>
typedef RThread ThreadType;
typedef RCriticalSection MutexType;
typedef RSemaphore SemaphoreType;

#undef M_TING_ASSERT
#define M_TING_ASSERT(x) __ASSERT_ALWAYS((x),User::Panic(_L("ASSERTION FAILED!"),3));

#else //assume pthread
#define M_PTHREAD
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <ctime>
typedef pthread_t ThreadType;
typedef pthread_mutex_t MutexType;
typedef pthread_cond_t CondType;
typedef sem_t SemaphoreType;

#if defined(sun) || defined(__sun)
#include <sched.h>	//	for sched_yield();
#endif

#endif

using namespace ting;

inline static ThreadType& CastToThread(void* handle){
    return *reinterpret_cast<ThreadType*>(handle);
};

//inline static const ThreadType& CastToThread(ting::Thread::SystemIndependentThreadHandle& thr){
//    return CastToThread(const_cast<ting::Thread::C_SystemIndependentThreadHandle&>(thr));
//};

inline static MutexType& CastToMutex(void* handle){
    return *reinterpret_cast<MutexType*>(handle);
};

#if !defined(__WIN32__) && !defined(__SYMBIAN32__)
inline static CondType& CastToCondVar(void* handle){
    return *reinterpret_cast<CondType*>(handle);
};
#endif

inline static SemaphoreType& CastToSemaphore(void* handle){
    return *reinterpret_cast<SemaphoreType*>(handle);
};

#ifdef __WIN32__
static DWORD __stdcall RunThread(void *data)
#elif defined(__SYMBIAN32__)
static TInt RunThread(TAny *data)
#elif defined(M_PTHREAD) //pthread
static void* RunThread(void *data)
#else
#error "Unknown platform"
#endif
{
    ting::Thread *thr = reinterpret_cast<ting::Thread*>(data);
    try{
        thr->Run();
    }catch(...){
        M_TING_ASSERT(false)
    }

#ifdef M__PTHREAD
    pthread_exit(0);
#endif
    return 0;
};

Thread::Thread() :
		preallocatedQuitMessage(new QuitMessage(this)),
		isRunning(false),
        quitFlag(false)
{
	this->handle = new ThreadType();
#if defined(__WIN32__)
    CastToThread(this->handle) = NULL;
#endif
};

void Thread::Start(uint stackSize){
    //protect by mutex to avoid several Start() methods to be called by concurrent threads simultaneously
    ting::Mutex::LockerUnlocker mutexLockerUnlocker(this->mutex);

    if(this->isRunning){
        throw ting::Exc("Thread::Start(): Thread is already running");
    }

    ThreadType& t = CastToThread(this->handle);

#ifdef __WIN32__
    t = CreateThread(NULL, static_cast<size_t>(stackSize), &RunThread, reinterpret_cast<void*>(this), 0, NULL);
    if(t == NULL){
        throw ting::Exc("Thread::Start(): starting thread failed");
    }
#elif defined(__SYMBIAN32__)

    if( t.Create(_L("ting thread"), &RunThread,
                                       stackSize==0 ? KDefaultStackSize : stackSize,
                                       NULL, reinterpret_cast<TAny*>(this)) != KErrNone){
        throw ting::Exc("Thread::Start(): starting thread failed");
    }
    t.Resume();//start the thread execution
#else //pthread
    {
        pthread_attr_t attr;

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        pthread_attr_setstacksize (&attr, static_cast<size_t>(stackSize));

        if(pthread_create(&t, &attr, &RunThread, this) !=0){
            pthread_attr_destroy(&attr);
            throw ting::Exc("Thread::Start(): starting thread failed");
        }
        pthread_attr_destroy(&attr);
    }
#endif

    this->isRunning = true;
};

void Thread::Join(){
    //protect by mutex to avoid several Join() methods to be called by concurrent threads simultaneously
    ting::Mutex::LockerUnlocker mutexLockerUnlocker(this->mutex);

    if(!this->isRunning)
        return;

    ThreadType& t = CastToThread(this->handle);
#ifdef __WIN32__
    WaitForSingleObject(t, INFINITE);
    CloseHandle(t);
    t = NULL;
#elif defined(__SYMBIAN32__)
    {
        TRequestStatus reqStat;
        t.Logon(reqStat);
        User::WaitForRequest(reqStat);
        t.Close();
    }
#else //pthread
    pthread_join(t, 0);
#endif
    this->isRunning = false;
};

Thread::~Thread(){
    this->quitFlag = true;
    this->PushQuitMessage();
    this->Join();
    delete reinterpret_cast<ThreadType*>(this->handle);
};

void Thread::PushQuitMessage(){
//	ASSERT(this->preallocatedQuitMessage)
    this->PushMessage(this->preallocatedQuitMessage);
};

//static
void Thread::Sleep(uint msec){
#ifdef __WIN32__
    SleepEx(DWORD(msec), FALSE);// Sleep() crashes on mingw (I do not know why), this is why I use SleepEx() here.
#elif defined(__SYMBIAN32__)
    User::After(msec*1000);
#else //assume pthreads
    if(msec == 0){
#if defined(sun) || defined(__sun)
		sched_yield();
#else
        pthread_yield();
#endif
    }else{
        usleep(msec*1000);
    }
#endif
};



ting::Exc::Exc(const char* message) throw(){
    if(message==0)
        message = "unknown exception";

    size_t len = strlen(message);

#ifdef __SYMBIAN32__
    //if I'm right in symbian simple new operator does not throw or leave, it will return 0 in case of error
    this->msg = new char[len+1];
#else
    //we do not want another exception, use std::nothrow
    this->msg = new(std::nothrow) char[len+1];
#endif
    if(!this->msg)
        return;
    memcpy(this->msg, message, len);
    this->msg[len] = 0;//null-terminate
};

ting::Exc::~Exc() throw(){
    delete[] this->msg;
};



Mutex::Mutex(){
	this->handle = new MutexType();
	MutexType& m = CastToMutex(this->handle);
#ifdef __WIN32__
    InitializeCriticalSection(&m);
#elif defined(__SYMBIAN32__)
    if( m.CreateLocal() != KErrNone){
        throw ting::Exc("Mutex::Mutex(): failed creating mutex");
    }
#else //pthread
    pthread_mutex_init(&m, NULL);
#endif
};

Mutex::~Mutex(){
	MutexType& m = CastToMutex(this->handle);
#ifdef __WIN32__
    DeleteCriticalSection(&m );
#elif defined(__SYMBIAN32__)
    m.Close();
#else //pthread
    pthread_mutex_destroy(&m);
#endif
    delete reinterpret_cast<MutexType*>(this->handle);
};

void Mutex::Lock(){
	MutexType& m = CastToMutex(this->handle);
#ifdef __WIN32__
    EnterCriticalSection(&m);
#elif defined(__SYMBIAN32__)
    m.Wait();
#else //pthread
    pthread_mutex_lock(&m);
#endif
};

void Mutex::Unlock(){
	MutexType& m = CastToMutex(this->handle);
#ifdef __WIN32__
    LeaveCriticalSection(&m);
#elif defined(__SYMBIAN32__)
    m.Signal();
#else //pthread
    pthread_mutex_unlock(&m);
#endif
};


Semaphore::Semaphore(uint initialValue){
//    std::cout<<"Semaphore::Semaphore(): enter"<<std::endl;
	this->handle = new SemaphoreType();
	SemaphoreType& s = CastToSemaphore(this->handle);
#ifdef __WIN32__
    s = CreateSemaphore(NULL, initialValue, 0xffffff, NULL);
    if(s == NULL){
//        std::cout<<"Semaphore::Semaphore(): failed"<<std::endl;
        throw ting::Exc("Semaphore::Semaphore(): creating semaphore failed");
    }
#elif defined(__SYMBIAN32__)
    if(s.CreateLocal(initialValue) != KErrNone){
        throw ting::Exc("Semaphore::Semaphore(): creating semaphore failed");
    }
#else //pthread
    if( sem_init(&s, 0, initialValue) < 0 ){
        throw ting::Exc("Semaphore::Semaphore(): creating semaphore failed");
    }
#endif
//    std::cout<<"Semaphore::Semaphore(): exit"<<std::endl;
};

Semaphore::~Semaphore(){
	SemaphoreType& s = CastToSemaphore(this->handle);
#ifdef __WIN32__
    CloseHandle(s);
#elif defined(__SYMBIAN32__)
    s.Close();
#else //pthread
    sem_destroy(&s);
#endif
    delete reinterpret_cast<SemaphoreType*>(this->handle);
};

bool Semaphore::Wait(uint timeoutMillis){
//    std::cout<<"Semaphore::Wait(): enter"<<std::endl;
	SemaphoreType& s = CastToSemaphore(this->handle);
#ifdef __WIN32__
    switch( WaitForSingleObject(s, DWORD(timeoutMillis==0?INFINITE:timeoutMillis)) ){
        case WAIT_OBJECT_0:
//            std::cout<<"Semaphore::Wait(): exit"<<std::endl;
            return true;
        case WAIT_TIMEOUT:
            return false;
            break;
        default:
            throw ting::Exc("Semaphore::Wait(): wait failed");
            break;
    }
#elif defined(__SYMBIAN32__)
    if(timeoutMillis == 0){
        s.Wait();
    }else{
        throw ting::Exc("Semaphore::Wait(): timeout wait unimplemented on Symbian, TODO: implement");
    }
#else //pthread
    if(timeoutMillis == 0){
        int retVal;
        do{
            retVal = sem_wait(&s);
        }while(retVal == -1 && errno == EINTR);
        if(retVal < 0){
            throw ting::Exc("Semaphore::Wait(): wait failed");
        }
    }else{
        timespec ts;
        ts.tv_sec = timeoutMillis / 1000;
        ts.tv_nsec = (timeoutMillis%1000)*1000*1000;
        if(sem_timedwait(&s, &ts) != 0){
            if(errno == ETIMEDOUT)
                return false;
            else
                throw ting::Exc("Semaphore::Wait(): error");
        }
        return true;
    }
#endif
    return false;
};

void Semaphore::Post(){
//    std::cout<<"Semaphore::Post(): enter"<<std::endl;
	SemaphoreType& s = CastToSemaphore(this->handle);
#ifdef __WIN32__
    if( ReleaseSemaphore(s, 1, NULL) == 0 ){
        throw ting::Exc("Semaphore::Post(): releasing semaphore failed");
    }
#elif defined(__SYMBIAN32__)
    s.Signal();
#else //pthread
    if(sem_post(&s) < 0){
        throw ting::Exc("Semaphore::Post(): releasing semaphore failed");
    }
#endif
//    std::cout<<"Semaphore::Post(): exit"<<std::endl;
};


CondVar::CondVar(){
#if defined(__WIN32__) || defined(__SYMBIAN32__)
    this->mutex = new Mutex();
    this->sem_wait = new Semaphore();
    this->sem_done = new Semaphore();
    this->num_waiters = 0;
    this->num_signals = 0;
#else //pthread
    this->handle = new CondType();
    pthread_cond_init(&CastToCondVar(this->handle), NULL);
#endif
};

CondVar::~CondVar(){
#if defined(__WIN32__) || defined(__SYMBIAN32__)
    delete this->mutex;
    delete this->sem_wait;
    delete this->sem_done;
#else
    pthread_cond_destroy(&CastToCondVar(this->handle));
    delete reinterpret_cast<CondType*>(this->handle);
#endif
};

void CondVar::Wait(Mutex &mutex){
#if defined(__WIN32__) || defined(__SYMBIAN32__)
    this->mutex->Lock();
    ++this->num_waiters;
    this->mutex->Unlock();

    mutex.Unlock();

//    std::cout<<"CondVar::Wait(): going to wait"<<std::endl;

    this->sem_wait->Wait();

    this->mutex->Lock();
    if(this->num_signals > 0){
        this->sem_done->Post();
        --this->num_signals;
    }
    --this->num_waiters;
    this->mutex->Unlock();

    mutex.Lock();
#else
    pthread_cond_wait(&CastToCondVar(this->handle), &CastToMutex(mutex.handle));
#endif
};

void CondVar::Notify(){
#if defined(__WIN32__) || defined(__SYMBIAN32__)
    this->mutex->Lock();

    if(this->num_waiters > this->num_signals){
        ++this->num_signals;
//        std::cout<<"CondVar::Notify(): going to post"<<std::endl;
        this->sem_wait->Post();
        this->mutex->Unlock();
        this->sem_done->Wait();
    }else{
        this->mutex->Unlock();
    }
#else
    pthread_cond_signal(&CastToCondVar(this->handle));
#endif
};

#ifdef __WIN32__
static LARGE_INTEGER perfCounterFreq = {0};
#elif defined(__SYMBIAN32__)
static TUint32 counterFreq = 0;
#endif

ting::u32 ting::GetTicks(){
#ifdef __WIN32__
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

    return uint((ticks.QuadPart * 1000) / perfCounterFreq.QuadPart);
#elif defined(__SYMBIAN32__)
    if(counterFreq == 0){
        TInt freq;
        HAL::Get(HALData::EFastCounterFrequency, freq);
        counterFreq = freq;
    }
    return u32( (u64(User::FastCounter()) * 1000) / counterFreq );
#else
    clock_t ticks = clock();
    u64 msec = u64(ticks) * 1000 / CLOCKS_PER_SEC;
    return u32(msec & u32(-1));
#endif
};


void Queue::PushMessage(ting::MsgAutoPtr msg){
    {
        Mutex::LockerUnlocker mutexLockerUnlocker(this->mut);
        if(this->first){
//            M_ASSERT(this->last)
//            M_ASSERT(this->last->next == 0)
            this->last = this->last->next = msg.release();
//            M_ASSERT(this->last->next == 0)
        }else{
//            M_ASSERT(msg.IsValid())
            this->last = this->first = msg.release();
        }
    }
    this->cond.Notify();
};

ting::MsgAutoPtr Queue::PeekMsg(){
    Mutex::LockerUnlocker mutexLockerUnlocker(this->mut);
    if(this->first){
        Message* ret = this->first;
        this->first = this->first->next;
        return ting::MsgAutoPtr(ret);
    }

    return ting::MsgAutoPtr();
};

ting::MsgAutoPtr Queue::GetMsg(){
    Mutex::LockerUnlocker mutexLockerUnlocker(this->mut);
    if(this->first){
        Message* ret = this->first;
        this->first = this->first->next;
        return ting::MsgAutoPtr(ret);
    }

    this->cond.Wait(this->mut);

//    M_ASSERT(this->first)

    Message* ret = this->first;
    this->first = this->first->next;
    return ting::MsgAutoPtr(ret);
};

Queue::~Queue(){
    //M_DEBUG_TRACE(<<"C_Queue::~C_Queue(): enter"<<std::endl)
    Mutex::LockerUnlocker mutexLockerUnlocker(this->mut);
    Message *msg = this->first,
            *nextMsg;
    while(msg){
        //M_DEBUG_TRACE(<<"C_Queue::~C_Queue(): msg="<<msg<<std::endl)
        nextMsg = msg->next;
        //M_DEBUG_TRACE(<<"C_Queue::~C_Queue(): nextMsg="<<nextMsg<<std::endl)
        delete msg;
        msg = nextMsg;
    }
};

void QuitMessage::Handle(){
    this->thr->quitFlag = true;
};
