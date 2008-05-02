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

#ifndef M_LIB_TING_HPP
#define M_LIB_TING_HPP

#if defined(M_BUILD_DLL) & defined(WIN32)
#define M_DECLSPEC __declspec(dllexport)
#else
#define M_DECLSPEC
#endif

#include <memory>
#include <exception>
#include <new>

namespace ting{

//=================
//= Static Assert =
//=================
#ifndef DOC_DONT_EXTRACT //direction to doxygen not to extract this class
template <bool b> struct StaticAssert{
    virtual void STATIC_ASSERTION_FAILED()=0;
    virtual ~StaticAssert(){};
};
template <> struct StaticAssert<true>{};
#define M_TING_STATIC_ASSERT_II(x, l) struct StaticAssertInst_##l{ \
    ting::StaticAssert<x> STATIC_ASSERTION_FAILED;};
#define M_TING_STATIC_ASSERT_I(x, l) M_TING_STATIC_ASSERT_II(x, l)
#define M_TING_STATIC_ASSERT(x)  M_TING_STATIC_ASSERT_I(x, __LINE__)
#endif //~DOC_DONT_EXTRACT
//= ~Static Assert =


typedef unsigned int uint;
typedef unsigned char byte;
typedef unsigned int u32;
typedef unsigned long long int u64;

M_TING_STATIC_ASSERT( byte(-1) == 0xff)
M_TING_STATIC_ASSERT( u32(-1) == 0xffffffff)
M_TING_STATIC_ASSERT( u64(-1) == 0xffffffffffffffffLL)

//forward declarations
class CondVar;
class Queue;
class Thread;

/**
@brief Basic exception class
*/
class M_DECLSPEC Exc : public std::exception{
    char *msg;
public:
    /**
    @brief Exception constructor.
    @param message Pointer to the exception message null-terminated string. Constructor will copy the string into objects internal memory buffer.
    */
    Exc(const char* message = 0) throw(std::bad_alloc);
    virtual ~Exc()throw();
    
    /**
    @brief Returns a pointer to exception message.
    @return a pointer to objects internal memory buffer holding the exception message null-terminated string.
            Note, that after the exception object is destroyed the pointer returned by this method become invalid.
    */
    //override from std::exception
    const char *what()const throw(){
        return this->msg;//this->msg is never 0 (see Exc constructor for more info).
    };
};

/**
@brief Mutex object class
Mutex stands for "Mutual execution".
*/
class M_DECLSPEC Mutex{
    friend class CondVar;
    
public:
#ifndef DOC_DONT_EXTRACT //direction to doxygen not to extract this class
    //Size of this struct should be large enough to hold mutex handle on all
    //platforms.
    //pthread requires 24 bytes.
    struct SystemIndependentMutexHandle{
        ting::byte handle[24];
    } mut;
#endif //~DOC_DONT_EXTRACT
    
private:
    //forbid copying
    Mutex(const Mutex& m){};
    Mutex(Mutex& m){};
    Mutex& operator=(const Mutex& m){return *this;};
    
public:
    Mutex();
    ~Mutex();
    
    /**
    @brief Acquire mutex lock.
    If one thread acquired the mutex lock then all other threads
    attempting to acquire the lock on the same mutex will wait until the
    mutex lock will be released.
    */
    void Lock();
    /**
    @brief Release mutex lock.
    */
    void Unlock();
    
    /**
    @breif Helper class which automatically Locks the given mutex.
    This helper class automatically locks the given mutex in the constructor and
    unlocks the mutex in destructor. This class is useful if the code between
    mutex lock/unlock may return or throw an exception,
    then the mutex will not remain locked in such case.
    */
    class LockerUnlocker{
        Mutex *mut;
      public:
        LockerUnlocker(Mutex &m):
                mut(&m)
        {
            this->mut->Lock();
        };
        ~LockerUnlocker(){
            this->mut->Unlock();
        };
    };
};//~class Mutex

