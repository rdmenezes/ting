#include <vector>

#include <ting/debug.hpp>
#include <ting/Ref.hpp>


class TestClass : public ting::RefCounted{
public:
	int a;

	TestClass(){}
};


int main(int argc, char *argv[]){
//	TRACE(<< "Ref test" << std::endl)

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



	//test operator !()
	if(!a){
	}else{
		ASSERT_ALWAYS(false)
	}

	if(!b){
		ASSERT_ALWAYS(false)
	}



	TRACE_ALWAYS(<< "[PASSED]: Ref test" << std::endl)

	return 0;
}
