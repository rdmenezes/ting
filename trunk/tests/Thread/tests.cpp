#include "../../src/ting/debug.hpp"
#include "../../src/ting/Thread.hpp"
#include "../../src/ting/Buffer.hpp"
#include "../../src/ting/types.hpp"

#include "tests.hpp"



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



void Run(){

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



void Run(){
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


void Run(){
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



void Run(){
	TestRunnerThread runner;
	runner.Start();

	ting::Thread::Sleep(1000);

	ASSERT_ALWAYS(runner.success)

	runner.Join();
}


}//~namespace
