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

	~TestClass(){
		if(this->destroyed){
			*this->destroyed = true;
		}
	}
};



static void TestConversionToBool(){
	ting::Ref<TestClass> a;
	ting::Ref<TestClass> b(new TestClass());

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
	ting::Ref<TestClass> b(new TestClass());

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
		ting::Ref<TestClass> a(new TestClass());
		ASSERT_ALWAYS(a.IsValid())

		bool wasDestroyed = false;
		a->destroyed = &wasDestroyed;

		ting::WeakRef<TestClass> wr(a);
		ASSERT_ALWAYS(ting::Ref<TestClass>(wr).IsValid())
		ASSERT_ALWAYS(!wasDestroyed)

		a.Reset();

		ASSERT_ALWAYS(a.IsNotValid())
		ASSERT_ALWAYS(wasDestroyed)
		ASSERT_ALWAYS(ting::Ref<TestClass>(wr).IsNotValid())
	}//~for
}
}//~namespace



namespace TestExceptionThrowingFromRefCountedDerivedClassConstructor{

class TestClass : public ting::RefCounted{
public:
	TestClass(){
		throw ting::Exc("TestException!");
	}
};



static void Run(){
	try{
		ting::Ref<TestClass> a(new TestClass());
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
	~TestClass(){
		if(this->destroyed)
			*this->destroyed = true;
	}
};



void Run1(){
//	TRACE(<< "TestCreatingWeakRefFromRefCounted::Run(): enter" << std::endl)
	
	bool destroyed = false;
	
	TestClass *tc = new TestClass();
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

	TestClass *tc = new TestClass();
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



int main(int argc, char *argv[]){
//	TRACE(<< "Ref test" << std::endl)

	TestConversionToBool();
	TestOperatorLogicalNot();
	TestBasicWeakRefUseCase::Run();
	TestExceptionThrowingFromRefCountedDerivedClassConstructor::Run();
	TestCreatingWeakRefFromRefCounted::Run1();
	TestCreatingWeakRefFromRefCounted::Run2();

	//TODO: add more test cases

	TRACE_ALWAYS(<< "[PASSED]: Ref test" << std::endl)

	return 0;
}
