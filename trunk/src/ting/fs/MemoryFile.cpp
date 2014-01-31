/* 
 * File:   MemoryFile.cpp
 * Author: igagis
 * 
 * Created on January 31, 2014, 4:17 PM
 */

#include "MemoryFile.hpp"

using namespace ting;
using namespace fs;



//override
void MemoryFile::OpenInternal(E_Mode mode){
	this->ptr = this->data.Begin();
}



//override
size_t MemoryFile::ReadInternal(const ting::Buffer<ting::u8>& buf){
	ASSERT(this->data.End() - this->ptr >= 0)
	size_t numBytesRead = std::min(buf.SizeInBytes(), size_t(this->data.End() - this->ptr));
	memcpy(buf.Begin(), this->ptr, numBytesRead);
	this->ptr += numBytesRead;
	return numBytesRead;
}



//override
size_t MemoryFile::WriteInternal(const ting::Buffer<ting::u8>& buf){
	ASSERT(this->data.End() - this->ptr >= 0)
	size_t numBytesWritten = std::min(buf.SizeInBytes(), size_t(this->data.End() - this->ptr));
	memcpy(this->ptr, buf.Begin(), numBytesWritten);
	this->ptr += numBytesWritten;
	return numBytesWritten;
}



void MemoryFile::SeekForwardInternal(size_t numBytesToSeek){
	ASSERT(this->data.End() - this->ptr >= 0)
	if(size_t(this->data.End() - this->ptr) <= numBytesToSeek){
		this->ptr = this->data.End();
	}else{
		this->ptr += numBytesToSeek;
	}
}



void MemoryFile::SeekBackwardInternal(size_t numBytesToSeek){
	ASSERT(this->ptr - this->data.Begin() >= 0)
	if(size_t(this->ptr - this->data.Begin()) <= numBytesToSeek){
		this->ptr = this->data.Begin();
	}else{
		this->ptr -= numBytesToSeek;
	}
}



void MemoryFile::RewindInternal(){
	this->ptr = this->data.Begin();
}
