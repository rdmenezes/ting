#include <vector>

#include "../../src/ting/debug.hpp"
#include "../../src/ting/PoolStored.hpp"
#include "../../src/ting/Ptr.hpp"

#include "tests.hpp"



namespace BasicPoolStoredTest{

class TestClass : public ting::PoolStored<TestClass, 13>{
public:
	int a;
	int b;
	TestClass(){
//		TRACE(<<"TestClass(): constructed"<<std::endl)
	}
	~TestClass(){
//		TRACE(<<"~TestClass(): destructed"<<std::endl)
	}
};


void Run(){
	std::vector<ting::Ptr<TestClass> > vec;

	for(unsigned i = 0; i < 10000; ++i){
		vec.push_back(ting::Ptr<TestClass>(new TestClass()));
		vec.back()->a = int(i);
		vec.back()->b = -int(i);
	}

	while(vec.size() != 0){
		vec.pop_back();
	}
}

}//~namespace
