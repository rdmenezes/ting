#include <vector>

#include "../../src/ting/debug.hpp"
#include "../../src/ting/Ptr.hpp"
#include "../../src/ting/Timer.hpp"



namespace BasicTimerTest{

struct TestTimer1 : public ting::Timer{
	volatile bool *e;

	TestTimer1(volatile bool* exitFlag) :
			e(exitFlag)
	{}

	//override
	void OnExpired()throw(){
		TRACE_ALWAYS(<< "\t- timer1 fired!" << std::endl)
		*this->e = true;
	}
};



struct TestTimer2 : public ting::Timer{
	TestTimer2(){}

	//override
	void OnExpired()throw(){
		TRACE_ALWAYS(<< "\t- timer2 fired!" << std::endl)

		this->Start(2500);
	}
};



void Run(){
	TRACE_ALWAYS(<< "\tRunning BasicTimerTest, it will take about 5 seconds..." << std::endl)

	//should be volatile since can be changed by another threads. This is to avoid the checks of this variable to be optimized away by compiler.
	volatile bool exit = false;

	TestTimer1 timer1(&exit);
	TestTimer2 timer2;

	timer1.Start(5000);
	timer2.Start(2500);

//	TRACE_ALWAYS(<< "loop " << std::endl)
	
	for(unsigned i = 0; !exit; ++i){
		ting::Thread::Sleep(100);
		ASSERT_ALWAYS(i != 60)
	}

	ting::Thread::Sleep(50);
	
	while(!timer2.Stop()){
		ting::Thread::Sleep(50);
	}
}

}//~namespace



namespace SeveralTimersForTheSameInterval{

struct TestTimer : public ting::Timer{
	unsigned *e;
	ting::Mutex* m;

	TestTimer(unsigned* cnt, ting::Mutex* mut) :
			e(cnt),
			m(mut)
	{
		ASSERT_ALWAYS(this->m)
		ASSERT_ALWAYS(this->e)
	}

	//override
	void OnExpired()throw(){
//		TRACE_ALWAYS(<<"\t- timer fired!"<<std::endl)
		ting::Mutex::Guard mutexGuard(*this->m);
		++(*this->e);
	}
};



void Run(){
	TRACE_ALWAYS(<< "\tRunning SeveralTimersForTheSameInterval, it will take about 1 second..." << std::endl)
	
	ting::Mutex mutex;
	unsigned counter = 0;
	
	const unsigned DNumTimers = 100;
	
	typedef std::vector<ting::Ptr<TestTimer> > T_TimerList;
	typedef T_TimerList::iterator T_TimerIter;
	T_TimerList timers;
	
	for(unsigned i = 0; i != DNumTimers; ++i){
		timers.push_back(ting::Ptr<TestTimer>(
				new TestTimer(&counter, &mutex)
			));
	}
	
	for(T_TimerIter i = timers.begin(); i != timers.end(); ++i){
		(*i)->Start(500);
	}
	
	ting::Thread::Sleep(1000);
	
	ASSERT_ALWAYS(counter == DNumTimers)
}

}//~namespace



namespace StoppingTimers{

struct TestTimer : public ting::Timer{
	bool *e;

	TestTimer(bool* exitFlag) :
			e(exitFlag)
	{}

	//override
	void OnExpired()throw(){
//		TRACE_ALWAYS(<<"\t- timer1 fired!"<<std::endl)
		*this->e = true;
	}
};



void Run(){
	TRACE_ALWAYS(<< "\tRunning StoppingTimers, it will take about 3 seconds..." << std::endl)
	
	bool exit1 = false;
	bool exit2 = false;
	
	
	TestTimer timer1(&exit1);
	TestTimer timer2(&exit2);
	
	timer1.Start(3000);
	ASSERT_ALWAYS(!exit1)
	ASSERT_ALWAYS(!exit2)
	
	ting::Thread::Sleep(1000);
	ASSERT_ALWAYS(!exit1)
	ASSERT_ALWAYS(!exit2)
	
	timer2.Start(1000);
	
	ting::Thread::Sleep(500);
	ASSERT_ALWAYS(!exit1)
	ASSERT_ALWAYS(!exit2)
	
	bool stopRes = timer2.Stop();
	ASSERT_ALWAYS(stopRes)
	ASSERT_ALWAYS(!exit1)
	ASSERT_ALWAYS(!exit2)
	
	ting::Thread::Sleep(1000);
	ASSERT_ALWAYS(!exit1)
	ASSERT_ALWAYS(!exit2)
	
	ting::Thread::Sleep(1000);
	ASSERT_ALWAYS(exit1)
	ASSERT_ALWAYS(!exit2)
}

}//~namespace



int main(int argc, char *argv[]){
//	TRACE_ALWAYS(<< "Timer test " << std::endl)
	ting::TimerLib timerLib;

	BasicTimerTest::Run();
	SeveralTimersForTheSameInterval::Run();
	StoppingTimers::Run();

	TRACE_ALWAYS(<< "[PASSED]: Timer test" << std::endl)

	return 0;
}
