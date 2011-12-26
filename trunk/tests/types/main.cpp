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


int main(int argc, char *argv[]){

	TestReferenceToInited::Run();
	
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

	//test operator=
	{
		ting::Inited<int, 10> a;
		ASSERT_ALWAYS(a == 10)
		
		ting::Inited<int, 20> b;
		ASSERT_ALWAYS(b == 20)
		
		a = b;
		ASSERT_ALWAYS(a == 20)
		ASSERT_ALWAYS(b == a)
	}
	
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
	}
	
	TRACE_ALWAYS(<< "[PASSED]: types test" << std::endl)

	return 0;
}
