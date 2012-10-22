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


	explicit inline StrUTF8(const char* str);
	
	
	inline StrUTF8(const ting::Buffer<const ting::u8>& buf){
		this->InitInternal(buf.Begin(), buf.Size());
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
	
	
	//TODO: add stuff if needed
	
	void Reset()throw(){
		this->Destroy();
		this->s = 0;
	}
	
	inline bool IsValid()const throw(){
		return this->s != 0;
	}
	
	inline bool IsNotValid()const throw(){
		return !this->IsValid();
	}
	
private:
	void Destroy(){
		delete[] s;
	}
	
	void InitInternal(const ting::u8*, size_t len);
	
public:
	
	class Iterator{
		friend class StrUTF8;
		
		ting::u32 c;
		const ting::u8* n;
		
		Iterator(const ting::u8* begin) :
				n(begin)
		{
			if(this->n == 0){
				this->c = 0;
			}else{
				this->operator++();
			}
		}
	public:
		Iterator() :
				c(0)
		{}
		
		inline ting::u32 Char()const throw(){
			return this->c;
		}
		
		//prefix ++
		//if ++ end iterator then undefined behavior
		Iterator& operator++()throw(){
			ting::u8 b = *this->n;
			++this->n;
			if((b & 0x80) == 0){
				this->c = ting::u32(b);
				return *this;
			}
			
			this->c = (*this->n) & 0x3f;
			++this->n;
			
			unsigned i = 2;
			for(; ((b << i) >> 7); ++i, ++this->n){
				this->c <<= (6 * i);
				this->c |= (*this->n) & 0x3f;
			}
			this->c |= (ting::u32((b << (i + 1)) >> (i + 1)) << (6 * (i - 1)));
			
			return *this;
		}
		
		//no postfix ++ operator, there is no need in it.
		
		inline bool IsEnd()const throw(){
			return this->c == 0;
		}
		
		inline bool IsNotEnd()const throw(){
			return !this->IsEnd();
		}
	};
	
	
	Iterator Begin()const throw(){
		return Iterator(this->s);
	}
};



}//~namespace
