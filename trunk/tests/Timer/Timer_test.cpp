#include <vector>

#include "../../src/ting/debug.hpp"
#include "../../src/ting/Timer.hpp"


struct TestTimer1 : public ting::Timer{
    bool *e;

    TestTimer1(bool* exitFlag) :
            e(exitFlag)
    {}

    //override
    void OnExpired(){
        TRACE_ALWAYS(<<"\t- timer1 fired!"<<std::endl)
        *this->e = true;
    }
};

struct TestTimer2 : public ting::Timer{
    TestTimer2(){}

    //override
    void OnExpired(){
        TRACE_ALWAYS(<<"\t- timer2 fired!"<<std::endl)

        this->Start(2500);
    }
};



int main(int argc, char *argv[]){
//	TRACE_ALWAYS(<<"Timer test "<<std::endl)
	ting::TimerLib timerLib;

	bool exit = false;

    TestTimer1 timer1(&exit);
    TestTimer2 timer2;
    
	timer1.Start(5000);
	timer2.Start(2500);

//	TRACE_ALWAYS(<< "loop " << std::endl)
	while(!exit){}

    ting::Thread::Sleep(50);
    
	TRACE_ALWAYS(<<"[PASSED]: Timer test"<<std::endl)

	return 0;
}
