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
 * @file File wrapper allowing to set root path.
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include "File.hpp"

namespace ting{
namespace fs{

//TODO: doxygen
class RootDirFile : public File{
	std::unique_ptr<File> baseFile;
	std::string rootDir;
	
public:
	/**
	 * @param baseFile - a File to wrap.
	 * @param rootDir - path to the root directory to set. It should have trailing '/' character.
	 */
	RootDirFile(std::unique_ptr<File> baseFile, const std::string& rootDir) :
			baseFile(std::move(baseFile)),
			rootDir(rootDir)
	{}
	
	static std::unique_ptr<RootDirFile> New(std::unique_ptr<File> baseFile, const std::string& rootDir){
		return std::unique_ptr<RootDirFile>(baseFile, rootDir));
	}
	
	RootDirFile(const RootDirFile&) = delete;
	RootDirFile& operator=(const RootDirFile&) = delete;
	

	/**
	 * @brief Set root directory.
	 * Sets the root directory which holds the file system subtree. The file path
	 * set by SetPath() method will refer to a file path relative to the root directory.
	 * That means that all file operations like opening the file and other will be 
	 * performed on the actual file/directory referred by the final path which is a concatenation of
	 * the root directory and the path returned by Path() method. 
     * @param dir - path to the root directory to set. It should have trailing '/' character.
     */
	void SetRootDir(const std::string &dir){
		if(this->IsOpened()){
			throw File::IllegalStateExc("Cannot set root directory when file is opened");
		}
		this->rootDir = dir;
	}
	
private:
	void SetPathInternal(const std::string& pathName)override{
		this->File::SetPathInternal(this->rootDir + pathName);
	}

	
	
};

}
}
