#include "../../src/ting/types.hpp"



namespace TestReferenceToInited{

int Func(int& a){
	int v = 126;
	a = v;
	return v;
}

int FuncConst(const int& a){
	int v = 126;
	return v + a;
}

void Run(){
	{
		ting::Inited<int, 10> a;

		{
			const int& b = a;
			ASSERT_ALWAYS(b == a) //conversion to type
			ASSERT_ALWAYS(a == b) //operator==()
		}
		{
			int& b = a;
			ASSERT_ALWAYS(b == a) //conversion to type
			ASSERT_ALWAYS(a == b) //operator==()
		}
		{
			const int b = a;
			ASSERT_ALWAYS(b == a) //conversion to type
			ASSERT_ALWAYS(a == b) //operator==()
		}
		{
			int b = a;
			ASSERT_ALWAYS(b == a) //conversion to type
			ASSERT_ALWAYS(a == b) //operator==()
		}
		
		int res = Func(a);

		ASSERT_ALWAYS(a == res)
	}
	
	{
		ting::Inited<const int, 10> a;

		{
			const int& b = a;
			ASSERT_ALWAYS(b == a) //conversion to type
			ASSERT_ALWAYS(a == b) //operator==()
		}
		{
			int b = a;
			ASSERT_ALWAYS(b == a) //conversion to type
			ASSERT_ALWAYS(a == b) //operator==()
		}
		
		int res = FuncConst(a);

		ASSERT_ALWAYS(a + 126 == res)
	}
	
	{
		const ting::Inited<int, 10> a;

		{
			const int& b = a;
			ASSERT_ALWAYS(b == a) //conversion to type
			ASSERT_ALWAYS(a == b) //operator==()
		}
		{
			int b = a;
			ASSERT_ALWAYS(b == a) //conversion to type
			ASSERT_ALWAYS(a == b) //operator==()
		}
		
		int res = FuncConst(a);

		ASSERT_ALWAYS(a + 126 == res)
	}
	
	{
		const ting::Inited<const int, 10> a;

		{
			const int& b = a;
			ASSERT_ALWAYS(b == a) //conversion to type
			ASSERT_ALWAYS(a == b) //operator==()
		}
		{
			int b = a;
			ASSERT_ALWAYS(b == a) //conversion to type
			ASSERT_ALWAYS(a == b) //operator==()
		}
		
		int res = FuncConst(a);

		ASSERT_ALWAYS(a + 126 == res)
	}
}
}//~namespace



namespace TestOperators{
struct TestClass{
	int g;

