#pragma once

#include "../../src/ting/debug.hpp"

#include "tests.hpp"


inline void TestTingPtr(){
	BasicTest::Run();
	TypeCastTest::Run();
	ConstPtrTest::Run();

	TRACE_ALWAYS(<< "[PASSED]: Ptr test" << std::endl)
}
