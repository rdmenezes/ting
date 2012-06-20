#pragma once

#include "../../src/ting/debug.hpp"

#include "tests.hpp"


inline void TestTingBuffer(){
	TestStaticBufferCopyConstructor::Run();
	TestStaticBufferOperatorEquals::Run();

	TRACE_ALWAYS(<<"[PASSED]"<<std::endl)
}
