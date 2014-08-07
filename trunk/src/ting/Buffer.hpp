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
 * @author Ivan Gagis <igagis@gmail.com>
 * @brief buffer wrapper class.
 */


#pragma once

#include <array>
#include <vector>

#ifdef DEBUG
#	include <iostream>
#endif

#include "types.hpp"
#include "debug.hpp"



namespace ting{



/**
 * @brief Memory buffer wrapper template class.
 * This class is a wrapper of memory buffer, it encapsulates pointer to memory block and size of that memory block.
 */
template <class T> class Buffer{
	Buffer(const Buffer&) = delete;

protected:
	T* buf;
	size_t bufSize;

public:
	/**
	 * @brief Create a Buffer object.
	 * Creates a Buffer object which wraps given memory buffer of specified size.
	 * Note, the memory will not be freed upon this Buffer object destruction.
	 * @param bufPtr - pointer to the memory buffer.
	 * @param bufSize - size of the memory buffer.
	 */
	Buffer(T* bufPtr = nullptr, size_t bufSize = 0)noexcept :
			buf(bufPtr),
			bufSize(bufSize)
	{}
	
	
	
	Buffer(Buffer&& b){
		this->operator=(std::move(b));
	}
	
	
	
	Buffer& operator=(Buffer&& b){
		this->buf = b.buf;
		this->bufSize = b.bufSize;
		b.buf = 0;
		b.bufSize = 0;
	}
	
	
	/**
	 * @brief Create Buffer from std::array.
	 * Creates a Buffer object pointing to the contents of given std::array.
     * @param a - reference to instance of std::array.
     */
	template <class T_1, std::size_t array_size> Buffer(const std::array<T_1, array_size>& a) :
			buf(&(*const_cast<std::array<T_1, array_size>&>(a).begin())),
			bufSize(a.size())
	{}
	
	
	/**
	 * @brief Create Buffer from std::vector.
	 * Creates a Buffer object pointing to the contents of given std::vector.
     * @param v - reference to instance of std::vector.
     */
	template <class T_1> Buffer(const std::vector<T_1>& v) :
			buf(&(*v.begin())),
			bufSize(v.size())
	{}
	
	
	
	/**
	 * @brief get buffer size.
	 * @return number of elements in buffer.
	 */
	size_t size()const noexcept{
		return this->bufSize;
	}



	/**
	 * @brief get size of element.
	 * @return size of element in bytes.
	 */
	size_t SizeOfElem()const noexcept{
		return sizeof(this->buf[0]);
	}



	/**
	 * @brief get size of buffer in bytes.
	 * @return size of array in bytes.
	 */
	size_t SizeInBytes()const noexcept{
		return this->size() * this->SizeOfElem();
	}



	/**
	 * @brief access specified element of the buffer.
	 * Const version of Buffer::operator[].
	 * @param i - element index.
	 * @return reference to i'th element of the buffer.
	 */
	T& operator[](size_t i)const noexcept{
		ASSERT(i < this->size())
		return this->buf[i];
	}



	/**
	 * @brief access specified element of the buffer.
	 * @param i - element index.
	 * @return reference to i'th element of the buffer.
	 */
	T& operator[](size_t i)noexcept{
		ASSERT_INFO(i < this->size(), "operator[]: index out of bounds")
		return this->buf[i];
	}



	/**
	 * @brief get pointer to first element of the buffer.
	 * @return pointer to first element of the buffer.
	 */
	T* begin()noexcept{
		return this->buf;
	}



	/**
	 * @brief get pointer to first element of the buffer.
	 * @return pointer to first element of the buffer.
	 */
	T* begin()const noexcept{
		return this->buf;
	}



	/**
	 * @brief get pointer to "after last" element of the buffer.
	 * @return pointer to "after last" element of the buffer.
	 */
	T* end()noexcept{
		return this->buf + this->bufSize;
	}



	/**
	 * @brief get const pointer to "after last" element of the buffer.
	 * @return const pointer to "after last" element of the buffer.
	 */
	T* end()const noexcept{
		return this->buf + this->bufSize;
	}



	/**
	 * @brief Checks if pointer points somewhere within the buffer.
	 * @param p - pointer to check.
	 * @return true - if pointer passed as argument points somewhere within the buffer.
	 * @return false otherwise.
	 */
	bool Overlaps(const T* p)const noexcept{
		return this->begin() <= p && p <= (this->end() - 1);
	}



	operator const Buffer<const T>& ()const noexcept{
		return *reinterpret_cast<const Buffer<const T>* >(this);
	}


#ifdef DEBUG
	friend std::ostream& operator<<(std::ostream& s, const Buffer<T>& buf){
		for(size_t i = 0; i < buf.size(); ++i){
			s << "\t" << buf[i] << std::endl;
		}
		return s;
	}
#endif
};//~template class Buffer



}//~namespace
