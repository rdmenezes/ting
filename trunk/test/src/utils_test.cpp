#include <ting/debug.hpp>
#include <ting/utils.hpp>

using namespace ting;


int main(int argc, char *argv[]){
//	TRACE(<< "utils test" << std::endl)

	{
		u32 a = 13, b = 14;
		Exchange(a, b);

		ASSERT_ALWAYS(a == 14)
		ASSERT_ALWAYS(b == 13)
	}

	{
		float a = 13, b = 14;
		Exchange(a, b);

		ASSERT_ALWAYS(a == 14)
		ASSERT_ALWAYS(b == 13)
	}

	TRACE_ALWAYS(<< "[PASSED]: utils test" << std::endl)

	return 0;
}
