#pragma once

#include <ting/Singleton.hpp>


class TestSingleton : public ting::Singleton<TestSingleton>{
public:
	int a;

	TestSingleton() :
			a(32)
	{
//		TRACE(<< "TestSingleton(): constructed, this->a = " << this->a << std::endl)
	}
	~TestSingleton(){
//		TRACE(<< "~TestSingleton(): destructed" << std::endl)
	}
};
