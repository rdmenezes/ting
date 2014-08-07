#include "../../src/ting/debug.hpp"
#include "../../src/ting/Ref.hpp"
#include "../../src/ting/Exc.hpp"
#include "../../src/ting/mt/Thread.hpp"
#include "../../src/ting/Ref.hpp"

#include "tests.hpp"



namespace TestBoolRelatedStuff{

class TestClass : public ting::RefCounted{
public:
};



void TestConversionToBool(){
	ting::Ref<TestClass> a;
	ting::Ref<TestClass> b = ting::NewRefCounted<TestClass>();

	//test conversion to bool
	if(a){
		ASSERT_ALWAYS(false)
	}

	if(b){
	}else{
		ASSERT_ALWAYS(false)
	}
}



void TestOperatorLogicalNot(){
	ting::Ref<TestClass> a;
	ting::Ref<TestClass> b = ting::NewRefCounted<TestClass>();

	//test operator !()
	if(!a){
	}else{
		ASSERT_ALWAYS(false)
	}

	if(!b){
		ASSERT_ALWAYS(false)
	}
}

}//~namespace



namespace TestOperatorArrowAndOperatorStar{
class A : public ting::RefCounted{
public:
	int a;

	A() : a(98)
	{}
};


void Run1(){
	ting::Ref<A> a = ting::NewRefCounted<A>();
	ting::Ref<const A> ac = ting::NewRefCounted<A>();

	ASSERT_ALWAYS(a)
	ASSERT_ALWAYS(ac)

	ASSERT_ALWAYS(ac != a)
	ASSERT_ALWAYS(a != ac)

	a->a = 123;
	ASSERT_ALWAYS((*a).a == 123)

	(*a).a = 456;
	ASSERT_ALWAYS(a->a == 456)

	ASSERT_ALWAYS(ac->a == 98)
	ASSERT_ALWAYS((*ac).a == 98)

	ac = a;

	ASSERT_ALWAYS(ac == a)
	ASSERT_ALWAYS(a == ac)
}

void Run2(){
	const ting::Ref<A> a = ting::NewRefCounted<A>();
	const ting::Ref<const A> ac = ting::NewRefCounted<A>();

	ASSERT_ALWAYS(a)
	ASSERT_ALWAYS(ac)

	ASSERT_ALWAYS(ac != a)
	ASSERT_ALWAYS(a != ac)

	a->a = 123;
	ASSERT_ALWAYS((*a).a == 123)

	(*a).a = 456;
	ASSERT_ALWAYS(a->a == 456)

	ASSERT_ALWAYS(ac->a == 98)
	ASSERT_ALWAYS((*ac).a == 98)
}

}//~namespace



namespace TestBasicWeakRefUseCase{
class TestClass : public ting::RefCounted{
public:
	bool *destroyed;

	TestClass() :
		destroyed(0)
	{}

	~TestClass()noexcept{
		if(this->destroyed){
			*this->destroyed = true;
		}
	}
};

void Run1(){
	for(unsigned i = 0; i < 1000; ++i){
		ting::Ref<TestClass> a = ting::NewRefCounted<TestClass>();
		ASSERT_ALWAYS(a.IsValid())

		bool wasDestroyed = false;
		a->destroyed = &wasDestroyed;

		ting::WeakRef<TestClass> wr(a);
		ASSERT_ALWAYS(ting::Ref<TestClass>(wr).IsValid())
		ASSERT_ALWAYS(!wasDestroyed)

		a.Reset();

		ASSERT_ALWAYS(a.IsNotValid())
		ASSERT_ALWAYS(wasDestroyed)
		ASSERT_INFO_ALWAYS(ting::Ref<TestClass>(wr).IsNotValid(), "i = " << i)
	}//~for
}

void Run2(){
	ting::Ref<TestClass> a = ting::NewRefCounted<TestClass>();
	ASSERT_ALWAYS(a.IsValid())

	bool wasDestroyed = false;
	a->destroyed = &wasDestroyed;

	ting::WeakRef<TestClass> wr1(a);
	ting::WeakRef<TestClass> wr2(wr1);
	ting::WeakRef<TestClass> wr3;

	wr3 = wr1;

	ASSERT_ALWAYS(ting::Ref<TestClass>(wr1).IsValid())
	ASSERT_ALWAYS(ting::Ref<TestClass>(wr2).IsValid())
	ASSERT_ALWAYS(wr3.GetRef().IsValid())

	a.Reset();

	ASSERT_ALWAYS(a.IsNotValid())
	ASSERT_ALWAYS(wasDestroyed)

	ASSERT_ALWAYS(ting::Ref<TestClass>(wr1).IsNotValid())
	ASSERT_ALWAYS(ting::Ref<TestClass>(wr2).IsNotValid())
	ASSERT_ALWAYS(wr3.GetRef().IsNotValid())
}

}//~namespace



