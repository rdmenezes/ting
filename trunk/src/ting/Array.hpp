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

// ting 0.3
// Homepage: http://code.google.com/p/ting
// Author: Ivan Gagis <igagis@gmail.com>

// File description:
//	Array class, it is an auto pointer for new[] / delete[]

#ifndef M_Array_hpp
#define M_Array_hpp

//#define M_ENABLE_ARRAY_PRINT
#ifdef M_ENABLE_ARRAY_PRINT 
#define M_ARRAY_PRINT(x) LOG(x)
#else
#define M_ARRAY_PRINT(x)
#endif

#include "debug.hpp"
#include "types.hpp"
#include "Buffer.hpp"

namespace ting{

template <class T> class Array : public ting::Buffer<T>{

	inline void PrivateInit(uint arraySize){
		this->size = arraySize;
		if(this->size == 0){
			this->buf = 0;
			return;
		}

		M_ARRAY_PRINT(<< "Array::PrivateInit(): size = " << this->size << std::endl)
		try{
			this->buf = new T[arraySize];
		}catch(...){
			M_ARRAY_PRINT(<< "Array::Init(): exception caught" << this->size << std::endl)
			this->buf = 0;
			this->size = 0;
			throw;//rethrow the exception
		}
		M_ARRAY_PRINT(<< "Array::PrivateInit(): buf = " << static_cast<void*>(this->buf) << std::endl)
	};

  public:

	//TODO: why is this constructor explicit?
	explicit inline Array(uint arraySize = 0){
		this->PrivateInit(arraySize);
	};

	//TODO: implement
	//explicit inline Array(uint arraySize, const T& init)

	//copy constructor
	inline Array(const Array& a){
		this->operator=(a);
	};

	inline Array& operator=(const Array& a){
		//behavior similar to Ptr class
		this->size = a.size;
		this->buf = a.buf;
		const_cast<Array&>(a).size = 0;
		const_cast<Array&>(a).buf = 0;
		return (*this);
	};

	~Array(){
		M_ARRAY_PRINT(<< "Array::~Array(): invoked" << std::endl)
		delete[] this->buf;
		M_ARRAY_PRINT(<< "Array::~Array(): exit" << std::endl)
	};

	void Grow(uint deltaSize){
		ASSERT(false)//TODO: test this method. it is untested so far

		M_ARRAY_PRINT(<< "Array::Init(): buf = " << static_cast<void*>(this->buf) << std::endl)
		T* oldArr = this->buf;
		uint oldSize = this->size;
		try{
			this->PrivateInit(oldSize + deltaSize);
		}catch(...){
			this->buf = oldArr;
			this->size = oldSize;
			throw;
		}
		memcpy(this->buf, oldArr, oldSize);
		delete[] oldArr;
	};

	void Init(uint arraySize){
		M_ARRAY_PRINT(<< "Array::Init(): buf = " << static_cast<void*>(this->buf) << std::endl)
		this->~Array();
		this->PrivateInit(arraySize);
	};

	inline bool IsValid()const{
		return this->buf != 0;
	};

	operator bool(){
		return this->IsValid();
	};

	void Reset(){
		this->~Array();
		this->buf = 0;
		this->size = 0;
	};

	inline T* Begin(){
		return this->buf;
	}

	inline T* End(){
		return this->buf + this->size;
	}

	inline T* Buf(){
		return this->buf;
	};
	
	inline const T* Buf()const{
		return this->buf;
	};
};

}//~namespace ting
#endif //~once
