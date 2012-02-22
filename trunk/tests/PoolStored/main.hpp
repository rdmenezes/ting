#pragma once

#include "../../src/ting/debug.hpp"

#include "PoolStored_tests.hpp"


inline void TestTingPoolStored(){
	BasicPoolStoredTest::Run();
	
	TRACE_ALWAYS(<< "[PASSED]: PoolStored test" << std::endl)
}
