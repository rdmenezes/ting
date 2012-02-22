#pragma once

#include "../../src/ting/debug.hpp"


namespace BasicPoolStoredTest{
void Run();
}



inline void TestTingPoolStored(){
	BasicPoolStoredTest::Run();
	
	TRACE_ALWAYS(<< "[PASSED]: PoolStored test" << std::endl)
}
