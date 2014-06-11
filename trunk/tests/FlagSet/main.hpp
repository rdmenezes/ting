#pragma once

#include "../../src/ting/debug.hpp"

#include "tests.hpp"



inline void TestTingFlagSet(){
	TestFlagSet::Run();

	TRACE_ALWAYS(<<"[PASSED]: FlagSet test"<<std::endl)
}
