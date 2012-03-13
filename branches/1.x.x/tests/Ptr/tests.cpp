#include "../../src/ting/Ptr.hpp"

#include "tests.hpp"



namespace BasicTest{

class TestClass{
	bool *destroyed;
public:

	TestClass(bool* destroyed) :
			destroyed(destroyed)
	{
		ASSERT_ALWAYS(this->destroyed)
	}

	~TestClass(){
		*this->destroyed = true;
	}
};

void Run(){
	bool destroyed = false;
	
	{
		ting::Ptr<TestClass> p1(new TestClass(&destroyed));
		ting::Ptr<TestClass> p2;

		ASSERT_ALWAYS(p1.IsValid())
		ASSERT_ALWAYS(p2.IsNotValid())

		ASSERT_ALWAYS(!destroyed)

		p2 = p1;

		ASSERT_ALWAYS(p2.IsValid())
		ASSERT_ALWAYS(p1.IsNotValid())

		ASSERT_ALWAYS(!destroyed)
	}
	ASSERT_ALWAYS(destroyed)
}

}//~namespace



namespace TypeCastTest{

class Base{
	int b;
public:

	virtual void BaseFunc(){}

	virtual ~Base(){}
};



class Interface{
	int i;
public:

	virtual void InterfaceFunc(){}

	virtual ~Interface(){}
};



class TestClass : public Base, public Interface{
	int t;
public:
	
};



void Run(){
	//conversion constructor
	{
		ting::Ptr<TestClass> t(new TestClass);
		ASSERT_ALWAYS(t.IsValid())

	//	TRACE(<< "t = " << t.operator->() << " i = " << static_cast<Interface*>(t.operator->()) << std::endl)

		TestClass *tc = t.operator->();

		ting::Ptr<Interface> i(t);

		ASSERT_ALWAYS(t.IsNotValid())
		ASSERT_ALWAYS(i.IsValid())

		ASSERT_INFO_ALWAYS(tc == static_cast<TestClass*>(i.operator->()), "tc = " << tc << " static_cast<TestClass*>(i.operator->()) = " << static_cast<TestClass*>(i.operator->()))
		ASSERT_ALWAYS(tc == i.StaticCast<TestClass>())
	}

	//conversion operator=()
	{
		ting::Ptr<TestClass> t(new TestClass);
		ASSERT_ALWAYS(t.IsValid())

	//	TRACE(<< "t = " << t.operator->() << " i = " << static_cast<Interface*>(t.operator->()) << std::endl)

		TestClass *tc = t.operator->();

		ting::Ptr<Interface> i;
		i = t;

		ASSERT_ALWAYS(t.IsNotValid())
		ASSERT_ALWAYS(i.IsValid())

		ASSERT_INFO_ALWAYS(tc == static_cast<TestClass*>(i.operator->()), "tc = " << tc << " static_cast<TestClass*>(i.operator->()) = " << static_cast<TestClass*>(i.operator->()))
		ASSERT_ALWAYS(tc == i.StaticCast<TestClass>())
	}

}

}//~namespace



namespace ConstPtrTest{

class TestClass{
public:
	int a;

};

void Run(){
	{
		const ting::Ptr<TestClass> obj(new TestClass());

		obj->a = 10;
		
		(*obj).a = 20;

		//not allowed, since pointer is constant
//		obj.Reset();
	}
	
	{
		ting::Ptr<const TestClass> obj(new TestClass());

		//not allowed, since pointer points to constant data
//		obj->a = 10;
//		(*obj).a = 20;
		
		obj.Reset();
	}
}

}//~namespace
