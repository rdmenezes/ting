#include "../../src/ting/Buffer.hpp"
#include "../../src/ting/Array.hpp"
#include "../../src/ting/Exc.hpp"

#include "tests.hpp"



namespace BasicArrayTest{

void Run(){
	{
		ting::Array<std::uint8_t> a;
		ASSERT_ALWAYS(a.size() == 0)
		ASSERT_ALWAYS(a.begin() == 0)
		ASSERT_ALWAYS(a.end() == a.begin())
	}

	{
		ting::Array<std::uint8_t> a(143);
		ASSERT_ALWAYS(a.size() == 143)
		ASSERT_ALWAYS(a.begin() != 0)
		ASSERT_ALWAYS(a.end() == a.begin() + a.size())
	}

	//copy constructor
	{
		ting::Array<std::uint8_t> b(143);

		ting::Array<std::uint8_t> a(b);
		ASSERT_ALWAYS(a.size() == 143)
		ASSERT_ALWAYS(a.begin() != 0)
		ASSERT_ALWAYS(a.end() == a.begin() + a.size())
		ASSERT_ALWAYS(b.size() == 0)
		ASSERT_ALWAYS(b.begin() == 0)
		ASSERT_ALWAYS(b.end() == b.begin())
	}

	//operator=
	{
		ting::Array<std::uint8_t> b(143);

		ting::Array<std::uint8_t> a;
		a = b;
		ASSERT_ALWAYS(a.size() == 143)
		ASSERT_ALWAYS(a.begin() != 0)
		ASSERT_ALWAYS(a.end() == a.begin() + a.size())
		ASSERT_ALWAYS(b.size() == 0)
		ASSERT_ALWAYS(b.begin() == 0)
		ASSERT_ALWAYS(b.end() == b.begin())
	}

	//conversion to bool
	{
		ting::Array<std::uint8_t> b(143);
		ASSERT_ALWAYS(b)

		ting::Array<std::uint8_t> a;
		ASSERT_ALWAYS(!a)

		a = b;
		ASSERT_ALWAYS(a)
		ASSERT_ALWAYS(!b)
	}

	// IsValid() / IsNotValid()
	{
		ting::Array<std::uint8_t> b(143);
		ASSERT_ALWAYS(b.IsValid())
		ASSERT_ALWAYS(!b.IsNotValid())

		ting::Array<std::uint8_t> a;
		ASSERT_ALWAYS(!a.IsValid())
		ASSERT_ALWAYS(a.IsNotValid())
	}


	//Init() / Reset()
	{
		ting::Array<std::uint8_t> a;
		
		a.Init(143);
		ASSERT_ALWAYS(a)

		a.Reset();
		ASSERT_ALWAYS(!a)
	}
}

}//~namespace



namespace TestConstructorsAndDestructors{

class TestClass{
public:
	TestClass(){
		if(Inc() == Trig()){
			throw ting::Exc("Test exception");
		}
	}

	~TestClass(){
		Inc();
	}

	static unsigned Trig(unsigned setVal = 0){
		static unsigned value = 1;
		if(setVal == 0){
			return value;
		}else{
			value = setVal;
			return value;
		}
	}

