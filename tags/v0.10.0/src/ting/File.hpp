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



namespace ting{



//TODO: add doxygen docs throughout the file

//TODO: make File a Waitable?
class File{
	std::string path;

	//TODO:
	//add file permissions

protected:
	ting::Inited<bool, false> isOpened;

public:
	//define exception class
	class Exc : public ting::Exc{
	public:
		Exc() :
				ting::Exc("[File::Exc]: unknown")
		{}

		Exc(const std::string& descr):
				ting::Exc((std::string("[File::Exc]: ") + descr).c_str())
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
			path(pathName)
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

	std::string ExtractExtension()const;

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
	bool IsDir()const;

	virtual ting::Array<std::string> ListDirContents();

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

	//returns number of bytes actually skipped
	virtual void SeekForward(unsigned numBytesToSeek);

	//returns number of bytes actually skipped
	virtual void SeekBackward(unsigned numBytesToSeek);

	virtual void MakeDir();
	
public:
	ting::Array<ting::u8> LoadWholeFileIntoMemory();

	virtual bool Exists()const;

public:
	//TODO: doxygen
	class Guard{
		File& f;
	public:
		Guard(File &file, EMode mode);
		
		~Guard();
	};
};



}//~namespace
