#include "../../src/ting/debug.hpp"
#include "../../src/ting/Ref.hpp"
#include "../../src/ting/Exc.hpp"
#include "../../src/ting/Thread.hpp"
#include "../../src/ting/Ref.hpp"



namespace TestBoolRelatedStuff{

class TestClass : public ting::RefCounted{
public:
	static inline ting::Ref<TestClass> New(){
		return ting::Ref<TestClass>(new TestClass());
	}
};



static void TestConversionToBool(){
	ting::Ref<TestClass> a;
	ting::Ref<TestClass> b = TestClass::New();

	//test conversion to bool
	if(a){
		ASSERT_ALWAYS(false)
	}

	if(b){
	}else{
		ASSERT_ALWAYS(false)
	}
}



static void TestOperatorLogicalNot(){
	ting::Ref<TestClass> a;
	ting::Ref<TestClass> b = TestClass::New();

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

	static inline ting::Ref<A> New(){
		return ting::Ref<A>(new A());
	}
};


static void Run1(){
	ting::Ref<A> a = A::New();
	ting::Ref<const A> ac = A::New();

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

static void Run2(){
	const ting::Ref<A> a = A::New();
	const ting::Ref<const A> ac = A::New();

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

	static inline ting::Ref<TestClass> New(){
		return ting::Ref<TestClass>(new TestClass());
	}

	~TestClass()throw(){
		if(this->destroyed){
			*this->destroyed = true;
		}
	}
};

static void Run1(){
	for(unsigned i = 0; i < 1000; ++i){
		ting::Ref<TestClass> a = TestClass::New();
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

static void Run2(){
	ting::Ref<TestClass> a = TestClass::New();
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

	static inline ting::Ref<TestClass> New(){
		return ting::Ref<TestClass>(new TestClass());
	}
};



static void Run(){
	try{
		ting::Ref<TestClass> a = TestClass::New();
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

	static inline ting::Ref<TestClass> New(){
		return ting::Ref<TestClass>(new TestClass());
	}

	static inline TestClass* SimpleNew(){
		return new TestClass();
	}

	~TestClass()throw(){
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

	static ting::Ref<C> New(){
		return ting::Ref<C>(new C());
	}
};



void Run1(){
	ting::Ref<C> c = C::New();
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

	~C()throw(){
		this->destroyed = true;
	}

	static ting::Ref<C> New(bool& destroyed){
		return ting::Ref<C>(new C(destroyed));
	}
};

void Run1(){
	bool isDestroyed = false;

	ting::Ref<C> p = C::New(isDestroyed);

	ASSERT_ALWAYS(!isDestroyed)
	ASSERT_ALWAYS(p.IsValid())

	p.Reset();

	ASSERT_ALWAYS(p.IsNotValid())
	ASSERT_ALWAYS(isDestroyed)
}

void Run2(){
	bool isDestroyed = false;

	ting::Ref<A> p = C::New(isDestroyed);

	ASSERT_ALWAYS(!isDestroyed)
	ASSERT_ALWAYS(p.IsValid())

	p.Reset();

	ASSERT_ALWAYS(p.IsNotValid())
	ASSERT_ALWAYS(isDestroyed)
}

void Run3(){
	bool isDestroyed = false;

	ting::Ref<ting::RefCounted> p = C::New(isDestroyed);

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

	static inline ting::Ref<TestClass> New(){
		return ting::Ref<TestClass>(new TestClass());
	}
};

void Run1(){
	ting::Ref<TestClass> a = TestClass::New();
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

		ASSERT_ALWAYS(ting::Ref<const TestClass>(wb)->a == 1234)
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
	
	static inline ting::Ref<TestClass> New(){
		return ting::Ref<TestClass>(new TestClass());
	}
	
	static void operator delete(void* p){
		bool res = DeleteCalled(true);
		ASSERT_ALWAYS(!res);
		::operator delete(p);
	}
};

void Run(){
	{
		ASSERT_ALWAYS(!DeleteCalled(false))
		ting::Ref<ting::RefCounted> a = TestClass::New();

		ASSERT_ALWAYS(!DeleteCalled(false))
	}
	
	ASSERT_ALWAYS(DeleteCalled(false))
	ASSERT_ALWAYS(!DeleteCalled(false))
}
}//~namespace



int main(int argc, char *argv[]){
//	TRACE(<< "Ref test" << std::endl)

	TestOperatorArrowAndOperatorStar::Run1();
	TestOperatorArrowAndOperatorStar::Run2();

	TestAutomaticDowncasting::Run1();

	TestBoolRelatedStuff::TestConversionToBool();
	TestBoolRelatedStuff::TestOperatorLogicalNot();
	
	TestBasicWeakRefUseCase::Run1();
	TestBasicWeakRefUseCase::Run2();

	TestExceptionThrowingFromRefCountedDerivedClassConstructor::Run();
	
	TestCreatingWeakRefFromRefCounted::Run1();
	TestCreatingWeakRefFromRefCounted::Run2();
	
	TestVirtualInheritedRefCounted::Run1();
	TestVirtualInheritedRefCounted::Run2();
	TestVirtualInheritedRefCounted::Run3();

	TestConstantReferences::Run1();
	
	TestOverloadedOperatorDelete::Run();

	TRACE_ALWAYS(<< "[PASSED]: Ref test" << std::endl)

	return 0;
}
