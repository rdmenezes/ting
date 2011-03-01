#include <ting/debug.hpp>
#include <ting/Thread.hpp>
#include <ting/Buffer.hpp>




namespace TestJoinBeforeAndAfterThreadHasFinished{

class TestThread : public ting::Thread{
public:
	int a, b;

	//override
	void Run(){
		this->a = 10;
		this->b = 20;
		ting::Thread::Sleep(1000);
		this->a = this->b;
	}
};



static void Run(){

	//Test join after thread has finished
	{
		TestThread t;

		t.Start();

		ting::Thread::Sleep(2000);

		t.Join();
	}



	//Test join before thread has finished
	{
		TestThread t;

		t.Start();

		t.Join();
	}
}

}//~namespace


//====================
//Test many threads
//====================
namespace TestManyThreads{

class TestThread1 : public ting::MsgThread{
public:
	int a, b;

	//override
	void Run(){
		while(!this->quitFlag){
			this->queue.GetMsg()->Handle();
		}
	}
};



static void Run(){
	ting::StaticBuffer<TestThread1, 500> thr;

	for(TestThread1 *i = thr.Begin(); i != thr.End(); ++i){
		i->Start();
	}

	ting::Thread::Sleep(1000);

	for(TestThread1 *i = thr.Begin(); i != thr.End(); ++i){
		i->PushQuitMessage();
		i->Join();
	}
}

}//~namespace



//==========================
//Test immediate thread exit
//==========================

namespace TestImmediateExitThread{

class ImmediateExitThread : public ting::Thread{
public:

	//override
	void Run(){
		return;
	}
};


static void Run(){
	for(unsigned i = 0; i < 100; ++i){
		ImmediateExitThread t;
		t.Start();
		t.Join();
	}
}

}//~namespace



int main(int argc, char *argv[]){
//	TRACE(<< "Thread test" << std::endl)

	TestJoinBeforeAndAfterThreadHasFinished::Run();

	TestManyThreads::Run();

	TestImmediateExitThread::Run();

	TRACE_ALWAYS(<< "[PASSED]: Thread test" << std::endl)

	return 0;
}
