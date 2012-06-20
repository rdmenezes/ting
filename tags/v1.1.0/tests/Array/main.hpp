#pragma once

#include "../../src/ting/debug.hpp"

#include "Array_tests.hpp"


inline void TingTestArray(){
	BasicArrayTest::Run();
	TestConstructorsAndDestructors::Run();
	FromBufferConstructorTest::Run();

	TRACE_ALWAYS(<< "[PASSED]: Array test" << std::endl)
}
