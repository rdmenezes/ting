/* The MIT License:

Copyright (c) 2009-2014 Ivan Gagis <igagis@gmail.com>

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
 * @file WaitSet.hpp
 * @author Ivan Gagis <igagis@gmail.com>
 * @author Jose Luis Hidalgo <joseluis.hidalgo@gmail.com> - Mac OS X port
 * @brief Wait set.
 */

#pragma once

#include <vector>
#include <sstream>
#include <cerrno>
#include <cstdint>

#include "config.hpp"
#include "types.hpp"
#include "debug.hpp"
#include "Exc.hpp"
#include "ArrayAdaptor.hpp"


#if M_OS == M_OS_WINDOWS
#	include "windows.hpp"

#elif M_OS == M_OS_LINUX
#	include <sys/epoll.h>
#	include <unistd.h>

#elif M_OS == M_OS_MACOSX
#	include <sys/types.h>
#	include <sys/event.h>

#else
#	error "Unsupported OS"
#endif


//disable warning about throw specification is ignored.
#if M_COMPILER == M_COMPILER_MSVC
#	pragma warning(push) //push warnings state
#	pragma warning( disable : 4290)
#endif



namespace ting{



/**
 * @brief Base class for objects which can be waited for.
 * Base class for objects which can be used in wait sets.
 */
class Waitable{
	friend class WaitSet;

	bool isAdded = false;

	void* userData = nullptr;

public:
	enum EReadinessFlags{
		NOT_READY = 0,      // bin: 00000000
		READ = 1,           // bin: 00000001
		WRITE = 2,          // bin: 00000010
		READ_AND_WRITE = 3, // bin: 00000011
		ERROR_CONDITION = 4 // bin: 00000100
	};

protected:
	std::uint32_t readinessFlags = NOT_READY;

	Waitable() = default;



	bool IsAdded()const noexcept{
		return this->isAdded;
	}




	Waitable(const Waitable& w) = delete;
	
	Waitable(Waitable&& w);


	Waitable& operator=(Waitable&& w);



	void SetCanReadFlag()noexcept{
		this->readinessFlags |= READ;
	}

	void ClearCanReadFlag()noexcept{
		this->readinessFlags &= (~READ);
	}

	void SetCanWriteFlag()noexcept{
		this->readinessFlags |= WRITE;
	}

	void ClearCanWriteFlag()noexcept{
		this->readinessFlags &= (~WRITE);
	}

	void SetErrorFlag()noexcept{
		this->readinessFlags |= ERROR_CONDITION;
	}

	void ClearErrorFlag()noexcept{
		this->readinessFlags &= (~ERROR_CONDITION);
	}

	void ClearAllReadinessFlags()noexcept{
		this->readinessFlags = NOT_READY;
	}

public:
	virtual ~Waitable()noexcept{
		ASSERT(!this->isAdded)
	}

	/**
	 * @brief Check if "Can read" flag is set.
	 * @return true if Waitable is ready for reading.
	 */
	bool CanRead()const noexcept{
		return (this->readinessFlags & READ) != 0;
	}

	/**
	 * @brief Check if "Can write" flag is set.
	 * @return true if Waitable is ready for writing.
	 */
	bool CanWrite()const noexcept{
		return (this->readinessFlags & WRITE) != 0;
	}

	/**
	 * @brief Check if "error" flag is set.
	 * @return true if Waitable is in error state.
	 */
	bool ErrorCondition()const noexcept{
		return (this->readinessFlags & ERROR_CONDITION) != 0;
	}

	/**
	 * @brief Get user data associated with this Waitable.
	 * Returns the pointer to the user data which was previously set by SetUserData() method.
	 * @return pointer to the user data.
	 * @return zero pointer if the user data was not set.
	 */
	void* GetUserData()noexcept{
		return this->userData;
	}

	/**
	 * @brief Set user data.
	 * See description of GetUserData() for more details.
	 * @param data - pointer to the user data to associate with this Waitable.
	 */
	void SetUserData(void* data)noexcept{
		this->userData = data;
	}

#if M_OS == M_OS_WINDOWS
protected:
	virtual HANDLE GetHandle() = 0;

	virtual void SetWaitingEvents(std::uint32_t /*flagsToWaitFor*/){}

