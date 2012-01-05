/* The MIT License:

Copyright (c) 2009-2011 Ivan Gagis <igagis@gmail.com>

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

// Homepage: http://ting.googlecode.com



/**
 * @file PoolStored.hpp
 * @author Ivan Gagis <igagis@gmail.com>
 * @brief Memory Pool.
 * Alternative memory allocation functions for simple objects.
 * The main purpose of this facilities is to prevent memory fragmentation.
 */

#pragma once

#include <new>
#include <list>

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

//make sure theat we align PoolElem by int size when using MSVC compiler.
STATIC_ASSERT(sizeof(int) == 4)



template <size_t ElemSize, size_t NumElemsInChunk> class MemoryPool{		
	struct BufHolder{
		u8 buf[ElemSize];
	};
	
	struct Chunk;

	M_DECLARE_ALIGNED_MSVC(4) struct PoolElem : public BufHolder{
		PoolElem *next; //initialized only when the PoolElem is freed for the first time.
		Chunk* parent; //initialized only upon PoolElem allocation
	}
	//Align by sizeof(int) boundary, just to be more safe.
	//I once had a problem with pthread mutex when it was not aligned by 4 byte boundary,
	//so I resolved this by declaring PoolElem structure as aligned by sizeof(int).
	M_DECLARE_ALIGNED(sizeof(int));


	struct Chunk : public ting::Array<PoolElem>{ //consider using static buffer and self made linked list of Chunks
		ting::Inited<size_t, 0> numAllocated;
		
		ting::Inited<size_t, 0> freeIndex;
		
		ting::Inited<PoolElem*, 0> firstFree;
		
		Chunk() :
				ting::Array<PoolElem>(NumElemsInChunk)
		{}

		Chunk(const Chunk& c) :
				ting::Array<PoolElem>(c),
				numAllocated(c.numAllocated),
				freeIndex(c.freeIndex),
				firstFree(c.firstFree)
		{
			const_cast<Chunk&>(c).numAllocated = 0;//to prevent assert in destructor
			M_POOL_TRACE(<< "Chunk::Chunk(copy): invoked" << std::endl)
		}

		~Chunk(){
			ASSERT_INFO(this->numAllocated == 0, "this->numAllocated = " << this->numAllocated << " should be 0")
		}
		
		inline bool IsFull()const{
			return this->numAllocated == this->Suze();
		}
		
		inline void* Alloc(){
			if(this->numAllocated < this->Size()){
				PoolElem* ret = &this->operator[this->numAllocated];
				++this->numAllocated;
				ret->parent = this;
				return static_cast<BufHolder*>(ret);
			}
			
			ASSERT(this->firstFree)
			
			PoolElem* ret = this->firstFree;
			ASSERT(ret->parent == this)
			this->firstFree = ret->next;
			++this->numAllocated;
			return static_cast<BufHolder*>(ret);
		}

	private:
		Chunk& operator=(const Chunk&);//assignment is not allowed (no operator=() implementation provided)
	};


	typedef std::list<Chunk> T_ChunksList;
	typedef typename T_List::iterator T_ChunksIter;
	T_ChunksList chunks;
	ting::Mutex mutex; //TODO: consider using spinlock
	
public:
	~MemoryPool(){
		ASSERT_INFO(this->chunks.size() == 0, "MemoryPool: cannot destroy chunk list because it is not empty (" << this->chunks.size() << "). Check for static PoolStored objects, they are not allowed, e.g. static Ref/WeakRef are not allowed!")
	}

public:
	void* Alloc_ts(){
		ting::Mutex::Guard mutexGuard(this->mutex);
		
		if(this->chunks.size() == 0 || this->chunks.front().IsFull()){
			this->chunks.push_front(Chunk());
		}
		
		void* ret = this->chunks.front().Alloc();
		
		if(this->chunks.front().IsFull()){
			this->chunks.push_back(this->chunks.front());
			this->chunks.pop_front();
		}
		
		return ret;
	}

	void Free_ts(void* p){
		ting::Mutex::Guard mutexGuard(this->mutex);
		
		PoolElem* e = static_cast<PoolElem*>(static_cast<BufHolder*>(p));
		
		ASSERT(e->parent->numAllocated > 0)
		e->next = e->parent->firstFree;
		e->parent->firstFree = e;
		--e->parent->numAllocated;
		
		if(!e->parent->IsFull()){
			
		}
	}
};//~template class MemoryPool



template <size_t ElemSize, size_t NumElemsInChunk> class StaticMemoryPool{
	static MemoryPool<ElemSize, NumElemsInChunk> instance;
public:
	
	static inline void* Alloc_ts(){
		return instance.Alloc_ts();
	}
	
	static inline void Free_ts(void* p){
		instance.Free_ts(p);
	}
};



template <size_t ElemSize, size_t NumElemsInChunk> typename ting::MemoryPool<ElemSize, NumElemsInChunk> ting::StaticMemoryPool<ElemSize, NumElemsInChunk>::instance;



/**
 * @brief Base class for pool-stored objects.
 * If the class is derived from PoolStored it will override 'new' and 'delete'
 * operators for that class so that the objects would be stored in the
 * memory pool instead of using standard memory manager to allocate memory.
 * Storing objects in memory pool allows to avoid memory fragmentation.
 * PoolStored is only useful for systems with large amount of small and
 * simple objects which have to be allocated dynamically (i.e. using new/delete
 * operators).
 * For example, PoolStored is used in ting::Ref (reference counted objects)
 * class to allocate reference counting objects which holds number of references  and
 * pointer to reference counted object.
 * NOTE: class derived from PoolStored SHALL NOT be used as a base class further.
 */
template <class T> class PoolStored{

protected:
	//this should only be used as a base class
	PoolStored(){}

public:

	inline static void* operator new(size_t size){
		M_POOL_TRACE(<< "new(): size = " << size << std::endl)
		if(size != sizeof(T)){
			throw ting::Exc("PoolStored::operator new(): attempt to allocate memory block of incorrect size");
		}

		return StaticMemoryPool<sizeof(T), ((8192 / sizeof(T)) < 32) ? 32 : (8192 / sizeof(T))>::Alloc_ts();
	}

	inline static void operator delete(void *p){
		StaticMemoryPool<sizeof(T), ((8192 / sizeof(T)) < 32) ? 32 : (8192 / sizeof(T))>::Free_ts(p);
	}

private:
};



}//~namespace ting
