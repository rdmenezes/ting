#include "../../src/ting/debug.hpp"
#include "../../src/ting/atomic.hpp"
#include "../../src/ting/mt/Thread.hpp"
#include "../../src/ting/mt/Semaphore.hpp"
#include "../../src/ting/Buffer.hpp"
#include "../../src/ting/Ptr.hpp"

#include "tests.hpp"


using namespace ting;



namespace TestFetchAndAdd{

const unsigned DNumOps = 0xffff;

class Thread : public ting::mt::Thread{
	ting::atomic::S32 &a;
public:
	Thread(ting::atomic::S32 &a) :
			a(a)
	{}
	
	ting::mt::Semaphore sema;
	
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
	for(ting::Ptr<Thread>* i = threads.begin(); i != threads.end(); ++i){
		(*i) = ting::Ptr<Thread>(new Thread(a));
		(*i)->Start();
	}
	
	//wait till all the threads enter their Run() methods and start waiting on the semaphores
	ting::mt::Thread::Sleep(500);
	
	//signal all threads semaphores
//	TRACE(<< "Signalling..." << std::endl)
	for(ting::Ptr<Thread>* i = threads.begin(); i != threads.end(); ++i){
		(*i)->sema.Signal();
	}
	
	//wait for all threads to finish
	for(ting::Ptr<Thread>* i = threads.begin(); i != threads.end(); ++i){
		(*i)->Join();
	}
//	TRACE(<< "All threads finished" << std::endl)
	
	//Check atomic value
	ASSERT_ALWAYS(a.FetchAndAdd(0) == ting::s32(DNumOps * threads.size()))
}
}//~namespace



namespace TestCompareAndExchange{
void Run(){
	ting::atomic::S32 a(10);
	
	ASSERT_ALWAYS(a.CompareAndExchange(10, 9) == 10)
	ASSERT_ALWAYS(a.CompareAndExchange(9, 10) == 9)
	
	
}
}//~namespace



namespace TestFlag{
void Run(){
	ting::atomic::Flag f;
	
	ASSERT_ALWAYS(f.Get() == false)
	
	ASSERT_ALWAYS(f.Set(false) == false)
	ASSERT_ALWAYS(f.Set(true) == false)
	ASSERT_ALWAYS(f.Set(true) == true)
	ASSERT_ALWAYS(f.Set(false) == true)
}
}//~namespace
