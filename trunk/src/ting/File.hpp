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

/**
 * @file File abstract interface
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include <string>
#include <algorithm>
#include <cstdlib>

#include "debug.hpp"
#include "types.hpp"
#include "Buffer.hpp"
#include "Array.hpp"
#include "Exc.hpp"



//TODO: add doxygen docs throughout the file
class File{
	std::string path;

	//TODO:
	//add file permissions

protected:
	bool isOpened;

public:
	//define exception class
	class Exc : public ting::Exc{
	public:
		Exc() : ting::Exc("File::Exc: unknown")
		{}

		Exc(const std::string& descr):
			ting::Exc((std::string("File::Exc: ") + descr).c_str())
		{}
	};

	//modes of file opening
	enum EMode{
		READ,  //Open existing file for read only
		WRITE, //Open existing file for read and write
		CREATE //Create new file and open it for read and write. If file exists it will be replaced by empty file.
			   //This mode is used for C_File::Open() method only. Later the mode is stored as WRITE in the this->mode variable
	};

protected:
	EMode ioMode;//mode only matters when file is opened
public:

	inline File(const std::string& pathName = std::string()) :
			path(pathName),
			isOpened(false)
	{}

private:
	//no copying
	inline File(const File& f);

	//no assigning
	File& operator=(const File& f);
public:

	virtual ~File(){}

	inline void SetPath(const std::string& pathName){
		if(this->isOpened)
			throw File::Exc("cannot set path while file is opened");

		this->path = pathName;
	}

	inline const std::string& Path()const{
		return this->path;
	}

	std::string ExtractExtension()const{
		size_t dotPos = this->Path().rfind('.');
		if(dotPos == std::string::npos || dotPos == 0){//NOTE: dotPos is 0 for hidden files in *nix systems
			return std::string();
		}else{
			ASSERT(this->Path().size() > 0)
			ASSERT(this->Path().size() >= dotPos + 1)
			return std::string(this->Path(), dotPos + 1, this->Path().size() - (dotPos + 1));
		}
		ASSERT(false)
	}
	
	virtual void Open(EMode mode) = 0;

	virtual void Close() = 0;
	
	inline bool IsOpened()const{
		return this->isOpened;
	}

	/**
	 * @brief Returns true if path points to directory.
	 * Determines if the current path is a directory.
	 * This function just checks if the path finishes with '/'
	 * character, and if it does then it is a directory.
	 * Empty path refers to the current directory.
	 * @return true - if current path points to a directory.
	 * @return false - otherwise.
	 */
	inline bool IsDir()const{
		if(this->Path().size() == 0)
			return true;

		ASSERT(this->Path().size() > 0)
		char lastChar = this->Path()[this->Path().size() - 1];
		
		if(lastChar == '/')
			return true;

		return false;
	}

	virtual ting::Array<std::string> ListDirContents(){
		throw File::Exc("File::ListDirContents(): not supported for this File instance");
	}

	//returns number of bytes actually read
	virtual unsigned Read(
			ting::Buffer<ting::u8>& buf,
			unsigned numBytesToRead = 0, //0 means the whole buffer size
			unsigned offset = 0
		) = 0;

	//returns number of bytes actually written
	virtual unsigned Write(
			const ting::Buffer<ting::u8>& buf,
			unsigned numBytesToWrite = 0, //0 means the whole buffer size
			unsigned offset = 0
		) = 0;

        //number of bytes actually skipped
	virtual unsigned SeekForward(unsigned numBytesToSeek){
		if(!this->IsOpened())
			throw File::Exc("File::SeekForward(): file is not opened");

		//TODO: allocate limited size buffer and read in a loop
		ting::Array<ting::u8> buf(numBytesToSeek);
		unsigned numBytesRead = this->Read(buf);
		return numBytesRead;
	}
        
	virtual unsigned SeekBackward(unsigned numBytesToSeek){
		throw ting::Exc("File::SeekBackward(): unsupported");
	}

	virtual void MakeDir(){
		throw File::Exc("Make directory is not supported");
	}
private:
	static inline unsigned DReadBlockSize(){
		return 4096;
	}
public:
	ting::Array<ting::u8> LoadWholeFileIntoMemory(){
		if(this->IsOpened())
			throw File::Exc("file should not be opened");

		File::Guard fileCloser(*this);//make sure we close the file upon exit from the function

//		TRACE(<< "Opening file..." <<std::endl)

		//try to open the file
		this->Open(File::READ);

//		TRACE(<< "File opened" <<std::endl)

		//two arrays
		ting::Array<ting::u8> a(DReadBlockSize()); //start with 4kb
		ting::Array<ting::u8> b;

		//two pointers
		ting::Array<ting::u8> *currArr = &a;
		ting::Array<ting::u8> *freeArr = &b;
 
		unsigned numBytesRead = 0;
		unsigned numBytesReadLastOp = 0;
		for(;;){
			if( currArr->Size() < (numBytesRead + DReadBlockSize()) ){
				freeArr->Init( currArr->Size() + DReadBlockSize() );
				ASSERT(freeArr->Size() > numBytesRead);
				memcpy(freeArr->Begin(), currArr->Begin(), numBytesRead);
				currArr->Reset();//free memory
				std::swap(currArr, freeArr);
			}

			numBytesReadLastOp = this->Read(*currArr, DReadBlockSize(), numBytesRead);

			numBytesRead += numBytesReadLastOp;//update number of bytes read

			if(numBytesReadLastOp != DReadBlockSize())
				break;
		}//~for
		freeArr->Init(numBytesRead);
		memcpy(freeArr->Begin(), currArr->Begin(), numBytesRead);

//		TRACE(<< "File loaded" <<std::endl)

		return *freeArr;
	}

	virtual bool Exists()const{
		if(this->Path().size() == 0)
			return false;

		if(this->Path()[this->Path().size() - 1] == '/')
			throw File::Exc("Checking for directory existence is not supported");
		
		if(this->IsOpened())
			return true;

		//try opening and closing the file to find out if it exists or not
		ASSERT(!this->IsOpened())
		try{
			File::Guard fileCloser(const_cast<File&>(*this));//make sure we close the file upon exit from try/catch block
			const_cast<File* const>(this)->Open(READ);
		}catch(File::Exc &e){
			return false;//file opening failed, assume the file does not exist
		}
		return true;//file open succeeded => file exists
	}
private:

public:
	class Guard{
		File& f;
	public:
		Guard(File &file) :
				f(file)
		{}
		
		~Guard(){
			f.Close();
		}
	};
};

