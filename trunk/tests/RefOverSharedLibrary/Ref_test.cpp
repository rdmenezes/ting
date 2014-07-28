#include "../../src/ting/debug.hpp"

#include "TestClass.hpp"
#include "testso.hpp"


int main(int argc, char *argv[]){
	TRACE_ALWAYS(<< "Started" << std::endl)

	bool destroyed = false;

	ting::Ref<TestClass> tc = ting::New<TestClass>(&destroyed);

	DoSmth(tc);

	ASSERT_ALWAYS(!tc)
	ASSERT_ALWAYS(destroyed)

	TRACE_ALWAYS(<< "[PASSED]: Ref over shared library test" << std::endl)

	return 0;
}
