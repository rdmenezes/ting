#include <ting/debug.hpp>

#include "TestClass.hpp"
#include "testso.hpp"


int main(int argc, char *argv[]){
//	TRACE_ALWAYS(<< "Started" << std::endl)

	bool destroyed = false;

	DoSmth(TestClass::New(&destroyed));

	ASSERT_ALWAYS(destroyed)

	TRACE_ALWAYS(<< "[PASSED]: Ref over shared library test" << std::endl)

	return 0;
}
