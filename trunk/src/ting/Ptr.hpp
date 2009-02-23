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
//	Pointer wrapper

#ifndef M_Ptr_hpp
#define M_Ptr_hpp

#include "debug.hpp"

//#define M_ENABLE_PTR_PRINT
#ifdef M_ENABLE_PTR_PRINT
#define M_PTR_PRINT(x) LOG(x)
#else
#define M_PTR_PRINT(x)
#endif

namespace ting{

template <class T> class Ptr{
	void* p;
public:
	inline Ptr(T* ptr = 0) :
			p(ptr)
	{};
	
	//const copy constructor
	inline Ptr(const Ptr& ptr){
		M_PTR_PRINT(<<"Ptr::Ptr(copy): invoked, ptr.p="<<(ptr.p)<<std::endl)
		this->p = ptr.p;
		const_cast<Ptr&>(ptr).p = 0;
	};

	inline ~Ptr(){
		M_PTR_PRINT(<<"Ptr::~Ptr(): delete invoked, p="<<this->p<<std::endl)
		delete static_cast<T*>(p);
	};

	inline T* operator->(){
		ASSERT_INFO(this->p, "Ptr::operator->(): this->p is zero")
		return static_cast<T*>(p);
	};

	inline const T* operator->()const{
		ASSERT_INFO(this->p, "const Ptr::operator->(): this->p is zero")
		return static_cast<T*>(this->p);
	};

	inline Ptr& operator=(const Ptr& ptr){
		M_PTR_PRINT(<<"Ptr::operator=(Ptr&): enter, this->p="<<(this->p)<<std::endl)
		this->~Ptr();
		this->p = ptr.p;
		const_cast<Ptr&>(ptr).p = 0;
		M_PTR_PRINT(<<"Ptr::operator=(Ptr&): exit, this->p="<<(this->p)<<std::endl)
		return (*this);
	};

	inline bool operator==(const T* ptr)const{
		return this->p == ptr;
	};

	inline bool operator!=(const T* ptr)const{
		return !( *this == ptr );
	};
	inline bool operator!()const{
		return this->IsNotValid();
	};

	inline operator bool(){
		return this->IsValid();
	};

	inline T* Extract(){
		T* pp = static_cast<T*>(this->p);
		this->p = 0;
		return pp;
	};

	inline void Reset(){
		this->~Ptr();
		this->p = 0;
	};

	inline bool IsValid()const{
		return this->p != 0;
	};
	
	inline bool IsNotValid()const{
		return !this->IsValid();
	};

	template <class TS> TS* StaticCast(){
		return static_cast<TS*>(this->operator->());
	};
	
private:
	inline void* operator new(size_t){
		ASSERT(false)//forbidden
		return reinterpret_cast<void*>(0);
	};

	inline void operator delete(void*){
		ASSERT(false)//forbidden
	};
};

};//~namespace ting
#endif//~once