	//returns true if signaled
	virtual bool CheckSignaled(){
		return this->readinessFlags != 0;
	}

#elif M_OS == M_OS_LINUX || M_OS == M_OS_MACOSX || M_OS == M_OS_UNIX
public:
	/**
	 * @brief Get Unix file descriptor.
	 * This method is specific to Unix-based operating systems, like Linux, MAC OS X, Unix.
	 * This method is made public in order to ease embedding Waitables to existing epoll() sets.
	 * Use this method only if you know what you are doing!
	 */
	virtual int GetHandle() = 0;

#else
#	error "Unsupported OS"
#endif

};//~class Waitable





/**
 * @brief Set of Waitable objects to wait for.
 */
class WaitSet{
	const unsigned size;
	unsigned numWaitables = 0;//number of Waitables added

#if M_OS == M_OS_WINDOWS
	std::vector<Waitable*> waitables;
	std::vector<HANDLE> handles; //used to pass array of HANDLEs to WaitForMultipleObjectsEx()

#elif M_OS == M_OS_LINUX
	int epollSet;

	std::vector<epoll_event> revents;//used for getting the result from epoll_wait()
#elif M_OS == M_OS_MACOSX
	int queue; // kqueue
	
	std::vector<struct kevent> revents;//used for getting the result
#else
#	error "Unsupported OS"
#endif

public:

	/**
	 * @brief WaitSet related exception class.
	 */
	class Exc : public ting::Exc{
	public:
		Exc(const std::string& message = std::string()) :
				ting::Exc(message)
		{}
	};
	
	/**
	 * @brief Constructor.
	 * @param maxSize - maximum number of Waitable objects can be added to this wait set.
	 */
	WaitSet(unsigned maxSize) :
			size(maxSize)
#if M_OS == M_OS_WINDOWS
			,waitables(maxSize)
			,handles(maxSize)
	{
		ASSERT_INFO(maxSize <= MAXIMUM_WAIT_OBJECTS, "maxSize should be less than " << MAXIMUM_WAIT_OBJECTS)
		if(maxSize > MAXIMUM_WAIT_OBJECTS){
			throw Exc("WaitSet::WaitSet(): requested WaitSet size is too big");
		}
	}

#elif M_OS == M_OS_LINUX
			,revents(maxSize)
	{
		ASSERT(int(maxSize) > 0)
		this->epollSet = epoll_create(int(maxSize));
		if(this->epollSet < 0){
			throw Exc("WaitSet::WaitSet(): epoll_create() failed");
		}
	}
#elif M_OS == M_OS_MACOSX
			,revents(maxSize * 2)
	{
		this->queue = kqueue();
		if(this->queue == -1){
			throw Exc("WaitSet::WaitSet(): kqueue creation failed");
		}
	}
#else
#	error "Unsupported OS"
#endif



	/**
	 * @brief Destructor.
	 * Note, that destructor will check if the wait set is empty. If it is not, then an assert
	 * will be triggered.
	 * It is user's responsibility to remove any waitable objects from the waitset
	 * before the wait set object is destroyed.
	 */
	~WaitSet()noexcept{
		//assert the wait set is empty
		ASSERT_INFO(this->numWaitables == 0, "attempt to destroy WaitSet containig Waitables")
#if M_OS == M_OS_WINDOWS
		//do nothing
#elif M_OS == M_OS_LINUX
		close(this->epollSet);
#elif M_OS == M_OS_MACOSX
		close(this->queue);
#else
#	error "Unsupported OS"
#endif
	}



	/**
	 * @brief Get maximum size of the wait set.
	 * @return maximum number of Waitables this WaitSet can hold.
	 */
	unsigned Size()const noexcept{
		return this->size;
	}

	/**
	 * @brief Get number of Waitables already added to this WaitSet.
	 * @return number of Waitables added to this WaitSet.
	 */
	unsigned NumWaitables()const noexcept{
		return this->numWaitables;
	}


	/**
	 * @brief Add Waitable object to the wait set.
	 * @param w - Waitable object to add to the WaitSet.
	 * @param flagsToWaitFor - determine events waiting for which we are interested.
	 * @throw ting::WaitSet::Exc - in case the wait set is full or other error occurs.
	 */
	void Add(Waitable& w, Waitable::EReadinessFlags flagsToWaitFor);



	/**
	 * @brief Change wait flags for a given Waitable.
	 * Changes wait flags for a given waitable, which is in this WaitSet.
	 * @param w - Waitable for which the changing of wait flags is needed.
	 * @param flagsToWaitFor - new wait flags to be set for the given Waitable.
	 * @throw ting::WaitSet::Exc - in case the given Waitable object is not added to this wait set or
	 *                    other error occurs.
	 */
	void Change(Waitable& w, Waitable::EReadinessFlags flagsToWaitFor);



	/**
	 * @brief Remove Waitable from wait set.
	 * @param w - Waitable object to be removed from the WaitSet.
	 * @throw ting::WaitSet::Exc - in case the given Waitable is not added to this wait set or
	 *                    other error occurs.
	 */
	void Remove(Waitable& w)noexcept;



