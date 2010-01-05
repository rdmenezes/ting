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

// ting 0.4
// Homepage: http://code.google.com/p/ting
// Author: Ivan Gagis <igagis@gmail.com>

/**
 * @file utils.hpp
 * @brief Utility functions and classes.
 */

#pragma once

//#ifdef _MSC_VER //If Microsoft C++ compiler
//#pragma warning(disable:4290)
//#endif

#include <vector>

#include "debug.hpp" //debugging facilities
#include "types.hpp"
#include "Thread.hpp"

//define macro used to align structures in memory
#ifdef _MSC_VER //If Microsoft C++ compiler
#define M_DECLARE_ALIGNED(x) __declspec(align(x))

#elif defined(__GNUG__)//GNU g++ compiler
#define M_DECLARE_ALIGNED(x) __attribute__ ((aligned(x)))

#else
#error "unknown compiler"
#endif


namespace ting{



/**
 * @brief Exchange two values.
 */
template <class T> inline void Exchange(T &a, T &b){
	T tmp = a;
	a = b;
	b = tmp;
}



//quick exchange two unsigned 32bit integers
template <> inline void Exchange<u32>(u32& x, u32& y){
//	TRACE(<<"Exchange<u32>(): invoked"<<std::endl)
	//NOTE: Do not make y^=x^=y^=x;
	//Some compilers (e.g. gcc4.1) may generate incorrect code
	y ^= x;
	x ^= y;
	y ^= x;
}



//quick exchange two floats
template <> inline void Exchange<float>(float& x, float& y){
//	TRACE(<<"Exchange<float>(): invoked"<<std::endl)
	STATIC_ASSERT(sizeof(float) == sizeof(u32))
	Exchange<u32>(reinterpret_cast<u32&>(x), reinterpret_cast<u32&>(y));
}



/**
 * @brief Clamp value top.
 * This inline template function can be used to clamp the top of the value.
 * Example:
 * @code
 * int a = 30;
 *
 * //Clamp to 40. Value of 'a' remains unchanged,
 * //since it is already less than 40.
 * ting::ClampTop(a, 40);
 * std::cout << a << std::endl;
 *
 * //Clamp to 27. Value of 'a' is changed to 27,
 * //since it is 30 which is greater than 27.
 * ting::ClampTop(a, 27);
 * std::cout << a << std::endl;
 * @endcode
 * As a result, this will print:
 * @code
 * 30
 * 27
 * @endcode
 * @param v - reference to the value which top is to be clamped.
 * @param top - value to clamp the top to.
 */
template <class T> inline void ClampTop(T& v, const T top){
	if(v > top){
		v = top;
	}
}


/**
 * @brief Clamp value bottom.
 * Usage is analogous to ting::ClampTop.
 */
template <class T> inline void ClampBottom(T& v, const T bottom){
	if(v < bottom){
		v = bottom;
	}
}



/**
 * @brief convert byte order of 16 bit value to network format.
 * @param value - the value.
 * @param out_buf - pointer to the 2 byte buffer where the result will be placed.
 */
inline void ToNetworkFormat16(u16 value, byte* out_buf){
	*reinterpret_cast<u16*>(out_buf) = value;//assume little-endian
}



/**
 * @brief convert byte order of 32 bit value to network format.
 * @param value - the value.
 * @param out_buf - pointer to the 4 byte buffer where the result will be placed.
 */
inline void ToNetworkFormat32(u32 value, byte* out_buf){
	*reinterpret_cast<u32*>(out_buf) = value;//assume little-endian
}



/**
 * @brief Convert 16 bit value from network byte order to native byte order.
 * @param buf - pointer to buffer containig 2 bytes to convert from network format.
 * @return 16 bit unsigned integer converted from network byte order to native byte order.
 */
inline u16 FromNetworkFormat16(const byte* buf){
	return *reinterpret_cast<const u16*>(buf);//assume little-endian
}



/**
 * @brief Convert 32 bit value from network byte order to native byte order.
 * @param buf - pointer to buffer containig 4 bytes to convert from network format.
 * @return 32 bit unsigned integer converted from network byte order to native byte order.
 */
inline u32 FromNetworkFormat32(const byte* buf){
	return *reinterpret_cast<const u32*>(buf);//assume little-endian
}



class Listener{
	template <class T> friend class ListenersList;
	friend class ListenersListInternal;

