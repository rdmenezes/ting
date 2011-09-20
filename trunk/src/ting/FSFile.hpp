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
 * @file Ordinary file system File implementation
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include <cstdio>
#include <vector>

#ifdef __linux__
#include <dirent.h>
#include <sys/stat.h>
#include <cerrno>
#endif

#include "debug.hpp"
#include "File.hpp"



namespace ting{



class FSFile : public File{
	std::string rootDir;

	ting::Inited<FILE*, 0> handle;

protected:

	inline std::string TruePath()const{
		return this->rootDir + this->Path();
	}

public:
	FSFile(const std::string& pathName = std::string()) :
			File(pathName)
	{}

	~FSFile(){
		this->Close();
	}


	void SetRootDir(const std::string &dir);

	inline std::string GetRootDir()const{
		return this->rootDir;
	}


	//override
	virtual void Open(EMode mode);



	//override
	virtual void Close();



	//override
	virtual unsigned Read(
			ting::Buffer<ting::u8>& buf,
			unsigned numBytesToRead = 0,
			unsigned offset = 0
		);



	//override
	//returns number of bytes actually written
	virtual unsigned Write(
			const ting::Buffer<ting::u8>& buf,
			unsigned numBytesToWrite = 0,
			unsigned offset = 0
		);



	//override
	virtual void SeekForward(unsigned numBytesToSeek);



	//override
	virtual bool Exists()const;



	//override
	virtual void MakeDir();



public:
	//returns string of format: "/home/user/"
	static std::string GetHomeDir();



	//override
	virtual ting::Array<std::string> ListDirContents();
};



}//~namespace
