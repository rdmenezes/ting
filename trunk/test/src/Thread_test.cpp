#include <ting/debug.hpp>
#include <ting/Thread.hpp>
#include <ting/Buffer.hpp>


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



static void TestJoinAfterThreadHasFinished(){
	TestThread t;

	t.Start();

	ting::Thread::Sleep(2000);

	t.Join();
}



static void TestJoinBeforeThreadHasFinished(){
	TestThread t;

	t.Start();

	t.Join();
}




//Test many threads

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



static void TestManyThreads(){
	//TODO: make sure it runs with 1000 threads
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


int main(int argc, char *argv[]){
//	TRACE(<< "Thread test" << std::endl)

	TestJoinAfterThreadHasFinished();
	
	TestJoinBeforeThreadHasFinished();

	TestManyThreads();

	TRACE_ALWAYS(<< "[PASSED]: Thread test" << std::endl)

	return 0;
}