	TestClass() :
			g(14)
	{}
};

void Run(){
	//operator=(Inited)
	{
		ting::Inited<int, 10> a;
		ASSERT_ALWAYS(a == 10)
		
		ting::Inited<int, 20> b;
		ASSERT_ALWAYS(b == 20)
		
		a = b;
		ASSERT_ALWAYS(a == 20)
		ASSERT_ALWAYS(b == a)
	}
	
	//opertor=() with volatile value.
	//test Inited<volatile bool, false> assignment.
	//GCC throws a warning when a function returning a reference to volatile is called in a void context
	//If Inited::operator= returns just T& then we get that warning. Thus, made it to return Inited&
	{
		ting::Inited<volatile bool, false> v;
		ASSERT_ALWAYS(v == false)
		
		bool value = true;
		
		v = value;
		ASSERT_ALWAYS(v == true)
		
		value = false;
		
		bool anotherValue = true;
		
		(v = value) = anotherValue;
		ASSERT_ALWAYS(v == true)
		
		v = false;
		ASSERT_ALWAYS(v == false)
		
		ting::Inited<volatile bool, true> v2;
		
		v = v2;
		ASSERT_ALWAYS(v == true)
		
		(v = value) = v2;
		ASSERT_ALWAYS(v == true)
		
		ting::Inited<bool, false> v3;
		
		
		v = v3;
		ASSERT_ALWAYS(v == false)
		
		(v = value) = v3;
		ASSERT_ALWAYS(v == false)
	}
	
	//operator->()const
	{
		const ting::Inited<TestClass*, 0> p;
		
		p = new TestClass();
		
		ASSERT_ALWAYS(p->g == 14)
		p->g = 15;
		ASSERT_ALWAYS(p->g == 15)
		
		delete p;
	}
	
	//operator->()
	{
		ting::Inited<const TestClass*, 0> p;
		
		p = new TestClass();
		
		ASSERT_ALWAYS(p->g == 14)
		
		delete p;
	}
	
	//operator+=
	{
		ting::Inited<int, 10> a;
		
		a += 13;
		ASSERT_ALWAYS(a == 23)
		
		const ting::Inited<int, 20> a1;
		
		a1 += 13;
		ASSERT_ALWAYS(a1 == 33)
		
		const ting::Inited<const int, 30> a2;
		
		ASSERT_ALWAYS(a2 == 30)
	}
	
	//operator-=
	{
		ting::Inited<int, 10> a;
		
		a -= 13;
		ASSERT_ALWAYS(a == -3)
		
		const ting::Inited<int, 20> a1;
		
		a1 -= 13;
		ASSERT_ALWAYS(a1 == 7)
	}
	
	//operator*=
	{
		ting::Inited<int, 10> a;
		
		a *= 3;
		ASSERT_ALWAYS(a == 30)
		
		const ting::Inited<int, 20> a1;
		
		a1 *= 4;
		ASSERT_ALWAYS(a1 == 80)
	}
	
	//operator++()
	{
		ting::Inited<int, 10> a;
		
		++a;
		ASSERT_ALWAYS(a == 11)
		int ares = ++a;
		ASSERT_ALWAYS(ares == 12)
		++a = 35;
		ASSERT_ALWAYS(a == 35)
				
		const ting::Inited<int, 20> a1;
		
		++a1;
		ASSERT_ALWAYS(a1 == 21)
		int a1res = ++a1;
		ASSERT_ALWAYS(a1res == 22)
	}
	
	//operator++(int)
	{
		ting::Inited<int, 10> a;
		
		a++;
		ASSERT_ALWAYS(a == 11)
		int ares = a++;
		ASSERT_ALWAYS(ares == 11)
		ASSERT_ALWAYS(a == 12)
				
		const ting::Inited<int, 20> a1;
		
		a1++;
		ASSERT_ALWAYS(a1 == 21)
		int a1res = a1++;
		ASSERT_ALWAYS(a1res == 21)
		ASSERT_ALWAYS(a1 == 22)
	}
	
	//operator&
	{
		ting::Inited<int, 10> a;
		
		int* ap = &a;
		
		ASSERT_ALWAYS(*ap == 10)
	}
	
	//operator&()const
	{
		const ting::Inited<int, 10> a;
		
		int* ap = &a;
		
		ASSERT_ALWAYS(*ap == 10)
	}
}
}//~namespace



int main(int argc, char *argv[]){
	
	TestReferenceToInited::Run();
	TestOperators::Run();
	
	{
		ting::Inited<int, -10> a;
		ASSERT_ALWAYS(a == -10)

		a = 30;
		ASSERT_ALWAYS(a == 30)

		++a;
		ASSERT_ALWAYS(a == 31)

		--a;
		ASSERT_ALWAYS(a == 30)

		ASSERT_ALWAYS(a++ == 30)
		ASSERT_ALWAYS(a == 31)

		ASSERT_ALWAYS(a-- == 31)
		ASSERT_ALWAYS(a == 30)

		ting::Inited<int, -10> b;
		a = b;
		ASSERT_ALWAYS(a == -10)
		ASSERT_ALWAYS(b == -10)
		ASSERT_ALWAYS(a == b)
	}

	{
		ting::Inited<float, -10> a;
		ASSERT_ALWAYS(a == -10)

		a = 30;
		ASSERT_ALWAYS(a == 30)

		++a;
		ASSERT_ALWAYS(a == 31)

		--a;
		ASSERT_ALWAYS(a == 30)

		ASSERT_ALWAYS(a++ == 30)
		ASSERT_ALWAYS(a == 31)

		ASSERT_ALWAYS(a-- == 31)
		ASSERT_ALWAYS(a == 30)

		ting::Inited<float, -10> b;
		a = b;
		ASSERT_ALWAYS(a == -10)
		ASSERT_ALWAYS(b == -10)
		ASSERT_ALWAYS(a == b)
	}
	
	TRACE_ALWAYS(<< "[PASSED]: types test" << std::endl)

	return 0;
}