	static unsigned Inc(bool reset = false){
		static unsigned counter = 0;
		if(reset){
			counter = 0;
		}else{
			++counter;
		}
		return counter;
	}
};



void Run(){
	//test that default constructors and destructors are called
	{
		//set the value greater than number of objects we are going to create to
		//avoid triggering of an exception in constructor
		TestClass::Trig(100);

		{
			ASSERT_ALWAYS(TestClass::Inc(true) == 0)
			ting::Array<TestClass> a(12);
			ASSERT_ALWAYS(TestClass::Inc() == 13)
			
			ASSERT_ALWAYS(TestClass::Inc(true) == 0)
		}

		ASSERT_ALWAYS(TestClass::Inc() == 13)
	}

	//same as previous but using Init() / Reset()
	{
		//set the value greater than number of objects we are going to create to
		//avoid triggering of an exception in constructor
		TestClass::Trig(100);

		{
			ting::Array<TestClass> a;
			
			ASSERT_ALWAYS(TestClass::Inc(true) == 0)
			a.Init(12);
			ASSERT_ALWAYS(TestClass::Inc() == 13)

			ASSERT_ALWAYS(TestClass::Inc(true) == 0)
			a.Reset();
			ASSERT_ALWAYS(TestClass::Inc() == 13)

			ASSERT_ALWAYS(TestClass::Inc(true) == 0)
		}

		ASSERT_ALWAYS(TestClass::Inc() == 1)
	}

	//test that destructors are called when exception is thrown by one of the default constructor
	{
		TestClass::Trig(46); //set exception triggering value

		ASSERT_ALWAYS(TestClass::Inc(true) == 0)
		{
			try{
				ting::Array<TestClass> a(134);
				ASSERT_ALWAYS(false)
			}catch(std::exception &){
				//NOTE: because constructors and destructors will be called we need
				//      to multiply by 2, also we need to subtract 1 because for the triggering
				//      object the constructor will be called, but not the destructor.
				ASSERT_INFO_ALWAYS(TestClass::Inc() == ((46 * 2) - 1) + 1, (TestClass::Inc() - 1))
			}catch(...){
				ASSERT_ALWAYS(false)
			}
		}
	}

	//same as previous but using Init()
	{
		TestClass::Trig(46); //set exception triggering value

		ASSERT_ALWAYS(TestClass::Inc(true) == 0)
		{
			try{
				ting::Array<TestClass> a;
				a.Init(134);
				ASSERT_ALWAYS(false)
			}catch(std::exception &){
				//NOTE: because constructors and destructors will be called we need
				//      to multiply by 2, also we need to subtract 1 because for the triggering
				//      object the constructor will be called, but not the destructor.
				ASSERT_INFO_ALWAYS(TestClass::Inc() == ((46 * 2) - 1) + 1, (TestClass::Inc() - 1))
			}catch(...){
				ASSERT_ALWAYS(false)
			}
		}
	}
}

}//~namespace



namespace FromBufferConstructorTest{

class TestClass{
public:
	TestClass(){}

	TestClass(const TestClass& tc){
		if(Inc() == Trig()){
			throw ting::Exc("Test exception");
		}
	}

	~TestClass(){
		Inc();
	}

	static unsigned Trig(unsigned setVal = 0){
		static unsigned value = 1;
		if(setVal == 0){
			return value;
		}else{
			value = setVal;
			return value;
		}
	}

