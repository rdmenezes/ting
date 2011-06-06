#include "../../src/ting/debug.hpp"
#include "../../src/ting/math.hpp"



int main(int argc, char *argv[]){

	ASSERT_ALWAYS(ting::Sin((long double)(0)) == 0)
	ASSERT_ALWAYS(ting::Abs(ting::Sin(ting::DPi<long double>() / 2) - 1) < 0.00001)
	ASSERT_ALWAYS(ting::Abs(ting::Sin(ting::DPi<long double>())) < 0.00001)
	ASSERT_ALWAYS(ting::Abs(ting::Sin(ting::DPi<long double>() * 3 / 2) + 1) < 0.00001)

	ASSERT_ALWAYS(ting::Cos((long double)(0)) == 1)
	ASSERT_ALWAYS(ting::Abs(ting::Cos(ting::DPi<long double>() / 2)) < 0.00001)
	ASSERT_ALWAYS(ting::Abs(ting::Cos(ting::DPi<long double>()) + 1) < 0.00001)
	ASSERT_ALWAYS(ting::Abs(ting::Cos(ting::DPi<long double>() * 3 / 2)) < 0.00001)

	ASSERT_ALWAYS(ting::Exp((long double)(0)) == 1)
	ASSERT_ALWAYS(ting::Abs(ting::Exp(ting::DLnOf2<long double>()) - 2) < 0.00001)

	ASSERT_ALWAYS(ting::Ln((long double)(1)) == 0)
	ASSERT_ALWAYS(ting::Abs(ting::Ln((long double)(2)) - ting::DLnOf2<long double>()) < 0.00001)

	TRACE_ALWAYS(<<"[PASSED]: math test"<<std::endl)

	return 0;
}
