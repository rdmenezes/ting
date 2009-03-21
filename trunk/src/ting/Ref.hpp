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

#ifndef M_Ref_hpp
#define M_Ref_hpp

#include "debug.hpp"
#include "types.hpp"

//#define M_ENABLE_REF_PRINT
#ifdef M_ENABLE_REF_PRINT
#define M_REF_PRINT(x) LOG(<<"[REF]" x)
#else
#define M_REF_PRINT(x)
#endif

namespace ting{

template <class T> class Ref;//forward declaration

//base class for a reference counted object
class RefCounted{
	template <class T> friend class Ref;
	template <class T> friend class WeakRef;

//microsoft compiler forbids derivation from private class for WeakRef despite WeakRef is friend of this class.
#if _MSC_VER == 1500
public:
#else	
private:
#endif
	//base class for weak references
	class CWeakRefBase{
		friend class RefCounted;
	protected:
		RefCounted *rc;
		CWeakRefBase *next, *prev;
	public:
		CWeakRefBase() :
				rc(0),
				next(0),
				prev(0)
		{
			M_REF_PRINT(<<"CWeakRefBase::"<<__func__<<"(): invoked"<<std::endl)
		};
		CWeakRefBase(RefCounted* rcounted) :
				rc(rcounted),
				prev(0)
		{
			M_REF_PRINT(<<"CWeakRefBase::"<<__func__<<"(rcounted): enter"<<std::endl)
			if(this->rc){
				this->next = this->rc->weakRefList;
				if(this->next) this->next->prev = this;
				this->rc->weakRefList = this;
			}else
				this->next=0;
			M_REF_PRINT(<<"CWeakRefBase::"<<__func__<<"(rcounted): exit"<<std::endl)
		};
		virtual ~CWeakRefBase(){
			M_REF_PRINT(<<"CWeakRefBase::"<<__func__<<"(): enter, this="<<this<<std::endl)
			if(this->rc)
				if(this->rc->weakRefList == this)
					this->rc->weakRefList = this->next;
			if(this->prev){
				this->prev->next=this->next;
				M_REF_PRINT(<<"CWeakRefBase::"<<__func__<<"(): this->prev->next="<<(this->prev->next)<<std::endl)
			}
			if(this->next)
				this->next->prev=this->prev;
		};
	protected:
		CWeakRefBase& operator=(RefCounted *r){
			this->~CWeakRefBase();
			this->rc=r;
			M_REF_PRINT(<<"CWeakRefBase::operator=(): invoked, rc="<<(this->rc)<<std::endl)
			//TODO: this is a copy-paste from constructor, maybe do some refactoring here?
			this->prev=0;
			if(this->rc){
				this->next = this->rc->weakRefList;
				if(this->next) this->next->prev = this;
				this->rc->weakRefList = this;
			}else
				this->next=0;
			return *this;
		};
	};//~class CWeakRefBase

private:
	uint numRefs;

	CWeakRefBase* weakRefList;//weak references are organized into double linked list

	inline void AddRef()throw(){
		M_REF_PRINT(<<"RefCounted::AddRef(): invoked, this->numRefs="<<(this->numRefs)<<std::endl)
		++this->numRefs;
	};
	inline uint RemRef()throw(){
		--this->numRefs;
		return this->numRefs;
	};
protected:
	inline uint NumRefs()const throw(){
		return this->numRefs;
	};

	//only base classes can construct this class
	//i.e. use of this class is allowed only as a base class
	inline RefCounted()throw() :
			numRefs(0),
			weakRefList(0)
	{};

//	inline static void* operator new(size_t s){
//		return ::operator new(s);
//	};

	//destructor shall be virtual!!!
	virtual ~RefCounted(){
		M_REF_PRINT(<<"RefCounted::~RefCounted(): enter"<<std::endl)
		//turn weak references to this object to zero because the object is being destroyed
		for(CWeakRefBase *cwr = this->weakRefList; cwr; cwr = cwr->next){
			ASSERT(cwr)
			M_REF_PRINT(<<"RefCounted::~RefCounted(): cwr="<<cwr<<" cwr->next="<<(cwr->next)<<std::endl)
			cwr->rc = 0;//clear reference
			M_REF_PRINT(<<"RefCounted::~RefCounted(): reference cleared"<<std::endl)
		}
		M_REF_PRINT(<<"RefCounted::~RefCounted(): exit"<<std::endl)
	};

private:
	//copy constructor is private, no copying
	inline RefCounted(const RefCounted& rc){};
};

//Class WeakRef (template).
//Weak Reference to a reference counted object.
//T should be RefCounted!!!
template <class T> class WeakRef : public RefCounted::CWeakRefBase{
	friend class Ref<T>;
	inline T* GetRefCounted(){
		return static_cast<T*>(this->rc);
	};
  public:
	inline WeakRef() :
			RefCounted::CWeakRefBase(0)
	{};
	
	inline WeakRef(T* rc) :
			RefCounted::CWeakRefBase(rc)
	{};
	
	inline WeakRef(Ref<T> r);

	//copy constructor
	inline WeakRef(const WeakRef &r) :
			RefCounted::CWeakRefBase(r.rc)
	{};

	//returns true if the reference is valid (not 0)
	inline bool IsValid()const{
		M_REF_PRINT(<<"WeakRef::IsValid(): invoked, this->rc="<<(this->rc)<<std::endl)
		return (this->rc != 0);
	};

	inline WeakRef& operator=(const Ref<T> &r){
		this->RefCounted::CWeakRefBase::operator=(r.p);
		return *this;
	};

	inline WeakRef& operator=(const WeakRef<T> &r){
		this->RefCounted::CWeakRefBase::operator=(r.rc);
		return *this;
	};

	inline WeakRef& operator=(const T* r){
		this->RefCounted::CWeakRefBase::operator=(static_cast<RefCounted*>(const_cast<T*>(r)));
		return *this;
	};

	inline void Reset(){
		this->operator=(reinterpret_cast<const T*>(0));
	};

	inline bool operator==(const T* therc)const{
		return this->rc == therc;
	};

	inline bool operator==(const Ref<T> ref)const{
		return this->rc == ref.p;
	};

	inline bool operator!=(const T* therc)const{
		return !this->operator==(therc);
	};

	inline operator bool()const{
		return this->IsValid();
	};

private:
	inline static void* operator new(size_t size){
		ASSERT_ALWAYS(false)//forbidden
		//WeakRef objects can only be creaed on stack
		//or as a memer of other object or array
		return 0;
	}
};

//Class Ref (template).
//Reference to a reference counted object.
//T should be RefCounted!!!
template <class T> class Ref{
	friend class WeakRef<T>;
protected:
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
	
	//the int argument is just to make possible
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
	}

