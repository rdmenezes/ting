#include <ting/debug.hpp>

#include "TestSingleton.hpp"


void IncA();
void PrintA();


int main(int argc, char *argv[]){
	TRACE_ALWAYS(<< "Started" << std::endl)

	TestSingleton ts;
	
	TRACE_ALWAYS(<< "32 a = " << TestSingleton::Inst().a << std::endl)
	
	++TestSingleton::Inst().a;
	
	TRACE_ALWAYS(<< "33 a = " << TestSingleton::Inst().a << std::endl)
	
	IncA();
	
	TRACE_ALWAYS(<< "34 a = " << TestSingleton::Inst().a << std::endl)
	
	TestSingleton::Inst().a = 101;
	
	TRACE_ALWAYS(<< "expect 101" << std::endl)
	PrintA();
	
	return 0;
}