	static unsigned Inc(bool reset = false){
		static unsigned counter = 0;
		if(reset){
			counter = 0;
		}else{
			++counter;
		}
		return counter;
	}
};

void Run(){
	//test that copy constructors and destructors are called
	{
		//set the value greater than number of objects we are going to create to
		//avoid triggering of an exception in constructor
		TestClass::Trig(100);

		{
			std::array<TestClass, 12> buffer;

			{
				ASSERT_ALWAYS(TestClass::Inc(true) == 0)
				ting::Array<TestClass> a(buffer);
				ASSERT_ALWAYS(TestClass::Inc() == 13)
				
				ASSERT_ALWAYS(TestClass::Inc(true) == 0)
			}
			ASSERT_INFO_ALWAYS(TestClass::Inc() == 13, (TestClass::Inc() - 1))

			ASSERT_ALWAYS(TestClass::Inc(true) == 0)
		}

		ASSERT_INFO_ALWAYS(TestClass::Inc() == 13, (TestClass::Inc() - 1))
	}

	//same as previous but using Init() / Reset()
	{
		//set the value greater than number of objects we are going to create to
		//avoid triggering of an exception in constructor
		TestClass::Trig(100);

		{
			std::array<TestClass, 12> buffer;

			ting::Array<TestClass> a;

			ASSERT_ALWAYS(TestClass::Inc(true) == 0)
			a.Init(buffer);
			ASSERT_ALWAYS(TestClass::Inc() == 13)

			ASSERT_ALWAYS(TestClass::Inc(true) == 0)
			a.Reset();
			ASSERT_ALWAYS(TestClass::Inc() == 13)

			ASSERT_ALWAYS(TestClass::Inc(true) == 0)
		}

		ASSERT_ALWAYS(TestClass::Inc() == 13)
	}

	//test that destructors are called when exception is thrown by one of the copy constructor
	{
		std::array<TestClass, 134> buffer;

		TestClass::Trig(46); //set exception triggering value

		ASSERT_ALWAYS(TestClass::Inc(true) == 0)
		{
			try{
				ting::Array<TestClass> a(buffer);
				ASSERT_ALWAYS(false)
			}catch(std::exception &){
				//NOTE: because constructors and destructors will be called we need
				//      to multiply by 2, also we need to subtract 1 because for the triggering
				//      object the constructor will be called, but not the destructor.
				ASSERT_INFO_ALWAYS(TestClass::Inc() == ((46 * 2) - 1) + 1, (TestClass::Inc() - 1))
			}catch(...){
				ASSERT_ALWAYS(false)
			}
		}
	}

	//Same ass previous but using Init()
	{
		std::array<TestClass, 134> buffer;

		TestClass::Trig(46); //set exception triggering value

		ASSERT_ALWAYS(TestClass::Inc(true) == 0)
		{
			try{
				ting::Array<TestClass> a;
				a.Init(buffer);
				ASSERT_ALWAYS(false)
			}catch(std::exception &){
				//NOTE: because constructors and destructors will be called we need
				//      to multiply by 2, also we need to subtract 1 because for the triggering
				//      object the constructor will be called, but not the destructor.
				ASSERT_INFO_ALWAYS(TestClass::Inc() == ((46 * 2) - 1) + 1, (TestClass::Inc() - 1))
			}catch(...){
				ASSERT_ALWAYS(false)
			}
		}
	}
}

}//~namespace



namespace ArrayToConstConversionTest{

class TestClass{
public:
	int a;

	TestClass() :
			a(0)
	{}
};

//this type of argument seems useless, but should work for some cases
int Func(ting::Buffer<const TestClass>& buf){
	if(buf.size() == 0){
		return 0;
	}
	
	return buf[0].a;
}

int Func2(const ting::Buffer<const TestClass>& buf){
	if(buf.size() == 0){
		return 0;
	}
	
	return buf[0].a;
}

//this type of argument seems useless, but should work in some cases
int Func3(ting::Array<const TestClass>& buf){
	if(buf.size() == 0){
		return 0;
	}
	
	return buf[0].a;
}

//this type of argument seems useless, but should work
int Func4(const ting::Array<const TestClass>& buf){
	if(buf.size() == 0){
		return 0;
	}
	
	return buf[0].a;
}

int Func5(ting::Array<const TestClass> buf){
	if(buf.size() == 0){
		return 0;
	}
	
	return buf[0].a;
}

int Func6(const ting::Array<const TestClass> buf){
	if(buf.size() == 0){
		return 0;
	}
	
	return buf[0].a;
}

void Run(){
	{
		ting::Array<const TestClass> buf(20);

		Func(buf);
		Func2(buf);
		Func3(buf);
		Func4(buf);
	}
	
	{
		const ting::Array<const TestClass> buf(20);

//		Func(buf);
		Func2(buf);
//		Func3(buf);
		Func4(buf);
	}
	
	{
		ting::Array<TestClass> buf(20);
		
//		Func(buf);
		Func2(buf);
//		Func3(buf);
		Func4(buf);
	}
	
	{
		const ting::Array<TestClass> buf(20);
		
//		Func(buf);
		Func2(buf);
//		Func3(buf);
		Func4(buf);
	}
	
	{
		ting::Array<TestClass> buf(20);
		
		Func5(buf);
	}
	
	{
		ting::Array<TestClass> buf(20);
		
		Func6(buf);
	}
}

}//~namespace
