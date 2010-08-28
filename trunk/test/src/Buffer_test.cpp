#include <ting/Buffer.hpp>



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



int main(int argc, char *argv[]){
//	TRACE_ALWAYS(<<"Buffer test "<<std::endl)

	TestStaticBufferCopyConstructor::Run();


	TRACE_ALWAYS(<<"[PASSED]: Buffer test"<<std::endl)

	return 0;
}
