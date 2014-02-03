/* 
 * File:   BufferFile.cpp
 * Author: igagis
 * 
 * Created on January 31, 2014, 4:17 PM
 */

#include "BufferFile.hpp"

using namespace ting;
using namespace fs;



//override
void BufferFile::OpenInternal(E_Mode mode){
	this->ptr = this->data.Begin();
}



//override
size_t BufferFile::ReadInternal(const ting::Buffer<ting::u8>& buf){
	ASSERT(this->ptr <= this->data.End())
	size_t numBytesRead = std::min(buf.SizeInBytes(), size_t(this->data.End() - this->ptr));
	memcpy(buf.Begin(), this->ptr, numBytesRead);
	this->ptr += numBytesRead;
	ASSERT(this->data.Overlaps(this->ptr) || this->ptr == this->data.End())
	return numBytesRead;
}



//override
size_t BufferFile::WriteInternal(const ting::Buffer<const ting::u8>& buf){
	ASSERT(this->ptr <= this->data.End())
	size_t numBytesWritten = std::min(buf.SizeInBytes(), size_t(this->data.End() - this->ptr));
	memcpy(this->ptr, buf.Begin(), numBytesWritten);
	this->ptr += numBytesWritten;
	ASSERT(this->data.Overlaps(this->ptr) || this->ptr == this->data.End())
	return numBytesWritten;
}



//override
void BufferFile::SeekForwardInternal(size_t numBytesToSeek){
	ASSERT(this->ptr <= this->data.End())
	numBytesToSeek = std::min(size_t(this->data.End() - this->ptr), numBytesToSeek);
	this->ptr += numBytesToSeek;
	ASSERT(this->data.Overlaps(this->ptr) || this->ptr == this->data.End())
}



//override
void BufferFile::SeekBackwardInternal(size_t numBytesToSeek){
	ASSERT(this->ptr >= this->data.Begin())
	numBytesToSeek = std::min(size_t(this->ptr - this->data.Begin()), numBytesToSeek);
	this->ptr -= numBytesToSeek;
	ASSERT(this->data.Overlaps(this->ptr) || this->ptr == this->data.End())
}



//override
void BufferFile::RewindInternal(){
	this->ptr = this->data.Begin();
}
