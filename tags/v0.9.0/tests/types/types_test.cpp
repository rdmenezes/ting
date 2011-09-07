#include "../../src/ting/types.hpp"



int main(int argc, char *argv[]){

	{
		ting::Inited<int, -10> a;
		ASSERT_ALWAYS(a == -10)

		a = 30;
		ASSERT_ALWAYS(a == 30)

		++a;
		ASSERT_ALWAYS(a == 31)

		--a;
		ASSERT_ALWAYS(a == 30)

		ASSERT_ALWAYS(a++ == 30)
		ASSERT_ALWAYS(a == 31)

		ASSERT_ALWAYS(a-- == 31)
		ASSERT_ALWAYS(a == 30)

		ting::Inited<int, -10> b;
		a = b;
		ASSERT_ALWAYS(a == -10)
		ASSERT_ALWAYS(b == -10)
		ASSERT_ALWAYS(a == b)
	}

	{
		ting::Inited<float, -10> a;
		ASSERT_ALWAYS(a == -10)

		a = 30;
		ASSERT_ALWAYS(a == 30)

		++a;
		ASSERT_ALWAYS(a == 31)

		--a;
		ASSERT_ALWAYS(a == 30)

		ASSERT_ALWAYS(a++ == 30)
		ASSERT_ALWAYS(a == 31)

		ASSERT_ALWAYS(a-- == 31)
		ASSERT_ALWAYS(a == 30)

		ting::Inited<float, -10> b;
		a = b;
		ASSERT_ALWAYS(a == -10)
		ASSERT_ALWAYS(b == -10)
		ASSERT_ALWAYS(a == b)
	}

	TRACE_ALWAYS(<< "[PASSED]: types test" << std::endl)

	return 0;
}
