/* The MIT License:

Copyright (c) 2012-2014 Ivan Gagis <igagis@gmail.com>

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
 * @brief utf8 utilities.
 */

#pragma once

#include "types.hpp"
#include "ArrayAdaptor.hpp"
#include "util.hpp"


namespace ting{
namespace utf8{



/**
 * @brief Iterator to iterate through utf-8 encoded unicode characters.
 */
class Iterator{
	std::uint32_t c;
	const std::uint8_t* n;

public:
	/**
	 * @brief Create undefined iterator.
     */
	Iterator() :
			c(0)
	{}

	/**
	 * @brief Create iterator pointing to the begin of the given utf-8 encoded string.
     * @param begin - pointer to the first byte of the null-terminated utf-8 encoded string.
     */
	Iterator(const char* begin) :
			n(reinterpret_cast<const std::uint8_t*>(begin))
	{
		if(this->n == 0){
			this->c = 0;
		}else{
			this->operator++();
		}
	}

	/**
	 * @brief Get current unicode character.
     * @return unicode value of the character this interator is currently pointing to.
     */
	std::uint32_t Char()const NOEXCEPT{
		return this->c;
	}

	//no operator*() because it usually returns reference, don't want to break this contract.
	
	/**
	 * @brief Prefix increment.
	 * Move iterator to the next character in the string.
	 * If iterator points to the end of the string before this operation then the result of this operation is undefined.
     * @return reference to this iterator object.
     */
	Iterator& operator++()NOEXCEPT{
		std::uint8_t b = *this->n;
//		TRACE(<< "utf8::Iterator::operator++(): b = " << std::hex << unsigned(b) << std::endl)
		++this->n;
		if((b & 0x80) == 0){
			this->c = std::uint32_t(b);
			return *this;
		}

//		TRACE(<< "utf8::Iterator::operator++(): *n = " << std::hex << unsigned(*this->n) << std::endl)
		
		this->c = (*this->n) & 0x3f;
//		TRACE(<< "utf8::Iterator::operator++(): initial c = " << std::hex << c << std::endl)

		++this->n;

		unsigned i = 2;
		for(; (std::uint8_t(b << i) >> 7); ++i, ++this->n){
//			TRACE(<< "utf8::Iterator::operator++(): std::uint8_t(b << i) = " << std::hex << unsigned(std::uint8_t(b << i)) << std::endl)
//			TRACE(<< "utf8::Iterator::operator++(): ((b << i) >> 7) = " << std::hex << unsigned(std::uint8_t(b << i) >> 7) << std::endl)
//			TRACE(<< "utf8::Iterator::operator++(): *n = " << std::hex << unsigned(*this->n) << std::endl)
			this->c <<= 6;
//			TRACE(<< "utf8::Iterator::operator++(): c = " << std::hex << c << std::endl)
			this->c |= (*this->n) & 0x3f;
//			TRACE(<< "utf8::Iterator::operator++(): c = " << std::hex << c << std::endl)
		}
		this->c |= (std::uint32_t(std::uint8_t(b << (i + 1)) >> (i + 1)) << (6 * (i - 1)));
//		TRACE(<< "utf8::Iterator::operator++(): c = " << std::hex << c << std::endl)

		return *this;
	}

	//no postfix ++ operator, there is no need in it.

	/**
	 * @brief Check if iterator points to the end of the string.
     * @return true if iterator points to the end of the string.
	 * @return false otherwise.
     */
	bool IsEnd()const NOEXCEPT{
		return this->c == 0;
	}

	/**
	 * @brief Check if iterator points to the end of the string.
     * @return false if iterator points to the end of the string.
	 * @return true otherwise.
     */
	bool IsNotEnd()const NOEXCEPT{
		return !this->IsEnd();
	}
};



/**
 * @brief Fill buffer with UTF-32 string.
 * @param buf - buffer to fill.
 * @param str - iterator into the utf-8 string.
 * @return Number of characters filled in to the buffer.
 */
inline size_t FillBuffer(ting::ArrayAdaptor<std::uint32_t> buf, Iterator& str){
	std::uint32_t* p = buf.begin();
	for(; p != buf.end() && str.IsNotEnd(); ++p, ++str){
		*p = str.Char();
	}
	ASSERT(buf.Overlaps(p) || p == buf.end())
	return size_t(p - buf.begin());
}



}//~namespace
}//~namespace
