#pragma once

#include "../../src/ting/debug.hpp"
#include "../../src/ting/Timer.hpp"

#include "tests.hpp"


inline void TestTingTimer(){
	ting::timer::Lib timerLib;

	BasicTimerTest::Run();
	SeveralTimersForTheSameInterval::Run();
	StoppingTimers::Run();

	TRACE_ALWAYS(<< "[PASSED]: Timer test" << std::endl)
}
