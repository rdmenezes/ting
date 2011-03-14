#include <ting/debug.hpp>

#include "testso.hpp"



void DoSmth(ting::Ref<TestClass> r){
	ASSERT_ALWAYS(r.IsValid())

	r->a = 123;
}
