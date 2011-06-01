#include "../../src/ting/debug.hpp"
#include "../../src/ting/Thread.hpp"
#include "../../src/ting/Buffer.hpp"
#include "../../src/ting/types.hpp"




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
	//TODO: read ulimit
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



namespace TestNestedJoin{

class TestRunnerThread : public ting::Thread{
public:
	class TopLevelThread : public ting::Thread{
	public:

		class InnerLevelThread : public ting::Thread{
		public:

			//overrun
			void Run(){
			}
		} inner;

		//override
		void Run(){
			this->inner.Start();
			ting::Thread::Sleep(100);
			this->inner.Join();
		}
	} top;

	volatile bool success;

	TestRunnerThread() :
			success(false)
	{}

	//override
	void Run(){
		this->top.Start();
		this->top.Join();
		this->success = true;
	}
};



static void Run(){
	TestRunnerThread runner;
	runner.Start();

	ting::Thread::Sleep(1000);

	ASSERT_ALWAYS(runner.success)

	runner.Join();
}


}//~namespace



int main(int argc, char *argv[]){
//	TRACE(<< "Thread test" << std::endl)

//	TRACE(<< "running TestManyThreads..." << std::endl)
	TestManyThreads::Run();

//	TRACE(<< "running TestJoinBeforeAndAfterThreadHasFinished" << std::endl)
	TestJoinBeforeAndAfterThreadHasFinished::Run();

//	TRACE(<< "running TestImmediateExitThread" << std::endl)
	TestImmediateExitThread::Run();

//	TRACE(<< "running TestNestedJoin" << std::endl)
	TestNestedJoin::Run();

	TRACE_ALWAYS(<< "[PASSED]: Thread test" << std::endl)

	return 0;
}