	ting::uint numTimesAdded;
protected:
	Listener() :
			numTimesAdded(0)
	{}

public:
	virtual ~Listener(){
//		TRACE(<< "~Listener(): enter" << std::endl)
		ASSERT(this->numTimesAdded == 0)
//		TRACE(<< "~Listener(): exit" << std::endl)
	}

private:
	class ListenersListInternal{
		//Network state listeners can be added and removed from different threads,
		//so, protect the list of listeners by mutex.
		ting::Mutex mutex;

		typedef std::vector<Listener*> T_ListenerList;
		typedef T_ListenerList::iterator T_ListenerIter;
		T_ListenerList listeners;

		T_ListenerIter iter;
	protected:
		ListenersListInternal(){
//			this->iter = this->listeners.end();
		}

		inline bool HasNext()const{
			return this->iter != this->listeners.end();
		}

		inline Listener* Next(){
			ASSERT_INFO(this->HasNext(), "Next() called when there is no next listener")
			Listener* l = (*this->iter);
			++this->iter;
			return l;
		}
		
		virtual void NotificationRoutine() = 0;

	public:

		virtual ~ListenersListInternal(){
			ASSERT(this->listeners.size() == 0)
//			ASSERT(this->iter == this->listeners.end())
		}

		inline void Notify_ts(){
			ting::Mutex::Guard mutexGuard(this->mutex);
//			TRACE(<< "Notify_ts(): enter" << std::endl)
//			ASSERT(this->iter == this->listeners.end())//NOTE: do this ASSERT() after mutex is locked!!!
			this->iter = this->listeners.begin();
			this->NotificationRoutine();
			ASSERT(this->iter == this->listeners.end())
//			TRACE(<< "Notify_ts(): exit" << std::endl)
		}

		inline void Add_ts(Listener* listener){
			ASSERT(listener)
			ting::Mutex::Guard mutexGuard(this->mutex);
//			TRACE(<< "Add_ts(): enter" << std::endl)
//			ASSERT(this->iter == this->listeners.end())//NOTE: do this ASSERT() after mutex is locked!!!
			this->listeners.push_back(listener);
//			this->iter = this->listeners.end();
			++listener->numTimesAdded;
//			TRACE(<< "Add_ts(): exit" << std::endl)
		}

		inline void Remove_ts(Listener* listener){
			ASSERT(listener)
			ting::Mutex::Guard mutexGuard(this->mutex);
//			TRACE(<< "Remove_ts(): enter" << std::endl)
//			ASSERT(this->iter == this->listeners.end())//NOTE: do this ASSERT() after mutex is locked!!!

			for(T_ListenerIter i = this->listeners.begin();
					i != this->listeners.end();
					++i
				)
			{
				if((*i) == listener){
					this->listeners.erase(i);
					--listener->numTimesAdded;
//					TRACE(<< "Remove_ts(): exit" << std::endl)
					return;
				}
			}//~for

			ASSERT_INFO(false, "requested to remove listener which was not added to that list before")
		}
		
	};//~class ListenersListInternal
};//~class Listener



template <class T> class ListenersList : public Listener::ListenersListInternal{
protected:
	inline T* Next(){
		return static_cast<T*>(this->ListenersListInternal::Next());
	}

public:
	inline void Add_ts(T* listener){
		this->ListenersListInternal::Add_ts(listener);
	}

	inline void Remove_ts(T* listener){
		this->ListenersListInternal::Remove_ts(listener);
	}
};



}//~namespace ting

