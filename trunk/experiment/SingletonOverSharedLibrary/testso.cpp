#include <ting/debug.hpp>
#include "TestSingleton.hpp"



void IncA(){
	++(TestSingleton::Inst().a);
}



void PrintA(){
	TRACE_ALWAYS(<< "PrintA(): a = " << TestSingleton::Inst().a << std::endl)
}