	/**
	 * @brief wait for event.
	 * This function blocks calling thread execution until one of the Waitable objects in the WaitSet
	 * triggers. Upon return from the function, pointers to triggered objects are placed in the
	 * 'out_events' buffer and the return value from the function indicates number of these objects
	 * which have triggered.
	 * Note, that it does not change the readiness state of non-triggered objects.
	 * @param out_events - pointer to buffer where to put pointers to triggered Waitable objects.
	 *                     The buffer will not be initialized to 0's by this function.
	 *                     The buffer shall be large enough to hold maxmimum number of Waitables
	 *                     this WaitSet can hold.
	 *                     It is valid to pass 0 pointer, in that case this argument will not be used.
	 * @return number of objects triggered.
	 *         NOTE: for some reason, on Windows it can return 0 objects triggered.
	 * @throw ting::WaitSet::Exc - in case of errors.
	 */
	unsigned Wait(ArrayAdaptor<Waitable*> out_events){
		return this->Wait(true, 0, &out_events);
	}
	
	/**
	 * @brief wait for event.
	 * Same as Wait(const Buffer<Waitable*>& out_events) but does not return out_events.
     * @return number of objects triggered.
     */
	unsigned Wait(){
		return this->Wait(true, 0, 0);
	}


	/**
	 * @brief wait for event with timeout.
	 * The same as Wait() function, but takes wait timeout as parameter. Thus,
	 * this function will wait for any event or timeout. Note, that it guarantees that
	 * it will wait AT LEAST for specified number of milliseconds, or more. This is because of
	 * implementation for linux, if wait is interrupted by signal it will start waiting again,
	 * and so on.
	 * @param timeout - maximum time in milliseconds to wait for event.
	 * @param out_events - buffer where to put pointers to triggered Waitable objects.
	 *                     The buffer size must be equal or greater than the number of waitables
	 *                     currently added to the wait set.
	 * @return number of objects triggered. If 0 then timeout was hit.
	 *         NOTE: for some reason, on Windows it can return 0 before timeout was hit.
	 * @throw ting::WaitSet::Exc - in case of errors.
	 */
	unsigned WaitWithTimeout(std::uint32_t timeout, ArrayAdaptor<Waitable*> out_events){
		return this->Wait(false, timeout, &out_events);
	}
	
	/**
	 * @brief wait for event with timeout.
	 * Same as WaitWithTimeout(std::uint32_t timeout, const Buffer<Waitable*>& out_events) but
	 * does not return out_events.
     * @param timeout - maximum time in milliseconds to wait for event.
     * @return number of objects triggered. If 0 then timeout was hit.
	 *         NOTE: for some reason, on Windows it can return 0 before timeout was hit.
     */
	unsigned WaitWithTimeout(std::uint32_t timeout){
		return this->Wait(false, timeout, 0);
	}



private:
	unsigned Wait(bool waitInfinitly, std::uint32_t timeout, ArrayAdaptor<Waitable*>* out_events);
	
	
#if M_OS == M_OS_MACOSX
	void AddFilter(Waitable& w, int16_t filter);
	void RemoveFilter(Waitable& w, int16_t filter);
#endif

};//~class WaitSet



inline Waitable::Waitable(Waitable&& w) :
		isAdded(false),
		userData(w.userData),
		readinessFlags(NOT_READY)//Treat copied Waitable as NOT_READY
{
	//cannot move from waitable which is added to WaitSet
	if(w.isAdded){
		throw ting::WaitSet::Exc("Waitable::Waitable(move): cannot move Waitable which is added to WaitSet");
	}

	const_cast<Waitable&>(w).ClearAllReadinessFlags();
	const_cast<Waitable&>(w).userData = 0;
}



inline Waitable& Waitable::operator=(Waitable&& w){
	if(this->isAdded){
		throw ting::WaitSet::Exc("Waitable::Waitable(move): cannot move while this Waitable is added to WaitSet");
	}

	if(w.isAdded){
		throw ting::WaitSet::Exc("Waitable::Waitable(move): cannot move Waitable which is added to WaitSet");
	}

	ASSERT(!this->isAdded)

	//Clear readiness flags on moving.
	//Will need to wait for readiness again, using the WaitSet.
	this->ClearAllReadinessFlags();
	const_cast<Waitable&>(w).ClearAllReadinessFlags();

	this->userData = w.userData;
	const_cast<Waitable&>(w).userData = 0;
	return *this;
}



}//~namespace ting


//restore warnings state
#if M_COMPILER == M_COMPILER_MSVC
#	pragma warning(pop) //pop warnings state
#endif
