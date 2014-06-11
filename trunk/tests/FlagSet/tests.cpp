#include "../../src/ting/debug.hpp"
#include "../../src/ting/FlagSet.hpp"

#include "tests.hpp"



using namespace ting;



namespace TestFlagSet{

struct TestEnum{
	enum Type{
		ZEROTH,
		FIRST,
		SECOND,
		THIRD,
		FOURTH,
		FIFTH,
		SIXTH,
		SENENTH,
		EIGHTH,
		NINETH,
		TENTH,
		ELEVENTH,
		TWELVETH,
		THIRTEENTH,
		FOURTEENTH,
		FIFTEENTH,
		SIXTEENTH,
		SEVENTEENTH,
		NINETEENTH,
		TWENTYTH,
		TWENTY_FIRST,
		TWENTY_SECOND,
		TWENTY_THIRD,
		TWENTY_FOURTH,
		TWENTY_FIFTH,
		TWENTY_SIXTH,
		TWENTY_SENENTH,
		TWENTY_EIGHTH,
		TWENTY_NINETH,
		THIRTYTH,
		THIRTY_FIRST,
		THIRTY_SECOND,
		THIRTY_THIRD,
		THIRTY_FOURTH,
		THIRTY_FIFTH,
		THIRTY_SIXTH,
		THIRTY_SENENTH,
		THIRTY_EIGHTH,
		THIRTY_NINETH,

		ENUM_SIZE
	};
};

void Run(){
	ting::FlagSet<TestEnum> fs;
	
	fs.Set(TestEnum::EIGHTH, true).Set(TestEnum::SECOND, true).Set(TestEnum::EIGHTH, false);
	ASSERT_ALWAYS(!fs.Get(TestEnum::EIGHTH))
	ASSERT_ALWAYS(fs.Get(TestEnum::SECOND))
	
	
	TRACE_ALWAYS(<< "ENUM_SIZE = " << TestEnum::ENUM_SIZE << " sizeof(fs) = " << sizeof(fs) << " sizeof(index_t) = " << sizeof(ting::FlagSet<TestEnum>::index_t) << std::endl)
			
	TRACE_ALWAYS(<< "fs = " << fs << std::endl)
	
	{
		ting::FlagSet<TestEnum> fs;
		ASSERT_ALWAYS(fs.IsAllClear())
		ASSERT_ALWAYS(!fs.IsAllSet())
		
		fs.Set(fs.Size() - 1, true);
		ASSERT_ALWAYS(!fs.IsAllClear())
		ASSERT_ALWAYS(!fs.IsAllSet())
	}
	{
		ting::FlagSet<TestEnum> fs(true);
		ASSERT_ALWAYS(!fs.IsAllClear())
		ASSERT_ALWAYS(fs.IsAllSet())
		
		fs.Set(fs.Size() - 1, false);
		ASSERT_ALWAYS(!fs.IsAllClear())
		ASSERT_ALWAYS(!fs.IsAllSet())
	}
}

}//~namespace