/**
@brief Semaphore class.
*/
class M_DECLSPEC Semaphore{
public:
#ifndef DOC_DONT_EXTRACT //direction to doxygen not to extract this class
    //Size of this struct should be large enough to hold condvar handle on all
    //platforms.
    //pthread requires 16 bytes.
    struct SystemIndependentSemaphoreHandle{
        ting::byte handle[16];
    } sem;
#endif //~DOC_DONT_EXTRACT
    
private:
    //forbid copying
    Semaphore(const Semaphore& m){};
    Semaphore(Semaphore& m){};
    Semaphore& operator=(const Semaphore& m){return *this;};
public:
    
    Semaphore(uint initialValue = 0);
    ~Semaphore();
    
    bool Wait(uint timeoutMillis = 0);
    void Post();
};

class M_DECLSPEC CondVar{
public:
#ifndef DOC_DONT_EXTRACT //direction to doxygen not to extract this class
    //Size of this struct should be large enough to hold condvar handle on all
    //platforms.
    //pthread requires 48 bytes.
    struct SystemIndependentCondVarHandle{
        ting::byte handle[48];
    } cond;
#endif //~DOC_DONT_EXTRACT
private:
    //forbid copying
    CondVar(const CondVar& c){};
    CondVar(CondVar& c){};
    CondVar& operator=(const CondVar& c){return *this;};
public:
    CondVar();
    ~CondVar();

    void Wait(Mutex& mutex);

    void Notify();
};

class M_DECLSPEC Message{
    friend class Queue;
    
    Message *next;//pointer to the next message in a single-linked list
    
  protected:
    Message() :
            next(0)
    {};
    
  public:
    virtual ~Message(){};
    
    virtual void Handle()=0;
};

class M_DECLSPEC Queue{
    CondVar cond;
    
    Mutex mut;
    
    Message *first,
            *last;
  public:
    Queue() :
            first(0),
            last(0)
    {};
    ~Queue();
    
    void PushMessage(std::auto_ptr<Message> msg);
    
    std::auto_ptr<Message> PeekMsg();
    
    std::auto_ptr<Message> GetMsg();
};

class M_DECLSPEC QuitMessage : public Message{
    Thread *thr;
  public:
    QuitMessage(Thread* thread) :
            thr(thread)
    {
        if(!this->thr){
            //TODO: throw exception
        }
    };
    
    //override
    void Handle();
};

/**
@brief a base class for threads.
This class should be used as a base class for thread objects, one should override the
Thread::Run() method.
*/
class M_DECLSPEC Thread{
    friend class QuitMessage;
//    static int ThreadRunFunction(void* thrObj);
    
    ting::Mutex mutex;
public:
#ifndef DOC_DONT_EXTRACT //direction to doxygen not to extract this class
    struct SystemIndependentThreadHandle{
        ting::byte handle[sizeof(void*)];
    } thr;
#endif //~DOC_DONT_EXTRACT
    
protected:
    volatile bool quitFlag;//looks like it is no necessary to protect this flag by mutex, volatile will be enough
    volatile bool isRunning;//true if thread is running
    
    Queue queue;
public:
    Thread();
    
    virtual ~Thread();
    
    virtual void Run()=0;
    
    /**
    @brief Start thread execution.
    @param stackSize - size of the stack in bytes which should be allocated for this thread.
                       If stackSize is 0 then system default stack size is used.
    */
    //0 stacksize stands for default stack size (platform dependent)
    void Start(uint stackSize = 0);
    
    /**
    @brief Wait for thread finish its execution.
    */
    void Join();
    
    /**
    @brief Suspend the thread for a given number of milliseconds.
    @param msec - number of milliseconds the thread should be suspended.
    */
    static void Sleep(uint msec = 0);
    
    /**
    @brief Send 'Quit' message to thread's queue.
    */
    void PushQuitMessage(){
        this->PushMessage( std::auto_ptr<Message>(new QuitMessage(this)) );
    };

    /**
    @brief Send a message to thread's queue.
    @param msg - a message to send.
    */
    void PushMessage(std::auto_ptr<Message> msg){
        this->queue.PushMessage(msg);
    };
};

u32 GetTicks();

};//~namespace ting
#endif//~once
