#include <ting/debug.hpp>
#include <ting/Thread.hpp>
#include <ting/Buffer.hpp>
#include <ting/types.hpp>




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
		std::cout << "A" << std::endl;
		TestThread t;
		std::cout << "B" << std::endl;

		t.Start();
		std::cout << "C" << std::endl;

		ting::Thread::Sleep(2000);
		std::cout << "D" << std::endl;

		t.Join();
		std::cout << "E" << std::endl;
	}



	//Test join before thread has finished
	{
		std::cout << "A2" << std::endl;
		TestThread t;

		t.Start();
		std::cout << "B2" << std::endl;

		t.Join();
		std::cout << "C2" << std::endl;
	}
}

}//~namespace


//====================
//Test many threads
//====================
namespace TestManyThreads{

class TestThread1 : public ting::MsgThread {
public:
	int a, b;

	TestThread1()
	{
		static unsigned int counter = 0; 
		std::cout << "Test thread 1->" << counter++ << std::endl;
	}

	//override
	void Run(){
		std::cout << "Test thread 1 -> run" << std::endl;
		while(!this->quitFlag){
			this->queue.GetMsg()->Handle();
		}
	}
};



static void Run(){
	std::cout << "A3" << std::endl;
	ting::StaticBuffer<TestThread1, 10> thr;
	std::cout << "A3 bis" << std::endl;

	unsigned int c = 0;
	for(TestThread1 *i = thr.Begin(); i != thr.End(); ++i){
		std::cout << "blah blah " << c++ << std::endl;
		i->Start();
	}
	std::cout << "B3" << std::endl;

	ting::Thread::Sleep(1000);
	std::cout << "C3" << std::endl;

	for(TestThread1 *i = thr.Begin(); i != thr.End(); ++i){
		i->PushQuitMessage();
		i->Join();
	}
	std::cout << "D3" << std::endl;
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

	TRACE(<< "caca2" << std::endl)
	TestManyThreads::Run();

	TRACE(<< "caca1" << std::endl)
	TestJoinBeforeAndAfterThreadHasFinished::Run();

	TRACE(<< "caca3" << std::endl)
	TestImmediateExitThread::Run();

	TRACE(<< "caca4" << std::endl)
	TestNestedJoin::Run();

	TRACE(<< "caca5" << std::endl)
	TRACE_ALWAYS(<< "[PASSED]: Thread test" << std::endl)

	return 0;
}
