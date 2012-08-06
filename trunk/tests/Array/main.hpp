#pragma once

#include "../../src/ting/debug.hpp"

#include "tests.hpp"


inline void TingTestArray(){
	BasicArrayTest::Run();
	TestConstructorsAndDestructors::Run();
	FromBufferConstructorTest::Run();
	ArrayToConstConversionTest::Run();
	
	TRACE_ALWAYS(<< "[PASSED]: Array test" << std::endl)
}
