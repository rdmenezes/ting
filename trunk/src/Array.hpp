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

// ting 0.2
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

#include "gendef.hpp"

namespace ting{

template <class T> class Array{//TODO: add T* as iterator. because [] operator is slow
	uint size;
	
	T* arr;//NOTE: do not use Ptr class, because we call new[]

	inline void PrivateInit(uint arraySize){
		this->size = arraySize;
		if(this->size == 0){
			this->arr = 0;
			return;
		}

		M_ARRAY_PRINT(<<"Array::PrivateInit(): size="<<size<<std::endl)
		try{
			this->arr = new T[arraySize];
		}catch(...){
			M_ARRAY_PRINT(<<"Array::Init(): exception caught"<<size<<std::endl)
			this->arr = 0;
			this->size = 0;
			throw;//rethrow the exception
		}
		M_ARRAY_PRINT(<<"Array::PrivateInit(): arr="<<static_cast<void*>(arr)<<std::endl)
	};

  public:

	//TODO: why is this constructor explicit?
	explicit inline Array(uint arraySize = 0){
		this->PrivateInit(arraySize);
	};

	//copy constructor
	inline Array(const Array& a){
		this->operator=(a);
	};

	inline Array& operator=(const Array& a){
		//behavior similar to C_Ptr class
		this->size = a.size;
		this->arr = a.arr;
		const_cast<Array&>(a).size = 0;
		const_cast<Array&>(a).arr = 0;
		return (*this);
	};

	~Array(){
		M_ARRAY_PRINT(<<"Array::~Array(): invoked"<<std::endl)
		delete[] this->arr;
		M_ARRAY_PRINT(<<"Array::~Array(): exit"<<std::endl)
	};

	inline uint Size()const{
		return this->size;
	};

	//returns length of element in bytes
	inline uint SizeOfElem()const{
		return sizeof(*(this->arr));
	};

	//returns length of array in bytes
	inline uint SizeInBytes()const{
		return this->Size() * this->SizeOfElem();
	};

	inline T& operator[](uint i){
		ASSERT(i < this->Size() && this->arr)
		return this->arr[i];
	};
	inline const T& Elem(uint i)const{
		ASSERT(i < this->Size() && this->arr)
		return this->arr[i];
	};

	void Init(uint arraySize){
		M_ARRAY_PRINT(<<"Array::Init(): arr="<<(void*)arr<<std::endl)
		this->~Array();
		this->PrivateInit(arraySize);
	};

	inline bool IsValid()const{
		return this->arr != 0;
	};

	operator bool(){
		return this->IsValid();
	};

	void Reset(){
		this->~Array();
		this->arr = 0;
		this->size = 0;
	};

	T* Buf(){
		return this->arr;
	};
	
	const T* Buf()const{
		return this->arr;
	};
};

}//~namespace ting
#endif //~once
