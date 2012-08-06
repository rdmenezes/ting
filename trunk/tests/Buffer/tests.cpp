#include "../../src/ting/Buffer.hpp"

#include "tests.hpp"


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

void Run(){
	typedef ting::StaticBuffer<TestClass, 20> T_TestStaticBuffer;
	T_TestStaticBuffer arr;
	T_TestStaticBuffer brr(arr);

	ASSERT_ALWAYS(arr.Size() == brr.Size())
	for(size_t i = 0; i < arr.Size(); ++i){
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

void Run(){
	typedef ting::StaticBuffer<TestClass, 20> T_TestStaticBuffer;
	T_TestStaticBuffer arr;

	for(size_t i = 0; i < arr.Size(); ++i){
		arr[i].id = i;
	}

	T_TestStaticBuffer brr;
	TestClass* oldArrBegin = arr.Begin();
	TestClass* oldBrrBegin = brr.Begin();
	ASSERT_ALWAYS(brr.Begin() != arr.Begin())
	
	brr = arr;
	
	ASSERT_ALWAYS(brr.Begin() != arr.Begin())
	ASSERT_ALWAYS(arr.Begin() == oldArrBegin)
	ASSERT_ALWAYS(brr.Begin() == oldBrrBegin)

	ASSERT_ALWAYS(arr.Size() == brr.Size())
	for(size_t i = 0; i < arr.Size(); ++i){
		ASSERT_ALWAYS(arr[i].a == 0)
		ASSERT_INFO_ALWAYS(brr[i].a == 0, "brr[i].a = " << brr[i].a)
		ASSERT_INFO_ALWAYS(arr[i].id == brr[i].id, "arr[i].id = " << arr[i].id << " brr[i].id = " << brr[i].id)
		ASSERT_ALWAYS(arr[i].id == i)
	}
}

}//~namespace



namespace TestBufferConstCast{

class TestClass{
public:
	int a;

	TestClass() :
			a(0)
	{}
};

int Func(ting::Buffer<const TestClass>& buf){
	if(buf.Size() == 0){
		return 0;
	}
	
	return buf[0].a;
}

int Func2(const ting::Buffer<const TestClass>& buf){
	if(buf.Size() == 0){
		return 0;
	}
	
	return buf[0].a;
}

void Run(){
	{
		ting::StaticBuffer<const TestClass, 20> buf;

		Func(buf);
		Func2(buf);
	}
	
	{
		ting::StaticBuffer<TestClass, 20> buf;
		
//		Func(buf);
		Func2(buf);
	}
	
//	Func(ting::Buffer<TestClass>(0, 0));
	Func2(ting::Buffer<const TestClass>(0, 0));
}

}//~namespace
