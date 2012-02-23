#include "../../src/ting/debug.hpp"

#include "tests.hpp"


inline void TestTingAtomic(){
	TestFlag::Run();
	TestCompareAndExchange::Run();
	TestFetchAndAdd::Run();

	TRACE_ALWAYS(<< "[PASSED]" << std::endl)
}