namespace TestExceptionThrowingFromRefCountedDerivedClassConstructor{

class TestClass : public ting::RefCounted{
public:
	TestClass(){
		throw ting::Exc("TestException!");
	}
};



void Run(){
	try{
		ting::Ref<TestClass> a = ting::NewRefCounted<TestClass>();
		ASSERT_ALWAYS(false)
	}catch(ting::Exc&){
		//do nothing
	}
}

}//~namespace



namespace TestCreatingWeakRefFromRefCounted{

class TestClass : public ting::RefCounted{
public:
	bool *destroyed;

	TestClass() :
			destroyed(0)
	{}

	static void* operator new(size_t s){
		return ::operator new(s);
	}
	
	static inline TestClass* SimpleNew(){
		return new TestClass();
	}

	~TestClass()noexcept{
		if(this->destroyed){
			*this->destroyed = true;
		}
	}
};



void Run1(){
//	TRACE(<< "TestCreatingWeakRefFromRefCounted::Run(): enter" << std::endl)
	
	bool destroyed = false;
	
	TestClass *tc = TestClass::SimpleNew();
	ASSERT_ALWAYS(tc)
	tc->destroyed = &destroyed;

	ting::WeakRef<TestClass> wr(tc);
	ASSERT_ALWAYS(!destroyed)

	ting::WeakRef<ting::RefCounted> wrrc(tc);
	ASSERT_ALWAYS(!destroyed)

	wr.Reset();
	ASSERT_ALWAYS(!destroyed)

	wrrc.Reset();
	ASSERT_ALWAYS(!destroyed)

	wr = tc;//operator=()
	ASSERT_ALWAYS(!destroyed)

	//there is 1 weak reference at this point

	ting::Ref<TestClass> sr(tc);
	ASSERT_ALWAYS(!destroyed)

	sr.Reset();
	ASSERT_ALWAYS(destroyed)
	ASSERT_ALWAYS(ting::Ref<TestClass>(wr).IsNotValid())
}

void Run2(){
	bool destroyed = false;

	TestClass *tc = TestClass::SimpleNew();
	ASSERT_ALWAYS(tc)
	tc->destroyed = &destroyed;

	ting::WeakRef<TestClass> wr(tc);
	ASSERT_ALWAYS(!destroyed)

	wr.Reset();
	ASSERT_ALWAYS(!destroyed)

	//no weak references at this point

	ting::Ref<TestClass> sr(tc);
	ASSERT_ALWAYS(!destroyed)

	sr.Reset();
	ASSERT_ALWAYS(destroyed)
	ASSERT_ALWAYS(ting::Ref<TestClass>(wr).IsNotValid())
}

}//~namespace



namespace TestAutomaticDowncasting{
class A : public ting::RefCounted{
public:
	int a;

	A() : a(0)
	{}
};

class B : public A{
public:
	int b;

	B() : b(0)
	{}
};

class C : public B{
public:
	int c;

	C() : c(0)
	{}
};



void Run1(){
	ting::Ref<C> c = ting::NewRefCounted<C>();
	ASSERT_ALWAYS(c)

	const int DConstA = 1;
	const int DConstB = 3;
	const int DConstC = 5;

	c->a = DConstA;
	c->b = DConstB;
	c->c = DConstC;

	//A
	{
		ting::Ref<A> a(c);
		ASSERT_ALWAYS(a.IsValid())
		ASSERT_ALWAYS(a->a == DConstA)

		ASSERT_ALWAYS(a.DynamicCast<B>())
		ASSERT_ALWAYS(a.DynamicCast<B>()->a == DConstA)
		ASSERT_ALWAYS(a.DynamicCast<B>()->b == DConstB)

		ASSERT_ALWAYS(a.DynamicCast<C>())
		ASSERT_ALWAYS(a.DynamicCast<C>()->a == DConstA)
		ASSERT_ALWAYS(a.DynamicCast<C>()->b == DConstB)
		ASSERT_ALWAYS(a.DynamicCast<C>()->c == DConstC)
	}

	{
		ting::Ref<A> a;
		a = c;
		ASSERT_ALWAYS(a.IsValid())
		ASSERT_ALWAYS(a->a == DConstA)

		ASSERT_ALWAYS(a.DynamicCast<B>())
		ASSERT_ALWAYS(a.DynamicCast<B>()->a == DConstA)
		ASSERT_ALWAYS(a.DynamicCast<B>()->b == DConstB)

		ASSERT_ALWAYS(a.DynamicCast<C>())
		ASSERT_ALWAYS(a.DynamicCast<C>()->a == DConstA)
		ASSERT_ALWAYS(a.DynamicCast<C>()->b == DConstB)
		ASSERT_ALWAYS(a.DynamicCast<C>()->c == DConstC)
	}

	//B
	{
		ting::Ref<B> b(c);
		ASSERT_ALWAYS(b.IsValid())
		ASSERT_ALWAYS(b->a == DConstA)
		ASSERT_ALWAYS(b->b == DConstB)

		ASSERT_ALWAYS(b.DynamicCast<C>())
		ASSERT_ALWAYS(b.DynamicCast<C>()->a == DConstA)
		ASSERT_ALWAYS(b.DynamicCast<C>()->b == DConstB)
		ASSERT_ALWAYS(b.DynamicCast<C>()->c == DConstC)
	}

	{
		ting::Ref<B> b;
		b = c;
		ASSERT_ALWAYS(b.IsValid())
		ASSERT_ALWAYS(b->a == DConstA)
		ASSERT_ALWAYS(b->b == DConstB)

		ASSERT_ALWAYS(b.DynamicCast<C>())
		ASSERT_ALWAYS(b.DynamicCast<C>()->a == DConstA)
		ASSERT_ALWAYS(b.DynamicCast<C>()->b == DConstB)
		ASSERT_ALWAYS(b.DynamicCast<C>()->c == DConstC)
	}
}

}//~namespace


