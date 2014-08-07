//make sure all ting debugging facilities are turned on
#ifndef DEBUG
#define DEBUG
#endif

#include "../../src/ting/debug.hpp"
#include "../../src/ting/Ptr.hpp"

#include "tests.hpp"


namespace TestBasicDebugStuff{
	


bool DoSomethingAndReturnTrueOnSuccess(){
	return true;
}



class TestClass{
public:
	int a;
};



void Run(){
//	TRACE(<< "debug test" << std::endl)

	{
		int a = 13;
		ASSERT(a == 13)
		ASSERT(sizeof(int) == 4)
		ASSERT(sizeof(a) == 4 && sizeof(a) == sizeof(int))
	}

	{
		int a = 13;
		ASSERT_INFO(a == 13, "a is not 13, a is " << a)
	}

	{
		TestClass *c = new TestClass();

		//make sure, "c" is not 0 before accessing member a.
		ASS(c)->a = 13;
		int b = ASS(c)->a;
		ASSERT_ALWAYS(b == 13)
		delete c;
	}

	{
		ting::Ptr<TestClass> pc(new TestClass());

		//make sure, "pc" is valid before accessing member a.
		ASS(pc)->a = 13;
		int b = ASS(pc)->a;
		ASSERT_ALWAYS(b == 13)
	}


	{
		STATIC_ASSERT(sizeof(int) == 4)

		class TestClass{
		public:
			int a;

			//make sure that unsigned char type and signed char type are 8 bit wide.
			STATIC_ASSERT((unsigned char)(-1) == 0xFF)
			STATIC_ASSERT((signed char)(0xFF) == -1)
			STATIC_ASSERT(sizeof(unsigned char) == sizeof(signed char))
		};


		STATIC_ASSERT(sizeof(TestClass) == sizeof(int))
	}
}

}//~namespace
