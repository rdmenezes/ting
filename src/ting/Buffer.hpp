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

// Created on January 31, 2009, 11:04 PM

/**
 * @file Buffer.hpp
 * @brief buffer abstract class and static buffer wrapper.
 */
 

#pragma once

#include "types.hpp"
#include "debug.hpp"

namespace ting{


/**
 * @brief abstract buffer template class.
 * This class is supposed to be ancestor of all buffer-like objects.
 */
template <class T> class Buffer{
protected:
	T* buf;
	ting::uint size;


	
	inline Buffer(){}



	inline Buffer(T* buf_ptr, ting::uint buf_size) :
			buf(buf_ptr),
			size(buf_size)
	{}



	//forbid copying
	inline Buffer(const Buffer& b){
		ASSERT(false)
	}



	//forbid copying
	inline Buffer(Buffer& b){
		ASSERT(false)//not implemented yet
	}



public:
	/**
	 * @brief get buffer size.
	 * @return number of elements in buffer.
	 */
	inline ting::uint Size()const{
		return this->size;
	}



	/**
	 * @brief get size of element.
	 * @return size of element in bytes.
	 */
	inline ting::uint SizeOfElem()const{
		return sizeof(*(this->buf));
	}



	/**
	 * @brief get size of buffer in bytes.
	 * @return size of array in bytes.
	 */
	inline ting::uint SizeInBytes()const{
		return this->Size() * this->SizeOfElem();
	}



	/**
	 * @brief access specified element of the buffer.
	 * Const version of Buffer::operator[].
	 * @param i - element index.
	 * @return const reference to i'th element of the buffer.
	 */
	inline const T& operator[](uint i)const{
		ASSERT(i < this->Size())
		return this->buf[i];
	}



	/**
	 * @brief access specified element of the buffer.
	 * @param i - element index.
	 * @return reference to i'th element of the buffer.
	 */
	inline T& operator[](uint i){
		ASSERT_INFO(i < this->Size(), "operator[]: index out of bounds")
		return this->buf[i];
	}



	/**
	 * @brief get pointer to first element of the buffer.
	 * @return pointer to first element of the buffer.
	 */
	inline T* Begin(){
		return this->buf;
	}



	/**
	 * @brief get pointer to "after last" element of the buffer.
	 * @return pointer to "after last" element of the buffer.
	 */
	inline T* End(){
		ASSERT((this->buf + this->size) != 0)
		return this->buf + this->size;
	}



	/**
	 * @brief get pointer to first element of the buffer.
	 * @return pointer to first element of the buffer.
	 */
	inline T* Buf(){
		return this->buf;
	}



	/**
	 * @brief get pointer to first element of the buffer.
	 * Const version of Buffer::Buf().
	 * @return pointer to first element of the buffer.
	 */
	inline const T* Buf()const{
		return this->buf;
	}
};//~template class Buffer




/**
 * @brief static buffer class template.
 * The static buffer template.
 */
template <class T, ting::uint buf_size> class StaticBuffer : public Buffer<T>{
	T static_buffer[buf_size];
public:
	inline StaticBuffer(){
		this->buf = &this->static_buffer[0];
		this->size = buf_size;
	}
};



}//~namespace
