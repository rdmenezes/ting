#pragma once

#include "../../src/ting/debug.hpp"

#include "tests.hpp"



inline void TestTingSignal(){
	FuncConnectionTest::Run();
	MethodConnectionTest::Run();
	WeakRefConnectionTest::Run();
	MixedConnectionTest::Run();

	TRACE_ALWAYS(<< "[PASSED]: Signal test" << std::endl)
}
