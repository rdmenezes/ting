#include <ting/debug.hpp>
#include <ting/Thread.hpp>


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



int main(int argc, char *argv[]){
//	TRACE(<< "Thread test" << std::endl)

	TestJoinAfterThreadHasFinished();
	
	TestJoinBeforeThreadHasFinished();

	TRACE_ALWAYS(<< "[PASSED]: Thread test" << std::endl)

	return 0;
}
