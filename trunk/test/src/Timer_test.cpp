#include <vector>

#include <ting/debug.hpp>
#include <ting/Timer.hpp>

int main(int argc, char *argv[]){
//	TRACE_ALWAYS(<<"Timer test "<<std::endl)
	ting::TimerLib timerLib;

	bool exit = false;
	
	struct TestTimer1 : public ting::Timer{
		bool *e;

		TestTimer1(bool* exitFlag) :
				e(exitFlag)
		{}

		//override
		ting::u32 OnExpire(){
			TRACE_ALWAYS(<<"\t- timer1 fired!"<<std::endl)
			*this->e = true;
			return 0;
		}
	} timer1(&exit);

	struct TestTimer2 : public ting::Timer{
		//override
		ting::u32 OnExpire(){
			TRACE_ALWAYS(<<"\t- timer2 fired!"<<std::endl)
			return 1000;
		}
	} timer2;

	timer1.Start(5000);
	timer2.Start(2500);

//	TRACE_ALWAYS(<< "loop " << std::endl)
	while(!exit){}

	TRACE_ALWAYS(<<"[PASSED]: Timer test"<<std::endl)

	return 0;
}
