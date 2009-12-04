/* The MIT License:

Copyright (c) 2008 Ivan Gagis

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
//	Reference counting mechanism base classes

#pragma once

#include "debug.hpp"
#include "types.hpp"
#include "Thread.hpp"
#include "PoolStored.hpp"

//#define M_ENABLE_REF_PRINT
#ifdef M_ENABLE_REF_PRINT
#define M_REF_PRINT(x) TRACE(<<"[REF]" x)
#else
#define M_REF_PRINT(x)
#endif

namespace ting{

template <class T> class Ref;//forward declaration
template <class T> class WeakRef;//forward declaration

//base class for a reference counted object
class RefCounted{
	template <class T> friend class Ref;
	template <class T> friend class WeakRef;


	
private:
	inline uint AddRef()throw(){
		ASSERT(this->counter)
		M_REF_PRINT(<< "RefCounted::AddRef(): invoked, old numHardRefs = " << (this->counter->numHardRefs) << std::endl)
		Mutex::Guard mutexGuard(this->counter->mutex);
		M_REF_PRINT(<< "RefCounted::AddRef(): mutex locked " << std::endl)
		return ++(this->counter->numHardRefs);
	}



	inline uint RemRef()throw(){
		M_REF_PRINT(<< "RefCounted::RemRef(): invoked, old numHardRefs = " << (this->counter->numHardRefs) << std::endl)
		this->counter->mutex.Lock();
		M_REF_PRINT(<< "RefCounted::RemRef(): mutex locked" << std::endl)
		uint n = --(this->counter->numHardRefs);

		if(n == 0){//if no more references to the RefCounted
			if(this->counter->numWeakRefs > 0){
				//there are weak references, they will now own the Counter object,
				//therefore, do not delete Counter, just clear the pointer to RefCounted.
				this->counter->p = 0;
			}else{//no weak references
				//NOTE: unlock before deleting because the mutex object is in Counter.
				this->counter->mutex.Unlock();
				M_REF_PRINT(<< "RefCounted::RemRef(): mutex unlocked" << std::endl)
				delete this->counter;
				return n;
			}
		}
		this->counter->mutex.Unlock();
		M_REF_PRINT(<< "RefCounted::RemRef(): mutex unlocked" << std::endl)

		return n;
	}



	struct Counter : public PoolStored<Counter>{
		RefCounted *p;
		Mutex mutex;
		uint numHardRefs;
		uint numWeakRefs;
		inline Counter(RefCounted *ptr) :
				p(ptr),
				numHardRefs(0),
				numWeakRefs(0)
		{
			M_REF_PRINT(<< "Counter::Counter(): counter object created" << std::endl)
		}

		inline ~Counter(){
			M_REF_PRINT(<< "Counter::~Counter(): counter object destroyed" << std::endl)
		}
	};



	Counter *counter;



protected:
	//only base classes can construct this class
	//i.e. use of this class is allowed only as a base class
	inline RefCounted() throw() :
			counter(new Counter(this))
	{
		ASSERT(this->counter)
	}



//	inline static void* operator new(size_t s){
//		return ::operator new(s);
//	}



public:
	//destructor shall be virtual!!!
	virtual ~RefCounted(){}



private:
	//copy constructor is private, no copying
	inline RefCounted(const RefCounted& rc){}
};//~class RefCounted



//Class Ref (template).
//Reference to a reference counted object.
//T should be RefCounted!!!
template <class T> class Ref{
	friend class WeakRef<T>;

	RefCounted *p;


	
public:
	template <class TS> inline Ref<TS> StaticCast(){
		return Ref<TS>(static_cast<TS*>(this->operator->()));
	}



	template <class TS> inline Ref<TS> DynamicCast(){
		TS* t = dynamic_cast<TS*>(this->operator->());
		if(t)
			return Ref<TS>(t);
		else
			return Ref<TS>();
	}



	template <class TS> inline const Ref<TS> DynamicCast()const{
		const TS* t = dynamic_cast<const TS*>(this->operator->());
		if(t)
			return Ref<TS>(const_cast<TS*>(t));
		else
			return Ref<TS>();
	}



	//NOTE: the int argument is just to make possible
	//auto conversion from 0 to invalid Ref object
	//i.e. it will be possible to write 'return 0;'
	//from the function returning Ref
	inline Ref(int v = 0) :
			p(0)
	{
		M_REF_PRINT(<<"Ref::Ref(): invoked, p="<<(this->p)<<std::endl)
	}



	//NOTE: this constructor should be explicit to prevent undesired conversions from T* to Ref<T>
	explicit inline Ref(T* rc) :
			p(static_cast<RefCounted*>(rc))
	{
		M_REF_PRINT(<<"Ref::Ref(rc): invoked, p="<<(this->p)<<std::endl)
		ASSERT_INFO(this->p, "Ref::Ref(rc): rc is 0")
		this->p->AddRef();
		M_REF_PRINT(<<"Ref::Ref(rc): exiting"<<(this->p)<<std::endl)
	}



	inline Ref(const WeakRef<T> &r);



	//copy constructor
	Ref(const Ref& r){
		M_REF_PRINT(<<"Ref::Ref(copy): invoked, r.p="<<(r.p)<<std::endl)
		this->p = r.p;
		if(this->p){
			this->p->AddRef();
		}
	}



	inline ~Ref(){
		M_REF_PRINT(<<"Ref::~Ref(): invoked, p="<<(this->p)<<std::endl)
		this->Destroy();
	}



	//returns true if the reference is valid (not 0)
	inline bool IsValid()const{
		M_REF_PRINT(<<"Ref::IsValid(): invoked, this->p="<<(this->p)<<std::endl)
		return (this->p != 0);
	}



	inline bool IsNotValid()const{
		M_REF_PRINT(<<"Ref::IsNotValid(): invoked, this->p="<<(this->p)<<std::endl)
		return !this->IsValid();
	}



	inline bool operator==(const Ref &r)const{
		return this->p == r.p;
	}



	inline bool operator!()const{
		return !this->IsValid();
	}



	//Safe conversion to bool type.
	//Because if using simple "operator bool()" it may result in chained automatic
	//conversion to undesired types such as int.
	typedef void (Ref::*unspecified_bool_type)();
	inline operator unspecified_bool_type() const{
		return this->IsValid() ? &Ref::Reset : 0;//Ref::Reset is taken just because it has matching signature
	}

//	inline operator bool(){
//		return this->IsValid();
//	}

	

	void Reset(){
		this->Destroy();
		this->p = 0;
	}



	Ref& operator=(const Ref &r){
		M_REF_PRINT(<<"Ref::operator=(): invoked, p="<<(this->p)<<std::endl)
		if(this == &r)
			return *this;//detect self assignment

		this->Destroy();

		this->p = r.p;
		if(this->p){
			this->p->AddRef();
		}
		return *this;
	}



	inline T& operator*(){
		M_REF_PRINT(<<"Ref::operator*(): invoked, p="<<(this->p)<<std::endl)
		ASSERT_INFO(this->p, "Ref::operator*(): this->p is zero")
		return static_cast<T&>(*this->p);
	}



	inline const T& operator*()const{
		M_REF_PRINT(<<"const Ref::operator*(): invoked, p="<<(this->p)<<std::endl)
		ASSERT_INFO(this->p, "const Ref::operator*(): this->p is zero")
		return static_cast<T&>(*this->p);
	}



	inline T* operator->(){
		M_REF_PRINT(<<"Ref::operator->(): invoked, p="<<(this->p)<<std::endl)
		ASSERT_INFO(this->p, "Ref::operator->(): this->p is zero")
		return static_cast<T*>(this->p);
	}



	inline const T* operator->()const{
		M_REF_PRINT(<<"Ref::operator->()const: invoked, p="<<(this->p)<<std::endl)
		ASSERT_INFO(this->p, "Ref::operator->(): this->p is zero")
		return static_cast<T*>(this->p);
	}



	//for type downcast
	template <typename TBase> inline operator Ref<TBase>(){
		//downcasting of invalid reference is also possible
		if(this->IsNotValid())
			return 0;

		M_REF_PRINT(<<"Ref::downcast(): invoked, p="<<(this->p)<<std::endl)

		//NOTE: static cast to T*, not to TBase*,
		//this is to forbid automatic upcast
		return Ref<TBase>(static_cast<T*>(this->p));
	}



private:
	inline void Destroy(){
		if(this->IsValid()){
			if(this->p->RemRef() == 0){
				ASSERT(this->IsValid())
				M_REF_PRINT(<< "Ref::Destroy(): deleting " << (this->p) << std::endl)
				delete static_cast<T*>(this->p);
				M_REF_PRINT(<< "Ref::Destroy(): object " << (this->p) << " deleted" << std::endl)
			}
		}
	}



	inline static void* operator new(size_t size){
		ASSERT_ALWAYS(false)//forbidden
		//Ref objects can only be created on stack
		//or as a member of other object or array
		return 0;
	}
};//~class Ref



//Class WeakRef (template).
//Weak Reference to a reference counted object.
//T should be RefCounted!!!
template <class T> class WeakRef{
	friend class Ref<T>;

	RefCounted::Counter *counter;


	
	inline void Init(const Ref<T> &r){
		if(r.IsNotValid()){
			this->counter = 0;
			return;
		}

		this->counter = r.p->counter;
		ASSERT(this->counter)

		this->counter->mutex.Lock();
		++(this->counter->numWeakRefs);
		this->counter->mutex.Unlock();
	}



	inline void Init(const WeakRef &r){
		this->counter = r.counter;
		if(this->counter == 0)
			return;

		this->counter->mutex.Lock();
		++(this->counter->numWeakRefs);
		this->counter->mutex.Unlock();
	}



	inline void Destroy(){
		if(this->counter == 0)
			return;

		this->counter->mutex.Lock();
		ASSERT(this->counter->numWeakRefs > 0)

		if(
				--(this->counter->numWeakRefs) == 0 &&
				this->counter->numHardRefs == 0
			)
		{
			this->counter->mutex.Unlock();
			delete this->counter;
			return;
		}else{
			this->counter->mutex.Unlock();
		}
	}


	
public:
	//NOTE: the int argument is just to make possible
	//auto conversion from 0 to invalid WeakRef
	//i.e. it will be possible to write 'return 0;'
	//from the function returning WeakRef
	inline WeakRef(int v = 0) :
			counter(0)
	{}



	inline WeakRef(const Ref<T> &r){
		this->Init(r);
	}



	//copy constructor
	inline WeakRef(const WeakRef &r){
		this->Init(r);
	}



	inline ~WeakRef(){
		this->Destroy();
	}



	inline WeakRef& operator=(const Ref<T> &r){
		//TODO: double mutex lock/unlock (one in destructor and one in Init). Optimize?
		this->Destroy();
		this->Init(r);
		return *this;
	}



	inline WeakRef& operator=(const WeakRef<T> &r){
		//TODO: double mutex lock/unlock (one in destructor and one in Init). Optimize?
		this->Destroy();
		this->Init(r);
		return *this;
	}



	inline void Reset(){
		this->Destroy();
		this->counter = 0;
	}


	
private:
	inline static void* operator new(size_t size){
		ASSERT_ALWAYS(false)//forbidden
		//WeakRef objects can only be creaed on stack
		//or as a memer of other object or array
		return 0;
	}
};//~class WeakRef



template <class T> inline Ref<T>::Ref(const WeakRef<T> &r){
	if(r.counter == 0){
		this->p = 0;
		return;
	}

	r.counter->mutex.Lock();

	this->p = r.counter->p;

	if(this->p){
		++(r.counter->numHardRefs);
	}
	
	r.counter->mutex.Unlock();
}



}//~namespace