	Ref& Seize(T* rc){
		this->operator=(Ref<T>(rc));
		return *this;
	}

	inline Ref(WeakRef<T> wr) :
			p(wr.GetRefCounted())
	{
		M_REF_PRINT(<<"Ref::Ref(wr): invoked, p="<<(this->p)<<std::endl)
		//ASSERT_INFO(this->p, "Ref::Ref(rc): rc is 0")
		if(this->IsValid())
			this->p->AddRef();
	}

	//copy constructor
	Ref(const Ref& r){
		M_REF_PRINT(<<"Ref::Ref(copy): invoked, r.p="<<(r.p)<<std::endl)
		this->p = r.p;
		if(this->IsValid())
			this->p->AddRef();
	}

	virtual ~Ref(){
		M_REF_PRINT(<<"Ref::~Ref(): invoked, p="<<(this->p)<<std::endl)
		if(this->IsValid()){
			M_REF_PRINT(<<"Ref::~Ref(): this->p->NumRefs()="<<(this->p->NumRefs())<<std::endl)
			if(this->p->RemRef() == 0){
				ASSERT(this->IsValid())
//                LOG(<<"Ref::~Ref(): deleting object "<<(this->p)<<std::endl)
				delete static_cast<T*>(this->p);
				M_REF_PRINT(<<"Ref::~Ref(): object "<<(this->p)<<" deleted"<<std::endl)
//                LOG(<<"Ref::~Ref(): object "<<(this->p)<<" deleted"<<std::endl)
			}
		}
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

	inline bool operator==(const RefCounted *rc)const{
		return this->p == rc;
	}

	inline bool operator!()const{
		return !this->IsValid();
	}

	inline operator bool()const{
		return this->IsValid();
	}

	void Reset(){
		this->~Ref();
		this->p = 0;
	}

	Ref& operator=(const Ref &r){
		M_REF_PRINT(<<"Ref::operator=(): invoked, p="<<(this->p)<<std::endl)
		if(this == &r)
			return *this;//detect self assignment

		this->~Ref();

		this->p = r.p;
		if(this->IsValid())
			this->p->AddRef();
		return *this;
	};

	inline T& operator*(){
		M_REF_PRINT(<<"Ref::operator*(): invoked, p="<<(this->p)<<std::endl)
		ASSERT_INFO(this->p, "Ref::operator*(): this->p is zero")
		return static_cast<T&>(*this->p);
	};

	inline const T& operator*()const{
		M_REF_PRINT(<<"const Ref::operator*(): invoked, p="<<(this->p)<<std::endl)
		ASSERT_INFO(this->p, "const Ref::operator*(): this->p is zero")
		return static_cast<T&>(*this->p);
	};

	inline T* operator->(){
		M_REF_PRINT(<<"Ref::operator->(): invoked, p="<<(this->p)<<std::endl)
		ASSERT_INFO(this->p, "Ref::operator->(): this->p is zero")
		return static_cast<T*>(this->p);
	};

	inline const T* operator->()const{
		M_REF_PRINT(<<"Ref::operator->()const: invoked, p="<<(this->p)<<std::endl)
		ASSERT_INFO(this->p, "Ref::operator->(): this->p is zero")
		return static_cast<T*>(this->p);
	};


	//for type downcast
	template <typename TBase> operator Ref<TBase>(){
		return Ref<TBase>(this->operator->());
	};


private:
	inline static void* operator new(size_t size){
		ASSERT_ALWAYS(false)//forbidden
		//Ref objects can only be creaed on stack
		//or as a memer of other object or array
		return 0;
	}
};

template <class T> inline WeakRef<T>::WeakRef(Ref<T> r) :
		RefCounted::CWeakRefBase(r.p)
{
	M_REF_PRINT(<<"WeakRef::"<<__func__<<"(Ref): invoked"<<std::endl)
};

};//~namespace igagis
#endif//~once
