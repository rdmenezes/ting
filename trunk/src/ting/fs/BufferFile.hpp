/* The MIT License:

Copyright (c) 2014 Ivan Gagis

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
 */

#pragma once

#include "File.hpp"

namespace ting{
namespace fs{

//TODO: doxygen
class BufferFile : public File{
	
private:
	BufferFile(const BufferFile&);
	BufferFile& operator=(const BufferFile&);
	
private:
	const ting::Buffer<ting::u8> data;
	ting::u8* ptr;
	
public:
	//NOTE: ownership of the buffer is not taken, buffer must remain alive during this object's lifetime.
	BufferFile(const ting::Buffer<ting::u8>& data) :
			data(data.Begin(), data.Size())
	{}
	
	virtual ~BufferFile()throw(){}

protected:
	//override
	void OpenInternal(E_Mode mode);
	
	//override
	void CloseInternal()throw(){}
	
	//override
	size_t ReadInternal(const ting::Buffer<ting::u8>& buf);
	
	//override
	size_t WriteInternal(const ting::Buffer<const ting::u8>& buf);
	
	//override
	void SeekForwardInternal(size_t numBytesToSeek);
	
	//override
	void SeekBackwardInternal(size_t numBytesToSeek);
	
	//override
	void RewindInternal();
};

}}//~namespace
