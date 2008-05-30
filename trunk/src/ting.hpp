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

#ifndef __SYMBIAN32__
#include <memory>
#include <exception>
#include <new>
#endif

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
#ifdef __SYMBIAN32__ // we have symbian which does not have std c++ library
class M_DECLSPEC Exc
#else
class M_DECLSPEC Exc : public std::exception
#endif
{
    char *msg;
public:
    /**
    @brief Exception constructor.
    @param message Pointer to the exception message null-terminated string. Constructor will copy the string into objects internal memory buffer.
    */
    Exc(const char* message = 0);
    virtual ~Exc();
    
    /**
    @brief Returns a pointer to exception message.
    @return a pointer to objects internal memory buffer holding the exception message null-terminated string.
            Note, that after the exception object is destroyed the pointer returned by this method become invalid.
    */
    //override from std::exception
    const char *what()const{
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
    Mutex(const Mutex& ){};
    Mutex(Mutex& ){};
    Mutex& operator=(const Mutex& ){return *this;};
    
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
    @brief Helper class which automatically Locks the given mutex.
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
    Semaphore(const Semaphore& ){};
    Semaphore(Semaphore& ){};
    Semaphore& operator=(const Semaphore& ){return *this;};
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
    CondVar(const CondVar& ){};
    CondVar(CondVar& ){};
    CondVar& operator=(const CondVar& ){return *this;};
public:
    CondVar();
    ~CondVar();

    void Wait(Mutex& mutex);

    void Notify();
};

#ifdef __SYMBIAN32__
template<typename _Tp> class auto_ptr{
    private:
      _Tp* _M_ptr;
      
    public:
      /// The pointed-to type.
      typedef _Tp element_type;
      
      /**
       *  @brief  An %auto_ptr is usually constructed from a raw pointer.
       *  @param  p  A pointer (defaults to NULL).
       *
       *  This object now @e owns the object pointed to by @a p.
       */
      explicit
      auto_ptr(element_type* __p = 0) throw() : _M_ptr(__p) { }

      /**
       *  @brief  An %auto_ptr can be constructed from another %auto_ptr.
       *  @param  a  Another %auto_ptr of the same type.
       *
       *  This object now @e owns the object previously owned by @a a,
       *  which has given up ownsership.
       */
      auto_ptr(auto_ptr& __a) throw() : _M_ptr(__a.release()) { }

      /**
       *  @brief  An %auto_ptr can be constructed from another %auto_ptr.
       *  @param  a  Another %auto_ptr of a different but related type.
       *
       *  A pointer-to-Tp1 must be convertible to a
       *  pointer-to-Tp/element_type.
       *
       *  This object now @e owns the object previously owned by @a a,
       *  which has given up ownsership.
       */
      template<typename _Tp1>
        auto_ptr(auto_ptr<_Tp1>& __a) throw() : _M_ptr(__a.release()) { }

      /**
       *  @brief  %auto_ptr assignment operator.
       *  @param  a  Another %auto_ptr of the same type.
       *
       *  This object now @e owns the object previously owned by @a a,
       *  which has given up ownsership.  The object that this one @e
       *  used to own and track has been deleted.
       */
      auto_ptr&
      operator=(auto_ptr& __a) throw()
      {
	reset(__a.release());
	return *this;
      }

      /**
       *  @brief  %auto_ptr assignment operator.
       *  @param  a  Another %auto_ptr of a different but related type.
       *
       *  A pointer-to-Tp1 must be convertible to a pointer-to-Tp/element_type.
       *
       *  This object now @e owns the object previously owned by @a a,
       *  which has given up ownsership.  The object that this one @e
       *  used to own and track has been deleted.
       */
      template<typename _Tp1>
        auto_ptr&
        operator=(auto_ptr<_Tp1>& __a) throw()
        {
	  reset(__a.release());
	  return *this;
	}

      /**
       *  When the %auto_ptr goes out of scope, the object it owns is
       *  deleted.  If it no longer owns anything (i.e., @c get() is
       *  @c NULL), then this has no effect.
       *
       *  @if maint
       *  The C++ standard says there is supposed to be an empty throw
       *  specification here, but omitting it is standard conforming.  Its
       *  presence can be detected only if _Tp::~_Tp() throws, but this is
       *  prohibited.  [17.4.3.6]/2
       *  @end maint
       */
      ~auto_ptr() { delete _M_ptr; }
      
      /**
       *  @brief  Smart pointer dereferencing.
       *
       *  If this %auto_ptr no longer owns anything, then this
       *  operation will crash.  (For a smart pointer, "no longer owns
       *  anything" is the same as being a null pointer, and you know
       *  what happens when you dereference one of those...)
       */
      element_type&
      operator*() const throw() 
      {
	_GLIBCXX_DEBUG_ASSERT(_M_ptr != 0);
	return *_M_ptr; 
      }
      
      /**
       *  @brief  Smart pointer dereferencing.
       *
       *  This returns the pointer itself, which the language then will
       *  automatically cause to be dereferenced.
       */
      element_type*
      operator->() const throw() 
      {
	_GLIBCXX_DEBUG_ASSERT(_M_ptr != 0);
	return _M_ptr; 
      }
      
      /**
       *  @brief  Bypassing the smart pointer.
       *  @return  The raw pointer being managed.
       *
       *  You can get a copy of the pointer that this object owns, for
       *  situations such as passing to a function which only accepts
       *  a raw pointer.
       *
       *  @note  This %auto_ptr still owns the memory.
       */
      element_type*
      get() const throw() { return _M_ptr; }
      
      /**
       *  @brief  Bypassing the smart pointer.
       *  @return  The raw pointer being managed.
       *
       *  You can get a copy of the pointer that this object owns, for
       *  situations such as passing to a function which only accepts
       *  a raw pointer.
       *
       *  @note  This %auto_ptr no longer owns the memory.  When this object
       *  goes out of scope, nothing will happen.
       */
      element_type*
      release() throw()
      {
	element_type* __tmp = _M_ptr;
	_M_ptr = 0;
	return __tmp;
      }
      
      /**
       *  @brief  Forcibly deletes the managed object.
       *  @param  p  A pointer (defaults to NULL).
       *
       *  This object now @e owns the object pointed to by @a p.  The
       *  previous object has been deleted.
       */
      void
      reset(element_type* __p = 0) throw()
      {
	if (__p != _M_ptr)
	  {
	    delete _M_ptr;
	    _M_ptr = __p;
	  }
      }
      
      /** @{
       *  @brief  Automatic conversions
       *
       *  These operations convert an %auto_ptr into and from an auto_ptr_ref
       *  automatically as needed.  This allows constructs such as
       *  @code
       *    auto_ptr<Derived>  func_returning_auto_ptr(.....);
       *    ...
       *    auto_ptr<Base> ptr = func_returning_auto_ptr(.....);
       *  @endcode
       */
//      auto_ptr(auto_ptr_ref<element_type> __ref) throw()
//      : _M_ptr(__ref._M_ptr) { }
      
//      auto_ptr&
//      operator=(auto_ptr_ref<element_type> __ref) throw()
//      {
//	if (__ref._M_ptr != this->get())
//	  {
//	    delete _M_ptr;
//	    _M_ptr = __ref._M_ptr;
//	  }
//	return *this;
//      }
      
//      template<typename _Tp1>
//        operator auto_ptr_ref<_Tp1>() throw()
//        { return auto_ptr_ref<_Tp1>(this->release()); }
//
//      template<typename _Tp1>
//        operator auto_ptr<_Tp1>() throw()
//        { return auto_ptr<_Tp1>(this->release()); }
      /** @}  */
};
#endif

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

#ifndef __SYMBIAN32__
typedef std::auto_ptr<Message> MsgAutoPtr;
#else
typedef auto_ptr<Message> MsgAutoPtr;
#endif

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
    
    void PushMessage(MsgAutoPtr msg);
    
    MsgAutoPtr PeekMsg();
    
    MsgAutoPtr GetMsg();
};

class M_DECLSPEC QuitMessage : public Message{
    Thread *thr;
  public:
    QuitMessage(Thread* thread) :
            thr(thread)
    {
        if(!this->thr){
            throw ting::Exc("QuitMessage::QuitMessage(): thread pointer passed is 0");
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
        this->PushMessage( ting::MsgAutoPtr(new QuitMessage(this)) );
    };

    /**
    @brief Send a message to thread's queue.
    @param msg - a message to send.
    */
    void PushMessage(ting::MsgAutoPtr msg){
        this->queue.PushMessage(msg);
    };
};

u32 GetTicks();

}//~namespace ting

//NOTE: do not put semicolon after namespace, some compilers issue a warning on this thinking that it is a declaration.

#endif//~once
