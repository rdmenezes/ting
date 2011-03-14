#include <ting/debug.hpp>

#include "testso.hpp"



void DoSmth(ting::Ref<TestClass>& r){
	ASSERT_ALWAYS(r.IsValid())

	ting::Ref<TestClass> v = r;
	v.Reset();

	r->a = 123;

	r.Reset();
}