namespace TestVirtualInheritedRefCounted{
class A : virtual public ting::RefCounted{
public:
	int a;
};

class B : virtual public ting::RefCounted{
public:
	int b;
};

class C : public A, B{
public:
	int c;

	bool& destroyed;

	C(bool& destroyed) :
			destroyed(destroyed)
	{}

	~C()noexcept{
		this->destroyed = true;
	}
};

void Run1(){
	bool isDestroyed = false;

	ting::Ref<C> p = ting::NewRefCounted<C>(isDestroyed);

	ASSERT_ALWAYS(!isDestroyed)
	ASSERT_ALWAYS(p.IsValid())

	p.Reset();

	ASSERT_ALWAYS(p.IsNotValid())
	ASSERT_ALWAYS(isDestroyed)
}

void Run2(){
	bool isDestroyed = false;

	ting::Ref<A> p = ting::NewRefCounted<C>(isDestroyed);

	ASSERT_ALWAYS(!isDestroyed)
	ASSERT_ALWAYS(p.IsValid())

	p.Reset();

	ASSERT_ALWAYS(p.IsNotValid())
	ASSERT_ALWAYS(isDestroyed)
}

void Run3(){
	bool isDestroyed = false;

	ting::Ref<ting::RefCounted> p = ting::NewRefCounted<C>(isDestroyed);

	ASSERT_ALWAYS(!isDestroyed)
	ASSERT_ALWAYS(p.IsValid())

	p.Reset();

	ASSERT_ALWAYS(p.IsNotValid())
	ASSERT_ALWAYS(isDestroyed)
}
}//~namespace



namespace TestConstantReferences{
class TestClass : public ting::RefCounted{
public:
	int a;

	mutable int b;
};

void Run1(){
	ting::Ref<TestClass> a = ting::NewRefCounted<TestClass>();
	ting::Ref<const TestClass> b(a);

	ASSERT_ALWAYS(a)
	ASSERT_ALWAYS(b)

	a->a = 1234;
	a->b = 425345;
	
	b->b = 2113245;

	{
		ting::WeakRef<TestClass> wa(a);
		ting::WeakRef<const TestClass> wb(a);
		ting::WeakRef<const TestClass> wb1(b);

		ASSERT_ALWAYS(ting::Ref<TestClass>(wa)->a == 1234)
		ASSERT_ALWAYS(ting::Ref<const TestClass>(wb)->a == 1234)
		ASSERT_ALWAYS(ting::Ref<const TestClass>(wb1)->a == 1234)
		ASSERT_ALWAYS(wb1.GetRef()->a == 1234)
	}

	{
		ting::WeakRef<TestClass> wa(a);
		ting::WeakRef<const TestClass> wb(wa);
		const ting::WeakRef<const TestClass>& wc = wa;

		ASSERT_ALWAYS(ting::Ref<const TestClass>(wc)->a == 1234)
	}	
}
}//~namespace



namespace TestOverloadedOperatorDelete{

bool DeleteCalled(bool called){
	static bool deleteCalled = false;
	
	bool ret = deleteCalled;
	
	deleteCalled = called;
	
	return ret;
}

class TestClass : public ting::RefCounted{
	int a;
public:
	
	static void operator delete(void* p){
		bool res = DeleteCalled(true);
		ASSERT_ALWAYS(!res);
		::operator delete(p);
	}
};

void Run(){
	{
		ASSERT_ALWAYS(!DeleteCalled(false))
		ting::Ref<ting::RefCounted> a = ting::NewRefCounted<TestClass>();

		ASSERT_ALWAYS(!DeleteCalled(false))
	}
	
	ASSERT_ALWAYS(DeleteCalled(false))
	ASSERT_ALWAYS(!DeleteCalled(false))
}
}//~namespace
