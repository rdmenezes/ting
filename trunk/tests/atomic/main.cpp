#include "../../src/ting/debug.hpp"
#include "../../src/ting/atomic.hpp"
#include "../../src/ting/Thread.hpp"
#include "../../src/ting/Buffer.hpp"
#include "../../src/ting/Ptr.hpp"


using namespace ting;



namespace TestFetchAndAdd{

const unsigned DNumOps = 0xffff;

class Thread : public ting::Thread{
	ting::atomic::S32 &a;
public:
	Thread(ting::atomic::S32 &a) :
			a(a)
	{}
	
	ting::Semaphore sema;
	
	//override
	void Run(){
//		TRACE(<< "Thread::Run(): enter" << std::endl)
		
		//wait for start signal
		this->sema.Wait();
		
		for(unsigned i = 0; i < DNumOps; ++i){
			this->a.FetchAndAdd(1);
		}
	}
};


void Run(){
	ting::atomic::S32 a;
	
	ting::StaticBuffer<ting::Ptr<Thread>, 100> threads;
	
	//Create and start all the threads
	for(ting::Ptr<Thread>* i = threads.Begin(); i != threads.End(); ++i){
		(*i) = ting::Ptr<Thread>(new Thread(a));
		(*i)->Start();
	}
	
	//wait till all the threads enter their Run() methods and start waiting on the semaphores
	ting::Thread::Sleep(500);
	
	//signal all threads semaphores
//	TRACE(<< "Signalling..." << std::endl)
	for(ting::Ptr<Thread>* i = threads.Begin(); i != threads.End(); ++i){
		(*i)->sema.Signal();
	}
	
	//wait for all threads to finish
	for(ting::Ptr<Thread>* i = threads.Begin(); i != threads.End(); ++i){
		(*i)->Join();
	}
//	TRACE(<< "All threads finished" << std::endl)
	
	//Check atomic value
	ASSERT_ALWAYS(a.FetchAndAdd(0) == ting::s32(DNumOps * threads.Size()))
}
}//~namespace



int main(int argc, char *argv[]){

	TestFetchAndAdd::Run();

	TRACE_ALWAYS(<< "[PASSED]" << std::endl)

	return 0;
}
