#include "../../../src/ting/Buffer.hpp"



namespace TestStaticBufferCopyConstructor{

class TestClass{
public:
	int a;

	TestClass() :
			a(0)
	{}

	TestClass(const TestClass& c){
		this->a = c.a + 1;
	}

	TestClass& operator=(const TestClass& tc){
		return *this;
	}
};

static void Run(){
	typedef ting::StaticBuffer<TestClass, 20> T_TestStaticBuffer;
	T_TestStaticBuffer arr;
	T_TestStaticBuffer brr(arr);

	ASSERT_ALWAYS(arr.Size() == brr.Size())
	for(unsigned i = 0; i < arr.Size(); ++i){
		ASSERT_ALWAYS(arr[i].a == 0)
		ASSERT_ALWAYS(brr[i].a == 1)
	}
}

}//~namespace



namespace TestStaticBufferOperatorEquals{

class TestClass{
public:
	unsigned id;
	int a;

	TestClass() :
			a(0)
	{}

	TestClass(const TestClass& tc){
		this->a = tc.a + 1;
	}
};

static void Run(){
	typedef ting::StaticBuffer<TestClass, 20> T_TestStaticBuffer;
	T_TestStaticBuffer arr;

	for(unsigned i = 0; i < arr.Size(); ++i){
		arr[i].id = i;
	}

	T_TestStaticBuffer brr;
	brr = arr;

	ASSERT_ALWAYS(arr.Size() == brr.Size())
	for(unsigned i = 0; i < arr.Size(); ++i){
		ASSERT_ALWAYS(arr[i].a == 0)
		ASSERT_INFO_ALWAYS(brr[i].a == 0, "brr[i].a = " << brr[i].a)
		ASSERT_INFO_ALWAYS(arr[i].id == brr[i].id, "arr[i].id = " << arr[i].id << " brr[i].id = " << brr[i].id)
		ASSERT_ALWAYS(arr[i].id == i)
	}
}

}//~namespace



int main(int argc, char *argv[]){
//	TRACE_ALWAYS(<<"Buffer test "<<std::endl)

	TestStaticBufferCopyConstructor::Run();
	TestStaticBufferOperatorEquals::Run();


	TRACE_ALWAYS(<<"[PASSED]: Buffer test"<<std::endl)

	return 0;
}
