#pragma once

#include "../../src/ting/debug.hpp"

#include "tests.hpp"


inline void TestTingTypes(){
	TestBasicInitedStuff::Run();
	TestReferenceToInited::Run();
	TestOperators::Run();
	
	TRACE_ALWAYS(<< "[PASSED]: types test" << std::endl)
}
