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

// Created on January 31, 2009, 11:04 PM
// File description:
//	Static buffer wrapper
 

#ifndef M_Buffer_hpp
#define	M_Buffer_hpp

#include "types.hpp"
#include "debug.hpp"

namespace ting{

template <class T> class Buffer{
protected:
	T* buf;
	ting::uint size;

	inline Buffer(){};

	inline Buffer(T* buf_ptr, ting::uint buf_size) :
			buf(buf_ptr),
			size(buf_size)
	{};

	inline Buffer(const Buffer& b){
		ASSERT(false)//not implemented yet
	};
	inline Buffer(Buffer& b){
		ASSERT(false)//not implemented yet
	};

public:
	inline ting::uint Size()const{
		return this->size;
	}

		//returns length of element in bytes
	inline ting::uint SizeOfElem()const{
		return sizeof(*(this->buf));
	};

	//returns length of array in bytes
	inline ting::uint SizeInBytes()const{
		return this->Size() * this->SizeOfElem();
	};

	inline const T& operator[](uint i)const{
		ASSERT(i < this->Size())
		return this->buf[i];
	};

	inline T& operator[](uint i){
		ASSERT(i < this->Size())
		return this->buf[i];
	};

//	inline operator const T*()const{
//		return this->buf;
//	};
//
//	inline operator T*(){
//		return const_cast<T*>(this->operator const T*());
//	};
};

template <class T, ting::uint buf_size> class StaticBuffer : public Buffer<T>{
	T static_buffer[buf_size];
public:
	inline StaticBuffer() :
			Buffer<T>(&this->static_buffer[0], buf_size)
	{};
};

}//~namespace
#endif //~once
