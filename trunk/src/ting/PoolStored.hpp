/* The MIT License:

Copyright (c) 2009 Ivan Gagis

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

// ting 0.3
// Homepage: http://code.google.com/p/ting
// Author: Ivan Gagis <igagis@gmail.com>

// File description:
//	Memory Pool

#ifndef M_PoolStored_hpp
#define M_PoolStored_hpp

#include <new>
#include <vector>

#include "debug.hpp"
#include "types.hpp"
#include "utils.hpp"
#include "Thread.hpp"
#include "Exc.hpp"
#include "Array.hpp"

//#define M_ENABLE_POOL_TRACE
#ifdef M_ENABLE_POOL_TRACE
#define M_POOL_TRACE(x) TRACE(<<"[POOL] ") TRACE(x)
#else
#define M_POOL_TRACE(x)
#endif

namespace ting{


template <class T> class PoolStored{
	
	template <ting::uint ElemSize, ting::uint NumElemsInChunk> class MemPool{
		struct BufHolder{
			ting::byte buf[ElemSize];
		};

		STATIC_ASSERT(false)//TODO: why doesn't it trigger?
		
		struct PoolElem : public BufHolder{
			bool isFree;
			PoolElem() :
					isFree(true)
			{}
		}
		//Align by sizeof(int) boundary, just to be more safe.
		//I once had a problem with pthread mutex when it was not aligned by 4 byte bounday,
		//so I resolved this by declaring PoolElem struct as aligned by sizeof(int).
		M_DECLARE_ALIGNED(sizeof(int));

		struct Chunk : public ting::Array<PoolElem>{
			ting::uint numAllocated;
			Chunk() :
					ting::Array<PoolElem>(NumElemsInChunk),
					numAllocated(0)
			{}
		};

		struct ChunksList{
			typedef std::vector<Chunk> T_List;
			typedef typename T_List::iterator T_Iter;
			T_List chunks;
			ting::Mutex mutex;
		};
		
		static ChunksList& Chunks(){
			static ChunksList chunks;
			return chunks;
		}

	public:
		static void* Alloc(){
			ChunksList &cl = Chunks();
			
			ting::Mutex::Guard mutlock(cl.mutex);

			//find chunk with free cell
			Chunk *chunk = 0;
			for(typename ChunksList::T_Iter i = cl.chunks.begin(); i != cl.chunks.end(); ++i){
				if((*i).numAllocated < (*i).Size()){
					chunk = &(*i);
				}
			}
			
			//create new chunk if necessary
			if(chunk == 0){
				cl.chunks.push_back(Chunk());
				chunk = &cl.chunks.back();
			}

			ASSERT(chunk)
			M_POOL_TRACE(<< "Alloc(): Free chunk = " << chunk << std::endl)

			//find free cell
			for(PoolElem* i = chunk->Begin(); i != chunk->End(); ++i){
				if(i->isFree){
					ASSERT(chunk->numAllocated < chunk->Size())
					i->isFree = false;
					++chunk->numAllocated;
					M_POOL_TRACE(<< "Alloc(): Free cell found = " << i << " sizeof(PoolElem) = " << sizeof(PoolElem) << std::endl)
					M_POOL_TRACE(<< "Alloc(): returning " << static_cast<BufHolder*>(i) << std::endl)
					return reinterpret_cast<void*>(static_cast<BufHolder*>(i));
				}
			}
			ASSERT(false)
		}

		static void Free(void* p){
			M_POOL_TRACE(<< "Free(): p = " << p << std::endl)
			if(!p)
				return;
			
			ChunksList &cl = Chunks();

			ting::Mutex::Guard mutlock(cl.mutex);
			
			//find chunk the p belongs to
			for(typename ChunksList::T_Iter i = cl.chunks.begin(); i != cl.chunks.end(); ++i){
				ASSERT((*i).numAllocated != 0)
				if((*i).End() > p && p >= (*i).Begin()){
					Chunk *chunk = &(*i);
					M_POOL_TRACE(<< "Free(): chunk found = " << chunk << std::endl)
					--(chunk->numAllocated);
					if(chunk->numAllocated == 0){
						cl.chunks.erase(i);
					}else{
						static_cast<PoolElem*>(
								reinterpret_cast<BufHolder*>(p)
							)->isFree = true;
					}
					return;
				}
			}
			ASSERT(false)
		}
	};

protected:
	//this should only be used as a base class
	PoolStored(){}

public:

#define M_MEMPOOL_TYPEDEF \
typedef MemPool< \
		sizeof(T), \
		((8192 / sizeof(T)) < 32) ? 32 : (8192 / sizeof(T)) \
	> T_MemoryPool;

	static void* operator new (size_t size){
		M_POOL_TRACE(<< "new(): size = " << size << std::endl)
		if(size != sizeof(T))
			throw ting::Exc("PoolStored::operator new(): attempt to allocate memory block of incorrect size");

		M_MEMPOOL_TYPEDEF

		return T_MemoryPool::Alloc();
	}

	static void operator delete (void *p){
		M_MEMPOOL_TYPEDEF
		
		T_MemoryPool::Free(p);
	}
	
#undef M_MEMPOOL_TYPEDEF

private:
};

}//~namespace ting
#endif//~once
