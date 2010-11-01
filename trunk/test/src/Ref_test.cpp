#include <ting/debug.hpp>
#include <ting/Ref.hpp>
#include <ting/Exc.hpp>
#include <ting/Thread.hpp>



class TestClass : public ting::RefCounted{
	int array[2048];
public:
	int a;

	bool *destroyed;

	TestClass() :
		destroyed(0)
	{}

	static inline ting::Ref<TestClass> New(){
		return ting::Ref<TestClass>(new TestClass());
	}

	~TestClass(){
		if(this->destroyed){
			*this->destroyed = true;
		}
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



namespace TestBasicWeakRefUseCase{
static void Run(){
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
	}catch(ting::Exc& e){
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
	{
	}

	static inline ting::Ref<TestClass> New(){
		return ting::Ref<TestClass>(new TestClass());
	}

	static inline TestClass* SimpleNew(){
		return new TestClass();
	}

	~TestClass(){
		if(this->destroyed)
			*this->destroyed = true;
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
		
		~C(){
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
}


int main(int argc, char *argv[]){
//	TRACE(<< "Ref test" << std::endl)

	TestConversionToBool();
	
	TestOperatorLogicalNot();
	
	TestBasicWeakRefUseCase::Run();
	
	TestExceptionThrowingFromRefCountedDerivedClassConstructor::Run();
	
	TestCreatingWeakRefFromRefCounted::Run1();
	TestCreatingWeakRefFromRefCounted::Run2();
	
	TestVirtualInheritedRefCounted::Run1();
	TestVirtualInheritedRefCounted::Run2();
	TestVirtualInheritedRefCounted::Run3();

	//TODO: add more test cases

	TRACE_ALWAYS(<< "[PASSED]: Ref test" << std::endl)

	return 0;
}
