/* The MIT License:

Copyright (c) 2012 Ivan Gagis <igagis@gmail.com>

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
 * @brief UTF8 string class.
 */

#pragma once

#include "types.hpp"
#include "Buffer.hpp"



namespace ting{



//TODO: doxygen
class StrUTF8{
	ting::Inited<ting::u8*, 0> s;//null-terminated
public:
	
	inline StrUTF8()throw(){}
	
	~StrUTF8()throw(){
		this->Destroy();
	}
	

	
	//move semantics
	inline StrUTF8(const StrUTF8& str){
		this->s = str.s;
		str.s = 0;
	}

	//move semantics
	StrUTF8& operator=(const StrUTF8& str){
		this->Destroy();
		this->s = str.s;
		str.s = 0;
		return *this;
	}
	
	
	
	void Init(const ting::Buffer<const ting::u8>& buf){
		this->Destroy();
		this->InitInternal(buf.Begin(), buf.Size());
	}
	
	
	
	StrUTF8& operator=(const char* str);
	
	
	//TODO:
	
	void Reset()throw(){
		this->Destroy();
		this->s = 0;
	}
	
private:
	void Destroy(){
		delete[] s;
	}
	
	void InitInternal(const char*, size_t len);
};



}//~namespace
