#include <ting/debug.hpp>
#include <ting/Ref.hpp>



class TestClass : public ting::RefCounted{
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



static void TestBasicWeakRefUseCase(){
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
}



int main(int argc, char *argv[]){
//	TRACE(<< "Ref test" << std::endl)

	TestConversionToBool();
	TestOperatorLogicalNot();
	TestBasicWeakRefUseCase();

	//TODO: add more test cases

	TRACE_ALWAYS(<< "[PASSED]: Ref test" << std::endl)

	return 0;
}
