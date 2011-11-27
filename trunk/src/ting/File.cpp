/* The MIT License:

Copyright (c) 2009-2011 Ivan Gagis

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

#include "File.hpp"



using namespace ting;



std::string File::ExtractExtension()const{
	size_t dotPos = this->Path().rfind('.');
	if(dotPos == std::string::npos || dotPos == 0){//NOTE: dotPos is 0 for hidden files in *nix systems
		return std::string();
	}else{
		ASSERT(dotPos > 0)
		ASSERT(this->Path().size() > 0)
		ASSERT(this->Path().size() >= dotPos + 1)
		
		//Check for hidden file on *nix systems
		if(this->Path()[dotPos - 1] == '/'){
			return std::string();
		}
		
		return std::string(this->Path(), dotPos + 1, this->Path().size() - (dotPos + 1));
	}
	ASSERT(false)
}



bool File::IsDir()const{
	if(this->Path().size() == 0)
		return true;

	ASSERT(this->Path().size() > 0)
	if(this->Path()[this->Path().size() - 1] == '/')
		return true;

	return false;
}



ting::Array<std::string> File::ListDirContents(){
	throw File::Exc("File::ListDirContents(): not supported for this File instance");
}



void File::SeekForward(unsigned numBytesToSeek){
	if(!this->IsOpened())
		throw File::Exc("SeekForward(): file is not opened");

	//TODO: allocate limited size buffer and read in a loop and use try catch to throw EOF exception
	ting::Array<ting::u8> buf(numBytesToSeek);
	this->Read(buf);
}



void File::SeekBackward(unsigned numBytesToSeek){
	throw ting::Exc("SeekBackward(): unsupported");
}


void File::Rewind(){
	throw ting::Exc("Rewind(): unsupported");
}



void File::MakeDir(){
	throw File::Exc("Make directory is not supported");
}



namespace{
unsigned DReadBlockSize = 4096;
}



ting::Array<ting::u8> File::LoadWholeFileIntoMemory(){
	if(this->IsOpened())
		throw File::Exc("file should not be opened");

	File::Guard fileGuard(*this, File::READ);//make sure we close the file upon exit from the function

	//two arrays
	ting::Array<ting::u8> a(DReadBlockSize); //start with 4kb
	ting::Array<ting::u8> b;

	//two pointers
	ting::Array<ting::u8> *currArr = &a;
	ting::Array<ting::u8> *freeArr = &b;

	unsigned numBytesRead = 0;
	unsigned numBytesReadLastOp = 0;
	for(;;){
		if( currArr->Size() < (numBytesRead + DReadBlockSize) ){
			freeArr->Init( currArr->Size() + DReadBlockSize );
			ASSERT(freeArr->Size() > numBytesRead);
			memcpy(freeArr->Begin(), currArr->Begin(), numBytesRead);
			currArr->Reset();//free memory
			std::swap(currArr, freeArr);
		}

		numBytesReadLastOp = this->Read(*currArr, DReadBlockSize, numBytesRead);

		numBytesRead += numBytesReadLastOp;//update number of bytes read

		if(numBytesReadLastOp != DReadBlockSize)
			break;
	}//~for
	freeArr->Init(numBytesRead);
	memcpy(freeArr->Begin(), currArr->Begin(), numBytesRead);

//	TRACE(<< "File loaded" <<std::endl)

	return *freeArr;
}



bool File::Exists()const{
	if(this->IsDir())
		throw File::Exc("File::Exists(): Checking for directory existence is not supported");

	if(this->IsOpened())
		return true;

	//try opening and closing the file to find out if it exists or not
	ASSERT(!this->IsOpened())
	try{
		File::Guard fileGuard(const_cast<File&>(*this), File::READ);
	}catch(File::Exc &e){
		return false;//file opening failed, assume the file does not exist
	}
	return true;//file open succeeded => file exists
}



File::Guard::Guard(File& file, EMode mode) :
		f(file)
{
	if(this->f.IsOpened())
		throw File::Exc("File::Guard::Guard(): file is already opened");

	this->f.Open(mode);
}



File::Guard::~Guard(){
	this->f.Close();
}
